#include "opencl_runtime.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

#ifndef LAB_KERNEL_DIR
#define LAB_KERNEL_DIR "kernels"
#endif

namespace lab {
#if LAB_HAS_OPENCL
namespace {
std::string platform_string(cl_platform_id platform, cl_platform_info field) {
    std::size_t size = 0;
    if (clGetPlatformInfo(platform, field, 0, nullptr, &size) != CL_SUCCESS || size == 0) return "unknown";
    std::string value(size, '\0');
    if (clGetPlatformInfo(platform, field, size, value.data(), nullptr) != CL_SUCCESS) return "unknown";
    if (!value.empty() && value.back() == '\0') value.pop_back();
    return value;
}

std::string device_string(cl_device_id device, cl_device_info field) {
    std::size_t size = 0;
    if (clGetDeviceInfo(device, field, 0, nullptr, &size) != CL_SUCCESS || size == 0) return "unknown";
    std::string value(size, '\0');
    if (clGetDeviceInfo(device, field, size, value.data(), nullptr) != CL_SUCCESS) return "unknown";
    if (!value.empty() && value.back() == '\0') value.pop_back();
    return value;
}

bool fp64_supported(cl_device_id device) {
    const std::string extensions = device_string(device, CL_DEVICE_EXTENSIONS);
    return extensions.find("cl_khr_fp64") != std::string::npos
        || extensions.find("cl_amd_fp64") != std::string::npos;
}

std::vector<cl_platform_id> get_platforms() {
    cl_uint count = 0;
    const cl_int err = clGetPlatformIDs(0, nullptr, &count);
    if (err != CL_SUCCESS || count == 0) return {};
    std::vector<cl_platform_id> platforms(count);
    if (clGetPlatformIDs(count, platforms.data(), nullptr) != CL_SUCCESS) return {};
    return platforms;
}

std::string device_type_name(cl_device_type type) {
    if (type & CL_DEVICE_TYPE_GPU) return "GPU";
    if (type & CL_DEVICE_TYPE_CPU) return "CPU OpenCL";
    if (type & CL_DEVICE_TYPE_ACCELERATOR) return "Accelerator";
    return "Other OpenCL device";
}
} // namespace
#endif

std::vector<DeviceDescription> list_opencl_devices() {
#if !LAB_HAS_OPENCL
    return {};
#else
    std::vector<DeviceDescription> result;
    for (const auto platform : get_platforms()) {
        cl_uint count = 0;
        if (clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, nullptr, &count) != CL_SUCCESS || count == 0) continue;
        std::vector<cl_device_id> devices(count);
        if (clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, count, devices.data(), nullptr) != CL_SUCCESS) continue;
        for (const auto device : devices) {
            cl_device_type type = 0;
            clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(type), &type, nullptr);
            result.push_back({
                platform_string(platform, CL_PLATFORM_NAME),
                device_string(device, CL_DEVICE_NAME),
                device_string(device, CL_DEVICE_VENDOR),
                device_string(device, CL_DEVICE_VERSION),
                device_type_name(type),
                fp64_supported(device)
            });
        }
    }
    return result;
#endif
}

std::string kernel_path(const std::string& filename) {
    return std::string(LAB_KERNEL_DIR) + "/" + filename;
}

