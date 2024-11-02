#include "RobustPrune.h"
#include <algorithm>
#include <queue>
#include <cmath>

template <typename T>
void RobustPrune<T>::Prune(Graph<T> &graph, const Point<T> &p, const vector<Point<T>> &candidateNeighbors, double distanceThreshold, int degreeBound)
{
    using DistancePair = pair<double, Point<T>>;
    auto distanceComparator = [](const DistancePair &a, const DistancePair &b)
    {
        return a.first > b.first;
    };

    vector<DistancePair> candidates;

    // Collect all candidates with their squared distances to p
    for (const auto &candidate : candidateNeighbors)
    {
        if (candidate != p) // Ensure candidate is not the point itself
        {
            double dist = p.SquaredDistanceTo(candidate);
            candidates.emplace_back(dist, candidate);
        }
    }
    for (const auto &neighbor : graph.GetNeighbors(p))
    {
        if (neighbor != p) // Ensure neighbor is not the point itself
        {
            double dist = p.SquaredDistanceTo(neighbor);
            candidates.emplace_back(dist, neighbor);
        }
    }

    sort(candidates.begin(), candidates.end(), distanceComparator);

    vector<Point<T>> prunedNeighbors;
    prunedNeighbors.reserve(degreeBound);

    while (!candidates.empty())
    {
        // Get the closest candidate and add to prunedNeighbors
        auto [_, currentCandidate] = candidates.back();
        candidates.pop_back();
        prunedNeighbors.push_back(currentCandidate);

        if (prunedNeighbors.size() == static_cast<size_t>(degreeBound))
        {
            break;
        }

        // Filter remaining candidates in place to retain only those meeting the distance requirement
        auto newEnd = remove_if(candidates.begin(), candidates.end(),
                                [&](const DistancePair &candidate)
                                {
                                    double distOtherToCurrent = currentCandidate.SquaredDistanceTo(candidate.second);
                                    return distanceThreshold * distOtherToCurrent <= candidate.first;
                                });

        // Resize the vector to only keep valid candidates
        candidates.erase(newEnd, candidates.end());
    }

    graph.SetNeighbors(p, prunedNeighbors);
}
