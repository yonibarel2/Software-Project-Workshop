#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <time.h>

/**
 * @brief Linked-list node for an n-D point.
 */
typedef struct Node {
    double *point;  /**< Coordinates of length dim (malloc'ed). */
    struct Node *next;  /**< Next node or NULL. */
} Node;

/* params for updating decomposition matrix H */
static const double EPS = 1e-4; /* stop condition */
static const double BETA = 0.5; /* the constant beta */
static const int MAX_ITERS = 300; /* max num of iterations */

/* functions prototype declarations */
int file_validate_and_to_point_list(FILE *in, int *dim_out, int *n_out, Node *point_lst_head);
static int read_line(FILE *in, char **buff_out, size_t *len_out);
static int parse_point(char *buff, int *dim, double **out_point);
static int read_and_check_row(FILE *in, int *dim, double **out_point);
static void free_tail_and_point(Node *head);
void free_list(Node *head);
int linked_list_to_2d_array(Node *head, int n, int dim, double **out_arr);

int sym(int dim, int n, const double *mat, double **out_A);
static double sym_entry(const double *p1,const double*p2, int dim);
int ddg(int n, const double *A, double **out_D);
int norm(int n, const double *A, const double *D, double **out_W);

int decomp_mat(int n, int k, const double *W, const double *H0, double **out);
static int update_decomp_mat(int n, int k, const double *W, const double *H, double **out_H);
static int mat_mul(int rows, int m, int cols, const double *A, const double *B, double **out);
static int mat_transpose(int rows, int cols, const double *mat, double **out);
void print_squared_matrix_floats_4f(int n, const double *mat);

/**
 * @brief Main entry: read points file, build matrix per goal, print result.
 * @param argc Argument count (expect 3).
 * @param argv Argument vector: [1]=goal ("sym","ddg","norm"), [2]=file name.
 * @return 0 on success, exits(1) with error message on failure.
 */
int main(int argc, char **argv) {
    const char *goal;
    FILE *in;
    Node *point_lst_head;
    int dim = 0, n = 0;
    double *points = NULL, *sym_mat = NULL, *ddg_mat = NULL, *norm_mat = NULL;
    if (argc !=  3) {printf("An Error Has Occurred\n"); exit(1);} /* Step 1: check argv and that goal is valid -> expecting argv[1] = goal, argv[2] = file_name */
    goal = argv[1];
    if (strcmp(goal, "sym") != 0 && strcmp(goal, "ddg") != 0 && strcmp(goal, "norm") != 0) {printf("An Error Has Occurred\n"); exit(1);}
    in = fopen(argv[2], "r"); if (!in) {printf("An Error Has Occurred\n"); exit(1);}
    point_lst_head = malloc(sizeof(*point_lst_head)); /* Step 2: validate and create points list. */
    if(!point_lst_head) {fclose(in); printf("An Error Has Occurred\n"); exit(1);}
    point_lst_head->next = NULL; point_lst_head->point = NULL;
    if (file_validate_and_to_point_list(in, &dim, &n, point_lst_head) != 1 || n <= 1) {printf("An Error Has Occurred\n"); free_list(point_lst_head); fclose(in); exit(1);}
    fclose(in);
    if (linked_list_to_2d_array(point_lst_head, n, dim, &points) != 1) {printf("An Error Has Occurred\n"); free_list(point_lst_head); free(points), exit(1);} /* Step 3: convert the linked list to a 2D array and free linked list */
    free_list(point_lst_head); /* Even if success -> we have the 2D array and no need for linked list of points*/
    if (strcmp(goal, "sym") == 0) { /* Step 4: run chosen algorithm (by goal) - and create n * n matrix */
        if (sym(dim, n,points, &sym_mat) == -1) {printf("An Error Has Occurred\n");free(points); exit(1);}
        print_squared_matrix_floats_4f(n, sym_mat); free(sym_mat);
    }
    else if (strcmp(goal, "ddg") == 0) {
        if (sym(dim, n,points, &sym_mat) == -1) {printf("An Error Has Occurred\n");free(points); exit(1);}
        if (ddg(n, sym_mat, &ddg_mat) == -1) {printf("An Error Has Occurred\n");free(points); free(sym_mat); exit(1);}
        free(sym_mat); print_squared_matrix_floats_4f(n, ddg_mat); free(ddg_mat);
    }
    else { /* goal = norm */
        if (sym(dim, n,points, &sym_mat) == -1) {printf("An Error Has Occurred\n");free(points); exit(1);}
        if (ddg(n, sym_mat, &ddg_mat) == -1) {printf("An Error Has Occurred\n");free(points); free(sym_mat); exit(1);}
        if (norm(n, sym_mat, ddg_mat, &norm_mat) == -1) {printf("An Error Has Occurred\n");free(points); free(sym_mat); free(ddg_mat); exit(1);}
        free(sym_mat); free (ddg_mat); print_squared_matrix_floats_4f(n, norm_mat); free(norm_mat);
    }
    free(points); return 0;
}

 /**
 * @brief Read CSV points into linked list, validating consistent dimension.
 * @param in Input FILE*.
 * @param dim_out Out: dimension (set from first row).
 * @param n_out Out: number of points read.
 * @param point_lst_head Preallocated head node to fill; subsequent nodes malloc'ed.
 * @return 1 on success, 0 if file empty, -1 on parse/allocation error.
 * @note Caller must free the list (and points) when done.
 */
