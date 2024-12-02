#include "Vamana.h"
#include <limits>
#include <numeric>
#include <algorithm>
#include <random>
#include <iomanip>
#include <cmath>

template <typename T>
Vamana<T>::Vamana(int k, int L, int R, double a)
    : K(k), L(L), R(R), A(a), VamanaGraph(), FilteredGraph(), StichedGraph(), Searcher(), Pruner() {}

template <typename T>
void Vamana<T>::FilteredVamanaIndexing(const vector<Point<T>> &data)
{
    // Step 1: Find the medoid of each filter using the filterMap for efficiency
    FilterMedoids = FindFilterMedoids(CreateFilterMap(data), 0);

    // Step 2: Generate a random permutation of the dataset
    vector<int> randomPermutation(data.size());
    iota(randomPermutation.begin(), randomPermutation.end(), 0);
    shuffle(randomPermutation.begin(), randomPermutation.end(), mt19937{random_device{}()});

    size_t total = randomPermutation.size();
    size_t step = total / 100; // Progress bar updates every 1%

    // Step 3: Iterate through points in random order
    for (size_t idx = 0; idx < total; ++idx)
    {
        size_t i = randomPermutation[idx];
        const auto &currentPoint = data[i];
        T currentFilter = currentPoint.GetFilter();

        // Step 3.1: Get the medoid (start node) for the current point's filter
        const auto &startNode = data[FilterMedoids[currentFilter]];

        // Step 3.2: Perform FilteredGreedySearch restricted to the current filter
        auto [_, visited] = Searcher.FilteredGreedySearch(
            FilteredGraph, currentPoint, {currentFilter}, // Restrict search to the same filter
            {startNode}, 0, L);

        // Step 3.3: Apply RobustPrune to prune the visited nodes
        Pruner.FilteredRobustPrune(FilteredGraph, currentPoint, visited, A, R);

        // Step 3.4: Update out-neighbors of the current point and ensure degree constraints
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

        // Step 3.5: Update progress bar
        if (idx % step == 0 || idx == total - 1)
        {
            int progress = static_cast<int>((100.0 * idx) / total);
            cout << "\rProgress: " << progress << "% [" << string(progress / 2, '=')
                 << string(50 - progress / 2, ' ') << "]" << flush;
        }
    }

    // Finalize the progress bar
    cout << "\rProgress: 100% [" << string(50, '=') << "] Done!" << endl;
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

        // Randomly sample `threshold` points from pointIDs
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
vector<Point<T>> Vamana<T>::FilteredSearch(const vector<Point<T>> &data, const Point<T> &query, const unordered_set<T> &filters) const
{
    // Step 1: Identify starting points for the search
    vector<Point<T>> startPoints;

    // Create a local filter set to handle cases where filters is empty
    unordered_set<T> localFilters = filters;

    // If filters is empty, populate localFilters with all keys from Medoids
    if (localFilters.empty())
    {
        for (const auto &medoid : FilterMedoids)
        {
            localFilters.insert(medoid.first);

            // Step 2: Perform the filtered greedy search
            auto [nearestStartPoint, _] = Searcher.FilteredGreedySearch(
                FilteredGraph,         // Graph containing the data points
                query,                 // The query point
                {medoid.first},        // The set of filters to consider
                {data[medoid.second]}, // Initial set of starting points
                1,                     // Number of results required
                1                      // Maximum candidate list size (adjustable)
            );

            startPoints.push_back(nearestStartPoint[0]);
        }
    }
    else
    {
        // If filters is not empty, process only the specified filters
        for (const auto &filter : localFilters)
        {
            if (FilterMedoids.find(filter) != FilterMedoids.end())
            {
                startPoints.push_back(data[FilterMedoids.at(filter)]);
            }
        }
    }

    // If no starting points are available, return an empty result
    if (startPoints.empty())
    {
        return {};
    }

    // Step 2: Perform the filtered greedy search
    auto [nearestNeighbors, _] = Searcher.FilteredGreedySearch(
        FilteredGraph, // Graph containing the data points
        query,         // The query point
        localFilters,  // The set of filters to consider
        startPoints,   // Initial set of starting points
        K,             // Number of results required
        L              // Maximum candidate list size (adjustable)
    );

    // Step 3: Return the nearest neighbors
    return nearestNeighbors;
}

template <typename T>
Point<T> Vamana<T>::FindMedoid(const vector<Point<T>> &data) const
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

    // cout << "Medoid point found: " << medoid << endl;

    return medoid;
}

