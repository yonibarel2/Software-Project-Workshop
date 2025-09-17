# SymNMF – program descriptions

This project implements the **Symmetric Non-negative Matrix Factorization (SymNMF)** algorithm and supports three modes:
1. Run SymNMF directly in **C** (matrix goals or full factorization).
2. Run SymNMF in **Python** via the C API.
3. Run SymNMF in **Python** via the C API, **pure Python K-means** and compare silhouette scores of both.

*Note: For further details, see `kmeans_intro.md`, `symnmf_intro.md`., `final-project.pdf`.*

## Inputs (passed as command line arguments)

### 1. C program (`symnmf`)
**1. `goal` (string):** one of:  
- `sym` – compute the similarity matrix.  
- `ddg` – compute the diagonal degree matrix.  
- `norm` – compute the normalized similarity matrix.  

**2. `stdin redirection` of a .txt file:** points formatted as `num,num,...,num\n`.  
*(No `k` argument here — factorization uses defaults inside C code.)*

 ### 2. Python (`symnmf.py` / API)
**1. `k` (int):** number of clusters (`1 < k < n`).  
**2. `goal` (string):** same options as above, and in addition:
- `symnmf` – run the SymNMF optimization algorithm.  

**3. `stdin redirection` of a .txt file:** points in the same text format as C.  

### 3. Analysis (`analysis.py`)
**1. `k` (int):** number of clusters (`1 < k < n`).  
**2. `stdin redirection` of a .txt file:** points in the same text format as C. 

## Output  
**C and python symnmf:** `sym`, `ddg`, `norm`: prints corresponding matrix.  
**python symnmf:** in addition, `symnmf`: prints decomposition matrix **H**.  
**python analysis:**: prints two silhouette scores (SymNMF vs K-means).

*Note: All results printed with 4 decimal places*

## Notes

1. If invalid input, memory issues, or file errors occur, the program prints: `An Error Has Occurred` and terminates.  
2. Matrices are represented as lists of rows, with comma-separated values.  
3. Both the SymNMF updates and the K-means algorithm run until convergence (ε = 1e-4) or until reaching the maximum of 300 iterations.

## Link to K-means demonstration:
[K-means Simulation](http://alekseynp.com/viz/k-means.html)

## Program Description
---
# from here to do
to explain that the repo contains a folder with all code files and helper files and then show what files in it - **program folder**: containing all the code 
- **symnmf.c and symnmf.h:** For compiling and running the C and Python modules. header ios for use of the module
- **Makefile:** For compiling and running the C and Python modules.  
- **symnmfmodule.c:** For compiling and running the C and Python modules.
- **symnmf.py:** For compiling and running the C and Python modules.  
- **setup.py:** For compiling and running the C and Python modules.  for connecting to c
- **kmeans.py:** For compiling and running the C and Python modules.  for connecting to c
- **analysis.py:** For compiling and running the C and Python modules.  for connecting to c


This repository contains implementations of the SymNMF algorithm in both Python and C, along with a Python C API wrapper.  
In addition, it includes a Python implementation of K-means, an analysis script for comparing their results, and support files such as `setup.py`, `Makefile`, and test data.

**README.md:** Explains the program structure and what does
**1. kmeans_intro.md:** K-means clusters algorithm explanation.
**1. symnmf_intro.md:** K-means clusters algorithm explanation.
**2. final_project_instructions.pdf:** Original task description.
**2. functions api folder for all code files (containing 4 files).pdf:** Original task description.
**3. tests folder** containing input files and corresponding expected output files for specific values of k and iterations (documented in test_readme.txt).


