import os
import re
import matplotlib.pyplot as plt

# Define the directory containing the files
directory = "./experiments"

# Function to parse filenames and extract parameters
def parse_filename(filename):
    match = re.search(r"K(\d+)_L(\d+)_R(\d+)_A([\d.]+)_threads_(\d+)", filename)
    if match:
        return {
            "K": int(match.group(1)),
            "L": int(match.group(2)),
            "R": int(match.group(3)),
            "A": float(match.group(4)),
            "threads": int(match.group(5))
        }
    return None

# Function to parse the file content and extract timing or recall
def parse_file_content(filepath):
    with open(filepath, "r") as file:
        content = file.read()
        time_match = re.search(r"Performing Vamana search: (\d+) ms", content)
        recall_match = re.search(r"Recall@K \(Total\): ([\d.]+)%", content)
        time = int(time_match.group(1)) if time_match else None
        recall = float(recall_match.group(1)) if recall_match else None
        return time, recall

# Lists to store data for filtered and stitched search tasks
filtered_data = []
stitched_data = []

# Process files in the directory
for filename in os.listdir(directory):
    if "search_filtered" in filename or "search_stitched" in filename:
        filepath = os.path.join(directory, filename)
        params = parse_filename(filename)
        if params:
            time, recall = parse_file_content(filepath)
            if time is not None and recall is not None:
                params["time"] = time
                params["recall"] = recall
                if "search_filtered" in filename:
                    filtered_data.append(params)
                elif "search_stitched" in filename:
                    stitched_data.append(params)

# Function to create and save recall plots
def plot_search_recall(data, title, output_file):
    plt.figure(figsize=(10, 6))
    for entry in data:
        plt.scatter(entry["K"], entry["recall"], label=f"L={entry['L']}, R={entry['R']}, A={entry['A']}, Threads={entry['threads']}")
    plt.title(title)
    plt.xlabel("K (Parameter)")
    plt.ylabel("Recall (%)")
    plt.legend()
    plt.grid(True)
    plt.savefig(output_file)
    plt.close()

# Function to create and save timing plots
def plot_search_timing(data, title, output_file):
    plt.figure(figsize=(10, 6))
    for entry in data:
        plt.scatter(entry["K"], entry["time"], label=f"L={entry['L']}, R={entry['R']}, A={entry['A']}, Threads={entry['threads']}")
    plt.title(title)
    plt.xlabel("K (Parameter)")
    plt.ylabel("Time (ms)")
    plt.legend()
    plt.grid(True)
    plt.savefig(output_file)
    plt.close()

# Plot and save recall results
if filtered_data:
    plot_search_recall(filtered_data, "Recall for Filtered Search Tasks", "filtered_search_recall.png")
    plot_search_timing(filtered_data, "Timing for Filtered Search Tasks", "filtered_search_timing.png")

if stitched_data:
    plot_search_recall(stitched_data, "Recall for Stitched Search Tasks", "stitched_search_recall.png")
    plot_search_timing(stitched_data, "Timing for Stitched Search Tasks", "stitched_search_timing.png")

print("Plots have been saved: filtered_search_recall.png, filtered_search_timing.png, stitched_search_recall.png, and stitched_search_timing.png")
