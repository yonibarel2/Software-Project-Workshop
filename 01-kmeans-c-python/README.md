# K-means - program descriptions

This program implements the K-means clustering algorithm and outputs the final centroids of the computed clusters.

*Note: For further details, see `kmeans_intro.pdf` and `kmeans_intro.md`.*

## Inputs (passed as command line arguments)
**1. `k` (int):** the number of clusters (requirements: `1 < k < n` ).  
**2. `iterations` - optional (int):** the maximum number of iterations to run the algorithm. if not povided, the default is 400 (requirements: 1 < iterations < 1000).  
**3. `stdin redirection` of a .txt file:** contains the points (requrements: each point on a separate row, formatted as num,num,...,num\n).  

## Output
Prints the list of final centroids of the clusters.

## Notes
1. If a cluster becomes empty, its centroid remains the same as in the previous iteration.  
2. Coordinates are represented as floats with 4 digits after the decimal point.  
3. The implementation assumes 1 < k < n (so k > 1).

## Link to K-means demonstration:
[K-means Simulation](http://alekseynp.com/viz/k-means.html)

## Program Description
This repository contains two separate implementations of the K-means algorithm: one in Python and one in C.  
Each implementation is in a separate folder containing:  
**1. README.md:** Explains the functionality of the program.  
**2. kmeans:** The program code (single file).  

In addition, the folder includes:
**1. kmeans_intro.md:** K-means clusters algorithm explanation.  
**2. kmeans_intro.pdf:** Original task description.  
**2. tests folder** containing input files and corresponding expected output files for specific values of k and iterations (documented in test_readme.txt).

