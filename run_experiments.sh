#!/bin/bash

# Directories and file paths
DATASET="dummy" # Change as needed
DATATYPE=""     # Add suffix like "-release-1m" if applicable
BASE_DATASET="data/${DATASET}-data${DATATYPE}.bin"
QUERY_DATASET="data/${DATASET}-queries${DATATYPE}.bin"
GROUND_TRUTH="data/${DATASET}-gt${DATATYPE}.bin"

GRAPH_DIR="indexes"
EXP_DIR="experiments"

# Graph creation parameters
GRAPH_K_VALUES=(50)          # Smaller K value for graphs
GRAPH_L_MULTIPLIER=2         # L is always 2x K
GRAPH_R_VALUES=(15 60)       # Limited R variations
GRAPH_ALPHA_VALUES=(1.1)     # Single Alpha value for smaller graphs

# Final graph creation parameters
FINAL_GRAPH_K=100
FINAL_GRAPH_L=$((FINAL_GRAPH_K * 2))
FINAL_GRAPH_R=60
FINAL_GRAPH_ALPHA=1.2

# Search experiment parameters
SEARCH_FIXED_K=50            # Fixed K for smaller graph experiments
SEARCH_FIXED_L=$((SEARCH_FIXED_K * 2))
SEARCH_K_VARIATIONS=(10 50 100) # K variations for final graphs

# Thread counts for final graph experiments
THREAD_COUNTS=(1 8 16)

# Ensure necessary directories exist
mkdir -p "$GRAPH_DIR" "$EXP_DIR"

# Compile the program
echo "Compiling the program..."
make clean
make all

# Step 1: Create smaller graphs (filtered and stitched)
for R in "${GRAPH_R_VALUES[@]}"; do
    for ALPHA in "${GRAPH_ALPHA_VALUES[@]}"; do
        for K in "${GRAPH_K_VALUES[@]}"; do
            L=$((K * GRAPH_L_MULTIPLIER))

            # Create filtered graph
            FILTERED_GRAPH_FILE="${GRAPH_DIR}/filtered_graph_K${K}_R${R}_A${ALPHA}.bin"
            FILTERED_GRAPH_EXP="${EXP_DIR}/filtered_graph_K${K}_R${R}_A${ALPHA}"
            echo "Creating Filtered Graph (K=${K}, L=${L}, R=${R}, Alpha=${ALPHA})..."
            ./bin/ConsoleApp $K $L $R $ALPHA "$BASE_DATASET" "$QUERY_DATASET" "$GROUND_TRUTH" "create-f" "$FILTERED_GRAPH_FILE" "$FILTERED_GRAPH_EXP"

            # Create stitched graph
            STITCHED_GRAPH_FILE="${GRAPH_DIR}/stitched_graph_K${K}_R${R}_A${ALPHA}.bin"
            STITCHED_GRAPH_EXP="${EXP_DIR}/stitched_graph_K${K}_R${R}_A${ALPHA}"
            echo "Creating Stitched Graph (K=${K}, L=${L}, R=${R}, Alpha=${ALPHA})..."
            ./bin/ConsoleApp $K $L $R $ALPHA "$BASE_DATASET" "$QUERY_DATASET" "$GROUND_TRUTH" "create-s" "$STITCHED_GRAPH_FILE" "$STITCHED_GRAPH_EXP"

            # Run search experiments for smaller graphs
            SEARCH_EXP_FILTERED="${EXP_DIR}/search_filtered_K${SEARCH_FIXED_K}_R${R}_A${ALPHA}"
            SEARCH_EXP_STITCHED="${EXP_DIR}/search_stitched_K${SEARCH_FIXED_K}_R${R}_A${ALPHA}"

            echo "Running Search Experiment (Filtered Graph: K=${SEARCH_FIXED_K})..."
            ./bin/ConsoleApp $SEARCH_FIXED_K $SEARCH_FIXED_L $R $ALPHA "$BASE_DATASET" "$QUERY_DATASET" "$GROUND_TRUTH" "search" "$FILTERED_GRAPH_FILE" "$SEARCH_EXP_FILTERED"

            echo "Running Search Experiment (Stitched Graph: K=${SEARCH_FIXED_K})..."
            ./bin/ConsoleApp $SEARCH_FIXED_K $SEARCH_FIXED_L $R $ALPHA "$BASE_DATASET" "$QUERY_DATASET" "$GROUND_TRUTH" "search" "$STITCHED_GRAPH_FILE" "$SEARCH_EXP_STITCHED"
        done
    done
done

