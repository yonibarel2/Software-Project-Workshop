# Project Descriptions

This project implements the **Symmetric Non-negative Matrix Factorization (SymNMF)** algorithm and supports three modes:

1. Run SymNMF directly in **C** (matrix goals or full factorization), compute and print (determined by users input):
- `sym` – The similarity matrix.  
- `ddg` – The diagonal degree matrix.  
- `norm` – The normalized similarity matrix.  

2. Run SymNMF in **Python** via the C API , compute and print (determined by users input):
- `sym` – The similarity matrix.  
- `ddg` – The diagonal degree matrix.  
- `norm` – The normalized similarity matrix.  
- `symnmf` – Run the SymNMF optimization algorithm and print the decomposed matrix.

3. Run SymNMF in **Python** via the C API, **pure Python K-means**, compare and print silhouette scores of both.

For further details, see `kmeans_intro.md`, `symnmf_intro.md`., `final_project_instructions.pdf`.

**Notes**:

1. If invalid input, memory issues, or file errors occur, the program prints: `An Error Has Occurred` and terminates.  
2. Matrices are represented as lists of rows, with comma-separated values.  
3. Both the SymNMF updates and the K-means algorithm run until convergence (ε = 1e-4) or until reaching the maximum of 300 iterations.
4. All results printed with 4 decimal places.

# Repository contents

**📦 project-root**  
├── **📄 README.md**  
├── **📄 kmeans_intro.md**: Explanation of the K-means clustering algorithm.    
├── **📄 symnmf_intro.md**: Explanation of the SymNMF algorithm.    
├── **📄 final_project_instructions.pdf**: The original project task description.  
├── **📂 final-project-code**: This folder holds all runnable source code files and a README with build and usage instructions.  
│&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;├── **📄 README.md**  
│&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;├── **📄 symnmf.c**: C implementation of SymNMF and its goals
│&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;├── **📄 symnmf.h**: C implementation of SymNMF header.    
│&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;├── **📄 symnmfmodule.c**: Python C API wrapper for calling the C code from Python.  
│&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;├── **📄 symnmf.py**: Python interface for running SymNMF (via the API).  
│&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;├── **📄 kmeans.py**: Pure Python implementation of K-means.  
│&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;├── **📄 analysis.py**: Script that runs both K-means and SymNMF, comparing results (e.g. silhouette score).  
│&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;├── **⚙️ setup.py**: Builds the Python C extension.  
│&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;└── **📄 Makefile**: Builds the standalone C program (`symnmf`).  
├── **📂 program-docs**  
│&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;├── **📄 README.md**  
│&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;├── **📄 analysis_py_doc.md**: Documentation for `analysis.py`.  
│&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;├── **📄 kmeans_py_doc.md**: Documentation for `kmeans.py`.  
│&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;├── **📄 symnmf_py_doc.md**: Documentation for `symnmf.py`.  
│&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;├── **📄 symnmf_c_doc.md**: Documentation for `symnmf.c`.  
│&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;├── **📄 symnmfmodule_c_doc.md**: Documentation for `symnmfmodule.c`.  
│&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;└── **📄 setup_py_Makefile_symnmf_h.md**: Documentation for `setup.py`, `Makefile`, and `symnmf.h`.  
└──────
# How To Use

1. To understand the project, start with the high-level documents in the root folder:

- 📄 **final_project_instructions.pdf** — the original project task description.  
- 📄 **kmeans_intro.md** — explanation of the K-means algorithm.  
- 📄 **symnmf_intro.md** — explanation of the SymNMF algorithm.  

These give the background and theory.  

2. For actual code usage, see the `final-project-code/` folder and its README.

4. For detailed module documentation, see the `program-docs/` folder and its README.



