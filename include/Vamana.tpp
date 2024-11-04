#include "Vamana.h"
#include <limits>
#include <numeric>
#include <algorithm>
#include <random>
#include <iomanip>

template <typename T>
Vamana<T>::Vamana(int k, int L, int R, double a) : K(k), L(L), R(R), A(a), VamanaGraph(), GreedySearcher(), RobustPruner() {}

template <typename T>
void Vamana<T>::BuildIndex(const vector<Point<T>> &data)
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
        auto [_, visitedNeighbors] = GreedySearcher.FindApproximateNeighbors(VamanaGraph, Medoid, point, 1, L);

        // Add directed edge
        for (const auto &neighbor : visitedNeighbors)
        {
            VamanaGraph.AddEdge(point, neighbor);
        }

        RobustPruner.Prune(VamanaGraph, point, visitedNeighbors, A, R);

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
                    RobustPruner.Prune(VamanaGraph, neighbor, outNeighbors, A, R);
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

    cout << "Medoid point found: " << medoid << endl;

    return medoid;
}

template <typename T>
vector<Point<T>> Vamana<T>::Search(const Point<T> &query, int k) const
{
    auto [approxNeighbors, _] = GreedySearcher.FindApproximateNeighbors(VamanaGraph, Medoid, query, k, L);
    return approxNeighbors;
}
