# K-means – Python implementation

This module clusters a CSV-sourced set of points into `k` groups using the standard K-means algorithm (L2 distance), with centroids initialized to the first `k` points.  
It also includes small helper functions for nearest-centroid search and centroid recomputation.

**In this project, `kmeans.py` is not the primary algorithm** (which was implemented in depth during HW1).  
Instead, it serves as a supporting module, imported and used by `analysis.py` to compare clustering quality against SymNMF.

***Note:** Before reading this guide, read the main repository `README.md` file.*

## Program Structure
**Execution starts in `k_means_clusters()` and is organized into three main stages:**  

**1. Arguments & input processing:**  
- **Expect:** `k (int)`, `iterations (int)`, `EPS (float)`, `dim (int)`, `points (list[tuple[float,...]])`.
- **Initialization:** The initial `k` centroids are set to the **first `k` points**.

**2. K-means iterative optimization:**  
For up to `iterations` rounds (or untill convergence):
- **Assignment step:** Each point is assigned to its nearest centroid (squared Euclidean distance) using  
  `min_squared_distance_idx(point, dim, k, centroids)`.
- **Update step:** For each cluster, recompute the centroid with  
  `calculate_cluster_centroid(cluster_points, dim, last_centroid)`.  
  If a cluster is empty, its centroid remains the previous one.
- **Convergence check:** Compute the Euclidean shift of each centroid.  
  If **all** centroid shifts are ≤ `EPS`, stop early.

**3. Output:**  
Return `clusters` as a list of length-`k` lists; each inner list contains tuples `(point_index, point_coordinates)`.  
Helper functions are pure and side-effect free.

## Functions Graphic view
***Note:** For best understanding, this diagram mirrors the runtime flow and helper calls.*

```mermaid

%% Better spacing & alignment
%%{init: {'flowchart': { 'nodeSpacing': 60, 'rankSpacing': 100, 'curve': 'linear' }}}%%

flowchart TD

%% --- Softer, darker theme per part ---
classDef part1 fill:#d6e4ff,stroke:#4a78d4,stroke-width:1px,color:#000;
classDef part2 fill:#d9f2e4,stroke:#4a9c68,stroke-width:1px,color:#000;
classDef part3 fill:#ffe4cc,stroke:#cc7a29,stroke-width:1px,color:#000;
classDef part4 fill:#f5d6eb,stroke:#a64ca6,stroke-width:1px,color:#000;
classDef main fill:#f7f7f7,stroke:#999999,stroke-width:2px,color:#000;

    A("k_means_clusters()")
    A --> |Part 1: Centroids initialization| B1
    A --> |Part 2: K-means iterative optimization| C0
    A --> |Part 3: Return clusters| D

    subgraph  
        B1("initialize centroids (first k points)")
    end

    subgraph  
        C0{"iteration t < iterations"}
        C0 --> |assignment| C1("min_squared_distance_idx()")
        C0 --> |update| C2("calculate_cluster_centroid()")
    end

    subgraph  
        D("clusters output (list of k lists of (idx, point))")
    end

    %% --- Apply classes ---
    class A main
    class B1 part1
    class C0,C1,C2 part2
    class D part3
```

## Functions API

| <div align="center">Function Name</div> | Description | Args | Returns | Errors / Notes |
|---|---|---|---|---|
| <div align="center" style="background-color:#d9f2e4;">k_means_clusters</div> | Run K-means: initialize centroids, iteratively assign points and update centroids until all shifts ≤ `EPS` or `iterations` reached. | `k: int` • `iterations: int` • `EPS: float` • `dim: int` • `points: list[tuple[float,...]]` | `list[list[tuple[int, tuple[float,...]]]]`: `k` clusters; each item is `(point_idx, point_tuple)`. | Assumes `1 < k < n`, `iterations ≥ 1`, `dim ≥ 1`. Centroids initialized to first `k` points. Early stop on convergence. |
| <div align="center" style="background-color:#d9f2e4;">min_squared_distance_idx</div> | Return index of the nearest centroid by **squared Euclidean** distance. | `point: tuple[float,...]` • `dim: int` • `k: int` • `centroids: list[list[float]]` | `int`: index (0-based) of closest centroid. | Pure helper; no side effects. Uses a simple loop over `k` centroids. |
| <div align="center" style="background-color:#d9f2e4;">calculate_cluster_centroid</div> | Compute centroid (mean) of points in a cluster. If cluster is empty, return a copy of `last_centroid`. | `cluster: list[list[float]]` • `dim: int` • `last_centroid: list[float]` | `list[float]`: centroid coordinates of length `dim`. | Handles empty clusters gracefully by keeping previous centroid. Pure helper. |

## Usage

This module is **not** a standalone program.  
It is imported and used by `analysis.py` to provide K-means clustering results for comparison with SymNMF.

**How it is used:**

```python
# Example inside analysis.py:

from kmeans import k_means_clusters

clusters = k_means_clusters(k, iterations, EPS, dim, points)
```
