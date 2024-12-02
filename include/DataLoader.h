#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <cstdint>
#include "Point.h"

using namespace std;

class DataLoader
{
public:
    static vector<Point<float>> LoadDataset(const string &filename);
    static vector<vector<float>> LoadQuerySet(const string &filename);
    static vector<vector<int>> ReadGroundTruth(const string &gt_path, const int num_dimensions);

    static vector<Point<float>> LoadFvecs(const string &filename);
    static vector<vector<int>> LoadIvecs(const string &filename);
};
