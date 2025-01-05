// Adapted from https://github.com/KevinZeng08/sigmod-2024-contest.git)

#include <fstream>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

#include "assert.h"
using namespace std;

/// @brief Save knng in binary format (uint32_t) with name "output.bin"
/// @param knn a (N * 100) shape 2-D vector
/// @param path target save path, the output knng should be named as
/// "output.bin" for evaluation
void SaveKNN(const vector<vector<uint32_t>> &knns,
             const string &path = "output.bin")
{
  ofstream ofs(path, ios::out | ios::binary);
  const int K = 100;
  const uint32_t N = knns.size();
  assert(knns.front().size() == K);
  for (unsigned i = 0; i < N; ++i)
  {
    auto const &knn = knns[i];
    ofs.write(reinterpret_cast<char const *>(&knn[0]), K * sizeof(uint32_t));
  }
  ofs.close();
}

/// @brief Reading binary data vectors. Raw data store as a (N x dim)
/// @param file_path file path of binary data
/// @param data returned 2D data vectors
/// @brief Reading binary data vectors. Raw data store as a (N x dim)
/// @param file_path file path of binary data
/// @param data returned 2D data vectors
void ReadBin(const string &file_path,
             const int num_dimensions,
             vector<vector<float>> &data)
{
  cout << "Reading Data: " << file_path << endl;
  ifstream ifs;
  ifs.open(file_path, ios::binary);
  assert(ifs.is_open());
  uint32_t N; // num of points
  ifs.read((char *)&N, sizeof(uint32_t));
  data.resize(N);
  cout << "# of points: " << N << endl;
  vector<float> buff(num_dimensions);
  int counter = 0;
  while (ifs.read((char *)buff.data(), num_dimensions * sizeof(float)))
  {
    vector<float> row(num_dimensions);
    for (int d = 0; d < num_dimensions; d++)
    {
      row[d] = static_cast<float>(buff[d]);
    }
    data[counter++] = move(row);
  }
  ifs.close();
  cout << "Finish Reading Data" << endl;
}