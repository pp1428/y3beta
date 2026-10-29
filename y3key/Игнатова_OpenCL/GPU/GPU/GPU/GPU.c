#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include <time.h>
#define MAX_SOURCE_SIZE (0x100000)

int main(void) {
	int m;
	printf("Enter matrix size:");
	scanf("%d", &m);
	float* B;
	B = (float*)malloc(m * m * sizeof(float));
	for (int i = 0; i < m; i++) {
		//B[i] = (int*)malloc(m * sizeof(int));
		for (int j = 0; j < m; j++) {
			printf("B[%d][%d]=", i + 1, j + 1);
			scanf("%f", &B[i * m + j]);
		}
	}

	// Load the kernel source code into the array source_str
	FILE* fp;
	char* source_str;
	size_t source_size;
	char kernelname[] = "C:\\Users\\Keyt\\Desktop\\3lab\\GPU\\kernel\\kernel\\kernel.cl";

	fp = fopen(kernelname, "r");

	if (!fp) {
		fprintf(stderr, "Failed to load kernel.\n");
		exit(1);
	}
	source_str = (char*)malloc(MAX_SOURCE_SIZE);
	source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
	fclose(fp);

	// Get platform and device information
	cl_platform_id platform_id = NULL;
	cl_device_id device_id = NULL;
	cl_uint ret_num_devices;
	cl_uint ret_num_platforms;
	cl_uint nP;
	cl_int ret = clGetPlatformIDs(0, NULL, &nP);//nP - кол-во платформ
	printf("Num platforms: %d\n", nP);
	ret = clGetPlatformIDs(nP, &platform_id, NULL);

	/*Platform name*/
	char* platformName = NULL;
	size_t nameSize = 0;
	//Obtaining platform Name Sizes
	cl_int errPlatformNameSize = clGetPlatformInfo(platform_id, CL_PLATFORM_NAME, 0, NULL, &nameSize);
	//Obtaining platform Names
	platformName = (char*)malloc(sizeof(char) * nameSize);
	cl_int errPlatformName = clGetPlatformInfo(platform_id, CL_PLATFORM_NAME, nameSize, platformName, 0);
	printf("Platform name: %s\n", platformName);

	cl_uint nD;
	ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 0, NULL, &nD);
	printf("Num device: %d\n", nD);

	ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, nD,
		&device_id, NULL);

	/*Device info*/
	size_t deviceNameSize = 0;
	cl_int errDeviceNameSize = clGetDeviceInfo(device_id, CL_DEVICE_NAME, 0, NULL, &deviceNameSize);

	char* deviceName = (char*)malloc(sizeof(char) * deviceNameSize);
	cl_int errDeviceName = clGetDeviceInfo(device_id, CL_DEVICE_NAME, deviceNameSize, deviceName, NULL);
	printf("Device name: %s\n", deviceName);
	printf("\n");
	/*Проверка поддерживается ли double*/
	cl_device_fp_config cfg;
	clGetDeviceInfo(device_id, CL_DEVICE_DOUBLE_FP_CONFIG, sizeof(cfg), &cfg, NULL);
	printf("Double FP config = %llu\n", cfg);

	// Create an OpenCL context
	cl_context context = clCreateContext(NULL, nD, &device_id, NULL, NULL, &ret);

	// Create a command queue
	cl_command_queue command_queue = clCreateCommandQueue(context, device_id, 0, &ret);

	// Create memory buffers on the device for each vector
	cl_mem b_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY,
		m * m * 2 * sizeof(cl_float), NULL, &ret);
	if (ret != CL_SUCCESS)
		printf("Creating buffers was unsuccessful.\n");
	cl_mem c_mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE,
		m * m * 2 * sizeof(cl_float), NULL, &ret);
	if (ret != CL_SUCCESS)
		printf("Creating buffers was unsuccessful.\n");
	cl_mem c_copy_mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE,
		m * m * 2 * sizeof(cl_float), NULL, &ret);
	if (ret != CL_SUCCESS)
		printf("Creating buffers was unsuccessful.\n");

	ret = clEnqueueWriteBuffer(command_queue, b_mem_obj, CL_TRUE, 0,
		m * m * 2 * sizeof(float), B, 0, NULL, NULL);

	// Create a program from the kernel source
	cl_program program = clCreateProgramWithSource(context, 1,
		(const char**)&source_str, (const size_t*)&source_size, &ret);

	// Build the program
	ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);


	/*Дополнение единичной матрицей*/
	clock_t start_time = clock();
	cl_kernel kernel = clCreateKernel(program, "vector_add", &ret);

	// Set the arguments of the kernel
	ret = clSetKernelArg(kernel, 0, sizeof(float), (void*)&b_mem_obj);
	ret = clSetKernelArg(kernel, 1, sizeof(float), (void*)&c_mem_obj);
	ret = clSetKernelArg(kernel, 2, sizeof(int), (int*)&m);

	size_t
		global[2] = { m, m * 2 };
	float* C = (float*)malloc(sizeof(float) * m * m * 2);
	cl_event event;
	ret = clEnqueueNDRangeKernel(command_queue, kernel, 2, NULL, global, NULL, 0, NULL, &event);
	clWaitForEvents(1, &event);

	cl_event readevent;
	ret = clEnqueueReadBuffer(command_queue, c_mem_obj, 1, 0, m * m * 2 * sizeof(cl_float), C, 0, 0, &readevent);
	clWaitForEvents(1, &readevent);


	float* C_copy = (float*)malloc(sizeof(float) * m * m * 2);
	for (int i = 0; i < m; i++) {
		for (int j = 0; j < m * 2; j++) {
			C_copy[i * m * 2 + j] = C[i * m * 2 + j];
		}
	}
	ret = clEnqueueWriteBuffer(command_queue, c_copy_mem_obj, CL_TRUE, 0,
		m * m * 2 * sizeof(float), C_copy, 0, NULL, NULL);

	/*Вычисление обратной*/
	for (int i = 0; i < m; i++) {
		float oporn;
		if (i == 0) oporn = 1;
		else oporn = C[(i - 1) * m * 2 + i - 1];//опорный элемент

		cl_kernel kernel2 = clCreateKernel(program, "vector_add2", &ret);
		ret = clSetKernelArg(kernel2, 0, sizeof(float), (void*)&c_mem_obj);
		ret = clSetKernelArg(kernel2, 1, sizeof(int), (int*)&m);
		ret = clSetKernelArg(kernel2, 2, sizeof(float), (void*)&c_copy_mem_obj);
		ret = clSetKernelArg(kernel2, 3, sizeof(float), (float*)&oporn);
		ret = clSetKernelArg(kernel2, 4, sizeof(int), (int*)&i);

		ret = clEnqueueNDRangeKernel(command_queue, kernel2, 2, NULL, global, NULL, 0, NULL, &event);
		clWaitForEvents(1, &event);
		ret = clEnqueueReadBuffer(command_queue, c_copy_mem_obj, CL_TRUE, 0, m * m * 2 * sizeof(int), C_copy, 0, NULL, &readevent);
		clWaitForEvents(1, &readevent);


		for (int i = 0; i < m; i++) {
			for (int j = 0; j < m * 2; j++) {
				C[i * m * 2 + j] = C_copy[i * m * 2 + j];
			}
		}

		ret = clEnqueueWriteBuffer(command_queue, c_mem_obj, CL_TRUE, 0,
			m * m * 2 * sizeof(float), C_copy, 0, NULL, NULL);


	}
	/*Приведение к нужному виду*/
	float determinant = C_copy[0];
	if (determinant == 0) {
		printf("There is no inverse matrix");
		ret = clFlush(command_queue);
		ret = clFinish(command_queue);
		ret = clReleaseKernel(kernel);
		ret = clReleaseProgram(program);
		ret = clReleaseMemObject(b_mem_obj);
		ret = clReleaseMemObject(c_mem_obj);
		ret = clReleaseCommandQueue(command_queue);
		ret = clReleaseContext(context);
		free(C_copy);
		free(B);
		free(C);
		system("pause");
		return 0;
	}
	else {
		cl_kernel kernel3 = clCreateKernel(program, "vector_add3", &ret);
		ret = clSetKernelArg(kernel3, 0, sizeof(float), (void*)&c_copy_mem_obj);
		ret = clSetKernelArg(kernel3, 1, sizeof(int), (int*)&m);
		ret = clSetKernelArg(kernel3, 2, sizeof(float), (float*)&determinant);
		ret = clEnqueueNDRangeKernel(command_queue, kernel3, 2, NULL, global, NULL, 0, NULL, &event);
		clWaitForEvents(1, &event);
		ret = clEnqueueReadBuffer(command_queue, c_copy_mem_obj, CL_TRUE, 0, m * m * 2 * sizeof(int), C_copy, 0, NULL, &readevent);
		clWaitForEvents(1, &readevent);
		printf("Inverse matrix:\n");
		for (int i = 0; i < m; i++) {
			for (int j = m; j < m * 2; j++) {
				printf("%f ", C_copy[i * m * 2 + j]);
			}
			printf("\n");
		}
	}


	clock_t end_time = clock(); // конечное время
	double search_time = ((double)(end_time - start_time) / CLOCKS_PER_SEC); // искомое время
	printf("Time %lf seconds\n", search_time);

	ret = clFlush(command_queue);
	ret = clFinish(command_queue);
	ret = clReleaseKernel(kernel);
	ret = clReleaseProgram(program);
	ret = clReleaseMemObject(b_mem_obj);
	ret = clReleaseMemObject(c_mem_obj);
	ret = clReleaseCommandQueue(command_queue);
	ret = clReleaseContext(context);
	free(B);
	free(C);
	system("pause");
	return 0;
}