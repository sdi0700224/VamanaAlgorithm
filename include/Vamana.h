#pragma once

#include <vector>
#include "Graph.h"
#include "GreedySearcher.h"
#include "RobustPruner.h"
#include "Point.h"
#include <gtest/gtest.h>

using namespace std;

template <typename T>
class Vamana
{
private:
    int K;
    int L;
    int R;
    double A;

    unordered_map<T, int> FilterMedoids;
    Point<T> Medoid;
    Graph<T> VamanaGraph;
    Graph<T> FilteredGraph;
    Graph<T> StitchedGraph;
    GreedySearcher<T> Searcher;
    RobustPruner<T> Pruner;

    void UpdateProgressBar(size_t current, size_t total, const string &message) const;
    vector<int> SampleRandomPoints(const vector<int> &pointIDs, int numSamples) const;
    unordered_map<T, vector<int>> CreateFilterMap(const vector<Point<T>> &data);
    unordered_map<T, int> FindFilterMedoids(const unordered_map<T, vector<int>> &filterMap,
                                            int threshold) const;
    Point<T> FindMedoid(const vector<Point<T>> &data, bool printMedoid = false) const;
    vector<Point<T>> PerformSearch(const Graph<T> &graph, const vector<Point<T>> &data,
                                   const Point<T> &query, const unordered_set<T> &filters) const;

    // Allow Google Test to access private methods for testing purposes
    FRIEND_TEST(VamanaTest, TestFindMedoid);

public:
    Vamana(int k, int l, int r, double a);

    void FilteredVamanaIndexing(const vector<Point<T>> &data);
    void VamanaIndexing(const vector<Point<T>> &data);
    void StitchedVamanaIndexing(const vector<Point<T>> &data,
                                int L_small, int R_small, int R_stitched);

    vector<Point<T>> FilteredSearch(const vector<Point<T>> &data, const Point<T> &query, const unordered_set<T> &filters) const;
    vector<Point<T>> StitchedSearch(const vector<Point<T>> &data, const Point<T> &query, const unordered_set<T> &filters) const;
    vector<Point<T>> Search(const Point<T> &query, int k) const;
};

#include "Vamana.tpp"
