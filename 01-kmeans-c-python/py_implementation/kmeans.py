import sys

def main():
    try:
        # Part 1: validate inputs and create points list.
        dim, points_arr = stdin_validate_and_to_point_list()
        k, iterations = argv_check(len(points_arr))

        #part 2: K-means algorithm
        centroids = k_means(k, iterations, dim, points_arr)

        round_coordinates(centroids)
        print("there were " + str(len(points_arr)) + " points of " +str(dim) + "D.")
        print("there were " + str(k) + " clusters")
        print("the max num of iterations is " + str(iterations))
        print("the final centroids are:", centroids)

    except ValueError as e:
        print(e)
        exit(1)
        
def stdin_validate_and_to_point_list() -> tuple[int, list[tuple[float, ...]]]:
    """Validate stdin input and return (dimension, points).

    Returns:
        tuple[int, tuple[tuple[float]]]: Dimension of the points and a tuple of all points.

    Raises:
        ValueError: If inputs are invalid.

    """
           
    #initialize points arr with first point and save expected dimention of a point
    points_arr: list[tuple[float]] = []
    first_line = first_line_to_point()
    if not first_line: raise ValueError("An Error Has Occurred")
    dim, first_point = first_line
    points_arr.append(first_point)

    # Ensure at least two points (since we expect 1 < k < n)
    second_point = read_and_check_row(dim)
    if not  second_point: raise ValueError("An Error Has Occurred")
    points_arr.append(second_point)

    # Read remaining points
    next_point = read_and_check_row(dim)
    while next_point:
        points_arr.append(next_point)
        next_point = read_and_check_row(dim)

    return (dim, points_arr)

def first_line_to_point() -> tuple[int, tuple[float, ...]] | None:
    """Read the first row from stdin, validate it, determine its dimension, and return both the dimension and the point.
    
    Returns:
        tuple[int, tuple[float]] | None: Dimension of the point and the point as a tuple of floats or None if line empty.
    
    """

    point = read_and_check_row(None)
    if point is None: return None
    
    dim = len(point)
    return (dim, point)

def read_and_check_row(dim: int | None = None) -> tuple[float, ...] | None:
    """Read a row from stdin, validate its length against the expected dimension, and return it as a tuple of floats.
    
    Args:
        dim (int | None): Expected dimension of the point, or None to skip length check.

    Returns:
        tuple[float] | None: Point as a tuple of floats or None if row empty.
    
    Raises:
        ValueError: If inputs are invalid.

    """
    
    point = []
    line = sys.stdin.readline()

    if not line: return None #handle empty line case
    
    #check if the line ends correctly with Enter
    if line.endswith('\r\n'): line = line[:-2]
    elif line.endswith('\n'): line = line[:-1]
    else: raise ValueError("An Error Has Occurred")

    coordinate_str_arr = line.split(',')

    if (dim is not None and len(coordinate_str_arr) != dim): raise ValueError("An Error Has Occurred") # check if the point is in right dimention
    
    for coordinate in coordinate_str_arr: #checks if all coordinates are valid floats and create a point if yes
        if coordinate == "": raise ValueError("An Error Has Occurred")
        for ch in coordinate:
            if ch.isspace(): raise ValueError("An Error Has Occurred")
        try:
            point.append(float(coordinate))
        except ValueError: raise ValueError("An Error Has Occurred")
    return tuple(point)

def argv_check(n: int) -> tuple[int, int]:
    """Check CMD args are valid (as expected in Table 1), excluding stdin dataflow.
    
    Args:
       n (int): Number of points 

    Returns:
        tuple[int, int]: (k, iterations)
    Raises:
        ValueError: If inputs are invalid.

    """
    
    if n <= 1: raise ValueError("An Error Has Occurred")
    if len(sys.argv) == 1: raise ValueError("Incorrect number of clusters!")
    if len(sys.argv) not in (2, 3): raise ValueError("An Error Has Occurred")
    
    try:
        k = int(sys.argv[1])
    except ValueError:
        raise ValueError("An Error Has Occurred")    
    if not (1 < k < n): raise ValueError ("Incorrect number of clusters!")


    if len(sys.argv) == 2: # iterations not provided -> default =400
        return k, 400
    else:
        try:
            iterations = int(sys.argv[2])
        except ValueError:
            raise ValueError("An Error Has Occurred")
        if not 1 < iterations < 1000: raise ValueError("Incorrect maximum iteration!")
        return k, iterations 

def k_means(k: int, iterations: int, dim: int, points_arr: list[tuple[float, ...]]) -> list[list[float]]:
    """Run the K-means algorithm.

    Args:
        k (int): Number of clusters (1 < k < n).
        iterations (int): Maximum iterations (default 400 if not provided by caller).
        dim (int): Point dimensionality.
        points_arr (list[tuple[float, ...], ...]): Points as a list of length-n tuples, each of length `dim`.

    Returns:
        list[list[float]]: The k centroids (each length `dim`), rounded to 4 decimals.

    Raises:
        ValueError: If inputs are invalid.

    """
    
    #step 1: initialise centroids
    centroids = [list(points_arr[i]) for i in range(k)]
 
    #step 2: repeat algorithm untill finish or untill we did iteratoins number
    for i in range(iterations):
        clusters = [[] for _ in range(k)]

        for point in points_arr: # sort points to clusters
            min_idx = min_squared_distance_idx(point, dim, k, centroids)
            clusters[min_idx].append(point)
        
        moved = False # flag to check if any centroid moved this iteration

        for cluster_num in range(k): #recalculate centroids
            new_centroid = calculate_cluster_centroid(clusters[cluster_num], dim, centroids[cluster_num])
            if new_centroid != centroids[cluster_num]: moved = True
            centroids[cluster_num] = new_centroid
        if not moved:
            return centroids # no centroid moved -> finished algorithm
    return centroids

def min_squared_distance_idx(point: tuple[float], dim: int, k: int, centroids: list[list[float]]) -> int:
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

def calculate_cluster_centroid(cluster: list[list[float]], dim: int, last_centroid: list[float]) -> list[float]:
    """Calculate the centroid of a cluster.

    If the cluster is empty, returns a copy of 'last_centroid'.

    Args:
        cluster (list[list[float]]): Points in the cluster.
        dim (int): Dimensionality of the points.
        last_centroid: Previous centroid to keep if 'cluster' is empty.

    Returns:
        list[float]: The centroid coordinates.
    """
    if not cluster: return list(last_centroid[:])
    
    centroid = []
    for coordinate in range(dim):
        total = 0
        for point in cluster: total += point[coordinate]
        centroid.append((1/len(cluster)) * total)
    return centroid

def round_coordinates(centroids: list[list[float]]) -> list[list[float]]:
    """Round each coordinate in each centroid to 4 decimal places.
    
    """
    for centroid in centroids:
        for i in range(len(centroid)): centroid[i] = round(centroid[i], 4)
    return centroids

if __name__ == "__main__":
    main()



