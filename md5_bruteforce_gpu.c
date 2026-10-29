// md5_bruteforce_gpu.c
#define CL_TARGET_OPENCL_VERSION 200

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
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

char* load_kernel_source(const char* filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) { printf("Cannot open %s\n", filename); exit(1); }
    fseek(fp, 0, SEEK_END); size_t size = ftell(fp); rewind(fp);
    char *src = (char*)malloc(size + 1);
    fread(src, 1, size, fp); src[size] = '\0';
    fclose(fp); return src;
}

int main(void) {
    cl_int err;
    cl_platform_id platform;
    clGetPlatformIDs(1, &platform, NULL);
    cl_device_id device;
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, 0, &err);

    char *src = load_kernel_source("md5_kernel.cl");
    cl_program program = clCreateProgramWithSource(context, 1, (const char**)&src, NULL, &err);
    err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    if (err != CL_SUCCESS) {
        size_t log_size;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        char *log = (char*)malloc(log_size);
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
        printf("Build error:\n%s\n", log);
        free(log); exit(1);
    }

    cl_kernel kernel = clCreateKernel(program, "md5_bruteforce", &err);
    CHECK_ERROR(err, "clCreateKernel");

    printf("=== MD5 Brute-force (GPU) ===\n");

    // Исправлено: правильный порядок байт для MD5("123456")
    uint32_t target_hash[4] = {0x39dc0ae1, 0xab59ba49, 0x57e056be, 0x3e880ff2};
    
    cl_mem d_target = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(target_hash), target_hash, &err);
    cl_mem d_found_flag = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int), NULL, &err);
    cl_mem d_found_val = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(uint64_t), NULL, &err);

    int zero = 0;
    clEnqueueWriteBuffer(queue, d_found_flag, CL_TRUE, 0, sizeof(int), &zero, 0, NULL, NULL);

    uint64_t max_range = 1000000; // Ищем в диапазоне до 1 млн
    uint64_t start_idx = 0;
    
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_target);
    clSetKernelArg(kernel, 1, sizeof(uint64_t), &start_idx);
    clSetKernelArg(kernel, 2, sizeof(uint64_t), &max_range);
    clSetKernelArg(kernel, 3, sizeof(cl_mem), &d_found_flag);
    clSetKernelArg(kernel, 4, sizeof(cl_mem), &d_found_val);

    size_t global_size = ((max_range + 63) / 64) * 64;
    size_t local_size = 64;

    printf("Searching for '123456' in range [0, %llu]...\n", (unsigned long long)max_range);
    
    double t0 = now_sec();
    clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);
    clFinish(queue); 
    double t1 = now_sec();

    int found_flag = 0;
    uint64_t found_val = 0;
    clEnqueueReadBuffer(queue, d_found_flag, CL_TRUE, 0, sizeof(int), &found_flag, 0, NULL, NULL);
    
    if (found_flag) {
        clEnqueueReadBuffer(queue, d_found_val, CL_TRUE, 0, sizeof(uint64_t), &found_val, 0, NULL, NULL);
        printf("Found! x = %llu, time = %.4f s\n", (unsigned long long)found_val, t1 - t0);
        printf("Speed: %.2f MH/s\n", (double)max_range / (t1 - t0) / 1e6);
    } else {
        printf("Not found in range. Time: %.4f s\n", t1 - t0);
    }

    // Бенчмарк на разных диапазонах
    printf("\n=== Benchmark (GPU) ===\n");
    printf("%12s %12s %14s\n", "range", "time, s", "hashes/sec");
    
    uint64_t sizes[] = {100000, 500000, 1000000, 5000000, 10000000};
    
    // Ставим несуществующий хэш, чтобы он перебрал весь диапазон до конца
    uint32_t dummy_target[4] = {0, 0, 0, 0}; 
    clEnqueueWriteBuffer(queue, d_target, CL_TRUE, 0, sizeof(dummy_target), dummy_target, 0, NULL, NULL);

    for (int i = 0; i < 5; i++) {
        uint64_t range = sizes[i];
        clEnqueueWriteBuffer(queue, d_found_flag, CL_TRUE, 0, sizeof(int), &zero, 0, NULL, NULL);
        
        // ИСПРАВЛЕНО: Обновляем аргумент max_range для ядра!
        clSetKernelArg(kernel, 2, sizeof(uint64_t), &range);
        
        global_size = ((range + 63) / 64) * 64;
        
        t0 = now_sec();
        clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);
        clFinish(queue);
        t1 = now_sec();

        double elapsed = t1 - t0;
        printf("%12llu %12.4f %14.0f\n", 
            (unsigned long long)range, elapsed, (double)range / elapsed);
    }

    clReleaseMemObject(d_target); clReleaseMemObject(d_found_flag); clReleaseMemObject(d_found_val);
    clReleaseKernel(kernel); clReleaseProgram(program); 
    clReleaseCommandQueue(queue); clReleaseContext(context); free(src);

    return 0;
}
