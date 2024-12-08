#include <gtest/gtest.h>
#include "Vamana.h"
#include "Point.h"
#include <vector>
#include <filesystem>

using namespace std;

class VamanaTest : public ::testing::Test
{
protected:
    const string stitchedLogFile = "test_stitched_vamana";

    int k = 10;     // Number of neighbors to search for
    int l = 20;     // List size for the search process
    int r = 15;     // Degree bound
    double a = 1.2; // Distance factor for pruning
    int dataSize = 100;
    Vamana<float> *vamana;

    void SetUp() override
    {
        vamana = new Vamana<float>(k, l, r, a);
    }

    void TearDown() override
    {
        if (filesystem::exists(stitchedLogFile))
        {
            filesystem::remove(stitchedLogFile);
        }
        delete vamana;
    }
};

TEST_F(VamanaTest, TestConstruction)
{
    ASSERT_NO_THROW({
        Vamana<float> test_vamana(k, l, r, a);
    });
}

TEST_F(VamanaTest, TestBuildIndex)
{
    vector<Point<float>> data;
    data.reserve(dataSize);

    // Generate points with incremental coordinates and IDs
    for (int i = 0; i < dataSize; ++i)
    {
        data.emplace_back(Point<float>({static_cast<float>(i % 10 + 1),
                                        static_cast<float>((i / 10) % 10 + 1),
                                        static_cast<float>(i % 5 + 1)},
                                       i));
    }

    EXPECT_NO_THROW(vamana->VamanaIndexing(data, false));
}

TEST_F(VamanaTest, TestSearch)
{
    vector<Point<float>> data;
    data.reserve(dataSize);

    // Generate points with incremental coordinates and IDs
    for (int i = 0; i < dataSize; ++i)
    {
        data.emplace_back(Point<float>({static_cast<float>(i % 10 + 1),
                                        static_cast<float>((i / 10) % 10 + 1),
                                        static_cast<float>(i % 5 + 1)},
                                       i));
    }

    vamana->VamanaIndexing(data, false);

    // Define a query point close to the generated data points
    Point<float> query({2.5, 3.5, 4.5});

    // Perform the search
    auto results = vamana->Search(query, k);

    // Check that exactly k neighbors are returned
    EXPECT_EQ((int)results.size(), k);

    // Check that distances are non-negative and in ascending order
    float previousDistance = -1.0f;
    for (const auto &point : results)
    {
        float currentDistance = point.DistanceTo(query);

        // Distance should be non-negative
        EXPECT_GE(currentDistance, 0.0);

        // Distance should be in non-decreasing order
        EXPECT_GE(currentDistance, previousDistance);

        previousDistance = currentDistance;
    }
}

TEST_F(VamanaTest, TestFindMedoid)
{
    vector<Point<float>> data = {
        Point<float>({1.0, 1.0, 1.0}, 0),
        Point<float>({2.0, 2.0, 2.0}, 1),
        Point<float>({3.0, 3.0, 3.0}, 2),
        Point<float>({5.0, 5.0, 5.0}, 3),
        Point<float>({1.5, 1.5, 1.5}, 4),
        Point<float>({4.0, 4.0, 4.0}, 5),
        Point<float>({6.0, 6.0, 6.0}, 6),
        Point<float>({2.5, 2.5, 2.5}, 7),
        Point<float>({3.5, 3.5, 3.5}, 8),
        Point<float>({4.5, 4.5, 4.5}, 9)};

    Point<float> medoid = vamana->FindMedoid(data);

    EXPECT_EQ(medoid, Point<float>({3.5, 3.5, 3.5}, 8));
}

TEST_F(VamanaTest, TestFilteredVamanaIndexing)
{
    vector<Point<float>> data;
    data.reserve(dataSize);

    // Generate points with filter values
    for (int i = 0; i < dataSize; ++i)
    {
        float filter = static_cast<float>(i % 3); // Assign filter values cyclically: 0, 1, 2
        data.emplace_back(Point<float>({i % 5 + 1.0f, i % 7 + 2.0f}, filter, i));
    }

    EXPECT_NO_THROW(vamana->FilteredVamanaIndexing(data, false));
}

TEST_F(VamanaTest, TestStitchedVamanaIndexing)
{
    vector<Point<float>> data;
    data.reserve(dataSize);

    // Generate points with filter values
    for (int i = 0; i < dataSize; ++i)
    {
        float filter = static_cast<float>(i % 4); // Assign filter values cyclically: 0, 1, 2, 3
        data.emplace_back(Point<float>({i % 5 + 1.0f, i % 7 + 2.0f}, filter, i));
    }

    EXPECT_NO_THROW(vamana->StitchedVamanaIndexing(data, 5, 10, 15, stitchedLogFile, false));
}

TEST_F(VamanaTest, TestFilteredSearch)
{
    vector<Point<float>> data;
    data.reserve(dataSize);

    // Generate points with filter values
    for (int i = 0; i < dataSize; ++i)
    {
        float filter = static_cast<float>(i % 3); // Assign filter values cyclically: 0, 1, 2
        data.emplace_back(Point<float>({i % 5 + 1.0f, i % 7 + 2.0f}, filter, i));
    }

    unordered_set<float> filters = {1.0f, 2.0f}; // Apply filters
    Point<float> query({3.0f, 4.0f});

    vamana->FilteredVamanaIndexing(data, false);
    auto results = vamana->FilteredSearch(data, query, filters);

    // Ensure the results are non-empty
    EXPECT_FALSE(results.empty());

    // Verify the results are sorted by distance
    float previousDistance = -1.0f;
    for (const auto &point : results)
    {
        float currentDistance = point.DistanceTo(query);

        // Distance should be non-negative
        EXPECT_GE(currentDistance, 0.0);

        // Distances should be in ascending order
        EXPECT_GE(currentDistance, previousDistance);

        previousDistance = currentDistance;
    }
}

