// det_host.c - хост-программа OpenCL для вычисления определителя методом Гаусса
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <CL/cl.h>

#define MAX_KERNEL_SIZE 16384

char* read_kernel_source(const char* path, size_t* outLen) {
    FILE* f = fopen(path, "r");
    char* src = (char*)malloc(MAX_KERNEL_SIZE);
    size_t n = fread(src, 1, MAX_KERNEL_SIZE, f);
    fclose(f);
    *outLen = n;
    return src;
}

int main(void) {
    const int n = 800;                       // размерность матрицы
    double* a = (double*)malloc(sizeof(double) * n * n);
    srand(42);
    for (int i = 0; i < n * n; i++) a[i] = (double)(rand() % 1000) / 10.0 + 1.0;

    // 1. Платформа и устройство
    cl_platform_id platform;
    cl_uint numPlatforms;
    clGetPlatformIDs(1, &platform, &numPlatforms);

    cl_device_id device;
    cl_uint numDevices;
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, &numDevices);

    // 2. Контекст и очередь команд
    cl_int status;
    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &status);
    cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, 0, &status);

    // 3. Буфер матрицы на устройстве
    cl_mem a_buf = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(double) * n * n, NULL, &status);
    clEnqueueWriteBuffer(queue, a_buf, CL_TRUE, 0, sizeof(double) * n * n, a, 0, NULL, NULL);

    // 4. Компиляция ядра
    size_t srcLen;
    char* src = read_kernel_source("det_kernel.cl", &srcLen);
    cl_program program = clCreateProgramWithSource(context, 1, (const char**)&src, &srcLen, &status);
    clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    cl_kernel kernel = clCreateKernel(program, "eliminate_step", &status);

    // 5. Размер рабочей группы: 256 потоков в группе (кратно размеру warp/wavefront)
    size_t localSize = 256;

    // 6. Последовательные шаги исключения k = 0 .. n-2
    for (int k = 0; k < n - 1; k++) {
        int rowsLeft = n - k - 1;                                  // сколько строк осталось обработать
        size_t globalSize = ((rowsLeft + localSize - 1) / localSize) * localSize; // округление вверх до кратного localSize

        clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&a_buf);
        clSetKernelArg(kernel, 1, sizeof(int), (void*)&n);
        clSetKernelArg(kernel, 2, sizeof(int), (void*)&k);

        clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &globalSize, &localSize, 0, NULL, NULL);
    }
    clFinish(queue);

    // 7. Чтение результата и вычисление определителя как произведения диагонали
    clEnqueueReadBuffer(queue, a_buf, CL_TRUE, 0, sizeof(double) * n * n, a, 0, NULL, NULL);
    double det = 1.0;
    for (int i = 0; i < n; i++) det *= a[i * n + i];
    printf("det = %g\n", det);

    clReleaseKernel(kernel); clReleaseProgram(program);
    clReleaseMemObject(a_buf); clReleaseCommandQueue(queue); clReleaseContext(context);
    free(a); free(src);
    return 0;
}
