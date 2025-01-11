#include <iostream>
#include <unordered_set>
#include <chrono>
#include <getopt.h>
#include <stdexcept>
#include "Vamana.h"
#include "Point.h"
#include "DataLoader.h"
#include "ArgumentParser.h"

// #define EXTRA_LOGGING

using namespace std;

template <typename Func>
auto MeasureExecutionTime(const string &taskName, Func &&func, map<string, long> &timingData) -> decltype(func())
{
    cout << taskName << "..." << endl;
    auto start = chrono::steady_clock::now();
    auto result = func();
    auto end = chrono::steady_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    timingData[taskName] = duration;
    cout << taskName << " completed in " << duration << " ms." << endl;
    return result;
}

enum class VamanaMode
{
    Filtered,
    Stitched,
    Search
};

template <typename T>
void RunVamanaProcess(Vamana<T> &vamana, const vector<Point<T>> &data, const vector<vector<float>> &queries,
                      const ParsedArguments &args, map<string, long> &timingData)
{
    VamanaMode mode;
    string modeName;
    string filePrefix;

    if (args.Operation == "create-f")
    {
        mode = VamanaMode::Filtered;
        modeName = "Filtered";
        filePrefix = args.Experiment + "_create_f";
    }
    else if (args.Operation == "create-s")
    {
        mode = VamanaMode::Stitched;
        modeName = "Stitched";
        filePrefix = args.Experiment + "_create_s";
    }
    else if (args.Operation == "search")
    {
        mode = VamanaMode::Search;
        modeName = "Search";
        filePrefix = args.Experiment + "_search";
    }
    else
    {
        throw std::invalid_argument("Invalid operation type");
    }

    if (mode != VamanaMode::Search)
    {
        // Indexing
        MeasureExecutionTime("Building " + modeName + " Vamana index", [&]()
                             {
                                 if (mode == VamanaMode::Filtered)
                                     vamana.FilteredVamanaIndexing(data, args.IndexPath);
                                 else if (mode == VamanaMode::Stitched)
                                     vamana.StitchedVamanaIndexing(data, args.L >> 2, args.R >> 2, args.R, args.IndexPath);
                                 return 0; }, timingData);

        // Save graph creation timing
        string timingFileName = filePrefix + "_graph_timing.log";
        ofstream timingFile(timingFileName);
        if (!timingFile.is_open())
            throw runtime_error("Unable to open file: " + timingFileName + " for writing graph timing.");

        timingFile << "Graph Creation Timing:\n";
        timingFile << "Task: Building " << modeName << " Vamana index\n";
        timingFile << "Time (ms): " << timingData["Building " + modeName + " Vamana index"] << endl;
        timingFile.close();

        cout << modeName << " Vamana index built successfully. Timing details saved to '" << timingFileName << "'." << endl;
        return;
    }

    vamana.LoadGraph(data, args.IndexPath);

    // Searching
    vector<vector<Point<T>>> allNeighbors(queries.size()); // Preallocate results with the same size as queries
    MeasureExecutionTime("Performing Vamana search", [&]()
                         {
#pragma omp parallel for schedule(dynamic)
                             for (size_t i = 0; i < queries.size(); ++i)
                             {
                                 const auto &queryVector = queries[i];
                                 vector<float> dimensionPart(queryVector.begin() + 4, queryVector.end());
                                 Point<T> query(dimensionPart);
                                 int queryType = static_cast<int>(queryVector[0]);
                                 vector<Point<T>> neighbors;

                                 if (queryType == 0) // Vector-only query (no filter)
                                 {
                                     neighbors = vamana.FilteredSearch(data, query, {});
                                 }
                                     
                                 else if (queryType == 1) // Vector query (with filter)
                                 {
                                     float categoricalFilter = queryVector[1];
                                     unordered_set<float> queryFilters = {categoricalFilter};
                                     neighbors = vamana.FilteredSearch(data, query, queryFilters);
                                 }
                                 else
                                     neighbors = {}; // Unsupported types 3 and 4

                                 allNeighbors[i] = move(neighbors);
                             }
                             return 0; }, timingData);

#ifdef EXTRA_LOGGING
    // Save search results
    string resultsFileName = filePrefix + "_results.log";
    ofstream resultsFile(resultsFileName);
    if (!resultsFile.is_open())
        throw runtime_error("Unable to open file: " + resultsFileName + " for writing search results.");

    resultsFile << "Vamana Search Results\n\n";
    for (size_t i = 0; i < allNeighbors.size(); ++i)
    {
        resultsFile << "Query " << i << ": ";
        for (const auto &neighbor : allNeighbors[i])
        {
            resultsFile << neighbor.GetIndex() << " ";
        }
        resultsFile << "\n";
    }
    resultsFile.close();
    cout << "Search results saved to '" << resultsFileName << "'." << endl;
#endif

    // Handle optional ground truth
    if (!args.GroundTruthPath.empty())
    {
        // Load ground truth
        auto groundTruth = MeasureExecutionTime("Loading ground truth data", [&]()
                                                { return DataLoader().ReadGroundTruth(args.GroundTruthPath, 100); }, timingData);

        if (groundTruth.size() != queries.size())
            throw runtime_error("Ground truth size mismatch with query size.");

        int totalMatchesType0 = 0, totalMatchesType1 = 0, totalMatches = 0;
        int totalValidQueriesType0 = 0, totalValidQueriesType1 = 0, totalValidQueries = 0;
        int totalRelevantItemsType0 = 0, totalRelevantItemsType1 = 0, totalRelevantItems = 0;

        for (size_t i = 0; i < queries.size(); ++i)
        {
            if (allNeighbors[i].empty())
                continue;

            unordered_set<int> foundNeighborIndices;
            for (const auto &neighbor : allNeighbors[i])
                foundNeighborIndices.insert(neighbor.GetIndex());

            // Determine the valid size of the ground truth vector (up to -1)
            auto validEndIt = find(groundTruth[i].begin(), groundTruth[i].end(), -1);
            int validSize = distance(groundTruth[i].begin(), validEndIt);
            // Count the number of relevant ground truth items for this query (min of K and valid size)
            int relevantItems = min(validSize, args.K);

            // Calculate matches for this query
            int matches = 0;
            for (int j = 0; j < validSize; ++j)
            {
                if (foundNeighborIndices.count(groundTruth[i][j]))
                    ++matches;
            }

            // Update counts based on query type
            int queryType = static_cast<int>(queries[i][0]);
            if (queryType == 0)
            {
                ++totalValidQueriesType0;
                totalMatchesType0 += matches;
                totalRelevantItemsType0 += relevantItems;
            }
            else if (queryType == 1)
            {
                ++totalValidQueriesType1;
                totalMatchesType1 += matches;
                totalRelevantItemsType1 += relevantItems;
            }

            // Update totals
            ++totalValidQueries;
            totalMatches += matches;
            totalRelevantItems += relevantItems;
        }

        // Calculate Recall@K for type 0, type 1, and total
        if (totalValidQueries > 0)
        {
            double recallAtKType0 = totalRelevantItemsType0 > 0 ? (static_cast<double>(totalMatchesType0) / totalRelevantItemsType0) * 100.0 : 0.0;
            double recallAtKType1 = totalRelevantItemsType1 > 0 ? (static_cast<double>(totalMatchesType1) / totalRelevantItemsType1) * 100.0 : 0.0;
            double recallAtKTotal = totalRelevantItems > 0 ? (static_cast<double>(totalMatches) / totalRelevantItems) * 100.0 : 0.0;

            // Save recall results with timing
            string recallFileName = filePrefix + "_recall.log";
            ofstream recallFile(recallFileName);
            if (!recallFile.is_open())
                throw runtime_error("Unable to open file: " + recallFileName + " for writing recall results.");

            recallFile << "\n=== Recall and Timing Results ===" << endl;
            recallFile << " - Recall@K (Type 0): " << recallAtKType0 << "%" << endl;
            recallFile << " - Recall@K (Type 1): " << recallAtKType1 << "%" << endl;
            recallFile << " - Recall@K (Total): " << recallAtKTotal << "%" << endl
                       << endl;
            recallFile << " - Total valid queries (Type 0): " << totalValidQueriesType0 << endl;
            recallFile << " - Total valid queries (Type 1): " << totalValidQueriesType1 << endl;
            recallFile << " - Total valid queries (Total): " << totalValidQueries << endl
                       << endl;
            recallFile << " - Total matches (Type 0): " << totalMatchesType0 << endl;
            recallFile << " - Total matches (Type 1): " << totalMatchesType1 << endl;
            recallFile << " - Total matches (Total): " << totalMatches << endl
                       << endl;
            recallFile << " - Total relevant items (Type 0): " << totalRelevantItemsType0 << endl;
            recallFile << " - Total relevant items (Type 1): " << totalRelevantItemsType1 << endl;
            recallFile << " - Total relevant items (Total): " << totalRelevantItems << endl
                       << endl;
            recallFile << "=== Timing Data (ms) ===" << endl;
            for (const auto &[task, time] : timingData)
            {
                recallFile << task << ": " << time << " ms" << endl;
            }
            recallFile.close();
            cout << "Recall and timing results saved to '" << recallFileName << "'." << endl;
        }
        else
        {
            cout << "\nNo valid queries to calculate recall." << endl;
        }

#ifdef EXTRA_LOGGING
        // Save detailed query report
        string reportFileName = filePrefix + "_query_report.log";
        ofstream reportFile(reportFileName);
        if (!reportFile.is_open())
            throw runtime_error("Unable to open file: " + reportFileName + " for writing detailed query comparison report.");

        reportFile << "Detailed Query Comparison Report\n\n";

        for (size_t i = 0; i < queries.size(); ++i)
        {
            if (allNeighbors[i].empty()) // Skip not-done queries
            {
                reportFile << "Query " << i << ": Skipped (No neighbors found)\n";
                continue;
            }

            int queryType = queries[i][0];
            unordered_set<int> foundNeighborIndices;
            for (const auto &neighbor : allNeighbors[i])
                foundNeighborIndices.insert(neighbor.GetIndex());

            int relevantItems = min(static_cast<int>(groundTruth[i].size()), args.K);
            int matches = 0;
            vector<bool> isMatch(groundTruth[i].size(), false);

            for (size_t j = 0; j < groundTruth[i].size(); ++j)
            {
                if (foundNeighborIndices.count(groundTruth[i][j]))
                {
                    ++matches;
                    isMatch[j] = true;
                }
            }

            double matchPercentage = relevantItems > 0 ? (static_cast<double>(matches) / relevantItems) * 100.0 : 0.0;

            // Write details of this query to the report
            reportFile << "Query " << i << " (Type " << queryType << "):\n";
            reportFile << "  Relevant Items (Ground Truth): ";
            for (size_t j = 0; j < groundTruth[i].size(); ++j)
            {
                reportFile << groundTruth[i][j];
                if (isMatch[j])
                    reportFile << "(Match)";
                reportFile << " ";
            }
            reportFile << "\n";
            reportFile << "  Neighbors Found: ";
            for (const auto &neighbor : allNeighbors[i])
            {
                reportFile << neighbor.GetIndex() << " ";
            }
            reportFile << "\n";
            reportFile << "  Matches: " << matches << " / " << relevantItems << "\n";
            reportFile << "  Match Percentage: " << matchPercentage << "%\n";
            reportFile << "\n";
        }

        reportFile.close();
        cout << "Detailed query comparison report saved to '" << reportFileName << "'." << endl;
#endif
    }
    else
    {
        cout << "\nGround truth not provided; skipping recall calculation." << endl;
    }
}

int main(int argc, char *argv[])
{
    try
    {
        ParsedArguments args = ArgumentParser::ParseArguments(argc, argv);
        ArgumentParser::DisplayParsedArguments(args);

        Vamana<float> vamana(args.K, args.L, args.R, args.Alpha);
        DataLoader loader;

        map<string, long> timingData;

        auto data = MeasureExecutionTime("Loading base dataset", [&]()
                                         { return loader.LoadDataset(args.BaseDatasetPath); }, timingData);
        auto queries = MeasureExecutionTime("Loading query dataset", [&]()
                                            { return loader.LoadQuerySet(args.QueryDatasetPath); }, timingData);

        RunVamanaProcess(vamana, data, queries, args, timingData);

        return EXIT_SUCCESS;
    }
    catch (const exception &e)
    {
        cerr << "Error: " << e.what() << endl;
        return EXIT_FAILURE;
    }
}
