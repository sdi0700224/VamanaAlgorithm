#pragma once

#include <unordered_map>
#include <vector>
#include "Point.h"

using namespace std;

template <typename T>
class Graph
{
public:
    void AddPoint(const Point<T> &point);
    void AddEdge(const Point<T> &point1, const Point<T> &point2);
    const vector<Point<T>> &GetNeighbors(const Point<T> &point) const;
    void SetNeighbors(const Point<T> &node, const vector<Point<T>> &neighbors);
    void Serialize(ostream &os) const;
    void Deserialize(istream &is);

private:
    unordered_map<Point<T>, vector<Point<T>>> AdjacencyList;
};

#include "Graph.tpp"
