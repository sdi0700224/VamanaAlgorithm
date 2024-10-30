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

// Test fixture for DataLoader tests
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

        // Remove the test_data directory itself if empty
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

    ASSERT_EQ(points.size(), 2); // Expect two points
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

    ASSERT_EQ(groundTruth.size(), 2); // Expect two entries
    EXPECT_EQ(groundTruth[0], (vector<int>{1, 2, 3}));
    EXPECT_EQ(groundTruth[1], (vector<int>{4, 5, 6}));
}

// Test loading a nonexistent .fvecs file
TEST_F(DataLoaderTest, LoadFvecs_FileNotFound)
{
    auto points = loader.LoadFvecs("nonexistent.fvecs");
    EXPECT_TRUE(points.empty()); // Expect empty result on error
}

// Test loading a nonexistent .ivecs file
TEST_F(DataLoaderTest, LoadIvecs_FileNotFound)
{
    auto groundTruth = loader.LoadIvecs("nonexistent.ivecs");
    EXPECT_TRUE(groundTruth.empty()); // Expect empty result on error
}

// Test loading an empty .fvecs file
TEST_F(DataLoaderTest, LoadFvecs_EmptyFile)
{
    ofstream file("test_data/empty.fvecs", ios::binary); // Create an empty file
    file.close();

    auto points = loader.LoadFvecs("test_data/empty.fvecs");
    EXPECT_TRUE(points.empty()); // Expect no points loaded from an empty file

    remove("test_data/empty.fvecs");
}

// Test loading an empty .ivecs file
TEST_F(DataLoaderTest, LoadIvecs_EmptyFile)
{
    ofstream file("test_data/empty.ivecs", ios::binary); // Create an empty file
    file.close();

    auto groundTruth = loader.LoadIvecs("test_data/empty.ivecs");
    EXPECT_TRUE(groundTruth.empty()); // Expect no data loaded from an empty file

    remove("test_data/empty.ivecs");
}
