import symnmf
import kmeans
import sys
from typing import List, Tuple
from sklearn.metrics import silhouette_score

EPS = 1e-4
ITERATIONS = 300

def H_to_symnmf_label_list(decomp_mat: List[List[float]]) -> List[int]:

    """Convert decomposition matrix H to label list.

    Args:
        decomp_mat (list[list[float]]): Decomposition matrix of shape (n x k),
                                        where each row corresponds to a point and each column to a cluster weight.

    Returns:
        list[int]: Label list where label_list[i] is the index of the cluster
                   with the largest value in row i."""
    
    n = len(decomp_mat)
    k = len(decomp_mat[0])
    label_list = [0] * n # Initiate a list where label_list[i] represents point i

    for i in range(n):
        best = 0
        for j in range(1, k): # Find index of largest value in row i (best cluster for point i)”
            if decomp_mat[i][j] > decomp_mat[i][best]: best = j
        label_list[i] = best
    return label_list

def clusters_to_kmeans_label_list(points: List[Tuple[float, ...]], k: int, iterations: int = 300, EPS: float = 1e-4) -> List[int]:

    """Run K-means clustering and return labels.

    Args:
        points (list[tuple[float, ...]]): Input points, each as a tuple of coordinates.
        k (int): Number of clusters (1 < k < n).
        iterations (int, optional): Maximum number of iterations. Defaults to 300.
        EPS (float, optional): Convergence threshold for centroid movement. Defaults to 1e-4.

    Returns:
        list[int]: Label list where label_list[i] is the cluster index of point i."""

    n = len(points)
    dim = len(points[0])
    
    clusters = kmeans.k_means_clusters(k, iterations, EPS, dim, points)
    label_list = [0] * n # Initiate a list where label_list[i] represents point i

    for i in range(len(clusters)): # Sort points to label list
        for j in range(len(clusters[i])):
            label_list[clusters[i][j][0]] = i
    
    return label_list

def main():
    
    """Program entry point.

    Reads command-line arguments (k, file path), validates input,
    and runs clustering with both SymNMF and K-means.
    Computes silhouette scores for each method and prints the results.

    Raises:
        ValueError: If arguments or clustering results are invalid.
        OSError: If the input file cannot be opened.
        MemoryError: If matrix construction fails."""
    
    try:
        # Step 1: validate args and create points list
        if len(sys.argv) != 3: # Argv num check (argv[1] = k,  argv[2] = file path)
            raise ValueError("An Error Has Occurred")
        with open(sys.argv[2], "r") as f: # Validate and read pooints to List[Tuple[float, ...]]
            points = symnmf.file_validate_and_to_point_list(f)
        k = int(sys.argv[1]) # Validate k
        if (not 1 < k < len(points)):
            raise ValueError("An Error Has Occurred")
        # Step 2: symnmf -> get H and labels
        updated_decomp_mat = symnmf.calculate_final_decomp_mat(points, k)
        symnmf_label_list = H_to_symnmf_label_list(updated_decomp_mat)
        if len(set(symnmf_label_list)) < 2:
            raise ValueError("An Error Has Occurred")
        # Step 3: kmeans -> get labels
        kmeans_label_list = clusters_to_kmeans_label_list(points, k, ITERATIONS, EPS)
        if len(set(kmeans_label_list)) < 2:
            raise ValueError("An Error Has Occurred")      
        # Step 4: compute silhouette scores
        symnmf_score = silhouette_score(points, symnmf_label_list, metric="euclidean")
        kmeans_score = silhouette_score(points, kmeans_label_list, metric="euclidean")
        # Print results (4 decimals)
        print(f"nmf: {symnmf_score:.4f}")
        print(f"kmeans: {kmeans_score:.4f}")
    except (ValueError, OSError, MemoryError) as e:
        print(e)
        sys.exit(1)

if __name__ == "__main__":
    main()
