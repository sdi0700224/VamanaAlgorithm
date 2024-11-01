#pragma once

#include <vector>
#include "Graph.h"
#include "GreedySearch.h"
#include "RobustPrune.h"
#include "Point.h"

using namespace std;

template <typename T>
class Vamana
{
public:
    Vamana(int k, int l, int r, double a);

    void BuildIndex(const vector<Point<T>> &data);
    vector<Point<T>> Search(const Point<T> &query, int k) const;

private:
    int K;    // Number of neighbors to search for
    int L;    // List size for the search process
    int R;    // Degree bound
    double A; // Distance factor for pruning

    Point<T> Medoid;
    Graph<T> VamanaGraph;
    GreedySearch<T> GreedySearcher;
    RobustPrune<T> RobustPruner;

    Point<T> FindMedoid(const vector<Point<T>> &data) const;
};

#include "Vamana.tpp"
