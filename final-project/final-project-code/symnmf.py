import symnmf_c_api as symnmf
import sys
from typing import IO, List, Tuple
import numpy as np
np.random.seed(1234) # Seed set once, for the whole program

def file_validate_and_to_point_list(f: IO[str]) -> List[Tuple[float, ...]]:
    """Validate file input and return (dimension, points).

    Args:
        f (IO[str]): Opened File in read mode that contains the points

    Returns:
        tuple[int, list[tuple[float]]]: Dimension of the points and a tuple of all points.

    Raises:
        ValueError: If inputs are invalid."""
           
    # Initialize points arr with first point and save expected dimention of a point
    points_arr: list[tuple[float, ...]] = []
    first_line = first_line_to_point(f)
    if not first_line: raise ValueError("An Error Has Occurred")
    dim, first_point = first_line
    points_arr.append(first_point)

    # Ensure at least two points (since we expect 1 < k < n)
    second_point = read_and_check_row(f, dim)
    if not second_point: raise ValueError("An Error Has Occurred")
    points_arr.append(second_point)

    # Read remaining points
    next_point = read_and_check_row(f, dim)
    while next_point:
        points_arr.append(next_point)
        next_point = read_and_check_row(f, dim)

    return points_arr

def first_line_to_point(f: IO[str]) -> Tuple[int, Tuple[float, ...]] | None:
    """Read the first row from file, validate it, determine its dimension, and return both the dimension and the point.
    
    Args:
        f (IO[str]): Opened File in read mode that contains the points
    
    Returns:
        tuple[int, tuple[float]] | None: Dimension of the point and the point as a tuple of floats or None if line empty. """

    point = read_and_check_row(f)
    if point is None: return None
    
    dim = len(point)
    return (dim, point)

def read_and_check_row(f: IO[str], dim: int | None = None) -> Tuple[float, ...] | None:
    """Read a row from file, validate its length against the expected dimension, and return it as a tuple of floats.
    
    Args:
        f (IO[str]): Opened File in read mode that contains the points
        dim (int | None): Expected dimension of the point, or None to skip length check.

    Returns:
        tuple[float] | None: Point as a tuple of floats or None if row empty.
    
    Raises:
        ValueError: If inputs are invalid."""
    
    point = []
    line = f.readline()

    if not line: return None # Handle empty line case
    
    # Check if the line ends correctly with Enter
    if line.endswith('\r\n'): line = line[:-2]
    elif line.endswith('\n'): line = line[:-1]
    else: raise ValueError("An Error Has Occurred")

    coordinate_str_arr = line.split(',')

    if (dim is not None and len(coordinate_str_arr) != dim): raise ValueError("An Error Has Occurred") # Check if the point is in right dimention
    
    for coordinate in coordinate_str_arr: # Checks if all coordinates are valid floats and create a point if yes
        if coordinate == "": raise ValueError("An Error Has Occurred")
        for ch in coordinate:
            if ch.isspace(): raise ValueError("An Error Has Occurred")
        try:
            point.append(float(coordinate))
        except ValueError: raise ValueError("An Error Has Occurred")
    return tuple(point)

def calculate_final_decomp_mat(points: List[Tuple[float]], k: int) -> List[List[float]]:
    """Compute the final decomposition matrix for SymNMF.

    Steps:
    1. Build normalized similarity matrix W from points.
    2. Initialize random non-negative decomposition matrix H0.
    3. Update H0 using the SymNMF decomposition routine.

    Args:
        points (list[list[float]]): Input data points.
        k (int): Number of clusters.

    Returns:
        list[list[float]]: Updated decomposition matrix H."""
    norm_mat = symnmf.py_norm(points) # Calculate W from points and convert to numpy Object
    norm_mat_np = np.array(norm_mat) # np form for np functs usage

    norm_mean = np.mean(norm_mat_np) # Average of all entries in norm
    scale = 2 * np.sqrt(norm_mean / k)
    decomp_mat = np.random.uniform(0, scale, size=(norm_mat_np.shape[0], k)) # Initianize decomp(0) np
    decomp_mat_list_form = decomp_mat.tolist()  # Transfer decomp_mat np to list[list[double]]

    updated_decomp_mat = symnmf.py_decomp(norm_mat, decomp_mat_list_form)
    return updated_decomp_mat

def print_matrix(mat: List[List[float]]) -> None:
    """Print a matrix with 4-decimal formatting.

    Args:
        mat (list[list[float]]): 2D matrix to print.

    Output:
        Prints each row on a new line, values comma-separated, rounded to 4 decimals."""

    for i in range(len(mat)):
        for j in range(len(mat[i])):
            end = "\n" if j == len(mat[i]) - 1 else ","
            print(f"{mat[i][j]:.4f}", end=end)

def main():
    """CLI entry point for SymNMF tasks.

    Usage (argv):
        argv[1] = k (int), with 1 < k < n
        argv[2] = goal: one of {"symnmf", "sym", "ddg", "norm"}
        argv[3] = path to input file

    Behavior:
        - Reads points from file into list[tuple[float, ...]] and infers dim.
        - For goal: compute relevant matrix ()using C-python API) and print it
    
    Output:
        Prints the resulting matrix using print_matrix (comma-separated, 4 decimals).

    Errors:
        On invalid args, file/IO, or memory issues: prints "An Error Has Occurred" and exits(1)."""
    
    try:
        # Step 1: validate args and create points list
        if len(sys.argv) != 4: # Argv num check (argv[1] = k, argv[2] = goal, argv[3] = file path)
            raise ValueError("An Error Has Occurred")
        goal = sys.argv[2] # Validate goal
        if goal not in {"symnmf", "sym", "ddg", "norm"}:
            raise ValueError("An Error Has Occurred")
        with open(sys.argv[3], "r") as f: # Validate and read pooints to list[tuple[float, ...]] and calculate dim of points
            points = file_validate_and_to_point_list(f)
        k = int(sys.argv[1]) # Validate k
        if (goal == "symnmf "and not 1 < k < len(points)):
            raise ValueError("An Error Has Occurred")
        
        # Step 2: run algorithm determined by "goal" - and print result
        if goal == "sym": sym_mat = symnmf.py_sym(points); print_matrix(sym_mat)
        elif goal == "ddg": ddg_mat = symnmf.py_ddg(points); print_matrix(ddg_mat)
        elif goal == "norm": norm_mat = symnmf.py_norm(points); print_matrix(norm_mat)
        else: updated_decomp_mat = calculate_final_decomp_mat(points, k); print_matrix(updated_decomp_mat) #goal = "symnmf"
        
    except (ValueError, OSError, MemoryError) as e: # Make sure all raises exceptions will catch
        print(e); sys.exit(1)

if __name__ == "__main__":
    main()
