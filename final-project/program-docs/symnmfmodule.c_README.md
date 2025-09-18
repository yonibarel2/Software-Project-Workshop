# SymNMF – Python C API module (`symnmfmodule.c`)

This module exposes C implementations to Python: it wraps the core routines (`sym`, `ddg`, `norm`, `decomp`) and returns Python list-of-lists results.  
It parses Python sequences into C matrices, calls the C algorithms from `symnmf.h`, and builds Python objects for the return values.

***Note:*** Before reading this guide, read the main repository `README.md` file.

## Program Structure
**Execution flows through Python wrappers and shared helpers:**

**1. Parse & validate Python arguments**  
Each wrapper (`py_sym`, `py_ddg`, `py_norm`, `py_decomp`) unpacks `args` and validates shapes/types.  
- `py_parse_points()` extracts the points object when applicable.  
- `extract_rowsdim_and_colsdim()` derives `n` and `dim`/`k`.

**2. Convert Python sequences → C matrices (contiguous)**  
`py_points_to_c_points()` allocates `double*` buffers and copies values row-by-row from Python to C (size checks enforced).

**3. Dispatch to core C algorithms (from `symnmf.h`)**  
- `py_sym`: calls `sym(dim, n, points, &sym_mat)`.  
- `py_ddg`: `sym(...)` → `ddg(n, sym_mat, &ddg_mat)`.  
- `py_norm`: `sym(...)` → `ddg(...)` → `norm(n, sym_mat, ddg_mat, &norm_mat)`.  
- `py_decomp`: converts `W (n×n)` and `H0 (n×k)` → `decomp_mat(n, k, W, H0, &H_new)`.

**Note: `py_decomp` back-and-forth routine**: 
Unlike the other wrappers, `py_decomp` is part of an iterative workflow:

1. Python side first calls `py_norm(points)` to obtain the normalized similarity matrix `W`.  
2. That `W` and an initial guess `H0` are passed into `py_decomp(W, H0)`.  
3. Inside C, `decomp_mat()` repeatedly updates `H` until convergence.  
4. The final improved `H` is converted back into a Python list-of-lists and returned.  

This creates a **cycle between Python and C**: Python orchestrates the high-level flow (`norm → decomp`), while C performs the heavy numerical updates.


**4. Build Python return values**  
`build_py_object_mat(n, k, mat, &py_mat)` creates a list-of-lists with floats and returns it to Python.

**5. Robust error handling & memory hygiene**  
On any failure, a Python exception is set, intermediate C buffers are freed, and the function returns `NULL`.  
`(void)self;` silences unused-parameter warnings under strict flags.

**6. Module table & init**  
`matMethods[]` maps Python names to wrappers; `PyInit_symnmf_c_api()` creates the module object.

## Functions Graphic view
***Note:** Python wrappers → shared helpers → core C → Python return.*

```mermaid

%% Better spacing & alignment
%%{init: {'flowchart': { 'nodeSpacing': 60, 'rankSpacing': 100, 'curve': 'linear' }}}%%

flowchart TD

%% --- Softer, darker theme per part ---
classDef part1 fill:#d6e4ff,stroke:#4a78d4,stroke-width:1px,color:#000;
classDef part2 fill:#d9f2e4,stroke:#4a9c68,stroke-width:1px,color:#000;
classDef part3 fill:#ffe4cc,stroke:#cc7a29,stroke-width:1px,color:#000;
classDef part4 fill:#f5d6eb,stroke:#a64ca6,stroke-width:1px,color:#000;
classDef main fill:#f7f7f7,stroke:#999999,stroke-width:2px,color:#000;

    A("py_* wrappers")
    A --> |Parse & validate args| B1
    A --> |Select wrapper| C0
    A --> |Return Python object| D
    A --> |Module registration| E

    subgraph  
        B1("py_parse_points()")
        B1 --> B2("extract_rowsdim_and_colsdim()")
        B2 --> B3("py_points_to_c_points()")
    end

    subgraph  
        C0{"wrapper"}
        C0 --> |py_sym| C1("py_sym()")
        C1 --> C1a("sym()")
        C0 --> |py_ddg| C2("py_ddg()")
        C2 --> C2a("sym()")
        C2 --> C2b("ddg()")
        C0 --> |py_norm| C3("py_norm()")
        C3 --> C3a("sym()")
        C3 --> C3b("ddg()")
        C3 --> C3c("norm()")
        C0 --> |py_decomp| C4("py_decomp()")
        C4 --> C4a("decomp_mat()")
        C4a --> C4b("update_decomp_mat()")
    end

    subgraph  
        D("build_py_object_mat()")
    end

    subgraph  
        E("matMethods[]")
        E --> E1("PyInit_symnmf_c_api()")
    end

    %% --- Apply classes ---
    class A main
    class B1,B2,B3 part1
    class C0,C1,C1a,C2,C2a,C2b,C3,C3a,C3b,C3c,C4,C4a,C4b part2
    class D part3
    class E,E1 part4
```
## Functions API