int file_validate_and_to_point_list(FILE *in, int *dim_out, int *n_out, Node *point_lst_head) {
    Node *curr_node;
    Node *new_node;
    int points_list_len = 0;
    int status;

    point_lst_head->next = NULL;
    point_lst_head->point = NULL;
    curr_node = point_lst_head; 

    status = read_and_check_row(in, dim_out, &point_lst_head->point); /* Read first row, define dim and add point to point_lst_head */
    if (status == 0) {*n_out = points_list_len; return 0;} /* 0 = empty (EOF at start) */
    else if (status == -1) {free_tail_and_point(point_lst_head); return -1;} /* -1 = invalid */
    else {points_list_len ++;}
    
    while (1) {
        new_node = malloc(sizeof(*new_node));
        if (!new_node) {free_tail_and_point(point_lst_head); return -1;}
        new_node->point = NULL;
        new_node->next = NULL;

        status = read_and_check_row(in, dim_out, &new_node->point); 
        if (status == 0) {free(new_node); *n_out = points_list_len; return 1;} /* EOF - no need to free for new. */
        else if (status == -1) {free(new_node); free_tail_and_point(point_lst_head); return -1;}
        else {
            curr_node->next = new_node;
            curr_node = curr_node->next;
            points_list_len++;
        }
    }
}

/**
 * @brief Read one CSV line from file into a malloc'ed buffer.
 * @param in Input FILE* (text stream).
 * @param buff_out Out: pointer to malloc'ed buffer holding the line (null-terminated).
 * @param len_out Out: length of the line (not counting '\0').
 * @return 1 on success, 0 on clean EOF, -1 on read/alloc error.
 */
static int read_line(FILE *in, char **buff_out, size_t *len_out) {
    size_t point_cap = 64, len = 0;
    char *buff = NULL, *tmp_char = NULL;
    int c, d;

    buff = malloc(point_cap);
    if (!buff) { return -1; }

    c = fgetc(in);
    while (c != EOF && c != '\n' && c != '\r') { /* Add chars to buff until newline or EOF. */
        if (len + 1 >= point_cap) { /* Resize buff if need. */
            point_cap *= 2;
            tmp_char = realloc(buff, point_cap);
            if (!tmp_char) { free(buff); return -1; }
            buff = tmp_char;
        }
        buff[len] = (char)c;
        len += 1;
        c = fgetc(in);
    }

    if (c == EOF && len > 0) { free(buff); return -1; } /* no final newline */
    if (c == EOF && len == 0) { free(buff); return 0; } /* clean EOF */

    if (c == '\r') { /* Check newline style and handle (\r or \n or \r\n) - if was /r/n then need to skip extra char /n */
        d = fgetc(in);
        if (d != '\n' && d != EOF) { ungetc(d, in); }
    }
    buff[len] = '\0';
    *buff_out = buff;
    *len_out  = len;
    return 1;
}

/**
 * @brief Parse a CSV line buffer into a point array, enforce consistent dim.
 * @param buff In/out: mutable string buffer (commas overwritten by '\0').
 * @param dim In/out: 0 for first row (sets dim), else must match fixed dim.
 * @param out_point Out: malloc'ed array of length *dim with parsed doubles; caller must free().
 * @return 1 on success, -1 on parse/alloc/validation error.
 */
