# SymNMF – algorithm introduction

## Problem setup

1. Assume we have $n$ points in $d$-dimensional space: $p_i = (x_{1i}, x_{2i}, ..., x_{di})$  
2. The user selects:  
   - $k =$ number of clusters (groups).  
   - "iterations" = the maximum number of updates allowed.  
   - Convergence tolerance $\epsilon = 10^{-4}$.

*Note:* in our program we set  iterations = 300 and $\epsilon = 10^{-4}$.

---

## Goal

Partition the $n$ points into $k$ clusters such that:  
- Points inside the same cluster are **highly similar**.  
- Points from different clusters are **dissimilar**.  

This is done by approximating the **normalized similarity matrix** $W$ with a factorization:

$$
W \approx H H^T
$$

where:  
- $H$ is an $n \times k$ non-negative matrix.  
- Each row of $H$ represents how a point belongs to the $k$ clusters.  

---

## Algorithm – Part 1: Constructing the similarity matrices

**Step 1 – Similarity matrix $A$**  
From the input points construct a similarity matrix $A$ defined by:

$$
A_{ij} =
\begin{cases}
e^{-\frac{\lVert x_i - x_j \rVert^2}{2}}, & i \neq j \\
0, & i = j
\end{cases}
$$

- $\lVert x_i - x_j \rVert^2$ is the squared Euclidean distance between points $i$ and $j$.  
- The exponential maps distances into similarities in the range $[0,1]$:  
  - If $x_i$ and $x_j$ are identical → $A_{ij} = 1$.  
  - If $x_i$ and $x_j$ are far apart → $A_{ij} \approx 0$.  

**Properties of $A$:**
1. $A$ is an $n \times n$ square matrix.  
2. Symmetric: $A_{ij} = A_{ji}$.  
3. Set $A_{ii} = 0$ for all $i$ (although mathematically it would be 1).  
4. Off-diagonal entries are in $[0,1]$.  

**Example**  
Assume we have $n = 3$ points in 2D space:  

$$
x_1 = (1,0), \quad x_2 = (0,1), \quad x_3 = (2,2)
$$

Compute squared distances:  

- $\lVert x_1 - x_2 \rVert^2 = (1-0)^2 + (0-1)^2 = 2$  
- $\lVert x_1 - x_3 \rVert^2 = (1-2)^2 + (0-2)^2 = 5$  
- $\lVert x_2 - x_3 \rVert^2 = (0-2)^2 + (1-2)^2 = 5$  

Now construct $A$:  

$$
A =
\begin{pmatrix}
0 & e^{-1} & e^{-2.5} \\
e^{-1} & 0 & e^{-2.5} \\
e^{-2.5} & e^{-2.5} & 0
\end{pmatrix}
$$

Numerical values:  

$$
A \approx
\begin{pmatrix}
0 & 0.3679 & 0.0821 \\
0.3679 & 0 & 0.0821 \\
0.0821 & 0.0821 & 0
\end{pmatrix}
$$

**Step 2 – Diagonal degree matrix $D$**  
From the similarity matrix $A$, construct a diagonal matrix $D$ defined by:

$$
D_{ii} = \sum_{j=1}^{n} A_{ij}, \quad D_{ij} = 0 \ \text{for } i \neq j
$$

- Each diagonal entry $D_{ii}$ is the total similarity of point $i$ to all other points.  
- Off-diagonal entries are always $0$.  

**Properties of $D$:**
1. $D$ is an $n \times n$ diagonal matrix.  
2. $D_{ii} \geq 0$ for all $i$.  
3. Captures the “connectivity strength” of each point.  

**Example**  
Using the similarity matrix $A$ from Step 1:

$$
A \approx
\begin{pmatrix}
0 & 0.3679 & 0.0821 \\
0.3679 & 0 & 0.0821 \\
0.0821 & 0.0821 & 0
\end{pmatrix}
$$

Compute diagonal entries:

- $D_{11} = 0.3679 + 0.0821 = 0.4500$  
- $D_{22} = 0.3679 + 0.0821 = 0.4500$  
- $D_{33} = 0.0821 + 0.0821 = 0.1642$  

So:

$$
D =
\begin{pmatrix}
0.4500 & 0 & 0 \\
0 & 0.4500 & 0 \\
0 & 0 & 0.1642
\end{pmatrix}
$$

**Step 3 – Normalized similarity matrix $W$**  
Define the normalized similarity matrix:

$$
W = D^{-\tfrac{1}{2}} A D^{-\tfrac{1}{2}}
$$

**Why?**  
In the SymNMF algorithm we want to partition the $n$ points into clusters where:  
- **Internal coherence:** points inside a cluster are similar.  
- **External separation:** points from different clusters are distinct.  

**Problem:**  
A “busy” node (large $d_i$) may pull many points into its cluster just because it connects a lot, not because those points are truly close to each other.  

**Need:**  
We want all points to contribute **fairly**, independent of how many strong connections they have.  

**Solution:**  
Matrix $W$ normalizes $A$ using $D$. Clustering on $W$ reflects **real neighborhoods** — sets of points tightly connected among themselves, not just to one popular node.  

