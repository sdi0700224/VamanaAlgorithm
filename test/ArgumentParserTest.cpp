#include "ArgumentParser.h"
#include <gtest/gtest.h>
#include <stdexcept>

using namespace std;

class ArgumentParserTest : public ::testing::Test
{
protected:
    ArgumentParser parser;

    void SetUp() override
    {
        optind = 1; // Reset getopt state
    }
};

TEST_F(ArgumentParserTest, ParsesArgumentsWithFlags)
{
    char *argv[] = {
        (char *)"program",
        (char *)"--k", (char *)"10",
        (char *)"--l", (char *)"20",
        (char *)"--r", (char *)"5",
        (char *)"--alpha", (char *)"0.5",
        (char *)"--base", (char *)"base_path",
        (char *)"--query", (char *)"query_path",
        (char *)"--groundtruth", (char *)"groundtruth_path",
        nullptr // Ensure null termination
    };
    int argc = 15;

    auto args = parser.ParseArguments(argc, argv);

    EXPECT_EQ(args.K, 10);
    EXPECT_EQ(args.L, 20);
    EXPECT_EQ(args.R, 5);
    EXPECT_DOUBLE_EQ(args.Alpha, 0.5);
    EXPECT_EQ(args.BaseDatasetPath, "base_path");
    EXPECT_EQ(args.QueryDatasetPath, "query_path");
    EXPECT_EQ(args.GroundTruthPath, "groundtruth_path");
}

TEST_F(ArgumentParserTest, ParsesArgumentsWithPositionals)
{
    char *argv[] = {
        (char *)"program",
        (char *)"50", (char *)"100",
        (char *)"60", (char *)"1.2",
        (char *)"data/dummy-data.bin",
        (char *)"data/dummy-queries.bin",
        (char *)"data/dummy-gt.bin",
        nullptr // Ensure null termination
    };
    int argc = 8;

    auto args = parser.ParseArguments(argc, argv);

    EXPECT_EQ(args.K, 50);
    EXPECT_EQ(args.L, 100);
    EXPECT_EQ(args.R, 60);
    EXPECT_DOUBLE_EQ(args.Alpha, 1.2);
    EXPECT_EQ(args.BaseDatasetPath, "data/dummy-data.bin");
    EXPECT_EQ(args.QueryDatasetPath, "data/dummy-queries.bin");
    EXPECT_EQ(args.GroundTruthPath, "data/dummy-gt.bin");
}

TEST_F(ArgumentParserTest, ThrowsOnMissingArguments)
{
    char *argv[] = {(char *)"program", (char *)"--k", (char *)"10"};
    int argc = 3;

    EXPECT_THROW(parser.ParseArguments(argc, argv), invalid_argument);
}

TEST_F(ArgumentParserTest, ThrowsOnInvalidArguments)
{
    char *argv[] = {
        (char *)"program",
        (char *)"--k", (char *)"-10", // Negative value
        (char *)"--l", (char *)"20",
        (char *)"--r", (char *)"5",
        (char *)"--alpha", (char *)"0.5",
        (char *)"--base", (char *)"base_path",
        (char *)"--query", (char *)"query_path",
        nullptr // Ensure null termination
    };
    int argc = 13;

    EXPECT_THROW(parser.ParseArguments(argc, argv), invalid_argument);
}

TEST_F(ArgumentParserTest, ParsesArgumentsWithoutGroundTruth)
{
    char *argv[] = {
        (char *)"program",
        (char *)"50", (char *)"100",
        (char *)"60", (char *)"1.2",
        (char *)"data/dummy-data.bin",
        (char *)"data/dummy-queries.bin",
        nullptr // Ensure null termination
    };
    int argc = 7;

    auto args = parser.ParseArguments(argc, argv);

    EXPECT_EQ(args.K, 50);
    EXPECT_EQ(args.L, 100);
    EXPECT_EQ(args.R, 60);
    EXPECT_DOUBLE_EQ(args.Alpha, 1.2);
    EXPECT_EQ(args.BaseDatasetPath, "data/dummy-data.bin");
    EXPECT_EQ(args.QueryDatasetPath, "data/dummy-queries.bin");
    EXPECT_TRUE(args.GroundTruthPath.empty());
}

TEST_F(ArgumentParserTest, ThrowsOnIncorrectNumberOfPositionals)
{
    char *argv[] = {(char *)"program", (char *)"10", (char *)"20", (char *)"5"};
    int argc = 4;

    EXPECT_THROW(parser.ParseArguments(argc, argv), invalid_argument);
}

TEST_F(ArgumentParserTest, ValidateThrowsOnInvalidData)
{
    ParsedArguments args;

    // Invalid K
    args.K = -1;
    args.L = 10;
    args.R = 5;
    args.Alpha = 0.5;
    args.BaseDatasetPath = "valid_path";
    args.QueryDatasetPath = "valid_query";

    EXPECT_THROW(parser.Validate(args), invalid_argument);

    // Invalid L
    args.K = 10;
    args.L = 0; // Non-positive
    EXPECT_THROW(parser.Validate(args), invalid_argument);

    // Invalid Alpha
    args.L = 10;
    args.Alpha = -0.1; // Negative alpha
    EXPECT_THROW(parser.Validate(args), invalid_argument);

    // Empty BaseDatasetPath
    args.Alpha = 0.5;
    args.BaseDatasetPath = "";
    EXPECT_THROW(parser.Validate(args), invalid_argument);

    // Empty QueryDatasetPath
    args.BaseDatasetPath = "valid_path";
    args.QueryDatasetPath = "";
    EXPECT_THROW(parser.Validate(args), invalid_argument);
}