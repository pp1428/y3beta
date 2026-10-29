#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <CL/cl.h>



#define MAX_KERNEL_SIZE	10000

char hashh[100] = "\0\0\0\0\0";	
int size = 1000000;			

void error(char message[], char errorCode[]) {
	printf("%s", message);
	//exit(1);
}


//Считывание из консоли
void readConsole() {
	printf("write output hash\n");
	gets(hashh);
	printf("%s\n\n", hashh);

	
	////exit(4);
	
}


int main(void) {
	printf("start\n\n");

	readConsole();

	clock_t start_time = clock();

	// Загрузка кода ядра в строковый массив
	FILE* file = fopen("kernel.cl", "r");
	if (!file) printf("Error: opening the file\n");
	char* kernelSRC = (char*)malloc(MAX_KERNEL_SIZE);
	if (!kernelSRC) printf("Error: loading kernel code\n");
	size_t kernelSize = fread(kernelSRC, 1, MAX_KERNEL_SIZE, file);
	fclose(file);

	// Получение информации о доступных платформах
	cl_platform_id* platforms = NULL;
	cl_uint platformsNum;
	cl_int clStatus = clGetPlatformIDs(0, NULL, &platformsNum);
	platforms = (cl_platform_id*)malloc(sizeof(cl_platform_id) * platformsNum);
	clStatus = clGetPlatformIDs(platformsNum, platforms, NULL);

	// Получение списка устройств и выбор устройства для исполнения кода
	cl_device_id* deviceList = NULL;
	cl_uint devicesNum;
	clStatus = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, 0, NULL, &devicesNum);
	deviceList = (cl_device_id*)malloc(sizeof(cl_device_id) * devicesNum);
	clStatus = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, devicesNum, deviceList, NULL);

	// Создание контекста для каждого устройства
	cl_context context = clCreateContext(NULL, devicesNum, deviceList, NULL, NULL, &clStatus);
	if (clStatus != CL_SUCCESS) printf("Error: creating context\n");

	// Создание очереди команд
	cl_command_queue commandQueue = clCreateCommandQueueWithProperties(context, deviceList[0], 0, &clStatus);
	if (clStatus != CL_SUCCESS) printf("Error: creating command queue\n");

	// Создание буфера памяти для каждого вектора
	cl_mem hashh_clmem = clCreateBuffer(context, CL_MEM_READ_WRITE, 100 * sizeof(char), NULL, &clStatus);
	if (clStatus != CL_SUCCESS) printf("Error: creating buffer\n");

	// Скопировать буфер A на устройство
	clStatus = clEnqueueWriteBuffer(commandQueue, hashh_clmem, CL_TRUE, 0, 100 * sizeof(char), hashh, 0, NULL, NULL);
	if (clStatus != CL_SUCCESS) printf("Error: write buffer\n");

	// Создать программу kernelSquare
	cl_program program = clCreateProgramWithSource(context, 1, (const char**)&kernelSRC, (const size_t*)&kernelSize, &clStatus);
	if (clStatus != CL_SUCCESS) printf("Error: creating program\n");

	// Собрать программу
	clStatus = clBuildProgram(program, 1, deviceList, NULL, NULL, NULL);
	if (clStatus != CL_SUCCESS) printf("Error: build program\n");


	cl_event event;
	size_t globalSize[2] = { 256, 256 };
	cl_event eventRead = NULL;

	for (int i = 0; i < size; i++) {
		// Создать процесс на устройстве
		cl_kernel kernel = clCreateKernel(program, "Counting", &clStatus);
		if (clStatus != CL_SUCCESS) printf("Error: creating kernel\n");

		// Передать аргументы в программу
		clStatus = clSetKernelArg(kernel, 0, sizeof(hashh_clmem), (void*)&hashh_clmem);
		if (clStatus != CL_SUCCESS) printf("Error: set arg 0");
		clStatus = clSetKernelArg(kernel, 1, sizeof(int), (int*)&size);
		if (clStatus != CL_SUCCESS) printf("Error: set arg 1");
		clStatus = clSetKernelArg(kernel, 2, sizeof(int), (int*)&i);
		if (clStatus != CL_SUCCESS) printf("Error: set arg 2");

		//Поставимть в очередь
		clStatus = clEnqueueNDRangeKernel(commandQueue, kernel, 2, NULL, globalSize, NULL, 0, NULL, &event);
		if (clStatus != CL_SUCCESS) printf("Error: enqueue\n");
		clWaitForEvents(1, &event);

		//Скопировать буфер на хостовое устройство
		clStatus = clEnqueueReadBuffer(commandQueue, hashh_clmem, CL_TRUE, 0, 100* sizeof(char), hashh, 0, 0, &eventRead);
		if (clStatus != CL_SUCCESS) printf("Error: read buffer\n");
		clWaitForEvents(1, &eventRead);

		clStatus = clReleaseKernel(kernel);
		if (hashh[70] == '1') {
			i = 90000000;
		}
		//printf("test buf !%c!\n", hashh[0]);
	}

	// Ожидание завершения всех команд
	clStatus = clFlush(commandQueue);
	clStatus = clFinish(commandQueue);

	if (hashh[70] == '1') {
		printf("%c%c%c%c%c\n", hashh[71], hashh[72], hashh[73], hashh[74], hashh[75]);
	}
	else {
		printf("ne poluchilos :(\n");
	}
	clock_t end_time = clock();
	
	printf("The execution time = %lf seconds\n", (double)(end_time - start_time) / CLOCKS_PER_SEC);

	

	// Освобождение памяти
	clStatus = clReleaseProgram(program);
	clStatus = clReleaseMemObject(hashh_clmem);
	clStatus = clReleaseCommandQueue(commandQueue);
	clStatus = clReleaseContext(context);
	//free(hashh);
	free(platforms);
	free(deviceList);

	system("pause");
	return 0;
}