| <div align="center">Function Name</div> | Description | Args | Returns | Errors / Notes |
|---|---|---|---|---|
| <div align="center" style="background-color:#d9f2e4;">py_sym</div> | Python wrapper: build S = sym(points) and return as list-of-lists. | `self: PyObject*` • `args: PyObject* (tuple: points)` | `PyObject*` (n×n list of lists) on success; `NULL` on error | Sets exception on error; `(void)self` to silence unused. |
| <div align="center" style="background-color:#d9f2e4;">py_ddg</div> | Python wrapper: compute D = ddg(sym(points)) and return as list-of-lists. | `self: PyObject*` • `args: PyObject* (tuple: points)` | `PyObject*` (n×n list of lists) on success; `NULL` on error | Sets exception; frees intermediates. |
| <div align="center" style="background-color:#d9f2e4;">py_norm</div> | Python wrapper: compute N = norm(sym(points), ddg(sym(points))) and return as list-of-lists. | `self: PyObject*` • `args: PyObject* (tuple: points)` | `PyObject*` (n×n list of lists) on success; `NULL` on error | Sets exception; frees intermediates. |
| <div align="center" style="background-color:#d9f2e4;">py_decomp</div> | Python wrapper: update decomposition H from norm matrix and initial H0. | `self: PyObject*` • `args: PyObject* (tuple: norm_obj, decomp_obj)` | `PyObject*` (n×k list of lists) on success; `NULL` on error | Sets exception; validates dims; frees intermediates. |
| <div align="center" style="background-color:#d6e4ff;">py_parse_points</div> | Parse Python args into points object. | `args: PyObject*` • `out_points_obj: PyObject**` | `int`: `1` on success, `-1` on parse error | Sets Python exception on error. |
| <div align="center" style="background-color:#d6e4ff;">extract_rowsdim_and_colsdim</div> | Extract matrix dimensions from Python sequence of sequences (list/tuple). | `mat_obj: PyObject*` • `rows_dim: int*` • `cols_dim: int*` | `int`: `1` on success, `-1` on error | DECREFs temp row; sets exception on error. |
| <div align="center" style="background-color:#d6e4ff;">py_points_to_c_points</div> | Convert Python seq-of-seqs to C heap matrix \[n][dim_or_k]. | `mat_obj: PyObject*` • `n: int` • `dim_or_k: int` • `out_mat: double**` | `int`: `1` on success, `-1` on error | Allocates `double*`; caller frees `*out_mat`; sets exception. |
| <div align="center" style="background-color:#ffe4cc;">build_py_object_mat</div> | Build a Python list-of-lists from C matrix \[n][k]. | `n: int` • `k: int` • `mat: double*` • `py_mat_out: PyObject**` | `int`: `1` on success, `-1` on error | Allocates Python lists/floats; caller DECREFs `*py_mat_out`. |
| <div align="center" style="background-color:#f5d6eb;">matMethods</div> | Method table for symnmf_c_api. Maps Python names (py_sym, py_ddg, py_norm, py_decomp) to their C wrapper functions. | – | – | Array terminates with `{NULL, NULL, 0, NULL}`. |
| <div align="center" style="background-color:#f5d6eb;">PyInit_symnmf_c_api</div> | Module initialization for symnmf_c_api. | – | `PyObject*` module on success; `NULL` on failure | Creates module from `symnmfmodule`. |

## Usage

This module is **not executed directly**.  
It is compiled into a Python extension using the provided `setup.py`:

```bash
python3 setup.py build_ext --inplace
```

Running this command is necessary to build the extension and enable using the C library from Python.


