#pragma once

#include <vector>
#include <type_traits>
#include <stdexcept>
#include <iostream>

using namespace std;

template <typename T>
class Point
{
    static_assert(is_arithmetic<T>::value, "Point coordinates must be of an arithmetic type");

public:
    Point();
    Point(const vector<T> &coordinates);
    Point(const vector<T> &coordinates, int index);
    Point(const vector<T> &coordinates, T filter, int index);
    Point(const Point &other);

    bool operator==(const Point<T> &other) const;
    bool operator<(const Point<T> &other) const;
    bool operator!=(const Point<T> &other) const;
    Point &operator=(const Point &other);

    double DistanceTo(const Point<T> &other) const;
    double SquaredDistanceTo(const Point<T> &other) const;
    const vector<T> &GetCoordinates() const;
    int GetIndex() const;
    T GetFilter() const;

    template <typename U>
    friend ostream &operator<<(ostream &os, const Point<U> &point);

private:
    int Index;
    T Filter;
    vector<T> Coordinates;
};

#include "Point.tpp"
