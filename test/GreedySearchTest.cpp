#include "GreedySearch.h"
#include "Graph.h"
#include "Point.h"
#include <gtest/gtest.h>
#include <vector>

using namespace std;

class GreedySearchTest : public ::testing::Test
{
protected:
    Graph<float> graph;
    GreedySearch<float> search;
    Point<float> point1, point2, point3, queryPoint;

    void SetUp() override
    {
        // Initialize points with example coordinates and indices
        point1 = Point<float>({1.0, 2.0}, 1);
        point2 = Point<float>({2.0, 3.0}, 2);
        point3 = Point<float>({5.0, 5.0}, 3);
        queryPoint = Point<float>({1.5, 2.5}, -1);

        graph.AddPoint(point1);
        graph.AddPoint(point2);
        graph.AddPoint(point3);
        graph.AddEdge(point1, point2);
        graph.AddEdge(point2, point3);
    }
};

// Test finding approximate neighbors with sufficient candidates
TEST_F(GreedySearchTest, FindApproximateNeighbors_SufficientCandidates)
{
    int numResults = 2;
    int maxCandidates = 3;

    auto result = search.FindApproximateNeighbors(graph, point1, queryPoint, numResults, maxCandidates);

    const auto &neighbors = result.first;

    cerr << "Neighbors found: ";
    for (const auto &neighbor : neighbors)
    {
        cerr << "Point(Index: " << neighbor.GetIndex() << ", Coordinates: ";
        for (float coord : neighbor.GetCoordinates())
        {
            cerr << coord << " ";
        }
        cerr << ") ";
    }
    cerr << endl;

    ASSERT_EQ(neighbors.size(), numResults);
    EXPECT_EQ(neighbors[0], point1); // point1 itself is considered a valid neighbor
    EXPECT_EQ(neighbors[1], point2);
}

// Test finding approximate neighbors with fewer candidates than results requested
TEST_F(GreedySearchTest, FindApproximateNeighbors_InsufficientCandidates)
{
    int numResults = 3;
    int maxCandidates = 1;

    auto result = search.FindApproximateNeighbors(graph, point1, queryPoint, numResults, maxCandidates);

    const auto &neighbors = result.first;
    const auto &candidates = result.second;

    cerr << "Neighbors found: ";
    for (const auto &neighbor : neighbors)
    {
        cerr << "Point(Index: " << neighbor.GetIndex() << ", Coordinates: ";
        for (float coord : neighbor.GetCoordinates())
        {
            cerr << coord << " ";
        }
        cerr << ") ";
    }
    cerr << endl;

    ASSERT_LE(neighbors.size(), maxCandidates);
    EXPECT_EQ(neighbors[0], point1); // point1 is the closest neighbor with maxCandidates = 1

    ASSERT_EQ(candidates.size(), maxCandidates);
}

// Test finding approximate neighbors when starting from an isolated point
TEST_F(GreedySearchTest, FindApproximateNeighbors_NoNeighbors)
{
    Point<float> isolatedPoint({10.0, 10.0}, 4);
    graph.AddPoint(isolatedPoint);

    int numResults = 2;
    int maxCandidates = 3;

    auto result = search.FindApproximateNeighbors(graph, isolatedPoint, queryPoint, numResults, maxCandidates);

    const auto &neighbors = result.first;
    const auto &visited = result.second;

    cerr << "Neighbors size: " << neighbors.size() << endl;
    cerr << "Candidates size: " << visited.size() << endl;

    // Confirm the isolated point is returned as its own neighbor
    ASSERT_EQ(neighbors.size(), 1); // Only the isolated point itself
    EXPECT_EQ(neighbors[0], isolatedPoint);

    // Confirm the isolated point is also included in visited
    ASSERT_EQ(visited.size(), 1);
    EXPECT_EQ(visited[0], isolatedPoint);
}