static int parse_point(char *buff, int *dim, double **out_point) {
    char *p = buff, *q = NULL;
    char *endptr = NULL, *comma = NULL;
    double *point = NULL, *tmp_double = NULL, coordinate;
    size_t max_dim = 8;
    int point_len = 0;

    if (*dim == 0) {point = malloc(max_dim * sizeof(double));} /* First point */
    else {point = malloc((*dim) * sizeof(double));}
    if (!point) {return -1;}
    while (1) {
        comma = strchr(p, ',');
        if (comma) {*comma = '\0';}
        if (*p == '\0') {free(point); return -1;} /* empty token */
        q = p;
        while (*q) {if (isspace((unsigned char)*q)) {free(point); return -1;} q++;}
        errno = 0; coordinate = strtod(p, &endptr); /* Parse double */
        if (*endptr != '\0' || errno == ERANGE) {free(point); return -1;}
        if (*dim == 0 && point_len >= (int)max_dim) { /* For first point - check if need to allocate more memory in buffer */
            max_dim *= 2;
            tmp_double = realloc(point, max_dim * sizeof(double));
            if (!tmp_double) {free(point); return -1;}
            point = tmp_double;
        }
        if (*dim != 0 && point_len >= *dim) {free(point); return -1;} /* Not first point and point_len > dim -> invalid */
        point[point_len] = coordinate; point_len ++;
        if (!comma) {break;}
        p = comma + 1;
    }
    if (*dim == 0) { /*First point - shrink arr and define dim */
        tmp_double = realloc(point, point_len * sizeof(double));
        if (!tmp_double) {free(point); return -1;}
        point = tmp_double; *dim = point_len;
    } 
    else if (point_len != *dim) {free(point); return -1;}
    *out_point = point; return 1;
}

/**
 * @brief Read one CSV line, parse doubles, enforce consistent dim.
 * @param in Input FILE* (text, CSV by commas).
 * @param dim In/out: 0 for first row (sets dim), else must match.
 * @param out_point Receives malloc'ed array of length *dim; caller must free().
 * @return 1 on success, 0 on clean EOF (empty line at end), -1 on parse/alloc/error.
 */ 
static int read_and_check_row(FILE *in, int *dim, double **out_point) {
    char *buff = NULL;
    size_t len = 0;
    int status, parse_status;

    /* Step 1: read line */
    status = read_line(in, &buff, &len);
    if (status <= 0) { /* 0 = clean EOF, -1 = error */
        return status;
    }

    /* Step 2: parse line into point */
    parse_status = parse_point(buff, dim, out_point);
    free(buff);

    if (parse_status == -1) {
        return -1;
    }
    return 1;
}

/**
 * @brief Free tail list starting from head->next and the point of head.
 * @param head Node whose tail and point to free (may be NULL).
 */
static void free_tail_and_point(Node *head) {
    if (!head) return;
    if (head->next) { free_list(head->next); head->next = NULL; }
    if (head->point) { free(head->point); head->point = NULL; }
}

/**
 * @brief Free linked list of Node and their point arrays.
 * @param head Head of list (may be NULL).
 */
void free_list(Node *head) {
    Node *next_node;
    while (head) {
        next_node = head->next;
        free(head->point);
        free(head);
        head = next_node;
    }
}

 /**
 * @brief Convert linked list of points into n×dim array.
 * @param head Head of list.
 * @param n Number of nodes.
 * @param dim Dimension of each point.
 * @param out_arr Receives malloc'ed n×dim array; caller must free().
 * @return 1 on success, -1 on failure.
 */
int linked_list_to_2d_array(Node *head, int n, int dim, double **out_arr) {
    double *arr;
    Node *p;
    int i, d;

    if (!head || n <= 0 || dim <= 0 || !out_arr) {
        return -1;
    }

    arr = malloc(n * dim * sizeof(double));
    if (!arr) {return -1;}

    p = head;
    for (i = 0; i < n; i++) {
        for (d = 0; d < dim; d++)
            arr[i * dim + d] = p->point[d];
        p = p->next;
    }
    *out_arr = arr;
    return 1;
}

 /**
 * @brief Build n×n symmetric affinity matrix A from n points in R^dim.
 * @param dim Point dimension.
 * @param n Number of points.
 * @param mat Input n×dim points.
 * @param out_A Receives malloc'ed n×n A; caller must free().
 * @return 1 on success, -1 on allocation failure.
 */
int sym(int dim, int n, const double *mat, double **out_A) {
    double *A;
    int i, j;
    double entry;
    
    A = malloc(n * n * sizeof *A); /* iInitialize the new n*n symmetric matrix */
    if (!A) return -1;

    for(i = 0; i < n; i++) { /* Add entries to sym matrix */
        A[i * n + i] = 0.0;
        for(j = i + 1; j < n; j++) {
            entry = sym_entry(mat + i * dim, mat + j * dim, dim);
            A[i * n + j] = entry;
            A[j * n + i] = entry;
        }
    }
    *out_A = A;
    return 1;
}

