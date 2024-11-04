#include <iostream>
#include <unordered_set>
#include <chrono>
#include "Vamana.h"
#include "Point.h"
#include "DataLoader.h"

using namespace std;

int main(int argc, char *argv[])
{
    if (argc != 8)
    {
        cerr << "Usage: " << argv[0] << " <k> <l> <r> <alpha> <base_dataset_path> <query_dataset_path> <ground_truth_path>" << endl;
        return EXIT_FAILURE;
    }

    int k = stoi(argv[1]);
    int l = stoi(argv[2]);
    int r = stoi(argv[3]);
    double a = stod(argv[4]);
    string baseDatasetPath = argv[5];
    string queryDatasetPath = argv[6];
    string groundTruthPath = argv[7];

    cout << "Parsed Arguments:" << endl;
    cout << " - k: " << k << endl;
    cout << " - L: " << l << endl;
    cout << " - R: " << r << endl;
    cout << " - a: " << a << endl;
    cout << " - Base dataset path: " << baseDatasetPath << endl;
    cout << " - Query dataset path: " << queryDatasetPath << endl;
    cout << " - Ground truth path: " << groundTruthPath << endl;

    DataLoader loader;
    Vamana<float> vamana(k, l, r, a);

    // Load base dataset
    auto start = chrono::steady_clock::now();
    cout << "Loading base dataset..." << endl;
    auto data = loader.LoadFvecs(baseDatasetPath);
    if (data.empty())
    {
        cerr << "Failed to load base dataset." << endl;
        return EXIT_FAILURE;
    }
    auto end = chrono::steady_clock::now();
    auto loadTime = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    cout << "Loaded " << data.size() << " points from the base dataset in " << loadTime << " ms." << endl;

    // Build the Vanama index
    start = chrono::steady_clock::now();
    vamana.BuildIndex(data);
    end = chrono::steady_clock::now();
    auto buildTime = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    cout << "Index built successfully in " << buildTime << " ms." << endl;

    // Load query dataset
    start = chrono::steady_clock::now();
    cout << "Loading query dataset..." << endl;
    auto queries = loader.LoadFvecs(queryDatasetPath);
    if (queries.empty())
    {
        cerr << "Failed to load query dataset." << endl;
        return EXIT_FAILURE;
    }
    end = chrono::steady_clock::now();
    auto queryLoadTime = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    cout << "Loaded " << queries.size() << " query points in " << queryLoadTime << " ms." << endl;

    // Load ground truth data
    start = chrono::steady_clock::now();
    cout << "Loading ground truth data..." << endl;
    auto groundTruth = loader.LoadIvecs(groundTruthPath);
    if (groundTruth.empty() || groundTruth.size() != queries.size())
    {
        cerr << "Failed to load ground truth or size mismatch with queries." << endl;
        return EXIT_FAILURE;
    }
    end = chrono::steady_clock::now();
    auto groundTruthLoadTime = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    cout << "Loaded ground truth for " << groundTruth.size() << " queries in " << groundTruthLoadTime << " ms." << endl;

    // Variables for calculating Recall@K(the percentage of correct neighbors found by the search algorithm)
    int totalMatches = 0;
    int totalQueries = queries.size();

    // Perform search and calculate Recall@K
    start = chrono::steady_clock::now();
    for (int i = 0; i < totalQueries; ++i)
    {
        // Find k-nearest neighbors
        auto neighbors = vamana.Search(queries[i], k);

        // Use point indexes to compare results with ground truth neighbors
        unordered_set<int> foundNeighborIndices;
        for (const auto &neighbor : neighbors)
        {
            foundNeighborIndices.insert(neighbor.GetIndex());
        }

        // Calculate the number of true positives (matches with ground truth)
        int matchCount = 0;
        for (int gtIndex : groundTruth[i])
        {
            if (foundNeighborIndices.count(gtIndex) > 0)
            {
                ++matchCount;
            }
        }
        totalMatches += matchCount;
    }
    end = chrono::steady_clock::now();
    auto searchTime = chrono::duration_cast<chrono::milliseconds>(end - start).count();

    // Calculate and print Recall@K
    double recallAtK = static_cast<double>(totalMatches) / (totalQueries * k);
    cout << "Recall@K: " << recallAtK * 100 << "%" << endl;
    cout << "Total search and recall calculation time: " << searchTime << " ms." << endl;

    // Summary of timings
    cout << "\nTiming Summary:" << endl;
    cout << " - Base dataset load time: " << loadTime << " ms" << endl;
    cout << " - Index build time: " << buildTime << " ms" << endl;
    cout << " - Query dataset load time: " << queryLoadTime << " ms" << endl;
    cout << " - Ground truth load time: " << groundTruthLoadTime << " ms" << endl;
    cout << " - Search and recall calculation time: " << searchTime << " ms" << endl;

    return EXIT_SUCCESS;
}
