#include "Vamana.h"
#include <limits>
#include <numeric>
#include <algorithm>
#include <random>
#include <iomanip>
#include <cmath>
#include <fstream>

template <typename T>
Vamana<T>::Vamana(int k, int L, int R, double a)
    : K(k), L(L), R(R), A(a), VamanaGraph(), FilteredGraph(), StitchedGraph(), Searcher(), Pruner() {}

template <typename T>
void Vamana<T>::UpdateProgressBar(size_t current, size_t total, const string &message) const
{
    size_t safeTotal = (total > 0) ? total : 1; // Prevent division by zero
    int progress = static_cast<int>((100.0 * current) / safeTotal);
    cout << "\r" << message << ": " << setw(3) << progress << "% ["
         << string(progress / 2, '=') << string(50 - progress / 2, ' ')
         << "]" << flush;
    if (current == safeTotal)
    {
        cout << " Done!" << endl;
    }
}

template <typename T>
void Vamana<T>::FilteredVamanaIndexing(const vector<Point<T>> &data)
{
    // Find medoid of each filter using the filterMap for efficiency
    if (FilterMedoids.empty())
    {
        FilterMedoids = FindFilterMedoids(CreateFilterMap(data), 0);
    }

    // Generate a random permutation of the dataset
    vector<int> randomPermutation(data.size());
    iota(randomPermutation.begin(), randomPermutation.end(), 0);
    shuffle(randomPermutation.begin(), randomPermutation.end(), mt19937{random_device{}()});

    size_t total = randomPermutation.size();
    size_t step = max(total / 100, static_cast<size_t>(1)); // Ensure step is at least 1

    // Iterate through points in random order
    for (size_t idx = 0; idx < total; ++idx)
    {
        size_t i = randomPermutation[idx];
        const auto &currentPoint = data[i];
        T currentFilter = currentPoint.GetFilter();

        // Get the medoid (start node) for the current point's filter
        const auto &startNode = data[FilterMedoids[currentFilter]];

        // Perform FilteredGreedySearch relative to the current filter
        auto [_, visited] =
            Searcher.FilteredGreedySearch(FilteredGraph, currentPoint,
                                          {currentFilter}, {startNode}, 0, L);

        // Apply RobustPrune to visited nodes
        Pruner.FilteredRobustPrune(FilteredGraph, currentPoint, visited, A, R);

        // Update out-neighbors of the current point
        for (const auto &neighbor : FilteredGraph.GetNeighbors(currentPoint))
        {
            auto outNeighbors = FilteredGraph.GetNeighbors(neighbor);
            if (find(outNeighbors.begin(), outNeighbors.end(), currentPoint) == outNeighbors.end())
            {
                // Ensure degree bound is respected
                if (outNeighbors.size() + 1 > static_cast<size_t>(R))
                {
                    outNeighbors.push_back(currentPoint);
                    Pruner.FilteredRobustPrune(FilteredGraph, neighbor, outNeighbors, A, R);
                }
                else
                {
                    FilteredGraph.AddEdge(neighbor, currentPoint);
                }
            }
        }

        // Check and update progress bar
        if (idx % step == 0 || idx == total - 1)
        {
            UpdateProgressBar(idx + 1, total, "Filtered Vamana Indexing");
        }
    }
}

template <typename T>
unordered_map<T, vector<int>> Vamana<T>::CreateFilterMap(const vector<Point<T>> &data)
{
    unordered_map<T, vector<int>> filterMap;

    for (size_t i = 0; i < data.size(); ++i)
    {
        filterMap[data[i].GetFilter()].push_back(static_cast<int>(i));
    }

    return filterMap;
}

template <typename T>
vector<int> Vamana<T>::SampleRandomPoints(const vector<int> &pointIDs, int numSamples) const
{
    vector<int> samples;
    sample(pointIDs.begin(), pointIDs.end(), back_inserter(samples), numSamples, mt19937{random_device{}()});
    return samples;
}

template <typename T>
unordered_map<T, int> Vamana<T>::FindFilterMedoids(const unordered_map<T, vector<int>> &filterMap,
                                                   int threshold) const
{
    unordered_map<T, int> medoidMap;     // Map filter to medoid point ID
    unordered_map<int, int> loadCounter; // Track load on each point (using point ID)

    for (const auto &[filter, pointIDs] : filterMap)
    {
        if (pointIDs.empty())
        {
            continue;
        }

        int numPointsToSample = (threshold > 0)
                                    ? min(threshold, static_cast<int>(pointIDs.size()))
                                    : static_cast<int>(pointIDs.size());

        // Randomly sample threshold points from pointIDs
        auto sampledPoints = SampleRandomPoints(pointIDs, numPointsToSample);

        // Find the point in the sample with the smallest load
        int minLoad = numeric_limits<int>::max();
        int medoidID = -1;
        for (int pointID : sampledPoints)
        {
            if (loadCounter[pointID] < minLoad)
            {
                minLoad = loadCounter[pointID];
                medoidID = pointID;
            }
        }

        // Assign the medoid and update its load count
        medoidMap[filter] = medoidID;
        loadCounter[medoidID]++;
    }

    return medoidMap;
}

