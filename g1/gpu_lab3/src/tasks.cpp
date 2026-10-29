#include "tasks.hpp"
#include "opencl_runtime.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <random>
#include <stdexcept>

namespace lab {
Matrix make_test_matrix(std::size_t n, std::uint32_t seed) {
    if (n == 0 || n > 4096) throw std::invalid_argument("Размер матрицы должен быть в диапазоне 1..4096");
    Matrix matrix(n * n, 0.0);
    std::mt19937 generator(seed);
    std::uniform_real_distribution<double> distribution(-1.0, 1.0);
    const double scale = 0.25 / static_cast<double>(n);

    for (std::size_t row = 0; row < n; ++row) {
        double row_sum = 0.0;
        for (std::size_t col = 0; col < n; ++col) {
            if (row == col) continue;
            const double value = distribution(generator) * scale;
            matrix[row * n + col] = value;
            row_sum += std::abs(value);
        }
        matrix[row * n + row] = 1.0 + row_sum;
    }
    return matrix;
}

double determinant_cpu(Matrix matrix, std::size_t n) {
    if (n == 0 || matrix.size() != n * n) throw std::invalid_argument("Размер массива не соответствует квадратной матрице");
    int sign = 1;
    double determinant = 1.0;

    for (std::size_t k = 0; k < n; ++k) {
        std::size_t pivot_row = k;
        double maximum = std::abs(matrix[k * n + k]);
        for (std::size_t row = k + 1; row < n; ++row) {
            const double value = std::abs(matrix[row * n + k]);
            if (value > maximum) {
                maximum = value;
                pivot_row = row;
            }
        }
        if (maximum < 1.0e-30) return 0.0;
        if (pivot_row != k) {
            for (std::size_t col = 0; col < n; ++col) {
                std::swap(matrix[k * n + col], matrix[pivot_row * n + col]);
            }
            sign = -sign;
        }

        const double pivot = matrix[k * n + k];
        determinant *= pivot;
        for (std::size_t row = k + 1; row < n; ++row) {
            const double factor = matrix[row * n + k] / pivot;
            for (std::size_t col = k + 1; col < n; ++col) {
                matrix[row * n + col] -= factor * matrix[k * n + col];
            }
        }
    }
    return determinant * static_cast<double>(sign);
}

bool nearly_equal(double a, double b, double relative_tolerance) {
    if (a == b) return true;
    if (!std::isfinite(a) || !std::isfinite(b)) return false;
    const double scale = std::max({1.0, std::abs(a), std::abs(b)});
    return std::abs(a - b) <= relative_tolerance * scale;
}

#if LAB_HAS_OPENCL
DeterminantKernels::DeterminantKernels(DeterminantKernels&& other) noexcept
    : program(other.program), pivot(other.pivot), eliminate(other.eliminate) {
    other.program = nullptr; other.pivot = nullptr; other.eliminate = nullptr;
}
DeterminantKernels& DeterminantKernels::operator=(DeterminantKernels&& other) noexcept {
    if (this != &other) {
        if (pivot) clReleaseKernel(pivot);
        if (eliminate) clReleaseKernel(eliminate);
        if (program) clReleaseProgram(program);
        program = other.program; pivot = other.pivot; eliminate = other.eliminate;
        other.program = nullptr; other.pivot = nullptr; other.eliminate = nullptr;
    }
    return *this;
}

DeterminantKernels::~DeterminantKernels() {
    if (pivot) clReleaseKernel(pivot);
    if (eliminate) clReleaseKernel(eliminate);
    if (program) clReleaseProgram(program);
}

DeterminantKernels build_determinant_kernels(const OpenCLRuntime& runtime) {
    if (!runtime.supports_fp64()) {
        throw std::runtime_error("Выбранный OpenCL GPU не сообщает поддержку double (cl_khr_fp64/cl_amd_fp64); определитель на GPU пропущен.");
    }
    DeterminantKernels result;
    result.program = runtime.build_program_from_file(kernel_path("determinant.cl"));
    cl_int error = CL_SUCCESS;
    result.pivot = clCreateKernel(result.program, "pivot_swap", &error);
    cl_check(error, "clCreateKernel(pivot_swap)");
    result.eliminate = clCreateKernel(result.program, "eliminate_submatrix", &error);
    cl_check(error, "clCreateKernel(eliminate_submatrix)");
    return result;
}

double determinant_gpu(const OpenCLRuntime& runtime,
                       const DeterminantKernels& kernels,
                       const Matrix& matrix,
                       std::size_t n) {
    if (n == 0 || matrix.size() != n * n) throw std::invalid_argument("Размер массива не соответствует квадратной матрице");
    if (n > static_cast<std::size_t>(std::numeric_limits<cl_int>::max())) throw std::invalid_argument("Матрица слишком велика для OpenCL kernel");

    cl_int error = CL_SUCCESS;
    const std::size_t bytes = matrix.size() * sizeof(double);
    cl_mem matrix_buffer = clCreateBuffer(runtime.context(), CL_MEM_READ_WRITE, bytes, nullptr, &error);
    cl_check(error, "clCreateBuffer(matrix)");
    cl_mem sign_buffer = clCreateBuffer(runtime.context(), CL_MEM_READ_WRITE, sizeof(cl_int), nullptr, &error);
    if (error != CL_SUCCESS) {
        clReleaseMemObject(matrix_buffer);
        cl_check(error, "clCreateBuffer(sign)");
    }

    try {
        cl_check(clEnqueueWriteBuffer(runtime.queue(), matrix_buffer, CL_TRUE, 0, bytes, matrix.data(), 0, nullptr, nullptr), "clEnqueueWriteBuffer(matrix)");
        const cl_int sign_initial = 1;
        cl_check(clEnqueueWriteBuffer(runtime.queue(), sign_buffer, CL_TRUE, 0, sizeof(sign_initial), &sign_initial, 0, nullptr, nullptr), "clEnqueueWriteBuffer(sign)");
        const cl_int n_arg = static_cast<cl_int>(n);
        std::size_t local = runtime.local_work_size();
        std::size_t kernel_max_local = local;
        if (clGetKernelWorkGroupInfo(kernels.eliminate, runtime.device(), CL_KERNEL_WORK_GROUP_SIZE,
                                     sizeof(kernel_max_local), &kernel_max_local, nullptr) == CL_SUCCESS) {
            local = std::max<std::size_t>(1, std::min(local, kernel_max_local));
        }

        for (std::size_t k = 0; k < n; ++k) {
            const cl_int k_arg = static_cast<cl_int>(k);
            cl_check(clSetKernelArg(kernels.pivot, 0, sizeof(matrix_buffer), &matrix_buffer), "clSetKernelArg(pivot.matrix)");
            cl_check(clSetKernelArg(kernels.pivot, 1, sizeof(n_arg), &n_arg), "clSetKernelArg(pivot.n)");
            cl_check(clSetKernelArg(kernels.pivot, 2, sizeof(k_arg), &k_arg), "clSetKernelArg(pivot.k)");
            cl_check(clSetKernelArg(kernels.pivot, 3, sizeof(sign_buffer), &sign_buffer), "clSetKernelArg(pivot.sign)");
            const std::size_t one = 1;
            cl_check(clEnqueueNDRangeKernel(runtime.queue(), kernels.pivot, 1, nullptr, &one, &one, 0, nullptr, nullptr), "clEnqueueNDRangeKernel(pivot)");

            const std::size_t remaining = n - k - 1;
            if (remaining > 0) {
                cl_check(clSetKernelArg(kernels.eliminate, 0, sizeof(matrix_buffer), &matrix_buffer), "clSetKernelArg(eliminate.matrix)");
                cl_check(clSetKernelArg(kernels.eliminate, 1, sizeof(n_arg), &n_arg), "clSetKernelArg(eliminate.n)");
                cl_check(clSetKernelArg(kernels.eliminate, 2, sizeof(k_arg), &k_arg), "clSetKernelArg(eliminate.k)");
                const std::size_t logical_items = remaining * remaining;
                const std::size_t global = ((logical_items + local - 1) / local) * local;
                cl_check(clEnqueueNDRangeKernel(runtime.queue(), kernels.eliminate, 1, nullptr, &global, &local, 0, nullptr, nullptr), "clEnqueueNDRangeKernel(eliminate)");
            }
        }

        Matrix result_matrix(n * n);
        cl_int sign = 1;
        cl_check(clEnqueueReadBuffer(runtime.queue(), matrix_buffer, CL_TRUE, 0, bytes, result_matrix.data(), 0, nullptr, nullptr), "clEnqueueReadBuffer(matrix)");
        cl_check(clEnqueueReadBuffer(runtime.queue(), sign_buffer, CL_TRUE, 0, sizeof(sign), &sign, 0, nullptr, nullptr), "clEnqueueReadBuffer(sign)");
        double determinant = static_cast<double>(sign);
        for (std::size_t i = 0; i < n; ++i) determinant *= result_matrix[i * n + i];
        clReleaseMemObject(sign_buffer);
        clReleaseMemObject(matrix_buffer);
        return determinant;
    } catch (...) {
        clReleaseMemObject(sign_buffer);
        clReleaseMemObject(matrix_buffer);
        throw;
    }
}

Md5Kernel::Md5Kernel(Md5Kernel&& other) noexcept : program(other.program), kernel(other.kernel) {
    other.program = nullptr; other.kernel = nullptr;
}
Md5Kernel& Md5Kernel::operator=(Md5Kernel&& other) noexcept {
    if (this != &other) {
        if (kernel) clReleaseKernel(kernel);
        if (program) clReleaseProgram(program);
        program = other.program; kernel = other.kernel;
        other.program = nullptr; other.kernel = nullptr;
    }
    return *this;
}

Md5Kernel::~Md5Kernel() {
    if (kernel) clReleaseKernel(kernel);
    if (program) clReleaseProgram(program);
}

Md5Kernel build_md5_kernel(const OpenCLRuntime& runtime) {
    Md5Kernel result;
    result.program = runtime.build_program_from_file(kernel_path("md5_bruteforce.cl"));
    cl_int error = CL_SUCCESS;
    result.kernel = clCreateKernel(result.program, "md5_search", &error);
    cl_check(error, "clCreateKernel(md5_search)");
    return result;
}

BruteForceResult md5_bruteforce_gpu(const OpenCLRuntime& runtime,
                                    const Md5Kernel& kernel,
                                    const Md5Words& target,
                                    const std::string& alphabet,
                                    int min_length,
                                    int max_length,
                                    std::uint64_t max_candidates) {
    if (alphabet.empty() || alphabet.size() > static_cast<std::size_t>(std::numeric_limits<cl_uint>::max())) {
        throw std::invalid_argument("Недопустимый алфавит");
    }
    if (min_length < 1 || max_length < min_length || max_length > 12) {
        throw std::invalid_argument("Ожидаются длины 1 <= min-length <= max-length <= 12");
    }

    cl_int error = CL_SUCCESS;
    std::string alphabet_mutable = alphabet;
    cl_mem alphabet_buffer = clCreateBuffer(runtime.context(), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                             alphabet_mutable.size(), alphabet_mutable.data(), &error);
    cl_check(error, "clCreateBuffer(alphabet)");
    cl_mem target_buffer = clCreateBuffer(runtime.context(), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                          sizeof(std::uint32_t) * target.size(), const_cast<std::uint32_t*>(target.data()), &error);
    if (error != CL_SUCCESS) {
        clReleaseMemObject(alphabet_buffer);
        cl_check(error, "clCreateBuffer(target)");
    }
    cl_mem found_buffer = clCreateBuffer(runtime.context(), CL_MEM_READ_WRITE, sizeof(cl_int) * 3, nullptr, &error);
    if (error != CL_SUCCESS) {
        clReleaseMemObject(target_buffer);
        clReleaseMemObject(alphabet_buffer);
        cl_check(error, "clCreateBuffer(found_state)");
    }

    BruteForceResult result;
    constexpr std::uint64_t BATCH_SIZE = 262144;
    const cl_uint alphabet_size = static_cast<cl_uint>(alphabet.size());
    const cl_mem alphabet_arg = alphabet_buffer;
    const cl_mem target_arg = target_buffer;
    const cl_mem found_arg = found_buffer;

    try {
        cl_check(clSetKernelArg(kernel.kernel, 0, sizeof(alphabet_arg), &alphabet_arg), "clSetKernelArg(md5.alphabet)");
        cl_check(clSetKernelArg(kernel.kernel, 1, sizeof(alphabet_size), &alphabet_size), "clSetKernelArg(md5.alphabet_size)");
        cl_check(clSetKernelArg(kernel.kernel, 2, sizeof(target_arg), &target_arg), "clSetKernelArg(md5.target)");
        cl_check(clSetKernelArg(kernel.kernel, 6, sizeof(found_arg), &found_arg), "clSetKernelArg(md5.found_state)");
        std::size_t local = runtime.local_work_size();
        std::size_t kernel_max_local = local;
        if (clGetKernelWorkGroupInfo(kernel.kernel, runtime.device(), CL_KERNEL_WORK_GROUP_SIZE,
                                     sizeof(kernel_max_local), &kernel_max_local, nullptr) == CL_SUCCESS) {
            local = std::max<std::size_t>(1, std::min(local, kernel_max_local));
        }

        for (int length = min_length; length <= max_length; ++length) {
            const std::uint64_t total_for_length = search_space_size(alphabet.size(), length);
            std::uint64_t offset = 0;
            const std::uint64_t allowed = std::min(total_for_length, max_candidates - result.checked);
            while (offset < allowed) {
                const std::uint64_t batch = std::min(BATCH_SIZE, allowed - offset);
                const cl_uint length_arg = static_cast<cl_uint>(length);
                const cl_ulong base_arg = static_cast<cl_ulong>(offset);
                const cl_ulong total_arg = static_cast<cl_ulong>(batch);
                cl_check(clSetKernelArg(kernel.kernel, 3, sizeof(length_arg), &length_arg), "clSetKernelArg(md5.length)");
                cl_check(clSetKernelArg(kernel.kernel, 4, sizeof(base_arg), &base_arg), "clSetKernelArg(md5.base_index)");
                cl_check(clSetKernelArg(kernel.kernel, 5, sizeof(total_arg), &total_arg), "clSetKernelArg(md5.total)");

                cl_int state[3] = {0, 0, 0};
                cl_check(clEnqueueWriteBuffer(runtime.queue(), found_buffer, CL_TRUE, 0, sizeof(state), state, 0, nullptr, nullptr), "clEnqueueWriteBuffer(found_state)");
                const std::size_t logical_items = static_cast<std::size_t>(batch);
                const std::size_t global = ((logical_items + local - 1) / local) * local;
                cl_check(clEnqueueNDRangeKernel(runtime.queue(), kernel.kernel, 1, nullptr, &global, &local, 0, nullptr, nullptr), "clEnqueueNDRangeKernel(md5_search)");
                cl_check(clEnqueueReadBuffer(runtime.queue(), found_buffer, CL_TRUE, 0, sizeof(state), state, 0, nullptr, nullptr), "clEnqueueReadBuffer(found_state)");
                result.checked += batch;
                offset += batch;

                if (state[0] != 0) {
                    const std::uint64_t found_index = (static_cast<std::uint64_t>(static_cast<std::uint32_t>(state[2])) << 32U)
                                                    | static_cast<std::uint32_t>(state[1]);
                    result.found = true;
                    result.password = candidate_from_index(found_index, alphabet, length);
                    clReleaseMemObject(found_buffer);
                    clReleaseMemObject(target_buffer);
                    clReleaseMemObject(alphabet_buffer);
                    return result;
                }
            }
            if (result.checked >= max_candidates && (length < max_length || allowed < total_for_length)) {
                result.limit_reached = true;
                break;
            }
        }
    } catch (...) {
        clReleaseMemObject(found_buffer);
        clReleaseMemObject(target_buffer);
        clReleaseMemObject(alphabet_buffer);
        throw;
    }

    if (!result.found && result.checked >= max_candidates) result.limit_reached = true;
    clReleaseMemObject(found_buffer);
    clReleaseMemObject(target_buffer);
    clReleaseMemObject(alphabet_buffer);
    return result;
}
#endif
} // namespace lab
