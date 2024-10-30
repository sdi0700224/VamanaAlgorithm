#include "Graph.h"
#include "Point.h"
#include <gtest/gtest.h>
#include <vector>

using namespace std;

class GraphTest : public ::testing::Test
{
protected:
    Graph<int> graph;
    Point<int> point1, point2, point3;

    void SetUp() override
    {
        // Initialize points with example coordinates and indices
        point1 = Point<int>({1, 2}, 1);
        point2 = Point<int>({3, 4}, 2);
        point3 = Point<int>({5, 6}, 3);
    }
};

// Test adding a point to the graph
TEST_F(GraphTest, AddPoint)
{
    graph.AddPoint(point1);
    auto neighbors = graph.GetNeighbors(point1);
    EXPECT_TRUE(neighbors.empty()); // A newly added point should have no neighbors
}

// Test adding directed edges between points in the graph
TEST_F(GraphTest, AddDirectedEdge)
{
    graph.AddPoint(point1);
    graph.AddPoint(point2);
    graph.AddEdge(point1, point2);

    auto neighbors1 = graph.GetNeighbors(point1);
    auto neighbors2 = graph.GetNeighbors(point2);

    ASSERT_EQ(neighbors1.size(), 1);
    EXPECT_EQ(neighbors1[0], point2); // Point1 should have Point2 as a neighbor

    EXPECT_TRUE(neighbors2.empty()); // Point2 should not have Point1 as a neighbor (directed)
}

// Test retrieving neighbors of a point after adding a directed edge
TEST_F(GraphTest, GetNeighbors)
{
    graph.AddPoint(point1);
    graph.AddPoint(point2);
    graph.AddEdge(point1, point2);

    auto neighbors = graph.GetNeighbors(point1);
    ASSERT_EQ(neighbors.size(), 1);
    EXPECT_EQ(neighbors[0], point2);

    auto neighbors2 = graph.GetNeighbors(point2);
    EXPECT_TRUE(neighbors2.empty()); // Point2 should have no neighbors
}

// Test setting neighbors directly for directed edges
TEST_F(GraphTest, SetNeighbors)
{
    vector<Point<int>> neighbors = {point2, point3};
    graph.AddPoint(point1);
    graph.SetNeighbors(point1, neighbors);

    auto result_neighbors = graph.GetNeighbors(point1);
    ASSERT_EQ(result_neighbors.size(), neighbors.size());
    EXPECT_EQ(result_neighbors[0], point2);
    EXPECT_EQ(result_neighbors[1], point3);
}

// Test for a point with no neighbors
TEST_F(GraphTest, NoNeighbors)
{
    graph.AddPoint(point1);
    auto neighbors = graph.GetNeighbors(point1);
    EXPECT_TRUE(neighbors.empty());
}

// Test adding duplicate directed edges only creates one directed link
TEST_F(GraphTest, DuplicateDirectedEdge)
{
    graph.AddPoint(point1);
    graph.AddPoint(point2);
    graph.AddEdge(point1, point2);
    graph.AddEdge(point1, point2); // Attempt to add the same directed edge again

    auto neighbors1 = graph.GetNeighbors(point1);
    EXPECT_EQ(neighbors1.size(), 1); // Ensure only one entry for point2 in point1's neighbors
}

// Test adding multiple directed edges from one point to different points
TEST_F(GraphTest, MultipleDirectedEdgesFromOnePoint)
{
    graph.AddPoint(point1);
    graph.AddPoint(point2);
    graph.AddPoint(point3);
    graph.AddEdge(point1, point2);
    graph.AddEdge(point1, point3);

    auto neighbors1 = graph.GetNeighbors(point1);
    ASSERT_EQ(neighbors1.size(), 2);
    EXPECT_EQ(neighbors1[0], point2);
    EXPECT_EQ(neighbors1[1], point3);

    // Ensure directed edges do not reciprocate
    EXPECT_TRUE(graph.GetNeighbors(point2).empty());
    EXPECT_TRUE(graph.GetNeighbors(point3).empty());
}
