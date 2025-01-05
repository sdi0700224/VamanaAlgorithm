// Adapted from https://github.com/KevinZeng08/sigmod-2024-contest.git)

#include "Utils.h"
#include "Io.h"

using namespace std;

int main(int argc, char **argv)
{
    if (argc < 4)
    {
        cerr << "Usage: " << argv[0] << " <data_file_path> <query_file_path> <gt_file_path>\n";
        cerr << "Example: " << argv[0] << " contest-data-release-1m.bin contest-queries-release-1m.bin contest-gt-release-1m.bin\n";
        return 1;
    }

    // Get file paths from the arguments
    string data_path = argv[1];
    string query_path = argv[2];
    string gt_path = argv[3];

    uint32_t num_data_dimensions = 102;

    // Read data points
    vector<vector<float>> nodes;
    ReadBin(data_path, num_data_dimensions, nodes);
    cout << "Number of nodes: " << nodes.size() << "\n";

    // Read queries
    uint32_t num_query_dimensions = num_data_dimensions + 2;
    vector<vector<float>> queries;
    ReadBin(query_path, num_query_dimensions, queries);

    // Generate ground truth and save to disk
    vector<vector<uint32_t>> gt;
    GetGroundTruth(nodes, queries, gt);
    SaveGroundTruth(gt, gt_path);

    // Read ground truth
    const int K = 100;
    vector<vector<uint32_t>> knns;
    ReadGroundTruth(gt_path, K, knns);

    // Calculate recall
    float recall = GetKNNRecall(knns, gt);
    cout << "Recall: " << recall << "\n";

    return 0;
}
