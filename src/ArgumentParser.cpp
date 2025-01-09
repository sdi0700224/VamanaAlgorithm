#include "ArgumentParser.h"
#include <iostream>
#include <cstdlib>

ParsedArguments ArgumentParser::ParseArguments(int argc, char *argv[])
{
    ParsedArguments args;

    // Define long options for flag parsing
    static struct option longOptions[] = {
        {"k", required_argument, nullptr, 'k'},
        {"l", required_argument, nullptr, 'l'},
        {"r", required_argument, nullptr, 'r'},
        {"alpha", required_argument, nullptr, 'a'},
        {"base", required_argument, nullptr, 'b'},
        {"query", required_argument, nullptr, 'q'},
        {"groundtruth", required_argument, nullptr, 'g'},
        {"operation", required_argument, nullptr, 'o'},
        {"index", required_argument, nullptr, 'i'},
        {"search", required_argument, nullptr, 's'},
        {nullptr, 0, nullptr, 0} // Termination of options
    };

    // Parse flags
    bool flagsUsed = false;
    int opt;
    while ((opt = getopt_long(argc, argv, "k:l:r:a:b:q:g:o:i:s", longOptions, nullptr)) != -1)
    {
        flagsUsed = true; // At least one flag detected
        switch (opt)
        {
        case 'k':
            args.K = stoi(optarg);
            break;
        case 'l':
            args.L = stoi(optarg);
            break;
        case 'r':
            args.R = stoi(optarg);
            break;
        case 'a':
            args.Alpha = stod(optarg);
            break;
        case 'b':
            args.BaseDatasetPath = optarg;
            break;
        case 'q':
            args.QueryDatasetPath = optarg;
            break;
        case 'g':
            args.GroundTruthPath = optarg;
            break;
        case 'o':
            args.Operation = optarg;
            break;
        case 'i':
            args.IndexPath = optarg;
            break;
        case 's':
            args.Experiment = optarg;
            break;
        default:
            throw invalid_argument("Invalid flag or missing argument.");
        }
    }

    // If no flags were used, fallback to positional arguments
    if (!flagsUsed)
    {
        int expectedArgs = 10; // Minimum required args for positional parsing
        if (argc < expectedArgs || argc > expectedArgs + 1)
        {
            throw invalid_argument(
                "Incorrect number of positional arguments.\n"
                "Usage: <k> <l> <r> <alpha> <base_dataset> <query_dataset> <groundtruth_dataset> <operation_mode> <index> [search]");
        }

        // Parse positional arguments
        args.K = stoi(argv[optind++]);
        args.L = stoi(argv[optind++]);
        args.R = stoi(argv[optind++]);
        args.Alpha = stod(argv[optind++]);
        args.BaseDatasetPath = argv[optind++];
        args.QueryDatasetPath = argv[optind++];
        args.GroundTruthPath = argv[optind++];
        args.Operation = argv[optind++];
        args.IndexPath = argv[optind++];
        if (argc == expectedArgs + 1)
        {
            args.Experiment = argv[optind++];
        }
    }

    // Validate the parsed arguments
    Validate(args);

    return args;
}

void ArgumentParser::DisplayParsedArguments(const ParsedArguments &args)
{
    cout << "Parsed Arguments:" << endl;
    cout << " - K: " << args.K << endl;
    cout << " - L: " << args.L << endl;
    cout << " - R: " << args.R << endl;
    cout << " - Alpha: " << args.Alpha << endl;
    cout << " - Base Dataset Path: " << args.BaseDatasetPath << endl;
    cout << " - Query Dataset Path: " << args.QueryDatasetPath << endl;
    cout << " - Ground Truth Path: " << args.GroundTruthPath << endl;
    cout << " - Operation: " << args.Operation << endl;
    cout << " - Index Path: " << args.IndexPath << endl;

    if (!args.Experiment.empty())
        cout << " - Search Name: " << args.Experiment << endl;
}

void ArgumentParser::Validate(const ParsedArguments &args)
{
    if (args.K <= 0)
        throw invalid_argument("K must be a positive integer.");
    if (args.L <= 0)
        throw invalid_argument("L must be a positive integer.");
    if (args.R <= 0)
        throw invalid_argument("R must be a positive integer.");
    if (args.Alpha <= 0.0)
        throw invalid_argument("Alpha must be a positive number.");

    if (args.BaseDatasetPath.empty())
        throw invalid_argument("Base Dataset Path must be provided and non-empty.");
    if (args.QueryDatasetPath.empty())
        throw invalid_argument("Query Dataset Path must be provided and non-empty.");
    if (args.GroundTruthPath.empty())
        throw invalid_argument("Ground Truth Path must be provided and non-empty.");
    if (args.Operation.empty() || (args.Operation != "create-f" && args.Operation != "create-s" && args.Operation != "search"))
        throw invalid_argument("Operation must be provided and be 'create-f', 'create-s' or 'search'.");
    if (args.IndexPath.empty())
        throw invalid_argument("Index Path must be provided and non-empty.");

    if (args.Experiment.empty() && args.Operation == "search")
        throw invalid_argument("Search Name, must be provided if operation is 'search'.");
}
