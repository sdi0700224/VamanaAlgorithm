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

# Function to parse the file content and extract timing
def parse_file_content(filepath):
    with open(filepath, "r") as file:
        content = file.read()
        match = re.search(r"Time \(ms\): (\d+)", content)
        if match:
            return int(match.group(1))
    return None

# Lists to store data for create_f and create_s tasks
create_f_data = []
create_s_data = []

# Process files in the directory
for filename in os.listdir(directory):
    if "create_f" in filename or "create_s" in filename:
        filepath = os.path.join(directory, filename)
        params = parse_filename(filename)
        if params:
            time = parse_file_content(filepath)
            if time is not None:
                params["time"] = time
                if "create_f" in filename:
                    create_f_data.append(params)
                elif "create_s" in filename:
                    create_s_data.append(params)

# Function to create and save plots
def plot_data(data, title, output_file):
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

# Plot and save results
if create_f_data:
    plot_data(create_f_data, "Timing for create_f Tasks", "create_f_timing.png")

if create_s_data:
    plot_data(create_s_data, "Timing for create_s Tasks", "create_s_timing.png")

print("Plots have been saved: create_f_timing.png and create_s_timing.png")
