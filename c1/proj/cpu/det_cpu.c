// Вычисление определителя квадратной матрицы методом Гаусса (LU-разложение с частичным выбором ведущего элемента)
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

double* gen_matrix(int n, unsigned seed) {
    double* a = (double*)malloc(sizeof(double) * n * n);
    srand(seed);
    for (int i = 0; i < n * n; i++) a[i] = (double)(rand() % 1000) / 10.0 + 1.0;
    return a;
}

// Возвращает определитель, приводя матрицу к верхнетреугольному виду методом Гаусса с выбором главного элемента по столбцу
double determinant(double* a, int n) {
    double det = 1.0;
    for (int k = 0; k < n; k++) {
        int piv = k;
        double maxval = fabs(a[k * n + k]);
        for (int i = k + 1; i < n; i++) {
            if (fabs(a[i * n + k]) > maxval) { maxval = fabs(a[i * n + k]); piv = i; }
        }
        if (maxval < 1e-12) return 0.0;
        if (piv != k) {
            for (int j = 0; j < n; j++) {
                double tmp = a[k * n + j]; a[k * n + j] = a[piv * n + j]; a[piv * n + j] = tmp;
            }
            det = -det;
        }
        det *= a[k * n + k];
        for (int i = k + 1; i < n; i++) {
            double factor = a[i * n + k] / a[k * n + k];
            for (int j = k; j < n; j++) a[i * n + j] -= factor * a[k * n + j];
        }
    }
    return det;
}

int main(int argc, char** argv) {
    int sizes[] = {100, 200, 400, 800, 1200, 1600};
    int ns = sizeof(sizes) / sizeof(sizes[0]);
    printf("n;time_ms;det\n");
    for (int s = 0; s < ns; s++) {
        int n = sizes[s];
        double* a = gen_matrix(n, 42 + n);
        clock_t t0 = clock();
        double d = determinant(a, n);
        clock_t t1 = clock();
        double ms = 1000.0 * (double)(t1 - t0) / CLOCKS_PER_SEC;
        printf("%d;%.3f;%.6g\n", n, ms, d);
        free(a);
    }
    return 0;
}
