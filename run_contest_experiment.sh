#!/bin/bash

# Directories and file paths
DATASET="contest" # Use contest dataset
DATATYPE=""       # Add suffix like "-release-1m" if applicable
BASE_DATASET="data/${DATASET}-data${DATATYPE}.bin"
QUERY_DATASET="data/${DATASET}-queries${DATATYPE}.bin"
GROUND_TRUTH="data/${DATASET}-gt${DATATYPE}.bin"

GRAPH_DIR="indexes"
EXP_DIR="experiments"

# Graph creation parameters
K=30
L=60
R=15
ALPHA=1.1

# Ensure necessary directories exist
mkdir -p "$GRAPH_DIR" "$EXP_DIR"

# Set threads to 8 for OpenMP
export OMP_NUM_THREADS=8

# Compile the program
echo "Compiling the program..."
make clean
make all

# Step 1: Create filtered graph
FILTERED_GRAPH_FILE="${GRAPH_DIR}/filtered_graph_contest_K${K}_R${R}_A${ALPHA}_threads_8.bin"
FILTERED_EXP="${EXP_DIR}/filtered_graph_contest_K${K}_R${R}_A${ALPHA}_threads_8"

echo "Creating Filtered Graph (K=$K, L=$L, R=$R, Alpha=$ALPHA, Threads=8)..."
./bin/ConsoleApp $K $L $R $ALPHA "$BASE_DATASET" "$QUERY_DATASET" "$GROUND_TRUTH" "create-f" "$FILTERED_GRAPH_FILE" "$FILTERED_EXP"

# Step 2: Create stitched graph
STITCHED_GRAPH_FILE="${GRAPH_DIR}/stitched_graph_contest_K${K}_R${R}_A${ALPHA}_threads_8.bin"
STITCHED_EXP="${EXP_DIR}/stitched_graph_contest_K${K}_R${R}_A${ALPHA}_threads_8"

echo "Creating Stitched Graph (K=$K, L=$L, R=$R, Alpha=$ALPHA, Threads=8)..."
./bin/ConsoleApp $K $L $R $ALPHA "$BASE_DATASET" "$QUERY_DATASET" "$GROUND_TRUTH" "create-s" "$STITCHED_GRAPH_FILE" "$STITCHED_EXP"

# Step 3: Run search experiments
SEARCH_EXP_FILTERED="${EXP_DIR}/search_filtered_contest_K${K}_L${L}_threads_8"
SEARCH_EXP_STITCHED="${EXP_DIR}/search_stitched_contest_K${K}_L${L}_threads_8"

echo "Running Search Experiment (Filtered Graph, Threads=8)..."
./bin/ConsoleApp $K $L $R $ALPHA "$BASE_DATASET" "$QUERY_DATASET" "$GROUND_TRUTH" "search" "$FILTERED_GRAPH_FILE" "$SEARCH_EXP_FILTERED"

echo "Running Search Experiment (Stitched Graph, Threads=8)..."
./bin/ConsoleApp $K $L $R $ALPHA "$BASE_DATASET" "$QUERY_DATASET" "$GROUND_TRUTH" "search" "$STITCHED_GRAPH_FILE" "$SEARCH_EXP_STITCHED"

echo "Contest experiment tasks completed successfully!"
