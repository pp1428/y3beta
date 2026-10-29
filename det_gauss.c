// det_gauss.c
// Вычисление определителя квадратной матрицы методом Гаусса
// с частичным выбором главного элемента по столбцу.
// Сборка: gcc -O2 -o det_gauss det_gauss.c -lm

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

static double now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

// ---- Определитель методом Гаусса ----
// Сложность: O(n^3). Память: O(n^2) (копия).
double det_gauss(const double *A, int n) {
    double *M = malloc((size_t)n * n * sizeof(double));
    if (!M) return NAN;
    for (int i = 0; i < n * n; i++) M[i] = A[i];

    double det = 1.0;
    int sign = 1;

    for (int i = 0; i < n; i++) {
        // Поиск ведущего элемента (по модулю)
        int pivot = i;
        double maxAbs = fabs(M[i * n + i]);
        for (int k = i + 1; k < n; k++) {
            double v = fabs(M[k * n + i]);
            if (v > maxAbs) { maxAbs = v; pivot = k; }
        }

        // Вырожденная матрица
        if (maxAbs < 1e-12) {
            free(M);
            return 0.0;
        }

        // Перестановка строк — знак меняется
        if (pivot != i) {
            for (int j = 0; j < n; j++) {
                double t = M[i * n + j];
                M[i * n + j] = M[pivot * n + j];
                M[pivot * n + j] = t;
            }
            sign = -sign;
        }

        // Исключение под ведущим элементом
        double inv_piv = 1.0 / M[i * n + i];
        for (int k = i + 1; k < n; k++) {
            double factor = M[k * n + i] * inv_piv;
            if (factor == 0.0) continue;
            for (int j = i; j < n; j++) {
                M[k * n + j] -= factor * M[i * n + j];
            }
        }
    }

    det = (double)sign;
    for (int i = 0; i < n; i++) det *= M[i * n + i];

    free(M);
    return det;
}

// ---- Генерация случайной матрицы ----
static void random_matrix(double *A, int n, unsigned seed) {
    srand(seed);
    for (int i = 0; i < n * n; i++) {
        A[i] = (double)rand() / RAND_MAX * 2.0 - 1.0; // [-1, 1]
    }
}

// ---- Эталонный определитель для маленьких матриц: явная формула ----
// (для n=1,2,3 — чтобы проверить корректность без numpy)
static double det_explicit(const double *A, int n) {
    if (n == 1) return A[0];
    if (n == 2) return A[0]*A[3] - A[1]*A[2];
    if (n == 3) {
        return A[0]*(A[4]*A[8] - A[5]*A[7])
             - A[1]*(A[3]*A[8] - A[5]*A[6])
             + A[2]*(A[3]*A[7] - A[4]*A[6]);
    }
    return NAN;
}

int main(void) {
    // ---------- Тесты корректности ----------
    printf("=== Correctness check ===\n");

    double eye3[9] = {1,0,0, 0,1,0, 0,0,1};
    printf("Identity 3x3: det = %g (expected 1)\n", det_gauss(eye3, 3));

    double a3[9] = {1,2,3, 4,5,6, 7,8,9};
    printf("Singular 3x3: det = %g (expected 0)\n", det_gauss(a3, 3));

    double b3[9] = {2,-1,0, -1,2,-1, 0,-1,2};
    printf("Tridiagonal 3x3: det = %g (expected 4)\n", det_gauss(b3, 3));

    // Случайные матрицы: сравниваем с явной формулой
    int ok = 1;
    for (int n = 1; n <= 3; n++) {
        double A[9];
        random_matrix(A, n, 42 + n);
        double d1 = det_gauss(A, n);
        double d2 = det_explicit(A, n);
        if (fabs(d1 - d2) > 1e-9) {
            printf("MISMATCH n=%d: gauss=%.10f explicit=%.10f\n", n, d1, d2);
            ok = 0;
        }
    }
    printf(ok ? "Random 1..3x3: OK\n" : "Random 1..3x3: FAIL\n");

    // ---------- Бенчмарк ----------
    printf("\n=== Benchmark (CPU, one thread) ===\n");
    printf("%8s %12s %14s\n", "n", "time, s", "det");

    int sizes[] = {100, 200, 300, 500, 800, 1000, 1500};
    for (int s = 0; s < (int)(sizeof(sizes)/sizeof(sizes[0])); s++) {
        int n = sizes[s];
        double *A = malloc((size_t)n * n * sizeof(double));
        random_matrix(A, n, 12345u + (unsigned)n);

        double t0 = now_sec();
        double d = det_gauss(A, n);
        double t1 = now_sec();

        printf("%8d %12.4f %14.6g\n", n, t1 - t0, d);
        free(A);
    }
    return 0;
}