# Step 2: Create final graphs (filtered and stitched) with thread variations
for THREADS in "${THREAD_COUNTS[@]}"; do
    echo "Setting OMP_NUM_THREADS=${THREADS} for final graph creation..."
    export OMP_NUM_THREADS=$THREADS

    # Create filtered graph
    FINAL_FILTERED_GRAPH_FILE="${GRAPH_DIR}/filtered_graph_K${FINAL_GRAPH_K}_R${FINAL_GRAPH_R}_A${FINAL_GRAPH_ALPHA}_threads_${THREADS}.bin"
    FINAL_FILTERED_EXP="${EXP_DIR}/filtered_graph_K${FINAL_GRAPH_K}_R${FINAL_GRAPH_R}_A${FINAL_GRAPH_ALPHA}_threads_${THREADS}"
    echo "Creating Final Filtered Graph (Threads=${THREADS})..."
    ./bin/ConsoleApp $FINAL_GRAPH_K $FINAL_GRAPH_L $FINAL_GRAPH_R $FINAL_GRAPH_ALPHA "$BASE_DATASET" "$QUERY_DATASET" "$GROUND_TRUTH" "create-f" "$FINAL_FILTERED_GRAPH_FILE" "$FINAL_FILTERED_EXP"

    # Create stitched graph
    FINAL_STITCHED_GRAPH_FILE="${GRAPH_DIR}/stitched_graph_K${FINAL_GRAPH_K}_R${FINAL_GRAPH_R}_A${FINAL_GRAPH_ALPHA}_threads_${THREADS}.bin"
    FINAL_STITCHED_EXP="${EXP_DIR}/stitched_graph_K${FINAL_GRAPH_K}_R${FINAL_GRAPH_R}_A${FINAL_GRAPH_ALPHA}_threads_${THREADS}"
    echo "Creating Final Stitched Graph (Threads=${THREADS})..."
    ./bin/ConsoleApp $FINAL_GRAPH_K $FINAL_GRAPH_L $FINAL_GRAPH_R $FINAL_GRAPH_ALPHA "$BASE_DATASET" "$QUERY_DATASET" "$GROUND_TRUTH" "create-s" "$FINAL_STITCHED_GRAPH_FILE" "$FINAL_STITCHED_EXP"
done

# Step 3: Run search experiments with varying K on the final graphs (using thread variations)
for THREADS in "${THREAD_COUNTS[@]}"; do
    echo "Setting OMP_NUM_THREADS=${THREADS} for final graph search experiments..."
    export OMP_NUM_THREADS=$THREADS

    for VARIED_K in "${SEARCH_K_VARIATIONS[@]}"; do
        VARIED_L=$((VARIED_K * 2))

        # Filtered graph experiment
        FINAL_FILTERED_GRAPH_FILE="${GRAPH_DIR}/filtered_graph_K${FINAL_GRAPH_K}_R${FINAL_GRAPH_R}_A${FINAL_GRAPH_ALPHA}_threads_${THREADS}.bin"
        SEARCH_EXP_NAME="${EXP_DIR}/search_filtered_K${VARIED_K}_threads_${THREADS}"
        echo "Running Search Experiment (Filtered Final Graph: K=${VARIED_K}, Threads=${THREADS})..."
        ./bin/ConsoleApp $VARIED_K $VARIED_L $FINAL_GRAPH_R $FINAL_GRAPH_ALPHA "$BASE_DATASET" "$QUERY_DATASET" "$GROUND_TRUTH" "search" "$FINAL_FILTERED_GRAPH_FILE" "$SEARCH_EXP_NAME"

        # Stitched graph experiment
        FINAL_STITCHED_GRAPH_FILE="${GRAPH_DIR}/stitched_graph_K${FINAL_GRAPH_K}_R${FINAL_GRAPH_R}_A${FINAL_GRAPH_ALPHA}_threads_${THREADS}.bin"
        SEARCH_EXP_NAME="${EXP_DIR}/search_stitched_K${VARIED_K}_threads_${THREADS}"
        echo "Running Search Experiment (Stitched Final Graph: K=${VARIED_K}, Threads=${THREADS})..."
        ./bin/ConsoleApp $VARIED_K $VARIED_L $FINAL_GRAPH_R $FINAL_GRAPH_ALPHA "$BASE_DATASET" "$QUERY_DATASET" "$GROUND_TRUTH" "search" "$FINAL_STITCHED_GRAPH_FILE" "$SEARCH_EXP_NAME"
    done
done

echo "All tasks completed successfully!"
