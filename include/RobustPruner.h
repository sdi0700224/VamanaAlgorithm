#pragma once

#include <vector>
#include "Point.h"
#include "Graph.h"

using namespace std;

template <typename T>
class RobustPruner
{
public:
    void FilteredRobustPrune(Graph<T> &graph, const Point<T> &p, const vector<Point<T>> &candidateNeighbors,
                             double alpha, int degreeBound);

    void RobustPrune(Graph<T> &graph, const Point<T> &p, const vector<Point<T>> &candidateNeighbors,
                     double alpha, int degreeBound);
};

#include "RobustPruner.tpp"