template <typename T>
void Vamana<T>::VamanaIndexing(const vector<Point<T>> &data)
{
    // Find central point(medoid) to act as a starting point
    Medoid = FindMedoid(data);

    vector<size_t> randomPermutation(data.size());
    // Initialize vector with values from 0 to data size -1
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
    size_t progressStep = max(totalPoints / 100, static_cast<size_t>(1)); // Ensure progressStep is not 0
    size_t currentProgress = 0;

    for (size_t i = 0; i < randomPermutation.size(); ++i)
    {
        // Access points in random order
        const auto &point = data[randomPermutation[i]];

        // Find approximate neighbors using Greedy Search
        auto [_, visitedNeighbors] = Searcher.GreedySearch(VamanaGraph, Medoid, point, 1, L);

        // Add directed edge
        for (const auto &neighbor : visitedNeighbors)
        {
            VamanaGraph.AddEdge(point, neighbor);
        }

        Pruner.RobustPrune(VamanaGraph, point, visitedNeighbors, A, R);

        // Update visitedNeighbors and prune if needed
        for (const auto &neighbor : visitedNeighbors)
        {
            auto outNeighbors = VamanaGraph.GetNeighbors(neighbor);

            // Examine point only if not in outNeighbors
            if (find(outNeighbors.begin(), outNeighbors.end(), point) == outNeighbors.end())
            {
                // If number of out neighbors plus current point exceeds R, inlude point and prune
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
            currentProgress = (i * 100) / totalPoints;
            cout << "\rBuilding index: " << setw(3) << currentProgress << "% complete" << flush;
        }
    }

    cout << "\rBuilding index: 100% complete\n"
         << flush;
}

template <typename T>
vector<Point<T>> Vamana<T>::Search(const Point<T> &query, int k) const
{
    auto [approxNeighbors, _] = Searcher.GreedySearch(VamanaGraph, Medoid, query, k, L);
    return approxNeighbors;
}

template <typename T>
void Vamana<T>::StitchedVamanaIndexing(
    const vector<Point<T>> &data,
    int L_small,
    int R_small,
    int R_stitched)
{
    // Create a map of filters to point groups
    auto filterMap = CreateFilterMap(data);
    unordered_map<T, vector<Point<T>>> labelGroups;

    for (const auto &[filter, pointIDs] : filterMap)
    {
        for (const auto &pointID : pointIDs)
        {
            labelGroups[filter].push_back(data[pointID]); // Append points to groups
        }
    }

    // Process each group independently
    for (const auto &[label, groupPoints] : labelGroups)
    {
        if (groupPoints.size() < 2)
        {
            // Skip groups with fewer than 2 points
            continue;
        }

        // Adjust parameters to ensure they are within valid bounds
        int effectiveL = min(L_small, (static_cast<int>(groupPoints.size()) / 2));
        int effectiveK = min(K, (static_cast<int>(groupPoints.size()) / 4));
        if (effectiveK == 0)
        {
            effectiveK++;
        }

        // Ensure R is more than log2(data.size()) but less than data.size()
        int minR = static_cast<int>(ceil(log2(groupPoints.size()))); // Ceil to round up
        int effectiveR = min(max(minR + 1, R_small), (static_cast<int>(groupPoints.size()) / 3));

        // Debug: Print values used for this Vamana indexing
        cout << "Processing label: " << label << endl;
        cout << "Group size: " << groupPoints.size() << endl;
        cout << "Effective K: " << effectiveK << endl;
        cout << "Effective L: " << effectiveL << endl;
        cout << "Effective R: " << effectiveR << endl;

        // Create a new Vamana instance for this group
        Vamana<T> vamana(effectiveK, effectiveL, effectiveR, A);

        // Index the group points using Vamana
        vamana.VamanaIndexing(groupPoints);

        // Integrate the Vamana subgraph into the main graph
        for (const auto &point : groupPoints)
        {
            // Get existing neighbors from the stitched graph
            auto existingNeighbors = StichedGraph.GetNeighbors(point);

            // Get new neighbors from the current Vamana subgraph
            const auto &newNeighbors = vamana.VamanaGraph.GetNeighbors(point);

            // Merge existing and new neighbors
            vector<Point<T>> mergedNeighbors = existingNeighbors;
            mergedNeighbors.insert(mergedNeighbors.end(), newNeighbors.begin(), newNeighbors.end());

            // Remove duplicates
            sort(mergedNeighbors.begin(), mergedNeighbors.end());
            mergedNeighbors.erase(unique(mergedNeighbors.begin(), mergedNeighbors.end()), mergedNeighbors.end());

            // Update the graph with the merged neighbors
            StichedGraph.SetNeighbors(point, mergedNeighbors);
        }
    }

    // Apply robust pruning to finalize the stitched graph
    RobustPruner<T> pruner;
    for (const auto &point : data)
    {
        const auto &neighbors = StichedGraph.GetNeighbors(point);
        pruner.FilteredRobustPrune(StichedGraph, point, neighbors, A, R_stitched);
    }

    FilterMedoids = FindFilterMedoids(CreateFilterMap(data), 0);
}

template <typename T>
vector<Point<T>> Vamana<T>::StitchedSearch(const vector<Point<T>> &data, const Point<T> &query, const unordered_set<T> &filters) const
{
    // Step 1: Identify starting points for the search
    vector<Point<T>> startPoints;

    // Create a local filter set to handle cases where filters is empty
    unordered_set<T> localFilters = filters;

    // If filters is empty, populate localFilters with all keys from Medoids
    if (localFilters.empty())
    {
        for (const auto &medoid : FilterMedoids)
        {
            localFilters.insert(medoid.first);

            // Step 2: Perform the filtered greedy search
            auto [nearestStartPoint, _] = Searcher.FilteredGreedySearch(
                StichedGraph,          // Graph containing the data points
                query,                 // The query point
                {medoid.first},        // The set of filters to consider
                {data[medoid.second]}, // Initial set of starting points
                1,                     // Number of results required
                1                      // Maximum candidate list size (adjustable)
            );

            startPoints.push_back(nearestStartPoint[0]);
        }
    }
    else
    {
        // If filters is not empty, process only the specified filters
        for (const auto &filter : localFilters)
        {
            if (FilterMedoids.find(filter) != FilterMedoids.end())
            {
                startPoints.push_back(data[FilterMedoids.at(filter)]);
            }
        }
    }

    // If no starting points are available, return an empty result
    if (startPoints.empty())
    {
        return {};
    }

    // Step 2: Perform the filtered greedy search
    auto [nearestNeighbors, _] = Searcher.FilteredGreedySearch(
        StichedGraph, // Graph containing the data points
        query,        // The query point
        localFilters, // The set of filters to consider
        startPoints,  // Initial set of starting points
        K,            // Number of results required
        L             // Maximum candidate list size (adjustable)
    );

    // Step 3: Return the nearest neighbors
    return nearestNeighbors;
}
