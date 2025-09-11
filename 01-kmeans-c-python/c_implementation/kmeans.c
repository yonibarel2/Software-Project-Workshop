#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

/**
 * @brief Linked-list node for an n-D point.
 */
typedef struct Node {
    double *point;  /**< Coordinates of length dim (malloc'ed). */
    struct Node *next;  /**< Next node or NULL. */
} Node;

int stdin_validate_and_to_point_list(int *dim_out, int *n_out, Node *point_lst_head);
static int read_and_check_row(int *dim, double **out_point);
int linked_list_to_2d_array(Node *head, int n, int dim, double (**out_arr)[dim]);
void free_list(Node *head);
static void free_tail_and_point(Node *head);
int argv_check(int argc, char **argv, int *k_and_iterations, int n);
int kmeans(int n, int k, int iterations, int dim, const double (*points)[dim], double (**centroids_out)[dim]);
static int find_min_idx(const double *point, int k, int dim, double (*centroids)[dim]);
static void print_centroids(int k, int dim, double centroids[k][dim]);

/**
 * @brief Program entry: read points, validate args, run k-means, print centroids.
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return 0 on normal exit (errors print and exit(1)).
 */
int main(int argc, char **argv) {

    //Step 1: validate and create points list.
    Node *point_lst_head = malloc(sizeof(*point_lst_head));    //create list head.
    if(!point_lst_head) {exit(1);}
    *point_lst_head = (Node){0};
    int dim = 0, n = 0;

    int status = stdin_validate_and_to_point_list(&dim, &n,  point_lst_head); 
    if (status != 1 || n <= 1) { free(point_lst_head); printf("An Error Has Occurred\n"); exit(1);} // 0 empty or -1 error

    //step 2: convert the linked list to a 2D array and free linked list.
    double (*points)[dim] = NULL;
    if (linked_list_to_2d_array(point_lst_head, n, dim, &points) != 1) {printf("An Error Has Occurred\n"); free_list(point_lst_head); exit(1);}
    free_list(point_lst_head);

    //Step 3: check args (k and iterations).
    int k_and_iterations[2];
    status = argv_check(argc, argv, k_and_iterations, n);
    if (status == -1) {free(points); exit(1);}

    //step 4: run kmeans algo
    double (*centroids)[dim] = NULL;
    status = kmeans(n, k_and_iterations[0], k_and_iterations[1], dim, (const double (*)[dim])points, &centroids);
    if (status == -1) {printf("An Error Has Occurred\n"); exit(1);}
    else {print_centroids(k_and_iterations[0], dim, centroids);}

    free(points);free(centroids);
    return 0;
}

/**
 * @brief Read points from stdin into a linked list and set dimension/count.
 *
 * Reads CSV lines from stdin, validates each row, and stores points in a
 * linked list starting at the given head node. On the first row, determines
 * and sets the dimension. On success, sets the number of points.
 *
 * @param dim_out Output: dimension of each point (set by first row).
 * @param n_out Output: number of points read.
 * @param point_lst_head Preallocated head node (must be malloc'ed by caller).
 *                       Tail nodes are allocated inside the function.
 * @return 1 if ok, 0 if stdin is empty, -1 on invalid input or allocation error.
 *
 * @note Caller must free the list (and points) when done.
 */
int stdin_validate_and_to_point_list(int *dim_out, int *n_out, Node *point_lst_head) {
    
    point_lst_head->next = NULL;   // ensure
    point_lst_head->point = NULL;  // ensure
    Node *curr_node = point_lst_head; 

    int points_list_len = 0;

    int status = read_and_check_row(dim_out, &point_lst_head->point); // read first row, define dim and add point to point_lst_head
    if (status == 0) {*n_out = points_list_len; return 0;} // 0 = empty (EOF at start)
    else if (status == -1) {free_tail_and_point(point_lst_head); return -1;} // -1 = invalid
    else {points_list_len ++;}
    
    while (1) {
        Node *new_node = malloc(sizeof(*new_node));
        if (!new_node) {free_tail_and_point(point_lst_head); return -1;}
        *new_node = (Node){0};

        status = read_and_check_row(dim_out, &new_node->point);
        if (status == 0) {free(new_node); *n_out = points_list_len; return 1;} // EOF - no need to free for new.
        else if (status == -1) {free(new_node); free_tail_and_point(point_lst_head); return -1;}
        else {
            curr_node->next = new_node;
            curr_node = curr_node->next;
            points_list_len++;
        }
    }
}

/**
 * @brief Read one CSV line, validate/parse to doubles, enforce dimension.
 * If *dim==0 (first row), sets *dim from parsed count.
 * @param dim Expected dimension or 0 for first row.
 * @param out_point Receives malloc'ed double[]; caller must free().
 * @return 1 ok, 0 EOF before any char, -1 invalid line/alloc failure.
 */