template <typename T>
vector<Point<T>> Vamana<T>::PerformSearch(const Graph<T> &graph, const vector<Point<T>> &data,
                                          const Point<T> &query, const unordered_set<T> &filters) const
{
    vector<Point<T>> startPoints;
    unordered_set<T> localFilters = filters;

    // If filters is empty, populate localFilters with all keys from FilterMedoids
    if (localFilters.empty())
    {
        for (const auto &medoid : FilterMedoids)
        {
            localFilters.insert(medoid.first);

            // Find approximately nearest point with label to query instead of medoid
            auto [nearestStartPoint, _] =
                Searcher.FilteredGreedySearch(graph, query, {medoid.first}, {data[medoid.second]}, 1, 1);

            if (!nearestStartPoint.empty())
            {
                startPoints.push_back(nearestStartPoint[0]);
            }
        }
    }
    else
    {
        // If filter is specified, process only the specified filters
        for (const auto &filter : localFilters)
        {
            if (FilterMedoids.find(filter) != FilterMedoids.end())
            {
                startPoints.push_back(data[FilterMedoids.at(filter)]);
            }
        }
    }

    if (startPoints.empty())
    {
        return {};
    }

    auto [nearestNeighbors, _] =
        Searcher.FilteredGreedySearch(graph, query, localFilters, startPoints, K, L);

    return nearestNeighbors;
}

template <typename T>
vector<Point<T>> Vamana<T>::FilteredSearch(const vector<Point<T>> &data, const Point<T> &query, const unordered_set<T> &filters) const
{
    return PerformSearch(
        FilteredGraph,
        data,
        query,
        filters);
}

template <typename T>
Point<T> Vamana<T>::FindMedoid(const vector<Point<T>> &data, bool printMedoid) const
{
    T minDistSum = numeric_limits<T>::max(); // Initialize to max possible value
    Point<T> medoid = data[0];

    for (const auto &candidate : data)
    {
        T distSum = 0;
        // Compute the distance between candidate and all the other points
        for (const auto &other : data)
        {
            distSum += candidate.DistanceTo(other);
        }

        // Point with smaller sum is the medoid
        if (distSum < minDistSum)
        {
            minDistSum = distSum;
            medoid = candidate;
        }
    }

    if (printMedoid)
    {
        cout << "Medoid point found: " << medoid << endl;
    }

    return medoid;
}

template <typename T>
void Vamana<T>::VamanaIndexing(const vector<Point<T>> &data)
{
    // Find the central point (medoid) to act as a starting point
    Medoid = FindMedoid(data);

    vector<size_t> randomPermutation(data.size());
    // Initialize vector with values from 0 to data size - 1
    iota(randomPermutation.begin(), randomPermutation.end(), 0);
    // Randomize the order of values using a random number generator
    shuffle(randomPermutation.begin(), randomPermutation.end(), mt19937{random_device{}()});

    // Initialize each point in the graph with exactly R unique neighbors
    for (const auto &point : data)
    {
        unordered_set<Point<T>> uniqueNeighbors;
        while (uniqueNeighbors.size() < static_cast<size_t>(R))
        {
            size_t randomIndex = randomPermutation[rand() % data.size()];
            const auto &candidate = data[randomIndex];

            // Ensure the candidate is not the current point
            if (candidate != point)
            {
                uniqueNeighbors.insert(candidate);
            }
        }
        vector<Point<T>> randomNeighbors(uniqueNeighbors.begin(), uniqueNeighbors.end());
        VamanaGraph.SetNeighbors(point, randomNeighbors);
    }

    // Progress indicator variables
    size_t totalPoints = data.size();
    size_t progressStep = max(totalPoints / 100, static_cast<size_t>(1)); // Ensure progressStep is at least 1

    // Iterate through the random permutation of points
    for (size_t i = 0; i < randomPermutation.size(); ++i)
    {
        // Access points in random order
        const auto &point = data[randomPermutation[i]];

        // Find approximate neighbors using Greedy Search
        auto [_, visitedNeighbors] = Searcher.GreedySearch(VamanaGraph, Medoid, point, 1, L);

        // Add directed edges from the current point to visited neighbors
        for (const auto &neighbor : visitedNeighbors)
        {
            VamanaGraph.AddEdge(point, neighbor);
        }

        Pruner.RobustPrune(VamanaGraph, point, visitedNeighbors, A, R);

        // Update visitedNeighbors and prune if needed
        for (const auto &neighbor : visitedNeighbors)
        {
            auto outNeighbors = VamanaGraph.GetNeighbors(neighbor);

            // Examine point only if it is not already in outNeighbors
            if (find(outNeighbors.begin(), outNeighbors.end(), point) == outNeighbors.end())
            {
                // If the number of out neighbors plus the current point exceeds R, prune
                if (outNeighbors.size() + 1 > static_cast<size_t>(R))
                {
                    outNeighbors.push_back(point);
                    Pruner.RobustPrune(VamanaGraph, neighbor, outNeighbors, A, R);
                }
                else
                {
                    VamanaGraph.AddEdge(neighbor, point);
                }
            }
        }

        // Update the progress bar every progressStep (1%)
        if (i % progressStep == 0 || i == randomPermutation.size() - 1)
        {
            UpdateProgressBar(i + 1, totalPoints, "Building index");
        }
    }
}