**Why it works:**  
Let $D = \mathrm{diag}(d_1, d_2, \dots, d_n)$.

- **Left multiplication ($DA$):** each row $i$ of $A$ is scaled by $d_i$.  
- **Right multiplication ($AD$):** each column $j$ of $A$ is scaled by $d_j$.  

This does not erase the structure of $A$; it only rescales rows or columns.  

In our case:  

$$
W_{ij} = \frac{A_{ij}}{\sqrt{d_i \, d_j}}
$$

So a link between $i$ and $j$ is considered strong in $W$ **only if** their similarity $A_{ij}$ is large *relative to both of their total degrees*.  

**Example**  
From Step 2 we had:

$$
D =
\begin{pmatrix}
0.4500 & 0 & 0 \\
0 & 0.4500 & 0 \\
0 & 0 & 0.1642
\end{pmatrix}
$$

and

$$
D^{-\tfrac{1}{2}} \approx
\begin{pmatrix}
1.4907 & 0 & 0 \\
0 & 1.4907 & 0 \\
0 & 0 & 2.4680
\end{pmatrix}
$$

Using $A$ from Step 1:

$$
A \approx
\begin{pmatrix}
0 & 0.3679 & 0.0821 \\
0.3679 & 0 & 0.0821 \\
0.0821 & 0.0821 & 0
\end{pmatrix}
$$

Now compute:

$$
W = D^{-\tfrac{1}{2}} A D^{-\tfrac{1}{2}}
$$

Result:

$$
W \approx
\begin{pmatrix}
0 & 0.8150 & 0.3016 \\
0.8150 & 0 & 0.3016 \\
0.3016 & 0.3016 & 0
\end{pmatrix}
$$

---

## Algorithm – Part 2: Factorization into $H$ (SymNMF)

**Goal**  
Find a non-negative matrix $H$ of size $n \times k$ such that:

$$
W \approx H H^T
$$

This is done by minimizing the reconstruction error:

$$
\lVert W - H H^T \rVert_F^2
$$

where $\lVert \cdot \rVert_F$ is the Frobenius norm (matrix version of Euclidean length).

**Step 1 — Initialize $H$**  
Randomly initialize $H$ with values from the interval:

$$
\[0 \:\ 2 \sqrt{\tfrac{m}{k}} \]
$$

where $m$ is the average of all entries of $W$.  

- If $H$ values are too small → $H H^T \approx 0$ and cannot match $W$.  
- If $H$ values are too large → $H H^T$ overshoots the scale of $W$.  
- Using $m$ and dividing by $k$ scales $H$ correctly and spreads energy across clusters.  
- Multiplying by 2 broadens the range to avoid uniform initialization.  

Thus, $H$ is not **100% random**: the scale is carefully chosen so $H H^T$ starts on the same order of magnitude as $W$.

**Step 2 — Update $H$ iteratively**  
At each iteration, update $H$ by:

$$
H_{ic} \leftarrow H_{ic} \cdot \frac{(W H)_{ic}}{\big((H H^T) H\big)_{ic}}
$$

- **Numerator $(WH)_{ic}$:** how much the data $W$ suggests point $i$ belongs to cluster $c$.  
- **Denominator $((H H^T)H)_{ic}$:** how much the current guess already explains it.  
- If the ratio $> 1$, $H_{ic}$ increases; if $< 1$, it decreases.  
- Multiplicative updates keep entries non-negative.  

**Step 3 — Convergence**  
Repeat until:  
- The change is small: $\lVert H^{(t+1)} - H^{(t)} \rVert_F < \epsilon$  
- Or the maximum number of iterations is reached.  

*Project defaults:* $\epsilon = 10^{-4}$ and maximum iterations = 300.  

**Step 4 — Derive clusters**  
Convert the soft memberships in $H$ to hard labels:

$$
\text{cluster}(i) = \arg\max_{c} H_{ic}
$$

Each point $i$ is assigned to the cluster where it has the largest membership value.

**Example**  
Suppose $n = 3$, $k = 2$. After convergence:

$$
H \approx
\begin{pmatrix}
0.92 & 0.10 \\
0.88 & 0.12 \\
0.15 & 0.80
\end{pmatrix}
$$

Cluster assignment:  
- Point 1 → cluster 1  
- Point 2 → cluster 1  
- Point 3 → cluster 2 

---

## Comments

- The problem is **non-convex**, meaning the error surface has many local minima rather than one global minimum. As a result, the algorithm can converge to different solutions depending on the initialization of $H$, and no run is guaranteed to find the single best answer.  
- This happens because the way we determine the clusters depends on the chosen metric – in SymNMF this is the Frobenius norm. This metric has its own advantages and disadvantages, and with it we usually only get approximations, not one unique exact partition.  
- To judge if a solution is good, we look at both:  
  1. The reconstruction error $\lVert W - H H^T \rVert_F^2$ (smaller is better).  
  2. The coherence of clusters – high similarities inside clusters and low similarities between clusters.