static int read_and_check_row(int *dim, double **out_point) {

    //Step 1: read line to buffer.
    size_t point_cap = 64, len = 0;
    char *buff = malloc(point_cap);
    if (!buff) { return -1;}
    
    int c = fgetc(stdin);
    while (c != EOF && c != '\n' && c != '\r') { // add chars to buff until newline or EOF.
        if (len + 1 >= point_cap) { // resize buff if need.
            point_cap *= 2;
            char *tmp = realloc(buff, point_cap);
            if (!tmp) {free(buff); return -1;}
            buff = tmp;
        }
        
        buff[len] = (char)c; // add next char to buffer.
        len += 1;
        c = fgetc(stdin);
    }
    /* reject if EOF without newline */
    if (c == EOF && len > 0) { free(buff); return -1;} // no final newline
    if (c == EOF && len == 0) {free(buff); return 0;} // End of file (might be empty)

    if (c == '\r') { // check newline style and handle (\r or \n or \r\n) check newline style and handle (\r or \n or \r\n) - if was /r/n then need to skip extra char /n
        int d = fgetc(stdin);
        if (d != '\n' && d != EOF) {ungetc(d, stdin);}
    }
    buff[len] = '\0'; // turn newline to '\0' -> end of last token
    
    // Step 2: tokenize by ','  parse with strtod and add to point arr
    double *point;
    size_t max_dim = 8;
    if (*dim == 0) { // first point
        point = malloc(max_dim * sizeof(double));
        if (!point) {free(buff); return -1;}
    }
    else {
        point = malloc((*dim) * sizeof(double)); 
        if (!point) {free(buff); return -1;}
    }

    int point_len = 0;
    char *p = buff;
    while(1) {
        char *comma = strchr(p, ',');
        if (comma) {(*comma = '\0');}
        if (*p == '\0') {free(buff); free(point);return -1;} // was empty token.
        for (char *q = p; *q; ++q) { // check if any spaces
            if (isspace((unsigned char)*q)) {free(buff); free(point);return -1;}
        }
         // parse double
        errno = 0;
        char *endptr;
        double coordinate = strtod(p, &endptr);
        if (*endptr != '\0' || errno == ERANGE) {free(buff); free(point); return -1;} // if couldnt convert
        
        //for first point - check if need to allocate more memory in buffer
        if (*dim == 0 && point_len >= (int)max_dim) {          // simple grow (e.g., double)
            max_dim *= 2;
            double *tmp = realloc(point, max_dim * sizeof(double));
            if(!tmp) {free(buff); free(point); return -1;}
            point = tmp;
        }
        
        //add coordinate to point        
        point[point_len] = coordinate;
        point_len ++; 
        if ((*dim != 0) && (point_len > *dim)) {free(buff); free(point); return -1;} // if not first point and point_len > dim -> not vaild

        if (!comma) break;
        p = comma + 1;
    }

    if (*dim == 0) { //if was first point - shrink arr and define dim
        double *tmp = realloc(point, point_len * sizeof(double));
        if (!tmp) { free(buff); free(point); return -1; }
        point = tmp;
        *dim = point_len;
    }
    else { // check point for valid dim.
        if (point_len != *dim) {free(buff); free(point); return -1;}
    }

    *out_point = point;
    free(buff);
    return 1;
} 

/**
 * @brief Convert a linked list of points to a contiguous [n][dim] array.
 * @param head First node (each node->point has length dim).
 * @param n Number of points.
 * @param dim Point dimension.
 * @param out_arr Receives malloc'ed array; caller must free().
 * @return 1 on success, -1 on failure/invalid args.
 */
int linked_list_to_2d_array(Node *head, int n, int dim, double (**out_arr)[dim]) {
    if (!head || n <= 0 || dim <= 0 || !out_arr) return -1;

    double (*arr)[dim] = malloc(n * sizeof *arr);
    if (!arr) {return -1;}

    Node *p = head;
    for (int i = 0; i < n; i++) {
        for (int d = 0; d < dim; d++)
            arr[i][d] = p->point[d];
        p = p->next;
    }
    *out_arr = arr;
    return 1;
}

/**
 * @brief Free entire list: all nodes and their point arrays.
 * @param head Head of the list (may be NULL).
 */
void free_list(Node *head) {
    while (head) {
        Node *next_node = head->next;
        free(head->point);
        free(head);
        head = next_node;
    }   
}

/**
 * @brief Free head->next... and head->point, but not the head node.
 * @param head List head (may be NULL).
 */
static void free_tail_and_point(Node *head) {
    if (!head) return;
    if (head->next) { free_list(head->next); head->next = NULL; }
    if (head->point) { free(head->point); head->point = NULL; }
}

/**
 * @brief Validate CLI: k and optional iterations; sets outputs or exits on error.
 * @param argc Argument count.
 * @param argv Argument vector.
 * @param k_and_iterations Out: [0]=k, [1]=iterations (default 400).
 * @param n Number of points (for checking 1<k<n).
 */
