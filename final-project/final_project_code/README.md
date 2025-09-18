# SymNMF – program descriptions

This project implements the **Symmetric Non-negative Matrix Factorization (SymNMF)** algorithm and supports three modes:
1. Run SymNMF directly in **C** (matrix goals or full factorization).
2. Run SymNMF in **Python** via the C API.
3. Run SymNMF in **Python** via the C API, **pure Python K-means** and compare silhouette scores of both.

*Note: For further details, see `kmeans_intro.md`, `symnmf_intro.md`., `final_project_instructions.pdf`.*

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

**1. program folder** The main folder with all source and helper files:

- **README.md:** Explains the functionality of the programs and provides functions API.
- **symnmf.c / symnmf.h** – C implementation of SymNMF and related goals; header for module use.  
- **symnmfmodule.c** – Python C API wrapper for calling the C code from Python.  
- **symnmf.py** – Python interface to run SymNMF (via API).  
- **kmeans.py** – Pure Python implementation of K-means.  
- **analysis.py** – Runs K-means and SymNMF, compares results with silhouette scores.  
- **Makefile** – Builds the C program (`symnmf`).  
- **setup.py** – Builds the Python C extension.

**2. kmeans_intro.md:** K-means clusters algorithm explanation.  
**3. symnmf_intro.md:** K-means clusters algorithm explanation.  
**2. final_project_instructions.pdf:** Original task description.
