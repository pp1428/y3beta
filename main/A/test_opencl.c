// test_opencl.c
// Базовый тест OpenCL: перемножение двух массивов на GPU.
// Сборка: gcc -O2 -o test_opencl test_opencl.c -lOpenCL
// (Если gcc не находит CL/cl.h, нужно указать пути к заголовкам и библиотекам через -I и -L)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#define CHECK_ERROR(err, msg) \
    if (err != CL_SUCCESS) { \
        printf("Error: %s (code %d)\n", msg, err); \
        exit(1); \
    }

int main(void) {
    cl_int err;

    // 1. Получаем платформу
    cl_platform_id platform;
    err = clGetPlatformIDs(1, &platform, NULL);
    CHECK_ERROR(err, "clGetPlatformIDs");

    // 2. Получаем устройство (GPU)
    cl_device_id device;
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    CHECK_ERROR(err, "clGetDeviceIDs");

    // 3. Создаём контекст и очередь команд
    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    CHECK_ERROR(err, "clCreateContext");

    cl_command_queue queue = clCreateCommandQueue(context, device, 0, &err);
    CHECK_ERROR(err, "clCreateCommandQueue");

    // 4. Читаем исходник ядра из файла
    FILE *fp = fopen("add.cl", "rb");
    if (!fp) {
        printf("Error: Cannot open add.cl\n");
        return 1;
    }
    fseek(fp, 0, SEEK_END);
    size_t source_size = ftell(fp);
    rewind(fp);
    char *source_str = (char *)malloc(source_size + 1);
    fread(source_str, 1, source_size, fp);
    source_str[source_size] = '\0';
    fclose(fp);

    // 5. Собираем программу
    cl_program program = clCreateProgramWithSource(context, 1, (const char **)&source_str, &source_size, &err);
    CHECK_ERROR(err, "clCreateProgramWithSource");

    err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    if (err != CL_SUCCESS) {
        // Если сборка упала, выводим лог ошибки
        size_t log_size;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        char *log = (char *)malloc(log_size);
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
        printf("Build error:\n%s\n", log);
        free(log);
        exit(1);
    }

    // 6. Создаём ядро
    cl_kernel kernel = clCreateKernel(program, "multiply_arrays", &err);
    CHECK_ERROR(err, "clCreateKernel");

    // 7. Данные на хосте (CPU)
    const int N = 1024;
    int *A = (int *)malloc(N * sizeof(int));
    int *B = (int *)malloc(N * sizeof(int));
    int *C = (int *)malloc(N * sizeof(int));
    
    for (int i = 0; i < N; i++) {
        A[i] = i;
        B[i] = 2 * i;
    }

    // 8. Буферы на устройстве (GPU)
    cl_mem d_A = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, N * sizeof(int), A, &err);
    cl_mem d_B = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, N * sizeof(int), B, &err);
    cl_mem d_C = clCreateBuffer(context, CL_MEM_WRITE_ONLY, N * sizeof(int), NULL, &err);

    // 9. Аргументы ядра
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_A);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &d_B);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), &d_C);
    clSetKernelArg(kernel, 3, sizeof(int), &N);

    // 10. Запуск
    size_t global_work_size = N;
    size_t local_work_size = 64; // Кратно 64 для AMD
    
    err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_work_size, &local_work_size, 0, NULL, NULL);
    CHECK_ERROR(err, "clEnqueueNDRangeKernel");

    // 11. Чтение результата
    err = clEnqueueReadBuffer(queue, d_C, CL_TRUE, 0, N * sizeof(int), C, 0, NULL, NULL);
    CHECK_ERROR(err, "clEnqueueReadBuffer");

    // 12. Проверка результатов
    int correct = 1;
    for (int i = 0; i < N; i++) {
        if (C[i] != A[i] * B[i]) {
            correct = 0;
            printf("Mismatch at %d: got %d, expected %d\n", i, C[i], A[i] * B[i]);
            break;
        }
    }

    if (correct) {
        printf("Success! All %d elements are correct.\n", N);
    }

    // 13. Очистка ресурсов
    clReleaseMemObject(d_A);
    clReleaseMemObject(d_B);
    clReleaseMemObject(d_C);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    free(source_str);
    free(A); free(B); free(C);

    return 0;
}
