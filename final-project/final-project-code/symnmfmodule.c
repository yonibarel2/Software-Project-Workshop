#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stdlib.h>
#include "symnmf.h"

/* Functions prototype declarations */
static int py_parse_points(PyObject *args, PyObject** out_points_obj);
static int extract_rowsdim_and_colsdim(PyObject *points_obj, int *n, int *dim);
static int py_points_to_c_points(PyObject* mat_obj, int n, int dim_or_k, double **out_mat);
static int build_py_object_mat(int n, int k, double *mat, PyObject **py_mat_out);
static PyObject* py_sym(PyObject *self, PyObject *args);
static PyObject* py_ddg(PyObject *self, PyObject *args);
static PyObject* py_norm(PyObject *self, PyObject *args);
static PyObject* py_decomp(PyObject *self, PyObject *args);

/**
 * @brief Python wrapper: build S = sym(points) and return as list-of-lists.
 * @param self CPython module/self (unused).
 * @param args Python tuple: one object `points` (sequence of n rows, each length dim).
 * @return New PyObject* (n×n list of lists) on success; NULL on error (sets exception).
 */
static PyObject* py_sym(PyObject *self, PyObject *args) {
    /* Step 1: parse args to a c points matrix */
    PyObject *points_obj, *py_mat;
    int n, dim;
    double *points = NULL;
    double *sym_mat = NULL;

    (void)self; /* silence unused parameter under -Wall -Wextra -Werror */

    if (py_parse_points(args, &points_obj) == -1) {return NULL;} /* extract PyObject points */
    if (extract_rowsdim_and_colsdim(points_obj, &n, &dim) == -1) {return NULL;} /* extract n and dim */
    if (py_points_to_c_points(points_obj, n, dim, &points) == -1) {return NULL;} /* transfer py matrix to c matrix */

    /* Step 2: call C function adn create sym matrix */
    if (sym(dim, n, points, &sym_mat) == -1) {free(points); PyErr_SetString(PyExc_MemoryError, "An Error Has Occurred"); return NULL;}
    free(points);
    
    /* Step 3: build python sym matrice (list of lists) and return it */
    if (build_py_object_mat(n, n, sym_mat, &py_mat) == -1) {free(sym_mat); return NULL;}
    free(sym_mat);
    return py_mat;
}

/**
 * @brief Python wrapper: compute D = ddg(sym(points)) and return as list-of-lists.
 * @param self CPython self/module (unused).
 * @param args Python tuple: one object `points` (n×dim sequence).
 * @return New PyObject* (n×n list of lists) on success; NULL on error (sets exception).
 */
static PyObject* py_ddg(PyObject *self, PyObject *args) {

    PyObject *points_obj, *py_mat;
    int n, dim;
    double *points = NULL;
    double *sym_mat = NULL;
    double *ddg_mat = NULL;

    (void)self; /* silence unused parameter under -Wall -Wextra -Werror */

    /* Step 1: parse args to a c points matrix */
    if (py_parse_points(args, &points_obj) == -1) {return NULL;}
    
    if (extract_rowsdim_and_colsdim(points_obj, &n, &dim) == -1) {return NULL;} /* extract n and dim */
    if (py_points_to_c_points(points_obj, n, dim, &points) == -1) {return NULL;} /* transfer py matrix to c matrix */

    /* Step 2: call C function and calculate sym matrix from points */
    if (sym(dim, n,points, &sym_mat) == -1) {free(points); PyErr_SetString(PyExc_MemoryError, "An Error Has Occurred"); return NULL;}
    free(points);
    
    /* Step 3: call C function and calculate ddg matrix from sym matrix */
    if (ddg(n, sym_mat, &ddg_mat) == -1) {free(sym_mat); PyErr_SetString(PyExc_MemoryError, "An Error Has Occurred"); return NULL;}
    free(sym_mat);

    /* Step 4: build python ddg matrice (list of lists) and return it */
    if (build_py_object_mat(n, n, ddg_mat, &py_mat) == -1) {free(ddg_mat); return NULL;}
    free(ddg_mat);
    return py_mat;
}

/**
 * @brief Python wrapper: compute N = norm(sym(points), ddg(sym(points))) and return as list-of-lists.
 * @param self CPython self/module (unused).
 * @param args Python tuple: one object `points` (n×dim sequence).
 * @return New PyObject* (n×n list of lists) on success; NULL on error (sets exception).
 */
