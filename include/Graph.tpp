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

template <typename T>
void Graph<T>::Serialize(ostream &os) const
{
    size_t map_size = AdjacencyList.size();
    os.write(reinterpret_cast<const char *>(&map_size), sizeof(map_size));
    for (const auto &[point, neighbors] : AdjacencyList)
    {
        point.Serialize(os);
        size_t neighbor_size = neighbors.size();
        os.write(reinterpret_cast<const char *>(&neighbor_size), sizeof(neighbor_size));
        for (const auto &neighbor : neighbors)
        {
            neighbor.Serialize(os);
        }
    }
}

template <typename T>
void Graph<T>::Deserialize(istream &is)
{
    size_t map_size;
    is.read(reinterpret_cast<char *>(&map_size), sizeof(map_size));
    AdjacencyList.clear();
    for (size_t i = 0; i < map_size; ++i)
    {
        Point<T> point;
        point.Deserialize(is);
        size_t neighbor_size;
        is.read(reinterpret_cast<char *>(&neighbor_size), sizeof(neighbor_size));
        vector<Point<T>> neighbors(neighbor_size);
        for (size_t j = 0; j < neighbor_size; ++j)
        {
            neighbors[j].Deserialize(is);
        }
        AdjacencyList[point] = neighbors;
    }
}