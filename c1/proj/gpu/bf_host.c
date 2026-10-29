// bf_host.c - хост-программа OpenCL для перебора паролей по SHA-256
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <CL/cl.h>
#include "sha256.h"

#define MAX_KERNEL_SIZE 16384
#define ALPH_LEN 36

char* read_kernel_source(const char* path, size_t* outLen) {
    FILE* f = fopen(path, "r");
    char* src = (char*)malloc(MAX_KERNEL_SIZE);
    size_t n = fread(src, 1, MAX_KERNEL_SIZE, f);
    fclose(f);
    *outLen = n;
    return src;
}

int main(void) {
    int passLen = 5;                      // длина перебираемого пароля
    const char* target = "zzzzz";         // искомый пароль (для проверки корректности)
    char targetHex[65];
    sha256_hex(target, passLen, targetHex);

    unsigned char targetBytes[32];
    for (int i = 0; i < 32; i++) {
        sscanf(targetHex + i * 2, "%2hhx", &targetBytes[i]);
    }

    unsigned long long space = 1;
    for (int i = 0; i < passLen; i++) space *= ALPH_LEN;

    // 1. Платформа, устройство, контекст, очередь
    cl_platform_id platform; cl_uint numPlatforms;
    clGetPlatformIDs(1, &platform, &numPlatforms);
    cl_device_id device; cl_uint numDevices;
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, &numDevices);
    cl_int status;
    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &status);
    cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, 0, &status);

    // 2. Буферы: целевой хэш и индекс найденного кандидата
    cl_mem hashBuf = clCreateBuffer(context, CL_MEM_READ_ONLY, 32, NULL, &status);
    clEnqueueWriteBuffer(queue, hashBuf, CL_TRUE, 0, 32, targetBytes, 0, NULL, NULL);

    cl_long notFound = -1;
    cl_mem foundBuf = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(cl_long), NULL, &status);
    clEnqueueWriteBuffer(queue, foundBuf, CL_TRUE, 0, sizeof(cl_long), &notFound, 0, NULL, NULL);

    // 3. Сборка ядра
    size_t srcLen;
    char* src = read_kernel_source("bf_kernel.cl", &srcLen);
    cl_program program = clCreateProgramWithSource(context, 1, (const char**)&src, &srcLen, &status);
    clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    cl_kernel kernel = clCreateKernel(program, "bruteforce", &status);

    // 4. Расчёт количества блоков и потоков:
    //    пространство перебора разбивается на 2D NDRange, чтобы не превысить
    //    максимально допустимый размер одного измерения (CL_DEVICE_MAX_WORK_ITEM_SIZES);
    //    локальный размер группы (блок) фиксирован — 256 потоков (кратно размеру warp/wavefront),
    //    глобальный размер округляется вверх до кратного локальному.
    size_t localSize[2] = { 256, 1 };
    size_t dimX = 65536; // ширина решётки потоков по измерению 0
    unsigned long long chunksTotal = (space + dimX - 1) / dimX;
    size_t globalSize[2] = {
        dimX,
        ((chunksTotal + localSize[0] - 1) / localSize[0]) * localSize[0]
    };

    clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&hashBuf);
    clSetKernelArg(kernel, 1, sizeof(int), (void*)&passLen);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), (void*)&foundBuf);
    cl_ulong offset = 0;
    clSetKernelArg(kernel, 3, sizeof(cl_ulong), (void*)&offset);

    clEnqueueNDRangeKernel(queue, kernel, 2, NULL, globalSize, localSize, 0, NULL, NULL);
    clFinish(queue);

    cl_long found;
    clEnqueueReadBuffer(queue, foundBuf, CL_TRUE, 0, sizeof(cl_long), &found, 0, NULL, NULL);
    printf("found index = %lld (space = %llu)\n", (long long)found, space);

    clReleaseKernel(kernel); clReleaseProgram(program);
    clReleaseMemObject(hashBuf); clReleaseMemObject(foundBuf);
    clReleaseCommandQueue(queue); clReleaseContext(context);
    free(src);
    return 0;
}
