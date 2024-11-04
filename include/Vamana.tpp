#include "Vamana.h"
#include <limits>
#include <numeric>
#include <algorithm>
#include <random>
#include <iomanip>

//Vamana constructor
//k : number of neighbors to return  
//L : limit on the number of candidates examined during search
//R : max number of neighbors for each point 
//a : pruning parameter --> if a=1 then pruning is more accurate(strict)
template <typename T>
Vamana<T>::Vamana(int k, int L, int R, double a) : K(k), L(L), R(R), A(a), VamanaGraph(), GreedySearcher(), RobustPruner() {}

template <typename T>
void Vamana<T>::BuildIndex(const vector<Point<T>> &data)
{
    //find a central point(medoid) to act as a starting point
    Medoid = FindMedoid(data); 

    //initialize random permutation of data
    vector<size_t> randomPermutation(data.size());
    //initialize vector with values from 0 to data size -1
    iota(randomPermutation.begin(), randomPermutation.end(), 0);
    //randomize the order of values using a random number generator 
    shuffle(randomPermutation.begin(), randomPermutation.end(), mt19937{random_device{}()});

    //initialize each point in the graph with exactly R unique neighbors
    for (const auto &point : data)
    {
        //crete an unordered set of unique neighbors
        unordered_set<Point<T>> uniqueNeighbors;
        while (uniqueNeighbors.size() < static_cast<size_t>(R))
        {
            //randomly select an index from data
            size_t randomIndex = randomPermutation[rand() % data.size()];
            const auto &candidate = data[randomIndex];

            //ensure the candidate is not the same as the point itself
            if (candidate != point) 
            {
                uniqueNeighbors.insert(candidate);
            }
        }
        //convert unique neighbors into a vector
        vector<Point<T>> randomNeighbors(uniqueNeighbors.begin(), uniqueNeighbors.end());
        VamanaGraph.SetNeighbors(point, randomNeighbors);
    }

    //progress indicator variables
    size_t totalPoints = data.size();
    //calculate the step for updating progress indicator,if total points is 1000
    //the progress will be updated every 10 iterations (10 is 1% of 1000),
    //also ensure that the progress is updated at least once
    size_t progressStep = max(totalPoints / 100, static_cast<size_t>(1));
    size_t currentProgress = 0;

    //for each point
    for (size_t i = 0; i < randomPermutation.size(); ++i)
    {
        //access a point in random order
        const auto &point = data[randomPermutation[i]];

        //find approximate neighbors using Greedy Search
        auto [_, visitedNeighbors] = GreedySearcher.FindApproximateNeighbors(VamanaGraph, Medoid, point, 1, L);

        //add direct edge
        for (const auto &neighbor : visitedNeighbors)
        {
            VamanaGraph.AddEdge(point, neighbor);
        }

        //pruning
        RobustPruner.Prune(VamanaGraph, point, visitedNeighbors, A, R);

        //update visitedNeighbors
        for (const auto &neighbor : visitedNeighbors)
        {
            auto outNeighbors = VamanaGraph.GetNeighbors(neighbor);

            //if the total number of neighbors exceeds R
            if (outNeighbors.size() + 1 > static_cast<size_t>(R))
            {
                //only add if point is not already in outNeighbors
                if (find(outNeighbors.begin(), outNeighbors.end(), point) == outNeighbors.end())
                {
                    outNeighbors.push_back(point); 
                }

                RobustPruner.Prune(VamanaGraph, neighbor, outNeighbors, A, R);
            }
            //if it doesn't, add edge 
            else
            {
                VamanaGraph.AddEdge(neighbor, point);
            }
        }

        //update the progress bar
        //update every progressStep (1%) or 
        //i is at last element,esuring that it finishes with 100% when all points are indexed
        if (i % progressStep == 0 || i == randomPermutation.size() - 1) 
        {
            currentProgress = (i * 100) / totalPoints;
            cout << "\rBuilding index: " << setw(3) << currentProgress << "% complete" << flush;
        }
    }

    cout << "\rBuilding index: 100% complete\n"
         << flush;
}

template <typename T>
Point<T> Vamana<T>::FindMedoid(const vector<Point<T>> &data) const
{
    T minDistSum = numeric_limits<T>::max(); //initialize min sum to max possible value
    Point<T> medoid = data[0]; //initialize medoid to the fisrt point of data

    //for each point
    for (const auto &candidate : data)
    {
        T distSum = 0;
        //compute the distance between candidate and all the other points
        for (const auto &other : data)
        {
            //add distance to sum
            distSum += candidate.DistanceTo(other);
        }
        //if it's smaller than midoid update midoid
        if (distSum < minDistSum)
        {
            minDistSum = distSum;
            medoid = candidate;
        }
    }

    cout << "Medoid point found: " << medoid << endl;

    return medoid;
}

template <typename T>
vector<Point<T>> Vamana<T>::Search(const Point<T> &query, int k) const
{
    //perform the greedy search starting from the medoid
    auto [approxNeighbors, _] = GreedySearcher.FindApproximateNeighbors(VamanaGraph, Medoid, query, k, L);
    return approxNeighbors;
}
