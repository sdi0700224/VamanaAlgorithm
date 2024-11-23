#include <iostream>
#include <unordered_set>
#include <chrono>
#include <getopt.h>
#include <stdexcept>
#include "Vamana.h"
#include "Point.h"
#include "DataLoader.h"

using namespace std;

template <typename Func>
auto MeasureExecutionTime(const string &taskName, Func &&func)
{
    cout << taskName << "..." << endl;
    auto start = chrono::steady_clock::now();
    auto result = func();
    auto end = chrono::steady_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    cout << taskName << " completed in " << duration << " ms." << endl;
    return result;
}

int main(int argc, char *argv[])
{
    // Default values for parameters
    int k = 0, l = 0, r = 0;
    double a = 0.0;
    string baseDatasetPath, queryDatasetPath, groundTruthPath;

    // Define long options for flag parsing
    static struct option longOptions[] = {
        {"k", required_argument, 0, 'k'},
        {"l", required_argument, 0, 'l'},
        {"r", required_argument, 0, 'r'},
        {"alpha", required_argument, 0, 'a'},
        {"base", required_argument, 0, 'b'},
        {"query", required_argument, 0, 'q'},
        {"groundtruth", required_argument, 0, 'g'},
        {0, 0, 0, 0} // Termination of options
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "k:l:r:a:b:q:g:", longOptions, NULL)) != -1)
    {
        switch (opt)
        {
        case 'k':
            k = stoi(optarg);
            break;
        case 'l':
            l = stoi(optarg);
            break;
        case 'r':
            r = stoi(optarg);
            break;
        case 'a':
            a = stod(optarg);
            break;
        case 'b':
            baseDatasetPath = optarg;
            break;
        case 'q':
            queryDatasetPath = optarg;
            break;
        case 'g':
            groundTruthPath = optarg;
            break;
        default:
            cerr << "Invalid flag or missing argument.\n";
            return EXIT_FAILURE;
        }
    }

    // Handle positional arguments if flags weren't provided
    int remainingArgs = argc - optind;
    if (remainingArgs >= 7)
    { // At least 7 positional arguments are expected
        if (k == 0)
            k = stoi(argv[optind]);
        if (l == 0)
            l = stoi(argv[optind + 1]);
        if (r == 0)
            r = stoi(argv[optind + 2]);
        if (a == 0.0)
            a = stod(argv[optind + 3]);
        if (baseDatasetPath.empty())
            baseDatasetPath = argv[optind + 4];
        if (queryDatasetPath.empty())
            queryDatasetPath = argv[optind + 5];
        if (groundTruthPath.empty())
            groundTruthPath = argv[optind + 6];
    }

    // Validate all parameters
    if (k <= 0 || l <= 0 || r <= 0 || a <= 0.0 || baseDatasetPath.empty() || queryDatasetPath.empty() || groundTruthPath.empty())
    {
        cerr << "Usage: " << argv[0] << " -k <value> -l <value> -r <value> -a <value> -b <base_dataset> -q <query_dataset> -g <groundtruth_dataset>" << endl;
        cerr << "Or: " << argv[0] << " <k> <l> <r> <alpha> <base_dataset> <query_dataset> <groundtruth_dataset>" << endl;
        return EXIT_FAILURE;
    }

    // Print parsed arguments
    cout << "Parsed Arguments:" << endl;
    cout << " - k: " << k << endl;
    cout << " - L: " << l << endl;
    cout << " - R: " << r << endl;
    cout << " - Alpha: " << a << endl;
    cout << " - Base dataset path: " << baseDatasetPath << endl;
    cout << " - Query dataset path: " << queryDatasetPath << endl;
    cout << " - Ground truth path: " << groundTruthPath << endl;

    try
    {
        // Initialize objects
        DataLoader loader;
        Vamana<float> vamana(k, l, r, a);

        // Load datasets
        auto data = MeasureExecutionTime("Loading base dataset", [&]()
                                         { return loader.LoadFvecs(baseDatasetPath); });
        if (data.empty())
            throw runtime_error("Failed to load base dataset or dataset is empty.");

        MeasureExecutionTime("Building Vamana index", [&]()
                             {
                                 vamana.BuildIndex(data);
                                 return 0; // Return dummy value to satisfy the lambda requirement
                             });

        auto queries = MeasureExecutionTime("Loading query dataset", [&]()
                                            { return loader.LoadFvecs(queryDatasetPath); });
        if (queries.empty())
            throw runtime_error("Failed to load query dataset or dataset is empty.");

        auto groundTruth = MeasureExecutionTime("Loading ground truth data", [&]()
                                                { return loader.LoadIvecs(groundTruthPath); });
        if (groundTruth.empty() || groundTruth.size() != queries.size())
        {
            throw runtime_error("Failed to load ground truth data or size mismatch with queries.");
        }

        // Search and evaluate Recall@K
        int totalMatches = 0;
        int totalQueries = queries.size();

        MeasureExecutionTime("Performing search and calculating Recall@K", [&]()
                             {
                                 for (int i = 0; i < totalQueries; ++i)
                                 {
                                     // Find k-nearest neighbors
                                     auto neighbors = vamana.Search(queries[i], k);

                                     // Convert neighbor results to a set of indices
                                     unordered_set<int> foundNeighborIndices;
                                     for (const auto &neighbor : neighbors)
                                     {
                                         foundNeighborIndices.insert(neighbor.GetIndex());
                                     }

                                     // Calculate the number of true positives
                                     int matchCount = 0;
                                     for (int gtIndex : groundTruth[i])
                                     {
                                         if (foundNeighborIndices.count(gtIndex))
                                         {
                                             ++matchCount;
                                         }
                                     }
                                     totalMatches += matchCount;
                                 }
                                 return 0; // Dummy return
                             });

        // Calculate Recall@K
        double recallAtK = static_cast<double>(totalMatches) / (totalQueries * k) * 100.0;

        // Output results
        cout << "\n=== Results ===" << endl;
        cout << " - Recall@K: " << recallAtK << "%" << endl;
        cout << " - Total queries: " << totalQueries << endl;
        cout << " - Total matches: " << totalMatches << endl;

        return EXIT_SUCCESS;
    }
    catch (const exception &e)
    {
        cerr << "Error: " << e.what() << endl;
        return EXIT_FAILURE;
    }
}