/**
 * @brief Compute entry value = exp(-||p1-p2||^2 / 2).
 * @param p1 First point (length dim).
 * @param p2 Second point (length dim).
 * @param dim Dimension.
 * @return Entry value in (0,1].
 */
static double sym_entry(const double *p1, const double *p2, int dim) {
    double dist = 0.0;
    double diff;
    double entry;
    int i;

    for(i = 0; i < dim; i++) { /* Calculate squared Euclidean distance */
        diff = p1[i] - p2[i];
        dist += diff * diff;
    }
    entry= exp(-dist / 2.0);
    return entry;
}

 /**
 * @brief Build n×n diagonal degree matrix D from sym matrix A.
 * @param n Number of rows/cols.
 * @param A Input n×n sym matrix.
 * @param out_D Receives malloc'ed n×n D; caller must free().
 * @return 1 on success, -1 on allocation failure.
 */
int ddg(int n, const double *A, double **out_D) {
    double *D;
    int i, j;
    double diag_sum;

    D = calloc(n * n, sizeof *D); /* Initialize the new n * n ddg matrix - all entries initialized to 0.0 */
    if (!D) {return -1;}

    for(i = 0; i < n; i++) { /* Add diag entries to ddg matrix */
        diag_sum = 0.0;
        for(j = 0; j < n; j++) {
            diag_sum += A[i * n + j];
        }
        D[i * n + i] = diag_sum;
    }
    *out_D = D;
    return 1;
}

/**
 * @brief Compute normalized matrix W = D^(-1/2) A D^(-1/2).
 * @param n Number of rows/cols.
 * @param A Input n×n sym matrix.
 * @param D Input n×n ddg matrix.
 * @param out_W Receives malloc'ed n×n normalized matrix; caller must free().
 * @return 1 on success, -1 on allocation failure.
 */
int norm(int n, const double *A, const double *D, double **out_W) {
    double *D_inversed_sqrt;
    double *tmp;
    int i;
    double d;

    D_inversed_sqrt = calloc(n * n, sizeof *D_inversed_sqrt);
    if (!D_inversed_sqrt) {return -1;}

    for (i = 0; i < n; i++) { /* Create D^(-1/2) */
        d = D[i * n + i];
        if (d == 0) d = EPS;
        D_inversed_sqrt[i * n + i] = 1.0 / sqrt(d);
    }

    tmp = NULL;
    if (mat_mul(n, n, n, D_inversed_sqrt, A, &tmp) == -1) {free(D_inversed_sqrt); return -1;}
    if (mat_mul(n, n, n, tmp, D_inversed_sqrt, out_W) == -1) {free(tmp); free(D_inversed_sqrt); return -1;}
    free(tmp); free(D_inversed_sqrt);
    return 1;
}

/**
 * @brief Iteratively update H starting from H0 until (||H_(t+1) - H_t||_F^2) < EPS or MAX_ITERS.
 * @param n Number of rows in W and H.
 * @param k Number of columns in H.
 * @param W Input n×n matrix.
 * @param H0 Initial n×k matrix.
 * @param out Receives malloc'ed final n×k H; caller must free().
 * @return 1 on success, -1 on failure.
 */
int decomp_mat(int n, int k, const double *W, const double *H0, double **out) {
    double *H_t, *H_t_plus1;
    int i, j, it;
    double diff, d;
    
    H_t = malloc(n * k * sizeof *H_t); /* H(t) <- H0 */
    if (!H_t) return -1;
    for (i = 0; i < n; ++i) {
        for (j = 0; j < k; ++j) {
            H_t[i * k + j] = H0[i * k + j];
        }
    }
    for (it = 0; it < MAX_ITERS; ++it) { /* Update H_t_plus1 */
        H_t_plus1 = NULL;
        if (update_decomp_mat(n, k, W, H_t, &H_t_plus1) == -1) {free(H_t); return -1;}
        
        diff = 0.0; /* Calculate ||H_(t+1) - H_t||_F^2 */
        for (i = 0; i < n; ++i) {
            for (j = 0; j < k; ++j) {
                d = H_t_plus1[i * k + j] - H_t[i * k + j];
                diff += d * d;
            }
        }
        if (diff < EPS) {free(H_t); *out = H_t_plus1; return 1;} /* retunr  H(t+1) */
        free(H_t); H_t = H_t_plus1;/* Next iteration: Ht <- H(t+1) */
    }
    *out = H_t; return 1; /* Max iters reached; return last Ht */
}

