#include "RobustPruner.h"
#include <algorithm>
#include <vector>
#include <unordered_set>

template <typename T>
void RobustPruner<T>::FilteredRobustPrune(Graph<T> &graph, const Point<T> &p, const vector<Point<T>> &candidateNeighbors,
                                          double alpha, int degreeBound)
{
    using DistancePair = pair<double, Point<T>>;

    // Step 1: Initialize candidate set
    vector<DistancePair> candidates;
    auto addCandidate = [&](const Point<T> &candidate)
    {
        if (candidate != p)
        {
            double dist = p.SquaredDistanceTo(candidate);
            candidates.emplace_back(dist, candidate);
        }
    };

    for (const auto &candidate : candidateNeighbors)
    {
        addCandidate(candidate);
    }
    for (const auto &neighbor : graph.GetNeighbors(p))
    {
        addCandidate(neighbor);
    }

    // Sort candidates based on distance (smallest distance first)
    sort(candidates.begin(), candidates.end(),
         [](const DistancePair &a, const DistancePair &b)
         { return a.first > b.first; });

    // Step 2: Pruning loop
    vector<Point<T>> prunedNeighbors;
    prunedNeighbors.reserve(degreeBound);

    while (!candidates.empty())
    {
        auto [dist, currentCandidate] = candidates.back();
        candidates.pop_back();

        // Add to pruned neighbors only if degree bound is not exceeded
        prunedNeighbors.push_back(currentCandidate);
        if (prunedNeighbors.size() == static_cast<size_t>(degreeBound))
        {
            break;
        }

        // Filter and distance conditions
        auto filterCondition = [&](const Point<T> &pPrime) -> bool
        {
            const auto &Fp = p.GetFilter();                       // Filter of p
            const auto &FpPrime = pPrime.GetFilter();             // Filter of pPrime
            const auto &FpCurrent = currentCandidate.GetFilter(); // Filter of currentCandidate

            return (Fp == FpPrime) && (FpPrime == FpCurrent);
        };

        auto newEnd = remove_if(candidates.begin(), candidates.end(),
                                [&](const DistancePair &candidate)
                                {
                                    const auto &pPrime = candidate.second;

                                    if (!filterCondition(pPrime))
                                    {
                                        return false; // Keep these candidates
                                    }

                                    // Distance condition based on alpha
                                    double distToCurrent = currentCandidate.SquaredDistanceTo(pPrime);
                                    return alpha * distToCurrent <= candidate.first;
                                });

        // Erase removed candidates
        candidates.erase(newEnd, candidates.end());
    }

    // Step 4: Update graph with pruned neighbors
    graph.SetNeighbors(p, prunedNeighbors);
}

template <typename T>
void RobustPruner<T>::RobustPrune(Graph<T> &graph, const Point<T> &p, const vector<Point<T>> &candidateNeighbors, double alpha, int degreeBound)
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
                                    return alpha * distOtherToCurrent <= candidate.first;
                                });

        // Resize the vector to only keep valid candidates
        candidates.erase(newEnd, candidates.end());
    }

    graph.SetNeighbors(p, prunedNeighbors);
}
