#include <gtest/gtest.h>
#include "Vamana.h"
#include "Point.h"
#include <vector>

using namespace std;

class VamanaTest : public ::testing::Test
{
protected:
    int k = 5;    // Number of neighbors to search for
    int l = 10;   // List size for the search process
    int r = 7;    // Degree bound
    double a = 1; // Distance factor for pruning
    int dataSize = 100;
    Vamana<float> *vamana;

    void SetUp() override
    {
        vamana = new Vamana<float>(k, l, r, a);
    }

    void TearDown() override
    {
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
    data.reserve(100);

    // Generate 100 points with incremental coordinates and IDs
    for (int i = 0; i < dataSize; ++i)
    {
        float x = static_cast<float>(i % 10 + 1);
        float y = static_cast<float>((i / 10) % 10 + 1);
        float z = static_cast<float>(i % 5 + 1);
        data.emplace_back(Point<float>({x, y, z}, i));
    }

    EXPECT_NO_THROW(vamana->VamanaIndexing(data));
}

TEST_F(VamanaTest, TestSearch)
{
    vector<Point<float>> data;
    data.reserve(100);

    // Generate 100 points with incremental coordinates and IDs
    for (int i = 0; i < dataSize; ++i)
    {
        float x = static_cast<float>(i % 10 + 1);
        float y = static_cast<float>((i / 10) % 10 + 1);
        float z = static_cast<float>(i % 5 + 1);
        data.emplace_back(Point<float>({x, y, z}, i));
    }

    vamana->VamanaIndexing(data);

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
