// det_gauss_gpu.c
// Сборка: gcc -O2 -o det_gauss_gpu det_gauss_gpu.c -lOpenCL -lm
#define CL_TARGET_OPENCL_VERSION 200

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <CL/cl.h>

#define CHECK_ERROR(err, msg) \
    if (err != CL_SUCCESS) { \
        printf("Error: %s (code %d)\n", msg, err); \
        exit(1); \
    }

static double now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

static void random_matrix(double *A, int n, unsigned seed) {
    srand(seed);
    for (int i = 0; i < n * n; i++) {
        A[i] = (double)rand() / RAND_MAX * 2.0 - 1.0;
    }
}

// Хелпер для загрузки исходника ядра
char* load_kernel_source(const char* filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) { printf("Cannot open %s\n", filename); exit(1); }
    fseek(fp, 0, SEEK_END); size_t size = ftell(fp); rewind(fp);
    char *src = (char*)malloc(size + 1);
    if (fread(src, 1, size, fp) != size) {
        printf("Error reading %s\n", filename);
        exit(1);
    }
    src[size] = '\0';
    fclose(fp); return src;
}

int main(void) {
    cl_int err;

    // --- Инициализация OpenCL ---
    cl_platform_id platform;
    err = clGetPlatformIDs(1, &platform, NULL);
    CHECK_ERROR(err, "clGetPlatformIDs");

    cl_device_id device;
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    CHECK_ERROR(err, "clGetDeviceIDs");

    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    CHECK_ERROR(err, "clCreateContext");

    // По умолчанию очередь in-order (команды выполняются строго последовательно)
    cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, 0, &err);
    CHECK_ERROR(err, "clCreateCommandQueueWithProperties");

    // --- Компиляция ядра ---
    char *src = load_kernel_source("gauss.cl");
    cl_program program = clCreateProgramWithSource(context, 1, (const char**)&src, NULL, &err);
    CHECK_ERROR(err, "clCreateProgramWithSource");

    err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    if (err != CL_SUCCESS) {
        size_t log_size;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        char *log = (char*)malloc(log_size);
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
        printf("Build error:\n%s\n", log);
        free(log); exit(1);
    }

    cl_kernel k_pivot = clCreateKernel(program, "pivot_and_swap", &err);
    CHECK_ERROR(err, "clCreateKernel pivot_and_swap");
    cl_kernel k_elim  = clCreateKernel(program, "eliminate", &err);
    CHECK_ERROR(err, "clCreateKernel eliminate");

    // --- Бенчмарк ---
    printf("=== Benchmark (GPU, OpenCL) ===\n");
    printf("%8s %12s %14s\n", "n", "time, s", "det");

    int sizes[] = {100, 200, 300, 500, 800, 1000, 1500};
    for (int s = 0; s < (int)(sizeof(sizes)/sizeof(sizes[0])); s++) {
        int n = sizes[s];
        size_t mat_size = (size_t)n * n * sizeof(double);

        double *A = (double*)malloc(mat_size);
        random_matrix(A, n, 12345u + (unsigned)n);

        int host_sign = 1;

        // Буферы на GPU (матрица и знак определителя)
        cl_mem d_M = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, mat_size, A, &err);
        CHECK_ERROR(err, "clCreateBuffer d_M");
        cl_mem d_sign = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(int), &host_sign, &err);
        CHECK_ERROR(err, "clCreateBuffer d_sign");

        double t0 = now_sec();

        // Устанавливаем неизменяемые аргументы ядер заранее
        clSetKernelArg(k_pivot, 0, sizeof(cl_mem), &d_M);
        clSetKernelArg(k_pivot, 1, sizeof(cl_mem), &d_sign);
        clSetKernelArg(k_pivot, 2, sizeof(int), &n);

        clSetKernelArg(k_elim, 0, sizeof(cl_mem), &d_M);
        clSetKernelArg(k_elim, 1, sizeof(int), &n);

        size_t g1 = 1, l1 = 1;
        size_t l2 = 64; // Размер рабочей группы (кратно 64 для AMD / 32 для NVIDIA)
        // ВАЖНО: g2 обязан быть кратен l2 в OpenCL! Округляем n вверх до кратного 64:
        size_t g2 = ((size_t)n + l2 - 1) / l2 * l2;

        // Основной цикл Гаусса (выполняется на CPU, ставит ядра в очередь на GPU)
        for (int i = 0; i < n; i++) {
            // Шаг 1: Поиск пивота и перестановка (1 поток)
            err = clSetKernelArg(k_pivot, 3, sizeof(int), &i);
            CHECK_ERROR(err, "clSetKernelArg k_pivot i");
            err = clEnqueueNDRangeKernel(queue, k_pivot, 1, NULL, &g1, &l1, 0, NULL, NULL);
            CHECK_ERROR(err, "clEnqueueNDRangeKernel k_pivot");

            // Шаг 2: Исключение (g2 потоков)
            err = clSetKernelArg(k_elim, 2, sizeof(int), &i);
            CHECK_ERROR(err, "clSetKernelArg k_elim i");
            err = clEnqueueNDRangeKernel(queue, k_elim, 1, NULL, &g2, &l2, 0, NULL, NULL);
            CHECK_ERROR(err, "clEnqueueNDRangeKernel k_elim");
        }

        // Читаем результат и знак обратно
        err = clEnqueueReadBuffer(queue, d_M, CL_TRUE, 0, mat_size, A, 0, NULL, NULL);
        CHECK_ERROR(err, "clEnqueueReadBuffer d_M");
        err = clEnqueueReadBuffer(queue, d_sign, CL_TRUE, 0, sizeof(int), &host_sign, 0, NULL, NULL);
        CHECK_ERROR(err, "clEnqueueReadBuffer d_sign");

        double t1 = now_sec();

        // Считаем определитель (знак * произведение диагонали)
        double det = (double)host_sign;
        if (host_sign != 0) {
            for (int i = 0; i < n; i++) {
                det *= A[i * n + i];
            }
        } else {
            det = 0.0;
        }

        printf("%8d %12.4f %14.6g\n", n, t1 - t0, det);

        free(A);
        clReleaseMemObject(d_M);
        clReleaseMemObject(d_sign);
    }

    // --- Очистка ---
    clReleaseKernel(k_pivot); clReleaseKernel(k_elim);
    clReleaseProgram(program); clReleaseCommandQueue(queue);
    clReleaseContext(context); free(src);

    return 0;
}