static PyObject* py_norm(PyObject *self, PyObject *args) {
    /* Step 1: parse args to a c points matrix */
    PyObject *points_obj, *py_mat;
    int n, dim;
    double *points = NULL;
    double *sym_mat = NULL;
    double *ddg_mat = NULL;
    double *norm_mat = NULL;

    (void)self; /* silence unused parameter under -Wall -Wextra -Werror */
    
    if (py_parse_points(args, &points_obj) == -1) {return NULL;} /* extract PyObject points */
    if (extract_rowsdim_and_colsdim(points_obj, &n, &dim) == -1) {return NULL;} /* extract n and dim */
    if (py_points_to_c_points(points_obj, n, dim, &points) == -1) return NULL; /* transfer py matrix to c matrix */

    /* Step 2: call C function and calculate sym matrix from points */
    if (sym(dim ,n ,points, &sym_mat) == -1) {free(points); PyErr_SetString(PyExc_MemoryError, "An Error Has Occurred"); return NULL;}
    free(points);
    
    /* Step 3: call C function and calculate ddg matrix from sym matrix */
    if (ddg(n, sym_mat, &ddg_mat) == -1) {free(sym_mat); PyErr_SetString(PyExc_MemoryError, "An Error Has Occurred"); return NULL;}

    /*Step 4: call C function and calculate norm matrix from sym and ddg matrix */
    if (norm(n, sym_mat, ddg_mat, &norm_mat) == -1) {free(sym_mat); free(ddg_mat); PyErr_SetString(PyExc_MemoryError, "An Error Has Occurred"); return NULL;}
    free(sym_mat); free(ddg_mat);

    /* Step 5: build python norm matrice (list of lists) and return it */
    if (build_py_object_mat(n, n, norm_mat, &py_mat) == -1) {free(norm_mat); return NULL;}
    free(norm_mat);
    return py_mat;
}

/**
 * @brief Python wrapper: update decomposition H from norm matrix and initial H0.
 * @param self CPython self/module (unused).
 * @param args Python tuple: (norm_obj=n×n, decomp_obj=n×k).
 * @return New PyObject* (n×k list of lists) on success; NULL on error (sets exception).
 */
static PyObject* py_decomp(PyObject *self, PyObject *args) {
    PyObject *norm_obj, *decomp_obj, *py_mat;
    int n, k;
    double *norm_mat = NULL;
    double *decomp_mat_0 = NULL;
    double *updated_decomp_mat = NULL;
        
    (void)self; /* Silence unused parameter under -Wall -Wextra -Werror */

    /* Step 1: extract and validate points PyObject from py args */
    if (!PyArg_ParseTuple(args, "OO", &norm_obj, &decomp_obj)) {PyErr_SetString(PyExc_ValueError, "An Error Has Occurred"); return NULL;} /*get python objects W and H_0*/
    
    /* Extract n and k */
    if (extract_rowsdim_and_colsdim(decomp_obj, &n, &k) == -1) return NULL;

    /* Transfer py norm matrix to c matrix */
    if (py_points_to_c_points(norm_obj, n, n, &norm_mat) == -1) return NULL;
    
    /* Transfer py decomp matrix to C matrix*/
    if (py_points_to_c_points(decomp_obj, n, k, &decomp_mat_0) == -1) {free(norm_mat); return NULL;}

    /* Step 2: call C function and calculate updated_decomp matrix from norm and decomp matrix */
    if (decomp_mat(n, k, norm_mat ,decomp_mat_0 ,&updated_decomp_mat) == -1) {free(norm_mat); free(decomp_mat_0); PyErr_SetString(PyExc_MemoryError, "An Error Has Occurred"); return NULL;}
    free(norm_mat); free(decomp_mat_0);

    /* Step 3: build python updated_decomp matrice (list of lists) and return it */
    if (build_py_object_mat(n, k, updated_decomp_mat, &py_mat) == -1) {free(updated_decomp_mat); return NULL;}
    free(updated_decomp_mat);
    return py_mat;
}

/**
 * @brief Method table for symnmf_c_api.
 * @details Maps Python names (py_sym, py_ddg, py_norm, py_decomp) 
 *          to their C wrapper functions.
 */
static PyMethodDef matMethods[] = {
    {"py_sym", (PyCFunction) py_sym, METH_VARARGS, PyDoc_STR("sym(points) -> list[list[float]]\nCompute similarity matrix from points.")},
    {"py_ddg", (PyCFunction) py_ddg, METH_VARARGS, PyDoc_STR("ddg(points) -> list[list[float]]\nCompute degree diagonal matrix from points.")},
    {"py_norm", (PyCFunction) py_norm, METH_VARARGS, PyDoc_STR("norm(points) -> list[list[float]]\nCompute normalized graph Laplacian from points.")},
    {"py_decomp", (PyCFunction) py_decomp, METH_VARARGS, PyDoc_STR("decomp(norm, H0) -> list[list[float]]\nUpdate decomposition matrix H from N and H0.")},

    {NULL, NULL, 0, NULL} /* end of array */
};

/**
 * @brief Module definition for symnmf_c_api.
 * @details Exposes Python wrappers: py_sym, py_ddg, py_norm, py_decomp.
 */
static struct PyModuleDef symnmfmodule = {
    PyModuleDef_HEAD_INIT,
    "symnmf_c_api",
    NULL,
    -1,
    matMethods 
};

/**
 * @brief Module initialization for symnmf_c_api.
 * @return New PyObject* module on success, NULL on failure.
 */
PyMODINIT_FUNC PyInit_symnmf_c_api(void) {
    PyObject *m;
    m = PyModule_Create(&symnmfmodule);
    if (!m) {
        return NULL;
    }
    return m;
}

