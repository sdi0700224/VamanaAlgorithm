#pragma once

#include <string>
#include <stdexcept>
#include <getopt.h>
#include <gtest/gtest.h>

using namespace std;

struct ParsedArguments
{
    int K = 0;
    int L = 0;
    int R = 0;
    double Alpha = 0.0;
    string BaseDatasetPath;
    string QueryDatasetPath;
    string GroundTruthPath;
};

class ArgumentParser
{
public:
    static ParsedArguments ParseArguments(int argc, char *argv[]);
    static void DisplayParsedArguments(const ParsedArguments &args);

private:
    static void Validate(const ParsedArguments &args);

    FRIEND_TEST(ArgumentParserTest, ValidateThrowsOnInvalidData);
};