/**
 * @brief One update step for H using W and current H (with EPS/BETA safeguards).
 * @param n Number of rows in W and H.
 * @param k Number of columns in H.
 * @param W Input n×n matrix.
 * @param H Current n×k matrix.
 * @param out_H Receives malloc'ed n×k updated H; caller must free().
 * @return 1 on success, -1 on failure.
 */
static int update_decomp_mat(int n, int k, const double *W, const double *H, double **out_H) {
    double *H_T = NULL, *tmp1 = NULL, *tmp2 = NULL, *tmp3 = NULL, *H_t_plus1 = NULL;
    int i, j;

    if (n <= 0 || k <= 0 || !W || !H || !out_H) return -1;

    if (mat_transpose(n, k, H, &H_T) == -1) {return -1;} /* H^T (kxn) */
    if (mat_mul(n, k, n, H, H_T, &tmp1) == -1) {free(H_T); return -1;} /* (H H^T) (n×n) */
    if (mat_mul(n, n, k, tmp1, H, &tmp2) == -1) {free(H_T); free(tmp1); return -1;} /* (H H^T) H (n×k) */
    free(H_T); free(tmp1);

    for (i = 0; i < n; i++) { /* Ensure not dividing by zero - if cell == 0 -> turn to EPS */
        for (j = 0; j < k; j++) {
            if (tmp2[i * k + j] == 0.0) {tmp2[i * k + j] = EPS;}
        }
    }
    if (mat_mul(n, n, k, W, H, &tmp3) == -1) {free(tmp2); return -1;} /* W H (n×k) */
    H_t_plus1 = malloc(n * k * sizeof *H_t_plus1);
    if (!H_t_plus1) {free(tmp2); free(tmp3); return -1;}

    for (i = 0; i < n; i++) {  /* Calculate H(t+1) */
        for (j = 0; j < k; j++) {
            H_t_plus1[i * k + j] = H[i * k + j] * ((1.0 - BETA) + BETA * (tmp3[i * k + j] / tmp2[i * k + j]));
        }
    }
    free(tmp2); free(tmp3);
    *out_H = H_t_plus1;
    return 1;
}

 /**
 * @brief Multiply two matrices: A(rows×m) and B(m×cols).
 * @param rows Number of rows in A.
 * @param m Shared dimension of A and B.
 * @param cols Number of columns in B.
 * @param A Left matrix (rows×m).
 * @param B Right matrix (m×cols).
 * @param out Receives malloc'ed rows×cols result; caller must free().
 * @return 1 on success, -1 on invalid args or allocation failure.
 */
static int mat_mul(int rows, int m, int cols, const double *A, const double *B, double **out) {
    int i, j, k;
    double aik;
    double *C;

    if (rows <= 0 || m <= 0 || cols <= 0 || !A || !B || !out) {return -1;}
    C = calloc(rows * cols, sizeof *C);
    if (!C) {return -1;}

    for (i = 0; i < rows; ++i) {
        for (j = 0; j < m; ++j) {
            aik = A[i * m + j];
            for (k = 0; k < cols; ++k) {
                C[i * cols + k] += aik * B[j * cols + k];
            }
        }
    }
    *out = C;
    return 1;
}

/**
 * @brief Compute transpose of a rows×cols matrix.
 * @param rows Number of rows in input.
 * @param cols Number of columns in input.
 * @param mat Input matrix.
 * @param out Receives malloc'ed cols×rows result; caller must free().
 * @return 1 on success, -1 on allocation failure.
 */
static int mat_transpose(int rows, int cols, const double *mat, double **out) {
    double *res;
    int i, j;

    res = malloc(rows * cols * sizeof *res);
    if (!res) {return -1;}
    for (i = 0; i < rows; ++i) {
        for (j = 0; j < cols; ++j) {
            res[j * rows + i] = mat[i * cols + j];
        }
    }

    *out = res;
    return 1;
}

/**
 * @brief Print an n×n matrix of doubles with 4 decimal places.
 * @param n Matrix dimension.
 * @param mat Input square matrix.
 */
void print_squared_matrix_floats_4f(int n, const double *mat) {
    int i, j;

    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            printf("%.4f", mat[i * n + j]);
            if (j == n - 1) {printf("\n");}
            else {printf(",");}
        }
    }         
}