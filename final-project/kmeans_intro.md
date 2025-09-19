# K-means - algorithm introduction

## Problem setup

1. Assume we have n points, in d dimensional space: $p_i = (x_1, x_2, ..., x_d)$  
2. The user selects: $k =$ number of clusters (groups), and "iterations" = the maximum number of times the algorithm will run.

---

## Goal

Partition the n points into k clusters such that each point is close to its cluster’s mean (clusters may vary in size).

1. Let $C_j$ be cluster j with size $|C_j|$  
2. Let $\mu_j = (M_{x_{1j}}, M_{x_{2j}}, ..., M_{x_{dj}})$ be the mean coordinates of cluster $j$:

$$
\mu_j =
\left(
\frac{1}{|C_j|} \sum_{p_i \in C_j} x_{1i}, \quad
\frac{1}{|C_j|} \sum_{p_i \in C_j} x_{2i}, \quad
\dots, \quad
\frac{1}{|C_j|} \sum_{p_i \in C_j} x_{di}
\right)
$$

The squared distance of $p_i \in C_j$ from its cluster’s mean is:

$$
d_{p_i}^2 = (x_{1i} - M_{x_{1j}})^2 + (x_{2i} - M_{x_{2j}})^2 + \dots + (x_{di} - M_{x_{dj}})^2
$$

The sum of squared distances for cluster $j$ is:

$$
S_j = \sum_{p_i \in C_j} d_{p_i}^2
$$

We aim to group the $n$ points into $k$ clusters to minimize the **Sum of Squared Errors (SSE):**

$$
\text{SSE} = \min \sum_{j=1}^k S_j
= \min \sum_{j=1}^k \sum_{p_i \in C_j}
\Big[
(x_{1i} - M_{x_{1j}})^2 + (x_{2i} - M_{x_{2j}})^2 + \dots + (x_{di} - M_{x_{dj}})^2
\Big]
$$

---

## Algorithm Example

Assume we have $n = 5$ points in 2D space and $k = 2$:

$$
p_1 = (1, 2) \quad p_2 = (5, 3) \quad p_3 = (2, 3) \quad p_4 = (7, 2) \quad p_5 = (1, 1)
$$


**Step 1 — Initialize centroids**  
Pick the first $k$ points as centroids:  

$$
\mu_1 = (1, 2) \quad \mu_2 = (5, 3)
$$


**Step 2 — Assign each point to its nearest centroid**  
For each point, calculate the squared distance to both centroids and choose the nearest:

$$
\begin{aligned}
p_1 :& d(p_1, \mu_1) = 0 \quad d(p_1, \mu_2) = 17 \quad\Rightarrow\quad p_1 \in C_1 \\
p_2 :& d(p_2, \mu_1) = 17 \quad d(p_2, \mu_2) = 0 \quad\Rightarrow\quad p_2 \in C_2 \\
p_3 :& d(p_3, \mu_1) = 2 \quad d(p_3, \mu_2) = 9 \quad\Rightarrow\quad p_3 \in C_1 \\
p_4 :& d(p_4, \mu_1) = 36 \quad d(p_4, \mu_2) = 5 \quad\Rightarrow\quad p_4 \in C_2 \\
p_5 :& d(p_5, \mu_1) = 1 \quad d(p_5, \mu_2) = 20 \quad\Rightarrow\quad p_5 \in C_1
\end{aligned}
$$


**Step 3 — Update centroids**  
Compute the mean of each cluster:

$$
\mu_1 = \left( \frac{1+2+1}{3}, \frac{2+3+1}{3} \right) = (1.33, 2)
$$

$$
\mu_2 = \left( \frac{5+7}{2}, \frac{3+2}{2} \right) = (6, 2.5)
$$


**Step 4 — Reassign points with updated centroids**  

$$
\begin{aligned}
p_1 :& d(p_1, \mu_1) = 0.11 \quad d(p_1, \mu_2) = 25.25 \quad\Rightarrow\quad p_1 \in C_1 \\
p_2 :& d(p_2, \mu_1) = 14.11 \quad d(p_2, \mu_2) = 1.25 \quad\Rightarrow\quad p_2 \in C_2 \\
p_3 :& d(p_3, \mu_1) = 1.778 \quad d(p_3, \mu_2) = 17.25 \quad\Rightarrow\quad p_3 \in C_1 \\
p_4 :& d(p_4, \mu_1) = 32.11 \quad d(p_4, \mu_2) = 1.25 \quad\Rightarrow\quad p_4 \in C_2 \\
p_5 :& d(p_5, \mu_1) = 1.11 \quad d(p_5, \mu_2) = 26.25 \quad\Rightarrow\quad p_5 \in C_1
\end{aligned}
$$


**Step 5 — Check condition**  
If the assignment is the same as the previous iteration → stop algorithm and return clusters centroids.  
Else → return to Step 2.

---

## Link to K-means demonstration:
[K-means Simulation](http://alekseynp.com/viz/k-means.html)  

---

**Comments**

- The algorithm is finite: $2 < \text{iterations} < k^n$.  
- In some versions (like HW\_1) we set a maximum number of iterations to limit runtime.  
- In each iteration, the SSE decreases or stays the same.  
- For the project: if a cluster becomes empty, keep its centroid from the previous iteration until it gains points again.  
- In each iteration, the SSE decreases or stays the same.  


