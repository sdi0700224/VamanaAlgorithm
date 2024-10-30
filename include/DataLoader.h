#pragma once

#include <string>
#include <vector>
#include "Point.h"

using namespace std;

class DataLoader
{
public:
    DataLoader() = default;

    vector<Point<float>> LoadFvecs(const string &filename);
    vector<vector<int>> LoadIvecs(const string &filename);
};
