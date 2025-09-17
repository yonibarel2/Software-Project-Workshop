# Software Project Workshop

## 📝 Overview
This repository contains projects from the **Software Project Workshop**, focused on implementing **machine learning clustering algorithms** in both **C** and **Python**.  
It demonstrates how to combine **C performance** with **Python usability** via the **C-Python API** and **NumPy** integration.

## 📂 Repository Structure
- **kmeans**  
  From-scratch K-Means implementation in **C** and **Python**, with a simple CLI and input validation.  
  👉 [See detailed README](kmeans/README.md)

- **final-project**  
  Full **C implementation of SymNMF**, exposed to Python through a **C-Python API**.  
  A pure Python implementation of **K-Means** is also included for comparison.  
  The `analysis.py` module allows running both algorithms, comparing their **centroids** and evaluating clustering   quality using the **silhouette coefficient**.  
  👉 [See detailed README](final-project/README.md)  

## 🎯 Main Goal
Show how clustering algorithms can be:

- Implemented efficiently in **C**  
- Exposed to **Python** via the **C-Python API**  
- Integrated with **NumPy** for practical **ML workflows**  
- Compared and analyzed to understand the **strengths and trade-offs of different clustering approaches** in various situations

## 🚀 Skills & Technologies
- **Languages**: C, Python  
- **APIs**: C-Python API  
- **Libraries**: NumPy  
- **Tools**: Makefile, setuptools
