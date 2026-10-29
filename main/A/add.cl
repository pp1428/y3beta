// add.cl
// Ядро OpenCL: поэлементное перемножение двух массивов.
// Вызывается для каждого индекса независимо.

__kernel void multiply_arrays(__global const int *A,
                              __global const int *B,
                              __global int *C,
                              const int N) {
    // Получаем глобальный ID текущего потока
    int id = get_global_id(0);
    
    // Защита от выхода за границы массива
    if (id < N) {
        C[id] = A[id] * B[id];
    }
}
