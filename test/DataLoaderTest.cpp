#include "DataLoader.h"
#include "Point.h"
#include <gtest/gtest.h>
#include <vector>
#include <fstream>
#include <filesystem>

using namespace std;

// Helper function to create a binary .fvecs file for testing
void CreateTestFvecs(const string &filename, const vector<vector<float>> &data)
{
    ofstream file(filename, ios::binary);
    for (const auto &vec : data)
    {
        int dim = vec.size();
        file.write(reinterpret_cast<const char *>(&dim), sizeof(int));
        file.write(reinterpret_cast<const char *>(vec.data()), dim * sizeof(float));
    }
    file.close();
}

// Helper function to create a binary .ivecs file for testing
void CreateTestIvecs(const string &filename, const vector<vector<int>> &data)
{
    ofstream file(filename, ios::binary);
    for (const auto &vec : data)
    {
        int dim = vec.size();
        file.write(reinterpret_cast<const char *>(&dim), sizeof(int));
        file.write(reinterpret_cast<const char *>(vec.data()), dim * sizeof(int));
    }
    file.close();
}

class DataLoaderTest : public ::testing::Test
{
protected:
    DataLoader loader;
    string testFvecsFile = "test_data/test_data.fvecs";
    string testIvecsFile = "test_data/test_data.ivecs";

    void SetUp() override
    {
        // Ensure test_data directory exists
        filesystem::create_directory("test_data");

        // Create test data
        vector<vector<float>> fvecsData = {{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}};
        vector<vector<int>> ivecsData = {{1, 2, 3}, {4, 5, 6}};

        // Create test files
        CreateTestFvecs(testFvecsFile, fvecsData);
        CreateTestIvecs(testIvecsFile, ivecsData);
    }

    void TearDown() override
    {
        // Remove individual test files if they exist
        if (filesystem::exists(testFvecsFile))
        {
            filesystem::remove(testFvecsFile);
        }
        if (filesystem::exists(testIvecsFile))
        {
            filesystem::remove(testIvecsFile);
        }

        // Remove the test_data directory itself
        filesystem::remove("test_data");
    }
};

// Test loading a valid .fvecs file with diagnostics
TEST_F(DataLoaderTest, LoadFvecs_ValidFile)
{
    auto points = loader.LoadFvecs(testFvecsFile);

    // Diagnostic output
    cerr << "Number of points loaded: " << points.size() << endl;
    for (size_t i = 0; i < points.size(); ++i)
    {
        cerr << "Point " << i << " coordinates: ";
        for (float coord : points[i].GetCoordinates())
        {
            cerr << coord << " ";
        }
        cerr << endl;
    }

    ASSERT_EQ((int)points.size(), 2); // Expect two points
    EXPECT_EQ(points[0].GetCoordinates(), (vector<float>{1.0, 2.0, 3.0}));
    EXPECT_EQ(points[1].GetCoordinates(), (vector<float>{4.0, 5.0, 6.0}));
}

// Test loading a valid .ivecs file with diagnostics
TEST_F(DataLoaderTest, LoadIvecs_ValidFile)
{
    auto groundTruth = loader.LoadIvecs(testIvecsFile);

    // Diagnostic output
    cerr << "Number of ground truth entries loaded: " << groundTruth.size() << endl;
    for (size_t i = 0; i < groundTruth.size(); ++i)
    {
        cerr << "Ground truth " << i << " values: ";
        for (int value : groundTruth[i])
        {
            cerr << value << " ";
        }
        cerr << endl;
    }

    ASSERT_EQ((int)groundTruth.size(), 2); // Expect two entries
    EXPECT_EQ(groundTruth[0], (vector<int>{1, 2, 3}));
    EXPECT_EQ(groundTruth[1], (vector<int>{4, 5, 6}));
}

// Test loading a nonexistent .fvecs file
TEST_F(DataLoaderTest, LoadFvecs_FileNotFound)
{
    auto points = loader.LoadFvecs("nonexistent.fvecs");
    EXPECT_TRUE(points.empty()); // Expect empty result
}

// Test loading a nonexistent .ivecs file
TEST_F(DataLoaderTest, LoadIvecs_FileNotFound)
{
    auto groundTruth = loader.LoadIvecs("nonexistent.ivecs");
    EXPECT_TRUE(groundTruth.empty()); // Expect empty result
}

// Test loading an empty .fvecs file
TEST_F(DataLoaderTest, LoadFvecs_EmptyFile)
{
    ofstream file("test_data/empty.fvecs", ios::binary); // Create an empty file
    file.close();

    auto points = loader.LoadFvecs("test_data/empty.fvecs");
    EXPECT_TRUE(points.empty()); // Expect no points loaded

    remove("test_data/empty.fvecs");
}

// Test loading an empty .ivecs file
TEST_F(DataLoaderTest, LoadIvecs_EmptyFile)
{
    ofstream file("test_data/empty.ivecs", ios::binary); // Create an empty file
    file.close();

    auto groundTruth = loader.LoadIvecs("test_data/empty.ivecs");
    EXPECT_TRUE(groundTruth.empty()); // Expect no data loaded

    remove("test_data/empty.ivecs");
}

