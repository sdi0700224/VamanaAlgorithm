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
    Point<float> point1, point2, point3, point4, point5, point6, point7, point8, point9, queryPoint;

    void SetUp() override
    {
        // Initialize points with coordinates and indexes
        point1 = Point<float>({1.0, 2.0}, 1);
        point2 = Point<float>({2.0, 3.0}, 2);
        point3 = Point<float>({5.0, 5.0}, 3);
        point4 = Point<float>({7.0, 8.0}, 4);
        point5 = Point<float>({10.0, 10.0}, 5);
        point6 = Point<float>({12.0, 15.0}, 6);
        point7 = Point<float>({14.0, 18.0}, 7);
        point8 = Point<float>({16.0, 20.0}, 8);
        point9 = Point<float>({20.0, 25.0}, 9);
        queryPoint = Point<float>({1.5, 2.5}, -1);

        // Add points to the graph
        graph.AddPoint(point1);
        graph.AddPoint(point2);
        graph.AddPoint(point3);
        graph.AddPoint(point4);
        graph.AddPoint(point5);
        graph.AddPoint(point6);
        graph.AddPoint(point7);
        graph.AddPoint(point8);
        graph.AddPoint(point9);

        // Add edges to create a larger connected graph
        graph.AddEdge(point1, point2);
        graph.AddEdge(point2, point3);
        graph.AddEdge(point3, point4);
        graph.AddEdge(point4, point5);
        graph.AddEdge(point5, point6);
        graph.AddEdge(point6, point7);
        graph.AddEdge(point7, point8);
        graph.AddEdge(point8, point9);
    }
};

// Test finding approximate neighbors with sufficient candidates
TEST_F(GreedySearchTest, FindApproximateNeighbors_SufficientCandidates)
{
    int numResults = 3;
    int maxCandidates = 4;

    auto result = search.FindApproximateNeighbors(graph, point1, queryPoint, numResults, maxCandidates);

    const auto &neighbors = result.first;
    const auto &visited = result.second;

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

    cerr << "Visited nodes: ";
    for (const auto &visit : visited)
    {
        cerr << "Point(Index: " << visit.GetIndex() << ", Coordinates: ";
        for (float coord : visit.GetCoordinates())
        {
            cerr << coord << " ";
        }
        cerr << ") ";
    }
    cerr << endl;

    ASSERT_EQ((int)neighbors.size(), numResults);
    EXPECT_EQ(neighbors[0], point1);
    EXPECT_EQ(neighbors[1], point2);
    EXPECT_EQ(neighbors[2], point3);
}

// Test with a larger graph
TEST_F(GreedySearchTest, FindApproximateNeighbors_LargerGraph)
{
    int numResults = 5;
    int maxCandidates = 10;

    auto result = search.FindApproximateNeighbors(graph, point1, queryPoint, numResults, maxCandidates);

    const auto &neighbors = result.first;
    const auto &visited = result.second;

    cerr << "Neighbors found (Larger Graph): ";
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

    cerr << "Visited nodes (Larger Graph): ";
    for (const auto &visit : visited)
    {
        cerr << "Point(Index: " << visit.GetIndex() << ", Coordinates: ";
        for (float coord : visit.GetCoordinates())
        {
            cerr << coord << " ";
        }
        cerr << ") ";
    }
    cerr << endl;

    // Verify the neighbors list matches the requested number of results
    ASSERT_EQ((int)neighbors.size(), numResults);
}

// Test with more expected results
TEST_F(GreedySearchTest, FindApproximateNeighbors_ExpectedOutput_LargerGraph)
{
    int numResults = 5;
    int maxCandidates = 10;

    auto result = search.FindApproximateNeighbors(graph, point1, queryPoint, numResults, maxCandidates);

    const auto &neighbors = result.first;
    const auto &visited = result.second;

    // Verify the neighbors match expected points
    ASSERT_EQ((int)neighbors.size(), numResults);
    EXPECT_EQ(neighbors[0].GetIndex(), 1);
    EXPECT_EQ(neighbors[0].GetCoordinates(), vector<float>({1, 2}));

    EXPECT_EQ(neighbors[1].GetIndex(), 2);
    EXPECT_EQ(neighbors[1].GetCoordinates(), vector<float>({2, 3}));

    EXPECT_EQ(neighbors[2].GetIndex(), 3);
    EXPECT_EQ(neighbors[2].GetCoordinates(), vector<float>({5, 5}));

    EXPECT_EQ(neighbors[3].GetIndex(), 4);
    EXPECT_EQ(neighbors[3].GetCoordinates(), vector<float>({7, 8}));

    EXPECT_EQ(neighbors[4].GetIndex(), 5);
    EXPECT_EQ(neighbors[4].GetCoordinates(), vector<float>({10, 10}));

    // Verify the visited nodes match expected points
    EXPECT_EQ(visited[0].GetIndex(), 1);
    EXPECT_EQ(visited[1].GetIndex(), 2);
    EXPECT_EQ(visited[2].GetIndex(), 3);
    EXPECT_EQ(visited[3].GetIndex(), 4);
    EXPECT_EQ(visited[4].GetIndex(), 5);
    EXPECT_EQ(visited[5].GetIndex(), 6);
    EXPECT_EQ(visited[6].GetIndex(), 7);
    EXPECT_EQ(visited[7].GetIndex(), 8);
}
