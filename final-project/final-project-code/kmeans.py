from typing import List, Tuple
import math

def k_means_clusters(k: int, iterations: int, EPS: float, dim: int, points: List[Tuple[float, ...]]) -> List[List[Tuple[int ,Tuple[float, ...]]]]:
    """Run the K-means algorithm.

    Args:
        k (int): Number of clusters (1 < k < n).
        iterations (int): Maximum iterations (default 400 if not provided by caller).
        dim (int): Point dimensionality.
        points (list[tuple[float, ...], ...]): Points as a list of length-n tuples, each of length `dim`.

    Returns:
        clusters (List[List[Tuple[int ,Tuple[float, ...]]]]): Clusters as a list of length-k lists, containing points as tuples of (point_idx, tuple of point coordinates)

    Raises:
        ValueError: If inputs are invalid."""
    
    # Step 1: initialize centroids
    centroids = [list(points[i]) for i in range(k)]
 
    # Step 2: Repeat algorithm until convergence or until reaching the max iterations.
    for i in range(iterations):
        clusters = [[] for _ in range(k)]

        for j in range(len(points)): # Sort points to clusters
            min_idx = min_squared_distance_idx(points[j], dim, k, centroids)
            clusters[min_idx].append((j, points[j]))
        
        centroid_small_shift = True # Flag to check if any centroid moved this iteration

        for cluster_num in range(k): # Recalculate centroids
            new_centroid = calculate_cluster_centroid([coords for (_, coords) in clusters[cluster_num]], dim, centroids[cluster_num])
            dist = 0
            for m in range(dim): # Euclidean distance
                dist += (new_centroid[m] - centroids[cluster_num][m]) * (new_centroid[m] - centroids[cluster_num][m])
            shift = math.sqrt(dist)
            if shift > EPS: centroid_small_shift = False

            centroids[cluster_num] = new_centroid # Update centroid
        
        if centroid_small_shift: return clusters # No centroid moved -> finished algorithm
    return clusters

def min_squared_distance_idx(point: Tuple[float, ...], dim: int, k: int, centroids: List[List[float]]) -> int:
    """Return the index of the nearest centroid by squared Euclidean distance.

    Args:
        point (tuple[float]): Coordinates of the point.
        dim (int): Dimensionality of the points.
        k (int): Number of centroids.
        centroids (list[list[float]]): List of centroids.

    Returns:
        int: Index (0-based) of the centroid closest to 'point'.
    
    """
    
    min_dist = float('inf')
    min_dist_idx = 0
    
    for i in range(k):
        dist = 0
        for j in range(dim):
            dist += pow((point[j] - centroids[i][j]), 2)
        if dist < min_dist:
            min_dist = dist
            min_dist_idx = i
    
    return min_dist_idx

def calculate_cluster_centroid(cluster: List[List[float]], dim: int, last_centroid: List[float]) -> List[float]:
    """Calculate the centroid of a cluster.

    If the cluster is empty, returns a copy of 'last_centroid'.

    Args:
        cluster (list[list[float]]): Points in the cluster.
        dim (int): Dimensionality of the points.
        last_centroid: Previous centroid to keep if 'cluster' is empty.

    Returns:
        list[float]: The centroid coordinates.
    """
    if not cluster: return last_centroid[:]   
    centroid = []
    for coordinate in range(dim):
        total = 0
        for point in cluster: total += point[coordinate]
        centroid.append((1/len(cluster)) * total)
    return centroid