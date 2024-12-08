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
Point<T>::Point() : Index(-1), Filter(-1), Coordinates() {}

template <typename T>
Point<T>::Point(const vector<T> &coordinates)
    : Index(-1), Filter(-1), Coordinates(coordinates) {}

template <typename T>
Point<T>::Point(const vector<T> &coordinates, int index)
    : Index(index), Filter(-1), Coordinates(coordinates) {}

template <typename T>
Point<T>::Point(const vector<T> &coordinates, T filter, int index)
    : Index(index), Filter(filter), Coordinates(coordinates) {}

template <typename T>
Point<T>::Point(const Point &other)
    : Index(other.Index), Filter(other.Filter), Coordinates(other.Coordinates) {}

template <typename T>
bool Point<T>::operator==(const Point<T> &other) const
{
    return Coordinates == other.Coordinates && Filter == other.Filter && Index == other.Index;
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
Point<T> &Point<T>::operator=(const Point &other)
{
    if (this != &other)
    {
        Index = other.Index;
        Filter = other.Filter;
        Coordinates = other.Coordinates;
    }
    return *this;
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

template <typename T>
T Point<T>::GetFilter() const
{
    return Filter;
}

template <typename T>
void Point<T>::Serialize(ostream &os) const
{
    size_t coord_size = Coordinates.size();
    os.write(reinterpret_cast<const char *>(&Index), sizeof(Index));
    os.write(reinterpret_cast<const char *>(&Filter), sizeof(Filter));
    os.write(reinterpret_cast<const char *>(&coord_size), sizeof(coord_size));
    os.write(reinterpret_cast<const char *>(Coordinates.data()), coord_size * sizeof(T));
}

template <typename T>
void Point<T>::Deserialize(istream &is)
{
    size_t coord_size;
    is.read(reinterpret_cast<char *>(&Index), sizeof(Index));
    is.read(reinterpret_cast<char *>(&Filter), sizeof(Filter));
    is.read(reinterpret_cast<char *>(&coord_size), sizeof(coord_size));
    Coordinates.resize(coord_size);
    is.read(reinterpret_cast<char *>(Coordinates.data()), coord_size * sizeof(T));
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
