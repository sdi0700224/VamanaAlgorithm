#include "Point.h"
#include <cmath>
#include <functional>

// Custom hash specialization for Point<T>
namespace std
{
    template <typename T>
    struct hash<Point<T>>
    {
        size_t operator()(const Point<T> &point) const
        {
            size_t seed = 0;
            for (const T &coord : point.GetCoordinates())
            {
                seed ^= hash<T>{}(coord) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
            return seed;
        }
    };
}

template <typename T>
Point<T>::Point() : Coordinates(), Index(-1) {} // Initialize index to -1

template <typename T>
Point<T>::Point(const vector<T> &coordinates) : Coordinates(coordinates), Index(-1) {} // Default index

template <typename T>
Point<T>::Point(const vector<T> &coordinates, int index) : Coordinates(coordinates), Index(index) {}

template <typename T>
Point<T>::Point(const Point<T> &other) : Coordinates(other.Coordinates), Index(other.Index) {}

template <typename T>
bool Point<T>::operator==(const Point<T> &other) const
{
    return Coordinates == other.Coordinates;
}

template <typename T>
bool Point<T>::operator<(const Point<T> &other) const
{
    return Coordinates < other.Coordinates;
}

template <typename T>
bool Point<T>::operator!=(const Point<T> &other) const
{
    return !(*this == other);
}

template <typename T>
double Point<T>::DistanceTo(const Point<T> &other) const
{
    double squaredDistance = SquaredDistanceTo(other);
    return sqrt(max(squaredDistance, 0.0)); // Ensure non-negative before sqrt
}

// Squared distance without sqrt for better performance
template <typename T>
double Point<T>::SquaredDistanceTo(const Point<T> &other) const
{
    // Check dimensions sizes match
    if (Coordinates.size() != other.Coordinates.size())
    {
        throw invalid_argument("Points must have the same dimensions for distance calculation.");
    }

    double sum = 0.0;
    for (size_t i = 0; i < Coordinates.size(); ++i)
    {
        double diff = static_cast<double>(Coordinates[i]) - static_cast<double>(other.Coordinates[i]);
        sum += diff * diff;
    }
    return sum;
}

template <typename T>
const vector<T> &Point<T>::GetCoordinates() const
{
    return Coordinates;
}

template <typename T>
int Point<T>::GetIndex() const
{
    return Index;
}

template <typename U>
ostream &operator<<(ostream &os, const Point<U> &point)
{
    os << "Point(Index: " << point.Index << ", Coordinates: [";
    for (size_t i = 0; i < point.Coordinates.size(); ++i)
    {
        os << point.Coordinates[i];
        if (i != point.Coordinates.size() - 1)
            os << ", ";
    }
    os << "])";
    return os;
}
