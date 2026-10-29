// gauss.cl
// Ядро для распараллеливания метода Гаусса.

#pragma OPENCL EXTENSION cl_khr_fp64 : enable

// Ядро 1: Поиск ведущего элемента и перестановка строк.
// Выполняется всего одним потоком, так как это O(N) и данные уже на GPU.
__kernel void pivot_and_swap(__global double *M, __global int *sign, const int n, const int i) {
    int pivot = i;
    double maxAbs = fabs(M[i * n + i]);
    
    // Ищем максимум в столбце
    for (int k = i + 1; k < n; k++) {
        double v = fabs(M[k * n + i]);
        if (v > maxAbs) {
            maxAbs = v;
            pivot = k;
        }
    }
    
    // Вырожденная матрица
    if (maxAbs < 1e-12) {
        *sign = 0;
        return;
    }

    // Меняем строки местами, если нужно, и меняем знак определителя
    if (pivot != i) {
        for (int j = 0; j < n; j++) {
            double t = M[i * n + j];
            M[i * n + j] = M[pivot * n + j];
            M[pivot * n + j] = t;
        }
        *sign = -(*sign);
    }
}

// Ядро 2: Исключение (обнуление столбца).
// Запускается N потоков (округленных вверх до кратного размеру рабочей группы).
// Каждый поток обрабатывает свою строку k.
__kernel void eliminate(__global double *M, const int n, const int i) {
    int k = get_global_id(0);
    
    // Работаем только со строками ниже главной диагонали в пределах матрицы
    if (k > i && k < n) {
        double piv = M[i * n + i];
        if (fabs(piv) >= 1e-12) {
            double inv_piv = 1.0 / piv;
            double factor = M[k * n + i] * inv_piv;
            if (factor != 0.0) {
                for (int j = i; j < n; j++) {
                    M[k * n + j] -= factor * M[i * n + j];
                }
            }
        }
    }
}