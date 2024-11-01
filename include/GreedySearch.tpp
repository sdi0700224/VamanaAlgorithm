#include "GreedySearch.h"
#include <vector>
#include <set>
#include <unordered_set>
#include <utility>
#include <iostream>

template <typename T>
pair<vector<Point<T>>, vector<Point<T>>> GreedySearch<T>::FindApproximateNeighbors(
    const Graph<T> &graph,
    const Point<T> &startPoint,
    const Point<T> &queryPoint,
    int numResults,
    int maxCandidates) const
{
    set<pair<double, Point<T>>> visited;
    set<pair<double, Point<T>>> candidateSet;
    candidateSet.insert({startPoint.SquaredDistanceTo(queryPoint), startPoint});

    // Continue until we only have visited nodes left in candidateSet
    while (!candidateSet.empty())
    {
        // Find the closest unvisited point in candidateSet
        auto pStarIter = candidateSet.begin();
        while (pStarIter != candidateSet.end() && visited.count(*pStarIter) > 0)
        {
            ++pStarIter;
        }

        // If no unvisited nodes remain in candidateSet, terminate the loop
        if (pStarIter == candidateSet.end())
        {
            break;
        }

        Point<T> pStar = pStarIter->second;
        double pStarDist = pStarIter->first;

        visited.insert({pStarDist, pStar}); // Mark p* as visited

        // Get neighbors of p* and add them to candidates if not already visited
        for (const auto &neighbor : graph.GetNeighbors(pStar))
        {
            double dist = neighbor.SquaredDistanceTo(queryPoint);
            if (visited.count({dist, neighbor}) == 0) // Only consider unvisited neighbors
            {
                candidateSet.insert({dist, neighbor});
            }
        }

        // Maintain limited size for candidateSet by removing the farthest
        while ((int)candidateSet.size() > maxCandidates)
        {
            candidateSet.erase(--candidateSet.end());
        }
    }

    // Extract the closest k points from candidateSet
    vector<Point<T>> result;
    for (auto it = candidateSet.begin(); it != candidateSet.end() && (int)result.size() < numResults; ++it)
    {
        result.push_back(it->second);
    }

    // Convert visited set to vector
    vector<Point<T>> visitedVector;
    for (const auto &pair : visited)
    {
        visitedVector.push_back(pair.second);
    }

    return {result, visitedVector};
}
