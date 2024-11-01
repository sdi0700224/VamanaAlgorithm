#pragma once

#include <vector>
#include <unordered_set>
#include "Point.h"
#include "Graph.h"

using namespace std;

template <typename T>
class GreedySearch
{
public:
    pair<vector<Point<T>>, vector<Point<T>>> FindApproximateNeighbors(
        const Graph<T> &graph,
        const Point<T> &startPoint,
        const Point<T> &queryPoint,
        int numResults,
        int maxCandidates) const;
};

#include "GreedySearch.tpp"