#if LAB_HAS_OPENCL
std::string cl_error_name(cl_int error) {
    switch (error) {
        case CL_SUCCESS: return "CL_SUCCESS";
        case CL_DEVICE_NOT_FOUND: return "CL_DEVICE_NOT_FOUND";
        case CL_DEVICE_NOT_AVAILABLE: return "CL_DEVICE_NOT_AVAILABLE";
        case CL_COMPILER_NOT_AVAILABLE: return "CL_COMPILER_NOT_AVAILABLE";
        case CL_MEM_OBJECT_ALLOCATION_FAILURE: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
        case CL_OUT_OF_RESOURCES: return "CL_OUT_OF_RESOURCES";
        case CL_OUT_OF_HOST_MEMORY: return "CL_OUT_OF_HOST_MEMORY";
        case CL_BUILD_PROGRAM_FAILURE: return "CL_BUILD_PROGRAM_FAILURE";
        case CL_INVALID_VALUE: return "CL_INVALID_VALUE";
        case CL_INVALID_DEVICE: return "CL_INVALID_DEVICE";
        case CL_INVALID_CONTEXT: return "CL_INVALID_CONTEXT";
        case CL_INVALID_COMMAND_QUEUE: return "CL_INVALID_COMMAND_QUEUE";
        case CL_INVALID_MEM_OBJECT: return "CL_INVALID_MEM_OBJECT";
        case CL_INVALID_PROGRAM: return "CL_INVALID_PROGRAM";
        case CL_INVALID_KERNEL: return "CL_INVALID_KERNEL";
        case CL_INVALID_ARG_INDEX: return "CL_INVALID_ARG_INDEX";
        case CL_INVALID_WORK_GROUP_SIZE: return "CL_INVALID_WORK_GROUP_SIZE";
        default: return "OpenCL error " + std::to_string(error);
    }
}

void cl_check(cl_int error, const std::string& operation) {
    if (error != CL_SUCCESS) {
        throw std::runtime_error(operation + ": " + cl_error_name(error));
    }
}

OpenCLRuntime::OpenCLRuntime() {
    for (const auto platform : get_platforms()) {
        cl_uint count = 0;
        const cl_int count_error = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, nullptr, &count);
        if (count_error != CL_SUCCESS || count == 0) continue;
        std::vector<cl_device_id> devices(count);
        if (clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, count, devices.data(), nullptr) != CL_SUCCESS) continue;
        platform_ = platform;
        device_ = devices.front();
        break;
    }
    if (device_ == nullptr) {
        throw std::runtime_error("OpenCL GPU не найден. Установите драйвер производителя GPU и проверьте команду `lab3 devices`.");
    }

    device_name_ = device_string(device_, CL_DEVICE_NAME);
    supports_fp64_ = fp64_supported(device_);
    std::size_t max_work_group_size = 64;
    if (clGetDeviceInfo(device_, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(max_work_group_size), &max_work_group_size, nullptr) == CL_SUCCESS) {
        local_work_size_ = std::max<std::size_t>(1, std::min<std::size_t>(64, max_work_group_size));
    }
    cl_int error = CL_SUCCESS;
    context_ = clCreateContext(nullptr, 1, &device_, nullptr, nullptr, &error);
    cl_check(error, "clCreateContext");
    queue_ = clCreateCommandQueue(context_, device_, 0, &error);
    if (error != CL_SUCCESS) {
        if (context_) clReleaseContext(context_);
        context_ = nullptr;
        cl_check(error, "clCreateCommandQueue");
    }
}

OpenCLRuntime::~OpenCLRuntime() {
    if (queue_) clReleaseCommandQueue(queue_);
    if (context_) clReleaseContext(context_);
}

cl_program OpenCLRuntime::build_program_from_file(const std::string& path) const {
    std::ifstream input(path, std::ios::binary);
    if (!input) throw std::runtime_error("Не удалось прочитать OpenCL-ядро: " + path);
    std::ostringstream contents;
    contents << input.rdbuf();
    const std::string source = contents.str();
    const char* source_ptr = source.c_str();
    const std::size_t source_len = source.size();
    cl_int error = CL_SUCCESS;
    cl_program program = clCreateProgramWithSource(context_, 1, &source_ptr, &source_len, &error);
    cl_check(error, "clCreateProgramWithSource");
    error = clBuildProgram(program, 1, &device_, "", nullptr, nullptr);
    if (error != CL_SUCCESS) {
        std::size_t log_size = 0;
        clGetProgramBuildInfo(program, device_, CL_PROGRAM_BUILD_LOG, 0, nullptr, &log_size);
        std::string log(log_size, '\0');
        if (log_size) clGetProgramBuildInfo(program, device_, CL_PROGRAM_BUILD_LOG, log_size, log.data(), nullptr);
        clReleaseProgram(program);
        throw std::runtime_error("Не удалось собрать OpenCL-ядро из " + path + ":\n" + log);
    }
    return program;
}
#endif
} // namespace lab
