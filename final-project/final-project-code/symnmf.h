#ifndef SYMNMF_H
#define SYMNMF_H

/* Public interface: Symnmf core routines */

 /**
 * @brief Build n×n symmetric affinity matrix A from n points in R^dim.
 * @param dim Point dimension.
 * @param n Number of points.
 * @param mat Input n×dim points.
 * @param out_A Receives malloc'ed n×n A; caller must free().
 * @return 1 on success, -1 on allocation failure.
 */
int sym(int dim, int n, const double *mat, double **out_A);

 /**
 * @brief Build n×n diagonal degree matrix D from sym matrix A.
 * @param n Number of rows/cols.
 * @param A Input n×n sym matrix.
 * @param out_D Receives malloc'ed n×n D; caller must free().
 * @return 1 on success, -1 on allocation failure.
 */
int ddg(int n, const double *A, double **out_D);

/**
 * @brief Compute normalized matrix W = D^(-1/2) A D^(-1/2).
 * @param n Number of rows/cols.
 * @param A Input n×n sym matrix.
 * @param D Input n×n ddg matrix.
 * @param out_W Receives malloc'ed n×n normalized matrix; caller must free().
 * @return 1 on success, -1 on allocation failure.
 */
int norm(int n, const double *A, const double *D, double **out_W);

/**
 * @brief Iteratively update H starting from H0 until (||H_(t+1) - H_t||_F^2) < EPS or MAX_ITERS.
 * @param n Number of rows in W and H.
 * @param k Number of columns in H.
 * @param W Input n×n matrix.
 * @param H0 Initial n×k matrix.
 * @param out Receives malloc'ed final n×k H; caller must free().
 * @return 1 on success, -1 on failure.
 */
int decomp_mat(int n, int k, const double *W, const double *H0, double **out);

#endif /* SYMNMF_H */