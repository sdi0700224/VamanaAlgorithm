#include "Point.h"
#include <gtest/gtest.h>
#include <vector>
#include <cmath>

using namespace std;

// Helper function for floating-point comparison
::testing::AssertionResult AreEqual(float val1, float val2, float epsilon = 1e-5)
{
    if (fabs(val1 - val2) < epsilon)
        return ::testing::AssertionSuccess();
    else
        return ::testing::AssertionFailure() << "Values are not equal: " << val1 << " and " << val2;
}

class PointTest : public ::testing::Test
{
protected:
    vector<float> coords1{1.0, 2.0, 3.0};
    vector<float> coords2{4.0, 5.0, 6.0};
    Point<float> point1;
    Point<float> point2;

    void SetUp() override
    {
        point1 = Point<float>(coords1, 1, 1);
        point2 = Point<float>(coords2, 2, 2);
    }
};

// Test the default constructor
TEST_F(PointTest, DefaultConstructor)
{
    Point<int> default_point;
    EXPECT_EQ((int)default_point.GetCoordinates().size(), 0);
    EXPECT_EQ(default_point.GetIndex(), -1);
}

// Test the constructor with coordinates and index
TEST_F(PointTest, ConstructorWithCoordinatesAndIndex)
{
    EXPECT_EQ(point1.GetCoordinates(), coords1);
    EXPECT_EQ(point1.GetIndex(), 1);
}

// Test the copy constructor
TEST_F(PointTest, CopyConstructor)
{
    Point<float> copied_point(point1);
    EXPECT_EQ(copied_point, point1);
}

// Test the equality operator
TEST_F(PointTest, EqualityOperator)
{
    Point<float> another_point(coords1, 1);
    EXPECT_EQ(point1, another_point);
}

// Test the inequality operator
TEST_F(PointTest, InequalityOperator)
{
    EXPECT_NE(point1, point2);
}

// Test the less than operator
TEST_F(PointTest, LessThanOperator)
{
    EXPECT_LT(point1, point2);
}

// Test the DistanceTo function
TEST_F(PointTest, DistanceTo)
{
    float expected_distance = sqrt(pow(4.0 - 1.0, 2) + pow(5.0 - 2.0, 2) + pow(6.0 - 3.0, 2));
    EXPECT_TRUE(AreEqual(point1.DistanceTo(point2), expected_distance));
}

// Test the SquaredDistanceTo function
TEST_F(PointTest, SquaredDistanceTo)
{
    float expected_squared_distance = pow(4.0 - 1.0, 2) + pow(5.0 - 2.0, 2) + pow(6.0 - 3.0, 2);
    EXPECT_TRUE(AreEqual(point1.SquaredDistanceTo(point2), expected_squared_distance));
}

// Test the GetCoordinates function
TEST_F(PointTest, GetCoordinates)
{
    EXPECT_EQ(point1.GetCoordinates(), coords1);
}

// Test the GetIndex function
TEST_F(PointTest, GetIndex)
{
    EXPECT_EQ(point1.GetIndex(), 1);
}

TEST_F(PointTest, GetFilter)
{
    EXPECT_EQ(point1.GetFilter(), 1);
}

TEST_F(PointTest, Serialize)
{
    ostringstream oss;
    point1.Serialize(oss);
    string serialized_data = oss.str();

    EXPECT_FALSE(serialized_data.empty());
}

TEST_F(PointTest, Deserialize)
{
    // Serialize point1 into binary format
    ostringstream oss;
    point1.Serialize(oss);
    string serialized_data = oss.str();

    // Deserialize from binary format
    istringstream iss(serialized_data);
    Point<float> deserialized_point;
    deserialized_point.Deserialize(iss);

    EXPECT_EQ(deserialized_point.GetIndex(), point1.GetIndex());
    EXPECT_EQ(deserialized_point.GetFilter(), point1.GetFilter());
    EXPECT_EQ(deserialized_point.GetCoordinates(), point1.GetCoordinates());
}

// Test the ostream operator
TEST_F(PointTest, OstreamOperator)
{
    ostringstream oss;
    oss << point1;
    auto expected_output = "Point(Index: 1, Coordinates: [1, 2, 3])";
    EXPECT_EQ(oss.str(), expected_output);
}