TEST_F(VamanaTest, TestStitchedSearch)
{
    vector<Point<float>> data;
    data.reserve(dataSize);

    // Generate points with filter values
    for (int i = 0; i < dataSize; ++i)
    {
        float filter = static_cast<float>(i % 4); // Assign filter values cyclically: 0, 1, 2, 3
        data.emplace_back(Point<float>({i % 5 + 1.0f, i % 7 + 2.0f}, filter, i));
    }

    unordered_set<float> filters = {1.0f, 3.0f}; // Apply filters
    Point<float> query({3.0f, 4.0f});

    vamana->StitchedVamanaIndexing(data, 5, 10, 15, stitchedLogFile, false);
    auto results = vamana->StitchedSearch(data, query, filters);

    // Ensure the results are non-empty
    EXPECT_FALSE(results.empty());

    // Verify the results are sorted by distance
    float previousDistance = -1.0f;
    for (const auto &point : results)
    {
        float currentDistance = point.DistanceTo(query);

        // Distance should be non-negative
        EXPECT_GE(currentDistance, 0.0);

        // Distances should be in ascending order
        EXPECT_GE(currentDistance, previousDistance);

        previousDistance = currentDistance;
    }
}

TEST_F(VamanaTest, TestSampleRandomPoints)
{
    vector<int> pointIDs = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int numSamples = 5;

    auto sampledPoints = vamana->SampleRandomPoints(pointIDs, numSamples);

    // Ensure the number of samples matches the requested size
    EXPECT_EQ((int)sampledPoints.size(), numSamples);

    // Ensure all sampled points are within the original list
    for (int id : sampledPoints)
    {
        EXPECT_NE(find(pointIDs.begin(), pointIDs.end(), id), pointIDs.end());
    }
}

TEST_F(VamanaTest, TestCreateFilterMap)
{
    vector<Point<float>> data;
    for (int i = 0; i < dataSize; ++i)
    {
        float filter = static_cast<float>(i % 3); // Assign filter values cyclically: 0, 1, 2
        data.emplace_back(Point<float>({i % 5 + 1.0f, i % 7 + 2.0f}, filter, i));
    }

    auto filterMap = vamana->CreateFilterMap(data);

    // Ensure all filter keys are present
    for (float filterValue = 0; filterValue < 3; ++filterValue)
    {
        EXPECT_NE(filterMap.find(filterValue), filterMap.end());
    }

    // Ensure points are correctly mapped to their filters
    for (const auto &point : data)
    {
        auto filter = point.GetFilter();
        EXPECT_NE(find(filterMap[filter].begin(), filterMap[filter].end(), point.GetIndex()), filterMap[filter].end());
    }
}

TEST_F(VamanaTest, TestFindFilterMedoids)
{
    unordered_map<float, vector<int>> filterMap = {
        {0.0f, {0, 1, 2}},
        {1.0f, {3, 4, 5}},
        {2.0f, {6, 7, 8}}};

    int threshold = 0; // Choose medoids for each filter group

    auto filterMedoids = vamana->FindFilterMedoids(filterMap, threshold);

    // Ensure one medoid is chosen per filter
    EXPECT_EQ((int)filterMedoids.size(), (int)filterMap.size());

    // Ensure chosen medoids belong to their respective filter groups
    for (const auto &filterPair : filterMedoids)
    {
        auto filterValue = filterPair.first;
        auto medoid = filterPair.second;

        EXPECT_NE(find(filterMap[filterValue].begin(), filterMap[filterValue].end(), medoid), filterMap[filterValue].end());
    }
}

TEST_F(VamanaTest, TestPerformSearch)
{
    vector<Point<float>> data;
    data.reserve(dataSize);

    // Generate points with filter values
    for (int i = 0; i < dataSize; ++i)
    {
        float filter = static_cast<float>(i % 3); // Assign filter values cyclically: 0, 1, 2
        data.emplace_back(Point<float>({i % 5 + 1.0f, i % 7 + 2.0f}, filter, i));
    }

    Point<float> query({3.0f, 4.0f});

    // Index the data into Filtered and Stitched graphs
    vamana->FilteredVamanaIndexing(data, false);
    vamana->StitchedVamanaIndexing(data, 5, 10, 15, stitchedLogFile, false);

    // Perform search on both graphs
    auto resultsFiltered = vamana->PerformSearch(vamana->FilteredGraph, data, query, {});
    auto resultsStitched = vamana->PerformSearch(vamana->StitchedGraph, data, query, {});

    // Ensure the results are non-empty
    EXPECT_FALSE(resultsFiltered.empty());
    EXPECT_FALSE(resultsStitched.empty());

    // Verify the results from the Filtered graph are sorted by distance
    float previousDistance = -1.0f;
    for (const auto &point : resultsFiltered)
    {
        float currentDistance = point.DistanceTo(query);

        // Distance should be non-negative
        EXPECT_GE(currentDistance, 0.0);

        // Distances should be in ascending order
        EXPECT_GE(currentDistance, previousDistance);

        previousDistance = currentDistance;
    }

    // Verify the results from the Stitched graph are sorted by distance
    previousDistance = -1.0f;
    for (const auto &point : resultsStitched)
    {
        float currentDistance = point.DistanceTo(query);

        // Distance should be non-negative
        EXPECT_GE(currentDistance, 0.0);

        // Distances should be in ascending order
        EXPECT_GE(currentDistance, previousDistance);

        previousDistance = currentDistance;
    }
}
