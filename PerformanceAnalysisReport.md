# Performance Analysis Report

This report provides an analysis of the performance metrics observed in the graph creation and search tasks, focusing on timing and recall performance across different configurations of parameters (e.g., K, L, R, Alpha) and thread counts.

---

## 1. Graph Creation Performance

### **1.1 Filtered Graph Creation (`create_f` tasks)**

#### Plot:
![Filtered Graph Creation Timing](plots/create_f_timing.png)

#### Analysis:
- **Observation**:
  - **Higher K values**:
    - For K=100 (with L=200, R=60, A=1.2), timing increases significantly across all thread configurations.
    - Using 1 thread results in the highest timing (around 75,000 ms), while 8 threads provide the best performance (around 16,000 ms).
  - **Lower K values**:
    - For K=50 (with L=100, R=15, A=1.1), the timing difference is minimal between 8 threads and 16 threads.
- **Thread Performance**:
  - 8 threads consistently perform better than 16 threads, especially for larger K values.
  - This indicates that the overhead for managing additional threads (e.g., 16) negatively impacts the performance.

#### Implications:
- Filtered graph creation scales poorly with increasing thread count beyond 8 threads for larger configurations.
- For smaller graphs (lower K and R values), thread management overhead is less impactful, leading to negligible timing differences.

---

### **1.2 Stitched Graph Creation (`create_s` tasks)**

#### Plot:
![Stitched Graph Creation Timing](plots/create_s_timing.png)

#### Analysis:
- **Observation**:
  - Stitched graph creation is generally faster than filtered graph creation for similar configurations.
  - **Higher K values**:
    - For K=100 (with L=200, R=60, A=1.2), the timing is lower compared to filtered graphs.
    - 8 threads again outperform 16 threads, but the timing difference is less pronounced.
  - **Lower K values**:
    - Similar to filtered graphs, the timing differences are minimal for lower K values (e.g., K=50).

#### Implications:
- Stitched graphs benefit from reduced computational complexity, making them a time-efficient alternative for graph creation.
- Optimal thread usage (e.g., 8 threads) balances performance and efficiency.

---

## 2. Search Task Performance

### **2.1 Filtered Search Recall**

#### Plot:
![Filtered Search Recall](plots/filtered_search_recall.png)

#### Analysis:
- **Observation**:
  - Recall performance is consistently high, achieving nearly 100% for most configurations.
  - Lower K values (e.g., K=20) achieve competitive recall, indicating robustness of the algorithm.
  - Larger K values (e.g., K=100) ensure perfect recall but come with increased computational cost.

#### Implications:
- Applications demanding maximum recall should prioritize larger K values.
- For efficiency-focused applications, smaller K values may provide an adequate trade-off between timing and recall.

---

### **2.2 Filtered Search Timing**

#### Plot:
![Filtered Search Timing](plots/filtered_search_timing.png)

#### Analysis:
- **Observation**:
  - Timing increases significantly with larger K values.
  - 8 threads consistently outperform 16 threads, especially for higher K values.
  - For smaller configurations, the timing differences between 8 and 16 threads are minimal.

#### Implications:
- Optimal thread usage (e.g., 8 threads) provides the best performance-to-efficiency ratio.
- Larger configurations require careful balancing of thread count to minimize overhead.

---

### **2.3 Stitched Search Recall**

#### Plot:
![Stitched Search Recall](plots/stitched_search_recall.png)

#### Analysis:
- **Observation**:
  - Recall performance for stitched graphs is slightly lower than filtered graphs but remains above 98% for most configurations.
  - Larger K values maintain a recall advantage, while smaller K values (e.g., K=20) occasionally show reduced recall.

#### Implications:
- Stitched graphs are a viable option for applications requiring time efficiency with minimal recall trade-offs.
- Larger K values are recommended for recall-critical applications.

---

### **2.4 Stitched Search Timing**

#### Plot:
![Stitched Search Timing](plots/stitched_search_timing.png)

#### Analysis:
- **Observation**:
  - Stitched search timing is generally lower than filtered search timing for similar configurations.
  - 8 threads provide better performance compared to 16 threads, especially for higher K values.

#### Implications:
- Stitched graphs are ideal for time-sensitive applications, especially when paired with optimal thread configurations (e.g., 8 threads).

---

## 3. General Observations

- **Thread Count Efficiency**:
  - Across all tasks, 8 threads consistently outperform 16 threads due to reduced management overhead.
- **Impact of K and L**:
  - Larger K and L values increase computational costs but enhance recall performance, especially for filtered graphs.
- **Filtered vs. Stitched Graphs**:
  - Filtered graphs offer higher recall but incur greater computational costs.
  - Stitched graphs are more time-efficient, making them suitable for real-time applications.

---

## 4. Recommendations

1. **Thread Configuration**:
   - Use 8 threads as a default configuration for most tasks.
2. **K and L Selection**:
   - Optimize K and L based on application needs:
     - Use smaller K values for faster computations when recall is less critical.
     - Use larger K values for recall-critical applications.
3. **Graph Type**:
   - Prefer stitched graphs for time-sensitive tasks.
   - Use filtered graphs for applications demanding maximum recall.

---

This report summarizes the key findings and provides actionable insights for optimizing graph creation and search tasks based on the experimental results.