// Test loading a dataset with fixed 102 dimensions
TEST_F(DataLoaderTest, LoadDataset_ValidFile)
{
    string testDatasetFile = "test_data/test_dataset.bin";

    // Create dataset file with two vectors
    ofstream file(testDatasetFile, ios::binary);
    uint32_t num_vectors = 2; // Indicate two vectors in the file
    file.write(reinterpret_cast<const char *>(&num_vectors), sizeof(uint32_t));

    vector<float> vector1 = {1.0f, 0.0f};     // Filter = 1.0
    vector1.insert(vector1.end(), 100, 1.0f); // Append 100 coordinates with value 1.0

    vector<float> vector2 = {2.0f, 0.0f};     // Filter = 2.0
    vector2.insert(vector2.end(), 100, 2.0f); // Append 100 coordinates with value 2.0

    file.write(reinterpret_cast<const char *>(vector1.data()), 102 * sizeof(float));
    file.write(reinterpret_cast<const char *>(vector2.data()), 102 * sizeof(float));
    file.close();

    auto points = loader.LoadDataset(testDatasetFile);

    // Validate the number of points loaded
    ASSERT_EQ(points.size(), static_cast<size_t>(2));

    // Validate the first point
    EXPECT_EQ(points[0].GetFilter(), 1.0f);                          // Check the filter
    EXPECT_EQ(points[0].GetCoordinates(), vector<float>(100, 1.0f)); // Check coordinates

    // Validate the second point
    EXPECT_EQ(points[1].GetFilter(), 2.0f);                          // Check the filter
    EXPECT_EQ(points[1].GetCoordinates(), vector<float>(100, 2.0f)); // Check coordinates

    // Cleanup
    remove(testDatasetFile.c_str());
}

// Test loading an empty file
TEST_F(DataLoaderTest, LoadDataset_EmptyFile)
{
    string emptyDatasetFile = "test_data/empty_dataset.bin";

    // Create an empty file
    ofstream file(emptyDatasetFile, ios::binary);
    file.close();

    auto points = loader.LoadDataset(emptyDatasetFile);

    EXPECT_TRUE(points.empty()); // No points should be loaded

    // Cleanup
    remove(emptyDatasetFile.c_str());
}

// Test loading a dataset with incomplete data
TEST_F(DataLoaderTest, LoadDataset_IncompleteFile)
{
    string incompleteDatasetFile = "test_data/incomplete_dataset.bin";

    // Create a file with a vector missing some dimensions
    ofstream file(incompleteDatasetFile, ios::binary);
    uint32_t num_vectors = 1; // Declare one vector
    file.write(reinterpret_cast<const char *>(&num_vectors), sizeof(uint32_t));

    // Write only partial data for the vector (e.g., 50 dimensions instead of 102)
    vector<float> partialVector(50, 1.0f);
    file.write(reinterpret_cast<const char *>(partialVector.data()), 50 * sizeof(float));
    file.close();

    auto points = loader.LoadDataset(incompleteDatasetFile);

    EXPECT_TRUE(points.empty()); // No valid points should be loaded

    // Cleanup
    remove(incompleteDatasetFile.c_str());
}

// Test loading a nonexistent dataset file
TEST_F(DataLoaderTest, LoadDataset_FileNotFound)
{
    string nonexistentFile = "test_data/nonexistent_dataset.bin";

    auto points = loader.LoadDataset(nonexistentFile);

    EXPECT_TRUE(points.empty()); // Expect empty result since file doesn't exist
}

// Test loading a query set with fixed 104 dimensions
TEST_F(DataLoaderTest, LoadQuerySet_ValidFile)
{
    string testQuerySetFile = "test_data/test_queryset.bin";

    // Create query set file with two queries
    ofstream file(testQuerySetFile, ios::binary);
    uint32_t num_queries = 2; // Use fixed 104 dimensions
    file.write(reinterpret_cast<const char *>(&num_queries), sizeof(uint32_t));

    vector<float> query1(104, 1.0f); // 104 dimensions, all set to 1.0
    vector<float> query2(104, 2.0f); // 104 dimensions, all set to 2.0
    file.write(reinterpret_cast<const char *>(query1.data()), 104 * sizeof(float));
    file.write(reinterpret_cast<const char *>(query2.data()), 104 * sizeof(float));
    file.close();

    auto queries = loader.LoadQuerySet(testQuerySetFile);

    ASSERT_EQ(queries.size(), static_cast<size_t>(2)); // Explicit cast to size_t
    EXPECT_EQ(queries[0], vector<float>(104, 1.0f));
    EXPECT_EQ(queries[1], vector<float>(104, 2.0f));

    remove(testQuerySetFile.c_str());
}

// Test reading ground truth data
TEST_F(DataLoaderTest, ReadGroundTruth_ValidFile)
{
    string testGTFile = "test_data/test_groundtruth.bin";
    const int num_dimensions = 3; // Fixed number of dimensions

    // Create ground truth file with two entries
    ofstream file(testGTFile, ios::binary);
    uint32_t num_entries = 2;
    file.write(reinterpret_cast<const char *>(&num_entries), sizeof(uint32_t));

    vector<uint32_t> gt1 = {1, 2, 3};
    vector<uint32_t> gt2 = {4, 5, 6};
    file.write(reinterpret_cast<const char *>(gt1.data()), num_dimensions * sizeof(uint32_t));
    file.write(reinterpret_cast<const char *>(gt2.data()), num_dimensions * sizeof(uint32_t));
    file.close();

    auto groundTruth = loader.ReadGroundTruth(testGTFile, num_dimensions);

    ASSERT_EQ(groundTruth.size(), static_cast<size_t>(2)); // Explicit cast to size_t
    EXPECT_EQ(groundTruth[0], vector<int>({1, 2, 3}));
    EXPECT_EQ(groundTruth[1], vector<int>({4, 5, 6}));

    remove(testGTFile.c_str());
}
