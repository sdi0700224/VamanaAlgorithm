#include <iostream>
#include <unordered_set>
#include <chrono>
#include <getopt.h>
#include <stdexcept>
#include "Vamana.h"
#include "Point.h"
#include "DataLoader.h"
#include "ArgumentParser.h"

using namespace std;

template <typename Func>
auto MeasureExecutionTime(const string &taskName, Func &&func) -> decltype(func())
{
    cout << taskName << "..." << endl;
    auto start = chrono::steady_clock::now();
    auto result = func(); // This will deduce the return type of the passed function
    auto end = chrono::steady_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    cout << taskName << " completed in " << duration << " ms." << endl;
    return result;
}

int main(int argc, char *argv[])
{
    try
    {
        ParsedArguments args = ArgumentParser::ParseArguments(argc, argv);
        ArgumentParser::DisplayParsedArguments(args);

        Vamana<float> vamana(args.K, args.L, args.R, args.Alpha);
        DataLoader loader;

        // Load base dataset
        auto data = MeasureExecutionTime("Loading base dataset", [&]()
                                         { return loader.LoadDataset(args.BaseDatasetPath); });
        if (data.empty())
        {
            throw runtime_error("Failed to load base dataset or dataset is empty.");
        }

        // Build Vamana index
        MeasureExecutionTime("Building Vamana index", [&]()
                             {
                                 vamana.StitchedVamanaIndexing(data, args.L / 5, args.R / 5, 3 * args.R);
                                 return 0; });

        // Load query dataset
        auto queries = MeasureExecutionTime("Loading query dataset", [&]()
                                            { return loader.LoadQuerySet(args.QueryDatasetPath); });
        if (queries.empty())
        {
            throw runtime_error("Failed to load query dataset or dataset is empty.");
        }

        // Perform search
        vector<vector<Point<float>>> allNeighbors;
        MeasureExecutionTime("Performing search", [&]()
                             {
                                for (const auto &queryVector : queries)
                                {
                                    vector<float> dimensionPart(queryVector.begin() + 4, queryVector.end());
                                    Point<float> query(dimensionPart);

                                    int queryType = static_cast<int>(queryVector[0]);

                                    if (queryType == 0) // Vector-only query (no filter)
                                    {
                                        auto neighbors = vamana.StitchedSearch(data, query, {});
                                        if (neighbors.size() == 0)
                                        {
                                            //cerr << "Unfiltered search result is empty" << endl;
                                        }
                                        allNeighbors.push_back(neighbors);
                                    }
                                    else if (queryType == 1) // Vector query (with filter)
                                    {
                                        float categoricalFilter = queryVector[1];
                                        unordered_set<float> queryFilters = {categoricalFilter};

                                        auto neighbors = vamana.StitchedSearch(data, query, queryFilters);
                                        if (neighbors.size() == 0)
                                        {
                                            //cerr << "Filtered search result is empty" << endl;
                                        }
                                        allNeighbors.push_back(neighbors);
                                    }
                                    else // Skip unsupported query types 3 and 4
                                    {
                                        allNeighbors.emplace_back(); // Add an empty vector to maintain alignment
                                        continue;
                                    }
                                }
                                return 0; });

        // Handle optional ground truth
        if (!args.GroundTruthPath.empty())
        {
            // Load ground truth
            auto groundTruth = MeasureExecutionTime("Loading ground truth data", [&]()
                                                    { return loader.ReadGroundTruth(args.GroundTruthPath, 100); });
            if (groundTruth.size() != queries.size())
            {
                cerr << "Error: Ground truth size (" << groundTruth.size()
                     << ") does not match query size (" << queries.size() << ")." << endl;
                throw runtime_error("Ground truth size mismatch with query size.");
            }

            // Separate recall calculations for type 0 and type 1
            int totalMatchesType0 = 0, totalMatchesType1 = 0, totalMatches = 0;
            int totalValidQueriesType0 = 0, totalValidQueriesType1 = 0, totalValidQueries = 0;
            int totalRelevantItemsType0 = 0, totalRelevantItemsType1 = 0, totalRelevantItems = 0;

            for (size_t i = 0; i < queries.size(); ++i)
            {
                if (allNeighbors[i].empty()) // Skip not-done queries
                {
                    continue;
                }

                // Determine the type of query (assume `queries[i].GetType()` returns the type)
                int queryType = queries[i][0];

                // Convert neighbor results to a set of indices
                unordered_set<int> foundNeighborIndices;
                for (const auto &neighbor : allNeighbors[i])
                {
                    foundNeighborIndices.insert(neighbor.GetIndex());
                }

                // Determine the valid size of the ground truth vector (up to -1)
                auto validEndIt = std::find(groundTruth[i].begin(), groundTruth[i].end(), -1);
                int validSize = std::distance(groundTruth[i].begin(), validEndIt);

                // Count the number of relevant ground truth items for this query (min of K and valid size)
                int relevantItems = std::min(validSize, args.K);

                // Calculate matches for this query
                int matches = 0;
                for (int j = 0; j < validSize; ++j)
                {
                    if (foundNeighborIndices.count(groundTruth[i][j]))
                    {
                        ++matches;
                    }
                }

                // Update counts based on query type
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
                double recallAtKType0 = totalRelevantItemsType0 > 0 ? static_cast<double>(totalMatchesType0) / totalRelevantItemsType0 * 100.0 : 0.0;
                double recallAtKType1 = totalRelevantItemsType1 > 0 ? static_cast<double>(totalMatchesType1) / totalRelevantItemsType1 * 100.0 : 0.0;
                double recallAtKTotal = static_cast<double>(totalMatches) / totalRelevantItems * 100.0;

                // Output results
                cout << "\n=== Results ===" << endl
                     << endl;
                cout << " - Recall@K (Type 0): " << recallAtKType0 << "%" << endl;
                cout << " - Recall@K (Type 1): " << recallAtKType1 << "%" << endl;
                cout << " - Recall@K (Total): " << recallAtKTotal << "%" << endl
                     << endl;
                cout << " - Total valid queries (Type 0): " << totalValidQueriesType0 << endl;
                cout << " - Total valid queries (Type 1): " << totalValidQueriesType1 << endl;
                cout << " - Total valid queries (Total): " << totalValidQueries << endl
                     << endl;
                cout << " - Total matches (Type 0): " << totalMatchesType0 << endl;
                cout << " - Total matches (Type 1): " << totalMatchesType1 << endl;
                cout << " - Total matches (Total): " << totalMatches << endl
                     << endl;
                cout << " - Total relevant items (Type 0): " << totalRelevantItemsType0 << endl;
                cout << " - Total relevant items (Type 1): " << totalRelevantItemsType1 << endl;
                cout << " - Total relevant items (Total): " << totalRelevantItems << endl;
            }
            else
            {
                cout << "\nNo valid queries to calculate recall." << endl;
            }

            // Open a file for writing the detailed report
            ofstream reportFile("query_report.txt");
            if (!reportFile.is_open())
            {
                cerr << "Error: Unable to open report file for writing." << endl;
                return EXIT_FAILURE;
            }

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
                {
                    foundNeighborIndices.insert(neighbor.GetIndex());
                }

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

                double matchPercentage = relevantItems > 0 ? static_cast<double>(matches) / relevantItems * 100.0 : 0.0;

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

            // Close the report file
            reportFile.close();
            cout << "Detailed query comparison report saved to 'query_report.txt'." << endl;
        }
        else
        {
            cout << "\nGround truth not provided; skipping recall calculation." << endl;
        }

        return EXIT_SUCCESS;
    }
    catch (const exception &e)
    {
        cerr << "Error: " << e.what() << endl;
        return EXIT_FAILURE;
    }
}
