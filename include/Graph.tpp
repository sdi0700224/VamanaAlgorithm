#include "Graph.h"
#include <stdexcept>

template <typename T>
void Graph<T>::AddPoint(const Point<T> &point)
{
    if (AdjacencyList.find(point) == AdjacencyList.end())
    {
        AdjacencyList[point] = vector<Point<T>>(); // Initialize empty neighbor list
    }
}

template <typename T>
void Graph<T>::AddEdge(const Point<T> &point1, const Point<T> &point2)
{
    auto &neighbors = AdjacencyList[point1];
    if (find(neighbors.begin(), neighbors.end(), point2) == neighbors.end())
    {
        neighbors.push_back(point2); // Add only if point2 is not already a neighbor
    }
}

template <typename T>
const vector<Point<T>> &Graph<T>::GetNeighbors(const Point<T> &point) const
{
    static const vector<Point<T>> emptyVector; // Avoid reallocation
    auto it = AdjacencyList.find(point);
    if (it != AdjacencyList.end())
    {
        return it->second;
    }
    else
    {
        return emptyVector;
    }
}

template <typename T>
void Graph<T>::SetNeighbors(const Point<T> &node, const vector<Point<T>> &neighbors)
{
    AdjacencyList[node] = neighbors;
}
