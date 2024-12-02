#include "DataLoader.h"
#include <sstream>

vector<Point<float>> DataLoader::LoadDataset(const string &filename)
{
    ifstream file(filename, ios::binary);
    vector<Point<float>> points;

    if (!file.is_open())
    {
        cerr << "Error opening file: " << filename << endl;
        return points;
    }

    uint32_t num_vectors;
    file.read(reinterpret_cast<char *>(&num_vectors), sizeof(uint32_t));

    for (uint32_t i = 0; i < num_vectors; ++i)
    {
        vector<float> dataVector(102); // 102 dimensions: C, T, and 100 vector dimensions
        file.read(reinterpret_cast<char *>(dataVector.data()), 102 * sizeof(float));
        if (!file)
        {
            cerr << "Error reading vector data at index " << i << endl;
            break;
        }

        float filter = dataVector[0];                                        // Extract the categorical attribute (C) as the filter
        vector<float> coordinates(dataVector.begin() + 2, dataVector.end()); // Extract vector dimensions
        points.emplace_back(coordinates, filter, i);                         // Assign filter and index to Point
    }

    file.close();
    return points;
}

vector<vector<float>> DataLoader::LoadQuerySet(const string &filename)
{
    ifstream file(filename, ios::binary);
    vector<vector<float>> queries;

    if (!file.is_open())
    {
        cerr << "Error opening file: " << filename << endl;
        return queries;
    }

    uint32_t num_queries;
    file.read(reinterpret_cast<char *>(&num_queries), sizeof(uint32_t));

    for (uint32_t i = 0; i < num_queries; ++i)
    {
        vector<float> query(104); // 104 dimensions: query_type, v, l, r, and 100 vector dimensions
        file.read(reinterpret_cast<char *>(query.data()), 104 * sizeof(float));
        if (!file)
        {
            cerr << "Error reading query data at index " << i << endl;
            break;
        }
        queries.push_back(query); // Store the query vector
    }

    file.close();
    return queries;
}

vector<vector<int>> DataLoader::ReadGroundTruth(const string &gt_path, const int num_dimensions)
{
    cout << "Reading Ground Truth: " << gt_path << endl;
    ifstream ifs(gt_path, ios::binary);
    vector<vector<int>> gt;

    if (!ifs.is_open())
    {
        cerr << "Error opening file: " << gt_path << endl;
        return gt;
    }

    uint32_t N;
    ifs.read(reinterpret_cast<char *>(&N), sizeof(uint32_t));
    gt.resize(N);

    cout << "# of points: " << N << endl;
    vector<uint32_t> buff(num_dimensions);

    for (uint32_t i = 0; i < N; ++i)
    {
        ifs.read(reinterpret_cast<char *>(buff.data()), num_dimensions * sizeof(uint32_t));
        if (ifs.gcount() < static_cast<streamsize>(num_dimensions * sizeof(uint32_t)))
        {
            cerr << "Error reading data at point index " << i << endl;
            break;
        }

        vector<int> row(num_dimensions);
        for (int d = 0; d < num_dimensions; ++d)
        {
            row[d] = static_cast<int>(buff[d]);
        }

        gt[i] = move(row);
    }

    ifs.close();
    cout << "Finished Reading Ground Truth" << endl;
    return gt;
}

// Loads .fvecs files with indexed points
vector<Point<float>> DataLoader::LoadFvecs(const string &filename)
{
    ifstream file(filename, ios::binary);
    vector<Point<float>> points;

    if (!file.is_open())
    {
        cerr << "Error opening file: " << filename << endl;
        return points;
    }

    int index = 0;
    while (true)
    {
        int dimension;

        // Read dimension
        file.read(reinterpret_cast<char *>(&dimension), sizeof(int));
        if (!file)
        {
            break; // Stop if read fails (end of file)
        }

        // Read vector data
        vector<float> dataVector(dimension);
        file.read(reinterpret_cast<char *>(dataVector.data()), dimension * sizeof(float));
        if (!file)
        {
            break;
        }

        points.emplace_back(dataVector, index); // Create Point with coordinates and index
        ++index;                                // Increment index for next point
    }

    file.close();
    return points;
}

// Loads .ivecs files containing ground truth indices
vector<vector<int>> DataLoader::LoadIvecs(const string &filename)
{
    ifstream file(filename, ios::binary);
    vector<vector<int>> groundTruth;

    if (!file.is_open())
    {
        cerr << "Error opening file: " << filename << endl;
        return groundTruth;
    }

    while (!file.eof())
    {
        int dimension;
        file.read(reinterpret_cast<char *>(&dimension), sizeof(int));
        if (file.eof())
        {
            break;
        }

        vector<int> dataVector(dimension);
        file.read(reinterpret_cast<char *>(dataVector.data()), dimension * sizeof(int));

        groundTruth.push_back(dataVector); // Store the vector as a ground truth entry
    }

    file.close();
    return groundTruth;
}