template <typename T>
vector<Point<T>> Vamana<T>::Search(const Point<T> &query, int k) const
{
    auto [approxNeighbors, _] = Searcher.GreedySearch(VamanaGraph, Medoid, query, k, L);
    return approxNeighbors;
}

template <typename T>
void Vamana<T>::StitchedVamanaIndexing(const vector<Point<T>> &data, int L_small, int R_small, int R_stitched, string logFileName)
{
    // Create a map of filters to point groups
    auto filterMap = CreateFilterMap(data);
    unordered_map<T, vector<Point<T>>> labelGroups;

    // Group points by their filters
    for (const auto &[filter, pointIDs] : filterMap)
    {
        for (const auto &pointID : pointIDs)
        {
            labelGroups[filter].push_back(data[pointID]);
        }
    }

    ofstream logFile(logFileName);

    size_t totalGroups = labelGroups.size();
    size_t step = max(totalGroups / 100, static_cast<size_t>(1)); // Ensure step is at least 1
    size_t processedGroups = 0;

    // Process each group independently
    for (const auto &[label, groupPoints] : labelGroups)
    {
        // Skip groups with fewer than 2 points
        if (groupPoints.size() < 2)
        {
            ++processedGroups;
            if (processedGroups % step == 0 || processedGroups == totalGroups)
            {
                UpdateProgressBar(processedGroups, totalGroups, "Stitched Vamana Indexing");
            }
            continue;
        }

        // Adjust parameters to ensure they are within valid bounds
        int effectiveL = min(L_small, static_cast<int>(groupPoints.size())); // Use the smaller of L_small or group size
        int effectiveK = min(K, static_cast<int>(groupPoints.size()));       // Use the smaller of K or group size
        // Ensure R is more than log2(data size) but less than data size
        int minR = static_cast<int>(ceil(log2(groupPoints.size())));                        // Round up
        int effectiveR = min(max(minR, R_small), static_cast<int>(groupPoints.size() - 1)); // Keep within bounds

        logFile << "Processing label: " << label << endl;
        logFile << "Group size: " << groupPoints.size() << endl;
        logFile << "Effective K: " << effectiveK << endl;
        logFile << "Effective L: " << effectiveL << endl;
        logFile << "Effective R: " << effectiveR << endl
                << endl;

        Vamana<T> vamana(effectiveK, effectiveL, effectiveR, A);

        // Redirect cout to log file during VamanaIndexing
        streambuf *coutBuffer = cout.rdbuf(); // Save the current buffer for cout
        cout.rdbuf(logFile.rdbuf());          // Redirect cout to log file
        vamana.VamanaIndexing(groupPoints);   // Perform Vamana indexing for the current group
        cout.rdbuf(coutBuffer);               // Restore cout to its original buffer

        // Integrate the Vamana subgraph into the main graph
        for (const auto &point : groupPoints)
        {
            // Get existing neighbors from the stitched graph
            auto existingNeighbors = StitchedGraph.GetNeighbors(point);

            // Get new neighbors from the current Vamana subgraph
            const auto &newNeighbors = vamana.VamanaGraph.GetNeighbors(point);

            // Merge existing and new neighbors
            vector<Point<T>> mergedNeighbors = existingNeighbors;
            mergedNeighbors.insert(mergedNeighbors.end(), newNeighbors.begin(), newNeighbors.end());

            // Remove duplicates
            sort(mergedNeighbors.begin(), mergedNeighbors.end());
            mergedNeighbors.erase(unique(mergedNeighbors.begin(), mergedNeighbors.end()), mergedNeighbors.end());

            // Update the stitched graph with the merged neighbors
            StitchedGraph.SetNeighbors(point, mergedNeighbors);
        }

        ++processedGroups;
        // Update progress bar only at intervals defined by step
        if (processedGroups % step == 0 || processedGroups == totalGroups)
        {
            UpdateProgressBar(processedGroups, totalGroups, "Stitched Vamana Indexing");
        }
    }

    // Apply robust pruning to create the stitched graph
    RobustPruner<T> pruner;
    for (const auto &point : data)
    {
        const auto &neighbors = StitchedGraph.GetNeighbors(point);
        pruner.FilteredRobustPrune(StitchedGraph, point, neighbors, A, R_stitched);
    }

    // Find FilterMedoids, to be able to search later
    if (FilterMedoids.empty())
    {
        FilterMedoids = FindFilterMedoids(CreateFilterMap(data), 0);
    }

    logFile.close();
}

template <typename T>
vector<Point<T>> Vamana<T>::StitchedSearch(const vector<Point<T>> &data, const Point<T> &query, const unordered_set<T> &filters) const
{
    return this->PerformSearch(
        StitchedGraph,
        data,
        query,
        filters);
}
