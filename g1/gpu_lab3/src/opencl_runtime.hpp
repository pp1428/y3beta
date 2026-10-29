#pragma once

#include <cstddef>
#include <string>
#include <vector>

#ifndef LAB_HAS_OPENCL
#define LAB_HAS_OPENCL 0
#endif

#if LAB_HAS_OPENCL
#include <CL/cl.h>
#endif

namespace lab {
struct DeviceDescription {
    std::string platform;
    std::string name;
    std::string vendor;
    std::string version;
    std::string type;
    bool supports_fp64 = false;
};

std::vector<DeviceDescription> list_opencl_devices();

#if LAB_HAS_OPENCL
class OpenCLRuntime {
public:
    OpenCLRuntime();
    ~OpenCLRuntime();
    OpenCLRuntime(const OpenCLRuntime&) = delete;
    OpenCLRuntime& operator=(const OpenCLRuntime&) = delete;

    cl_context context() const { return context_; }
    cl_command_queue queue() const { return queue_; }
    cl_device_id device() const { return device_; }
    const std::string& device_name() const { return device_name_; }
    bool supports_fp64() const { return supports_fp64_; }
    std::size_t local_work_size() const { return local_work_size_; }
    cl_program build_program_from_file(const std::string& path) const;

private:
    cl_platform_id platform_ = nullptr;
    cl_device_id device_ = nullptr;
    cl_context context_ = nullptr;
    cl_command_queue queue_ = nullptr;
    std::string device_name_;
    bool supports_fp64_ = false;
    std::size_t local_work_size_ = 64;
};

std::string cl_error_name(cl_int error);
void cl_check(cl_int error, const std::string& operation);
#endif

std::string kernel_path(const std::string& filename);
} // namespace lab
