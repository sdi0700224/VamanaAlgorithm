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
    Medoid = FindMedoid(data);

    // Initialize random permutation of data
    vector<size_t> randomPermutation(data.size());
    iota(randomPermutation.begin(), randomPermutation.end(), 0);
    shuffle(randomPermutation.begin(), randomPermutation.end(), mt19937{random_device{}()});

    // Initialize each point in the graph with exactly R unique neighbors
    for (const auto &point : data)
    {
        vector<Point<T>> randomNeighbors;
        unordered_set<Point<T>> uniqueNeighbors;

        while (uniqueNeighbors.size() < static_cast<size_t>(R))
        {
            vector<Point<T>> shuffledData = data;
            shuffle(shuffledData.begin(), shuffledData.end(), mt19937{random_device{}()});

            for (const auto &candidate : shuffledData)
            {
                if (candidate != point && uniqueNeighbors.size() < static_cast<size_t>(R))
                {
                    uniqueNeighbors.insert(candidate);
                }
            }
        }

        randomNeighbors.assign(uniqueNeighbors.begin(), uniqueNeighbors.end());
        VamanaGraph.SetNeighbors(point, randomNeighbors);
    }

    // Progress indicator variables
    size_t totalPoints = data.size();
    size_t progressStep = totalPoints / 100;
    size_t currentProgress = 0;

    for (size_t i = 0; i < randomPermutation.size(); ++i)
    {
        const auto &point = data[randomPermutation[i]];

        auto [_, visitedNeighbors] = GreedySearcher.FindApproximateNeighbors(VamanaGraph, Medoid, point, 1, L);

        for (const auto &neighbor : visitedNeighbors)
        {
            VamanaGraph.AddEdge(point, neighbor);
        }

        RobustPruner.Prune(VamanaGraph, point, visitedNeighbors, A, R);

        for (const auto &neighbor : visitedNeighbors)
        {
            auto outNeighbors = VamanaGraph.GetNeighbors(neighbor);

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

        // Update the progress bar
        if (i % progressStep == 0 || i == randomPermutation.size() - 1) // Update every progressStep (1%)
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
    T minDistSum = numeric_limits<T>::max();
    Point<T> medoid = data[0];

    for (const auto &candidate : data)
    {
        T distSum = 0;
        for (const auto &other : data)
        {
            distSum += candidate.DistanceTo(other);
        }
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
    // Perform the greedy search starting from the medoid
    auto [approxNeighbors, _] = GreedySearcher.FindApproximateNeighbors(VamanaGraph, Medoid, query, k, L);
    return approxNeighbors;
}