int argv_check(int argc, char **argv, int *k_and_iterations, int n) { 
    /*
    checks if the command-line arguments are valid (as expected in Table 1)
    If invalid -> print an error and terminate
    If valid -> return (k, iterations)
    */
    if (argc == 1) {printf("Incorrect number of clusters!\n"); return -1;}

    if (argc != 2 && argc != 3) {printf("Incorrect number of clusters!\n"); return -1;}

    //check if K and iterations are a string representing an int
    char *end1;
    errno = 0;
    long k = strtol(argv[1],&end1, 10);
    if (errno == ERANGE || k > INT_MAX || end1 == argv[1] || *end1 != '\0' || k <= 1 || k >= n) {printf("Incorrect number of clusters!\n"); return -1;}
    
    k_and_iterations[0] = (int)k;

    if(argc == 2) { //iterations not provided -> default =400
        k_and_iterations[1] = 400;
    }
    
    else {
        char *end2;
        errno = 0;
        long iterations = strtol(argv[2],&end2, 10); 
        if (errno == ERANGE || iterations > INT_MAX || end2 == argv[2] || *end2 != '\0' || iterations <= 1 || iterations >= 1000) {printf("Incorrect number of clusters!\n"); return -1;}
        k_and_iterations[1] = (int)iterations;
    }
    return 1;
}

/**
 * @brief Run Lloyd's k-means on points.
 * @param n Number of points.
 * @param k Number of clusters.
 * @param iterations Max iterations (>=2, <=999).
 * @param dim Dimension.
 * @param points Input n×dim points.
 * @param centroids_out Receives malloc'ed k×dim centroids; caller must free().
 * @return 1 on success, -1 on allocation failure.
 */
int kmeans(int n, int k, int iterations, int dim, const double (*points)[dim], double (**centroids_out)[dim]) { // add arg that returns pointer to 2D arr malloced
    
    //step 1: label is the number of point and it says witch cluster it belongs to and counts is number of points in each cluster and sum is to sum later of
    int *labels = calloc(n, sizeof(*labels));
    int *counts = calloc(k, sizeof(*counts));
    double (*sums)[dim] = calloc((size_t)k, sizeof *sums);
    if (!labels || !counts || !sums) {free(labels); free(counts); free(sums); return -1;}

    double (*centroids)[dim] = malloc(k * sizeof *centroids);
    if (!centroids) {free(labels); free(counts); free(sums); return -1;}

    //step 2: initialize centroids
    for (int c = 0; c < k; ++c) {
        for (int d = 0; d < dim; ++d)
            centroids[c][d] = points[c][d];
    }
    
    for (int it = 0; it < iterations; it++) {
        memset(counts, 0, (size_t)k * sizeof *counts);
        memset(sums,   0, (size_t)k * sizeof *sums); // zero k×dim
        int moved = 0;
        
        for (int i = 0; i < n; i++) { // assign points to clusters
            int min_idx = find_min_idx(points[i], k, dim, centroids);
            if (labels[i] != min_idx) {moved = 1;}
            labels[i] = min_idx; // sort point to cluster
            counts[min_idx]++;
            for (int j = 0; j < dim; j++) {
                sums[min_idx][j] += points[i][j];
            }
        }
            for (int i = 0; i < k; i++) {
                if (counts[i] == 0) {continue;}
                for (int j = 0; j < dim; j++) {centroids[i][j] = sums[i][j] / counts[i];}
            }
            if(!moved) break;
        }
    *centroids_out = centroids; // hand off to caller
    free(sums); free(counts); free(labels);
    return 1;
}

/**
 * @brief Index of closest centroid to a point (squared Euclidean).
 * @param point Point of length dim.
 * @param k Number of centroids.
 * @param dim Dimension.
 * @param centroids k×dim centroid matrix.
 * @return Index in [0, k-1].
 */
static int find_min_idx(const double *point, int k, int dim, double (*centroids)[dim])
{
    int min_idx = 0;
    double best = 0.0;

    // distance to first centroid
    for (int d = 0; d < dim; ++d) {
        double t = point[d] - centroids[0][d];
        best += t * t;
    }

    // check the rest
    for (int c = 1; c < k; ++c) {
        double dist = 0.0;
        for (int d = 0; d < dim; ++d) {
            double t = point[d] - centroids[c][d];
            dist += t * t;
        }
        if (dist < best) {
            best = dist;
            min_idx = c;
        }
    }
    return min_idx;
}

/**
 * @brief Print k centroids (space-separated per row).
 * @param k Number of centroids.
 * @param dim Dimension per centroid.
 * @param centroids Centroid matrix.
 */
static void print_centroids(int k, int dim, double centroids[k][dim]) {
    for (int c = 0; c < k; ++c) {
        for (int d = 0; d < dim; ++d) {
            printf("%.4f%c", centroids[c][d], d == dim - 1 ? '\n' : ' ');
        }
    }
}
