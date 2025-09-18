# setup_py, Makefile and symnmf_h — quick guide

This short guide explains three important project files together: what they do, and how/when you should use them.

---

## Overview

| File | What it does | When you use it |
|---|---|---|
| `setup.py` | Builds the Python **C-extension** so Python code can call the C SymNMF functions. | When you want to use SymNMF directly from Python (`symnmf.py`, `analysis.py`). |
| `Makefile` | Compiles the **standalone C program** `symnmf` (command-line tool for `sym`, `ddg`, `norm`, `symnmf`). | When you want to run the SymNMF code directly from the terminal. |
| `symnmf.h` | The **C header file** with constants and function declarations shared by symnmf.c file to symnmfmodule.c (the Python wrapper). | Always included when compiling the C files. You don’t run this file, it just defines the interface. |

---

## `setup.py`

### What it does
- Tells Python how to build the C code into a Python module called `symnmf_c_api`.
- Includes both the main C file and the Python/C bridge code.

### How to use
Build the module in place:
```bash
python3 setup.py build_ext --inplace
```

### When to use
- Before running Python code that imports the C backend.  
- After making changes to any of the C files.

## `Makefile`

### What it does
- Automates building the symnmf program from symnmf.c.  
*Note: Uses strict C90 flags and links the math library*.

### How to use
Compile:

```bash
make
```

Run the program:  

```bash
./symnmf [sym | ddg | norm] path/to/points.txt
```

Clean up:

```bash
make clean
```

### When to use

- If you want to quickly test the C code from the command line.
- If you need to confirm the C code builds with strict compiler rules.

## `symnmf.h`

### What it does
- Holds constants and function prototypes (the “function menu”) for the Symmnf.c code.
- Makes sure symnmf.c and the Python/C wrapper use the same interface.

### What’s inside
- The relevant functions declarations for the symnmfmodule.c wrapper: `sym`, `ddg`, `norm`, `decomp_mat`.

### When to use
No direct usage - the header is icluded by `<symnmf.h>` on top of symnmfmodule.c code) 


