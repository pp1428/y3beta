#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void pivot_swap(__global double* matrix,
                         const int n,
                         const int k,
                         __global int* sign_state) {
    if (get_global_id(0) != 0) return;

    int pivot_row = k;
    double maximum = fabs(matrix[k * n + k]);
    for (int row = k + 1; row < n; ++row) {
        const double value = fabs(matrix[row * n + k]);
        if (value > maximum) {
            maximum = value;
            pivot_row = row;
        }
    }

    if (maximum < 1.0e-30) {
        matrix[k * n + k] = 0.0;
        return;
    }

    if (pivot_row != k) {
        for (int col = 0; col < n; ++col) {
            const double tmp = matrix[k * n + col];
            matrix[k * n + col] = matrix[pivot_row * n + col];
            matrix[pivot_row * n + col] = tmp;
        }
        sign_state[0] = -sign_state[0];
    }
}

__kernel void eliminate_submatrix(__global double* matrix,
                                  const int n,
                                  const int k) {
    const size_t remaining = (size_t)(n - k - 1);
    const size_t gid = get_global_id(0);
    if (remaining == 0 || gid >= remaining * remaining) return;

    const int row = k + 1 + (int)(gid / remaining);
    const int col = k + 1 + (int)(gid % remaining);
    const double pivot = matrix[k * n + k];
    if (fabs(pivot) < 1.0e-30) return;

    const double factor = matrix[row * n + k] / pivot;
    matrix[row * n + col] -= factor * matrix[k * n + col];
}