/** 
 * @brief Parse Python args into points object. 
 * @param args Input Python tuple (expected: one object). 
 * @param out_points_obj Out: receives PyObject* to points. 
 * @return 1 on success, -1 on parse error (sets Python exception). 
 */
static int py_parse_points(PyObject *args, PyObject** out_points_obj) {
    PyObject *points_obj; 
    if (!PyArg_ParseTuple(args, "O", &points_obj)) {PyErr_SetString(PyExc_ValueError, "An Error Has Occurred"); return -1;} /* Get python object */
    *out_points_obj = points_obj;
    return 1;
}

/**
 * @brief Extract matrix dimensions from Python sequence of sequences (list/tuple).
 * @param mat_obj Input Python object (list or tuple of rows).
 * @param rows_dim Out: number of rows.
 * @param cols_dim Out: number of columns.
 * @return 1 on success, -1 on error (sets Python exception).
 */
static int extract_rowsdim_and_colsdim(PyObject *mat_obj, int *rows_dim, int *cols_dim) {
    PyObject *row0;

    *rows_dim = PyObject_Length(mat_obj);
    if (*rows_dim <= 0) {PyErr_SetString(PyExc_ValueError, "An Error Has Occurred"); return -1;}
    
    row0 = PySequence_GetItem(mat_obj, 0);
    if (!row0) {PyErr_SetString(PyExc_ValueError, "An Error Has Occurred"); return -1;}
    *cols_dim = PyObject_Length(row0);
    Py_DECREF(row0);
    if (*cols_dim <= 0) {PyErr_SetString(PyExc_ValueError, "An Error Has Occurred"); return -1;}

    return 1;
}

/**
 * @brief Convert Python seq-of-seqs to C heap matrix [n][dim_or_k].
 * @param mat_obj Input Python sequence of length n; each row length dim_or_k.
 * @param n Expected number of rows.
 * @param dim_or_k Expected number of columns.
 * @param out_mat Out: receives malloc'ed double (*)[dim_or_k]; caller must free().
 * @return 1 on success, -1 on error (sets a Python exception).
 */
static int py_points_to_c_points(PyObject* mat_obj, int n, int dim_or_k, double **out_mat) {
    double *mat;
    int i, j;
    PyObject *row, *py_val;
    double c_val;

    if (!PySequence_Check(mat_obj) || PySequence_Size(mat_obj) != n) { PyErr_SetString(PyExc_ValueError, "An Error Has Occurred"); return -1;}
    
    mat = malloc(n * dim_or_k * sizeof *mat); /* Allocate C matrix */
    if (!mat) { PyErr_SetString(PyExc_MemoryError, "An Error Has Occurred"); return -1; }

    for (i = 0; i < n; i++) { /* Create C points from PyObject points */
        row = PySequence_GetItem(mat_obj, i);
        if (!row || !PySequence_Check(row) || PySequence_Size(row) != dim_or_k) {Py_XDECREF(row); free(mat); PyErr_SetString(PyExc_ValueError, "An Error Has Occurred"); return -1;}
        
        for (j = 0; j < dim_or_k; j++) {
            py_val = PySequence_GetItem(row, j);
            if (!py_val) { Py_DECREF(row); free(mat); PyErr_SetString(PyExc_ValueError, "An Error Has Occurred"); return -1; }
            
            c_val = PyFloat_AsDouble(py_val);
            Py_DECREF(py_val);

            if (PyErr_Occurred()) { Py_DECREF(row); free(mat); PyErr_SetString(PyExc_ValueError, "An Error Has Occurred"); return -1; }

            mat[i * dim_or_k + j] = c_val;
        }
        Py_DECREF(row);
    }
    *out_mat = mat;
    return 1;
}

/**
 * @brief Build a Python list-of-lists from C matrix [n][k].
 * @param n Number of rows.
 * @param k Number of columns.
 * @param mat Input C matrix [n][k].
 * @param py_mat_out Out: receives new PyObject* (list of lists); caller DECREFs.
 * @return 1 on success, -1 on error (sets Python exception).
 */
static int build_py_object_mat(int n, int k, double *mat, PyObject **py_mat_out) {
    PyObject *py_mat, *row, *num;
    int i, j;

    py_mat = PyList_New(n);
    if (!py_mat) {PyErr_Clear(); PyErr_SetString(PyExc_MemoryError, "An Error Has Occurred"); return -1;}
    for (i = 0; i < n; i++) {
        row = PyList_New(k);
        if (!row) {Py_DECREF(py_mat); PyErr_SetString(PyExc_MemoryError, "An Error Has Occurred"); return -1; }
        for (j = 0; j < k; j++) {
            num = PyFloat_FromDouble(mat[i * k + j]);
            if (!num) {PyErr_SetString(PyExc_MemoryError, "An Error Has Occurred"); Py_DECREF(row); Py_DECREF(py_mat); return -1;}
            PyList_SET_ITEM(row, j, num);
        }
        PyList_SET_ITEM(py_mat, i, row);
    }
    *py_mat_out = py_mat;
    return 1;
}