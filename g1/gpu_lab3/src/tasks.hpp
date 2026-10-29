#pragma once

#include "md5.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#if LAB_HAS_OPENCL
#include "opencl_runtime.hpp"
#endif

namespace lab {
using Matrix = std::vector<double>;

Matrix make_test_matrix(std::size_t n, std::uint32_t seed);
double determinant_cpu(Matrix matrix, std::size_t n);

#if LAB_HAS_OPENCL
struct DeterminantKernels {
    cl_program program = nullptr;
    cl_kernel pivot = nullptr;
    cl_kernel eliminate = nullptr;
    ~DeterminantKernels();
    DeterminantKernels() = default;
    DeterminantKernels(DeterminantKernels&& other) noexcept;
    DeterminantKernels& operator=(DeterminantKernels&& other) noexcept;
    DeterminantKernels(const DeterminantKernels&) = delete;
    DeterminantKernels& operator=(const DeterminantKernels&) = delete;
};

DeterminantKernels build_determinant_kernels(const OpenCLRuntime& runtime);
double determinant_gpu(const OpenCLRuntime& runtime,
                       const DeterminantKernels& kernels,
                       const Matrix& matrix,
                       std::size_t n);

struct Md5Kernel {
    cl_program program = nullptr;
    cl_kernel kernel = nullptr;
    ~Md5Kernel();
    Md5Kernel() = default;
    Md5Kernel(Md5Kernel&& other) noexcept;
    Md5Kernel& operator=(Md5Kernel&& other) noexcept;
    Md5Kernel(const Md5Kernel&) = delete;
    Md5Kernel& operator=(const Md5Kernel&) = delete;
};
Md5Kernel build_md5_kernel(const OpenCLRuntime& runtime);
BruteForceResult md5_bruteforce_gpu(const OpenCLRuntime& runtime,
                                    const Md5Kernel& kernel,
                                    const Md5Words& target,
                                    const std::string& alphabet,
                                    int min_length,
                                    int max_length,
                                    std::uint64_t max_candidates);
#endif

bool nearly_equal(double a, double b, double relative_tolerance = 1e-8);
} // namespace lab
