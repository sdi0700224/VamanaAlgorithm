#include "gtest/gtest.h"
#include "RobustPruner.h"
#include "Graph.h"
#include "Point.h"
#include <vector>
#include <algorithm>
#include <unordered_map>

using namespace std;

template <typename T>
class RobustPrunerTest : public ::testing::Test
{
protected:
    Graph<T> graph;
    Point<T> p;
    RobustPruner<T> pruner;

    void SetUp() override
    {
        p = Point<T>({0.0, 0.0});
        graph.AddPoint(p);
    }

    void TearDown() override
    {
        graph = Graph<T>(); // Reset the graph
    }

    // Helper function to add candidate neighbors
    void AddCandidateNeighbors(const vector<vector<T>> &coordsList, vector<Point<T>> &candidates)
    {
        for (const auto &coords : coordsList)
        {
            Point<T> neighbor(coords);
            graph.AddPoint(neighbor);
            candidates.push_back(neighbor);
        }
    }
};

typedef RobustPrunerTest<double> RobustPruneDoubleTest;

TEST_F(RobustPruneDoubleTest, PruneReducesToDegreeBound)
{
    vector<Point<double>> candidateNeighbors;
    AddCandidateNeighbors({{1.0, 0.0},
                           {0.0, 1.0},
                           {1.0, 1.0},
                           {-1.0, 0.0},
                           {0.0, -1.0}},
                          candidateNeighbors);

    double distanceThreshold = 1.5;
    int degreeBound = 3;

    pruner.RobustPrune(graph, p, candidateNeighbors, distanceThreshold, degreeBound);

    auto neighbors = graph.GetNeighbors(p);
    EXPECT_EQ((int)neighbors.size(), degreeBound) << "Failed at PruneReducesToDegreeBound: Expected " << degreeBound << " neighbors, but got " << neighbors.size();
}

TEST_F(RobustPruneDoubleTest, PruneWithDegreeBoundGreaterThanCandidates)
{
    vector<Point<double>> candidateNeighbors;
    AddCandidateNeighbors({{1.0, 0.0},
                           {0.0, 1.0}},
                          candidateNeighbors);

    double distanceThreshold = 1.5;
    int degreeBound = 5;

    pruner.RobustPrune(graph, p, candidateNeighbors, distanceThreshold, degreeBound);

    auto neighbors = graph.GetNeighbors(p);
    EXPECT_EQ(neighbors.size(), candidateNeighbors.size()) << "Failed at PruneWithDegreeBoundGreaterThanCandidates: Expected " << candidateNeighbors.size() << " neighbors, but got " << neighbors.size();

    for (const auto &neighbor : neighbors)
    {
        EXPECT_NE(find(candidateNeighbors.begin(), candidateNeighbors.end(), neighbor), candidateNeighbors.end())
            << "Failed at PruneWithDegreeBoundGreaterThanCandidates: Unexpected neighbor " << neighbor;
    }
}

TEST_F(RobustPruneDoubleTest, PruneWithInitialNeighbors)
{
    Point<double> existingNeighbor({-1.0, -1.0});
    graph.AddPoint(existingNeighbor);
    graph.SetNeighbors(p, {existingNeighbor});

    vector<Point<double>> candidateNeighbors;
    AddCandidateNeighbors({{1.0, 0.0},
                           {0.0, 1.0}},
                          candidateNeighbors);

    double distanceThreshold = 1.5;
    int degreeBound = 3;

    pruner.RobustPrune(graph, p, candidateNeighbors, distanceThreshold, degreeBound);

    auto neighbors = graph.GetNeighbors(p);
    EXPECT_EQ((int)neighbors.size(), 3) << "Failed at PruneWithInitialNeighbors: Expected 3 neighbors, but got " << neighbors.size();

    vector<Point<double>> expectedNeighbors = {
        existingNeighbor,
        Point<double>({1.0, 0.0}),
        Point<double>({0.0, 1.0})};

    for (const auto &neighbor : neighbors)
    {
        EXPECT_NE(find(expectedNeighbors.begin(), expectedNeighbors.end(), neighbor), expectedNeighbors.end())
            << "Failed at PruneWithInitialNeighbors: Unexpected neighbor " << neighbor;
    }
}

TEST_F(RobustPruneDoubleTest, PruneWithNoCandidates)
{
    vector<Point<double>> candidateNeighbors;
    double distanceThreshold = 1.5;
    int degreeBound = 3;

    pruner.RobustPrune(graph, p, candidateNeighbors, distanceThreshold, degreeBound);

    auto neighbors = graph.GetNeighbors(p);
    EXPECT_TRUE(neighbors.empty()) << "Failed at PruneWithNoCandidates: Expected empty vector when there are no candidate neighbors";
}
