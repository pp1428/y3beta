#include "md5.hpp"
#include "opencl_runtime.hpp"
#include "tasks.hpp"

#include <chrono>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
using Clock = std::chrono::steady_clock;
enum class DeviceMode { Auto, Cpu, Gpu };

struct Options {
    std::string command = "help";
    DeviceMode device = DeviceMode::Auto;
    std::size_t size = 128;
    std::uint32_t seed = 42;
    std::string target;
    std::string charset = "digits";
    int min_length = 1;
    int max_length = 4;
    std::uint64_t max_candidates = 5'000'000;
    std::string csv = "benchmark.csv";
    bool quick = false;
};

double elapsed_ms(Clock::time_point start) {
    return std::chrono::duration<double, std::milli>(Clock::now() - start).count();
}

void print_help() {
    std::cout << R"HELP(
ЛР №3 — вычисления на GPU с OpenCL

Команды:
  lab3 devices
      Показать OpenCL-платформы и устройства.
  lab3 demo [--device auto|cpu|gpu]
      Демонстрация определителя и MD5-брутфорса (находит пароль 042).
  lab3 det [--size N] [--seed N] [--device auto|cpu|gpu]
      Вычислить определитель случайной диагонально преобладающей матрицы.
  lab3 md5 --target HEX [--charset digits|lower|alnum] [--min-length N]
           [--max-length N] [--max-candidates N] [--device auto|cpu|gpu]
      Перебрать кандидаты фиксированного алфавита и диапазона длин.
  lab3 benchmark [--quick] [--csv FILE]
      Сравнить CPU и GPU, записать результаты в CSV.
  lab3 test [--device auto|cpu|gpu]
      Проверить MD5, определитель и (если доступен) OpenCL.

Режимы устройства:
  cpu  — только CPU;
  gpu  — требовать OpenCL GPU;
  auto — предпочесть GPU, при отсутствии использовать CPU.

Примеры:
  lab3 det --size 128 --device auto
  lab3 md5 --target 202cb962ac59075b964b07152d234b70 --charset digits --min-length 1 --max-length 3
  lab3 benchmark
)HELP";
}

std::string require_value(int& i, int argc, char** argv, const std::string& option) {
    if (i + 1 >= argc) throw std::invalid_argument("После параметра " + option + " требуется значение");
    return argv[++i];
}

Options parse_options(int argc, char** argv) {
    Options options;
    if (argc >= 2) options.command = argv[1];
    for (int i = 2; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--device") {
            const auto value = require_value(i, argc, argv, arg);
            if (value == "cpu") options.device = DeviceMode::Cpu;
            else if (value == "gpu") options.device = DeviceMode::Gpu;
            else if (value == "auto") options.device = DeviceMode::Auto;
            else throw std::invalid_argument("--device принимает только auto, cpu или gpu");
        } else if (arg == "--size") {
            options.size = static_cast<std::size_t>(std::stoull(require_value(i, argc, argv, arg)));
        } else if (arg == "--seed") {
            options.seed = static_cast<std::uint32_t>(std::stoul(require_value(i, argc, argv, arg)));
        } else if (arg == "--target") {
            options.target = require_value(i, argc, argv, arg);
        } else if (arg == "--charset") {
            options.charset = require_value(i, argc, argv, arg);
        } else if (arg == "--min-length") {
            options.min_length = std::stoi(require_value(i, argc, argv, arg));
        } else if (arg == "--max-length") {
            options.max_length = std::stoi(require_value(i, argc, argv, arg));
        } else if (arg == "--max-candidates") {
            options.max_candidates = std::stoull(require_value(i, argc, argv, arg));
        } else if (arg == "--csv") {
            options.csv = require_value(i, argc, argv, arg);
        } else if (arg == "--quick") {
            options.quick = true;
        } else if (arg == "--help" || arg == "-h") {
            options.command = "help";
        } else {
            throw std::invalid_argument("Неизвестный параметр: " + arg);
        }
    }
    return options;
}

std::string alphabet_for(const std::string& name) {
    if (name == "digits") return "0123456789";
    if (name == "lower") return "abcdefghijklmnopqrstuvwxyz";
    if (name == "alnum") return "0123456789abcdefghijklmnopqrstuvwxyz";
    throw std::invalid_argument("--charset принимает digits, lower или alnum");
}


void show_devices() {
#if LAB_HAS_OPENCL
    const auto devices = lab::list_opencl_devices();
    if (devices.empty()) {
        std::cout << "OpenCL-устройства не найдены. Проверьте драйвер GPU и OpenCL runtime.\n";
        return;
    }
    std::cout << "Найденные OpenCL-устройства:\n";
    for (std::size_t i = 0; i < devices.size(); ++i) {
        const auto& d = devices[i];
        std::cout << "  " << (i + 1) << ". " << d.name << " [" << d.type << "]\n"
                  << "     Платформа: " << d.platform << "\n"
                  << "     Производитель: " << d.vendor << "\n"
                  << "     Версия: " << d.version << "; FP64: " << (d.supports_fp64 ? "да" : "нет") << "\n";
    }
#else
    std::cout << "Программа собрана без OpenCL SDK. Доступны CPU-реализации.\n"
              << "Установите OpenCL headers + ICD loader и пересоберите проект.\n";
#endif
}

void run_determinant(const Options& options, bool demo = false) {
    const std::size_t n = demo ? 64 : options.size;
    const auto matrix = lab::make_test_matrix(n, options.seed);
    const auto cpu_start = Clock::now();
    const double cpu_result = lab::determinant_cpu(matrix, n);
    const double cpu_ms = elapsed_ms(cpu_start);
    std::cout << "Определитель методом Гаусса, n=" << n << "\n"
              << "  CPU: " << std::fixed << std::setprecision(3) << cpu_ms << " ms; det="
              << std::setprecision(12) << cpu_result << "\n";

    if (options.device == DeviceMode::Cpu) return;
#if LAB_HAS_OPENCL
    try {
        lab::OpenCLRuntime runtime;
        std::cout << "  OpenCL GPU: " << runtime.device_name() << "\n";
        auto kernels = lab::build_determinant_kernels(runtime);
        const auto gpu_start = Clock::now();
        const double gpu_result = lab::determinant_gpu(runtime, kernels, matrix, n);
        const double gpu_ms = elapsed_ms(gpu_start);
        const bool ok = lab::nearly_equal(cpu_result, gpu_result, 1e-8);
        std::cout << "  GPU: " << std::fixed << std::setprecision(3) << gpu_ms << " ms; det="
                  << std::setprecision(12) << gpu_result << "\n"
                  << "  Сверка CPU/GPU: " << (ok ? "OK" : "ОШИБКА") << "\n";
        if (!ok) throw std::runtime_error("CPU и GPU получили разные определители");
    } catch (const std::exception& error) {
        if (options.device == DeviceMode::Gpu) throw;
        std::cout << "  GPU пропущен: " << error.what() << "\n";
    }
#else
    if (options.device == DeviceMode::Gpu) throw std::runtime_error("Программа собрана без OpenCL; пересоберите с OpenCL SDK.");
    std::cout << "  GPU пропущен: при сборке OpenCL SDK не найден.\n";
#endif
}

void run_md5(const Options& options, bool demo = false) {
    const std::string target_hex = demo ? lab::md5_hex("042") : options.target;
    if (!lab::is_hex_md5(target_hex)) {
        throw std::invalid_argument("Нужно передать MD5-цель через --target: 32 шестнадцатеричных символа");
    }
    const std::string alphabet = demo ? "0123456789" : alphabet_for(options.charset);
    const int min_length = demo ? 3 : options.min_length;
    const int max_length = demo ? 3 : options.max_length;
    const std::uint64_t budget = demo ? 10'000 : options.max_candidates;
    const auto target = lab::md5_words_from_hex(target_hex);
    std::cout << "MD5 brute force: target=" << target_hex << "; alphabet=" << alphabet.size()
              << " символов; длины " << min_length << ".." << max_length << "; лимит=" << budget << " кандидатов\n";

    if (options.device == DeviceMode::Cpu) {
        const auto start = Clock::now();
        const auto result = lab::md5_bruteforce_cpu(target, alphabet, min_length, max_length, budget);
        const auto ms = elapsed_ms(start);
        std::cout << "  CPU: " << std::fixed << std::setprecision(3) << ms << " ms; проверено=" << result.checked << "\n";
        if (result.found) std::cout << "  Найдено: \"" << result.password << "\"; MD5=" << lab::md5_hex(result.password) << "\n";
        else std::cout << "  Совпадение не найдено" << (result.limit_reached ? " (достигнут лимит)" : " (пространство исчерпано)") << ".\n";
        return;
    }

#if LAB_HAS_OPENCL
    try {
        lab::OpenCLRuntime runtime;
        auto kernel = lab::build_md5_kernel(runtime);
        std::cout << "  Устройство: " << runtime.device_name() << "\n";
        const auto start = Clock::now();
        const auto result = lab::md5_bruteforce_gpu(runtime, kernel, target, alphabet, min_length, max_length, budget);
        const auto ms = elapsed_ms(start);
        std::cout << "  GPU/OpenCL: " << std::fixed << std::setprecision(3) << ms << " ms; проверено work-items=" << result.checked << "\n";
        if (result.found) {
            const std::string actual_hash = lab::md5_hex(result.password);
            std::cout << "  Найдено: \"" << result.password << "\"; MD5=" << actual_hash << "\n";
            if (lab::md5_words_from_hex(actual_hash) != target) throw std::runtime_error("Внутренняя проверка найденного пароля не прошла");
        } else {
            std::cout << "  Совпадение не найдено" << (result.limit_reached ? " (достигнут лимит)" : " (пространство исчерпано)") << ".\n";
        }
    } catch (const std::exception& error) {
        if (options.device == DeviceMode::Gpu) throw;
        std::cout << "  GPU недоступен: " << error.what() << "\n  Переключаюсь на CPU.\n";
        const auto start = Clock::now();
        const auto result = lab::md5_bruteforce_cpu(target, alphabet, min_length, max_length, budget);
        const auto ms = elapsed_ms(start);
        std::cout << "  CPU: " << std::fixed << std::setprecision(3) << ms << " ms; проверено=" << result.checked << "\n";
        if (result.found) std::cout << "  Найдено: \"" << result.password << "\"; MD5=" << lab::md5_hex(result.password) << "\n";
        else std::cout << "  Совпадение не найдено" << (result.limit_reached ? " (достигнут лимит)" : " (пространство исчерпано)") << ".\n";
    }
#else
    if (options.device == DeviceMode::Gpu) throw std::runtime_error("Программа собрана без OpenCL; пересоберите с OpenCL SDK.");
    std::cout << "  OpenCL SDK не найден при сборке; использую CPU.\n";
    const auto start = Clock::now();
    const auto result = lab::md5_bruteforce_cpu(target, alphabet, min_length, max_length, budget);
    const auto ms = elapsed_ms(start);
    std::cout << "  CPU: " << std::fixed << std::setprecision(3) << ms << " ms; проверено=" << result.checked << "\n";
    if (result.found) std::cout << "  Найдено: \"" << result.password << "\"; MD5=" << lab::md5_hex(result.password) << "\n";
    else std::cout << "  Совпадение не найдено" << (result.limit_reached ? " (достигнут лимит)" : " (пространство исчерпано)") << ".\n";
#endif
}

struct CsvRow {
    std::string task, backend;
    std::uint64_t input_size = 0;
    double time_ms = 0.0;
    std::string result;
    std::uint64_t checked = 0;
    bool found = false;
};

void write_csv(const std::string& path, const std::vector<CsvRow>& rows) {
    std::ofstream output(path);
    if (!output) throw std::runtime_error("Не удалось открыть CSV для записи: " + path);
    output << "task,backend,input_size,time_ms,result,checked,found\n";
    output << std::setprecision(17);
    for (const auto& row : rows) {
        output << row.task << ',' << row.backend << ',' << row.input_size << ',' << row.time_ms << ",\""
               << row.result << "\"," << row.checked << ',' << (row.found ? 1 : 0) << '\n';
    }
}

void run_benchmark(const Options& options) {
    const std::vector<std::size_t> sizes = options.quick ? std::vector<std::size_t>{16, 32, 64}
                                                        : std::vector<std::size_t>{32, 64, 128, 256};
    std::vector<CsvRow> rows;
#if LAB_HAS_OPENCL
    std::unique_ptr<lab::OpenCLRuntime> runtime;
    std::unique_ptr<lab::DeterminantKernels> det_kernels;
    std::unique_ptr<lab::Md5Kernel> md5_kernel;
    try {
        runtime = std::make_unique<lab::OpenCLRuntime>();
        std::cout << "GPU для бенчмарка: " << runtime->device_name() << "\n";
        try {
            det_kernels = std::make_unique<lab::DeterminantKernels>(lab::build_determinant_kernels(*runtime));
        } catch (const std::exception& error) {
            std::cout << "GPU-определитель пропущен: " << error.what() << "\n";
        }
        try {
            md5_kernel = std::make_unique<lab::Md5Kernel>(lab::build_md5_kernel(*runtime));
        } catch (const std::exception& error) {
            std::cout << "GPU MD5 пропущен: " << error.what() << "\n";
        }
    } catch (const std::exception& error) {
        std::cout << "GPU не будет участвовать в бенчмарке: " << error.what() << "\n";
        runtime.reset(); det_kernels.reset(); md5_kernel.reset();
    }
#else
    std::cout << "OpenCL SDK не найден при сборке — будет измерен только CPU.\n";
#endif

    std::cout << "\nОпределитель (время включает копирование/выделение памяти, но не компиляцию kernel):\n";
    for (const std::size_t n : sizes) {
        const auto matrix = lab::make_test_matrix(n, 42);
        const auto cpu_start = Clock::now();
        const double cpu_result = lab::determinant_cpu(matrix, n);
        const double cpu_ms = elapsed_ms(cpu_start);
        rows.push_back({"determinant", "CPU", n, cpu_ms, std::to_string(cpu_result), 0, true});
        std::cout << "  n=" << n << " CPU=" << std::fixed << std::setprecision(3) << cpu_ms << " ms";
#if LAB_HAS_OPENCL
        if (runtime && det_kernels) {
            try {
                const auto gpu_start = Clock::now();
                const double gpu_result = lab::determinant_gpu(*runtime, *det_kernels, matrix, n);
                const double gpu_ms = elapsed_ms(gpu_start);
                const bool ok = lab::nearly_equal(cpu_result, gpu_result, 1e-8);
                rows.push_back({"determinant", "GPU", n, gpu_ms, std::to_string(gpu_result), 0, ok});
                std::cout << "; GPU=" << gpu_ms << " ms; сверка=" << (ok ? "OK" : "ОШИБКА");
                if (!ok) throw std::runtime_error("Определители CPU/GPU не совпадают");
            } catch (const std::exception& error) {
                std::cout << "; GPU ошибка: " << error.what();
            }
        }
#endif
        std::cout << "\n";
    }

    const std::string secret = options.quick ? "999" : "999999";
    const std::string alphabet = "0123456789";
    const auto target_hex = lab::md5_hex(secret);
    const auto target = lab::md5_words_from_hex(target_hex);
    const std::uint64_t md5_budget = options.quick ? 1'000ULL : 1'000'000ULL;
    const int length = static_cast<int>(secret.size());
    std::cout << "\nMD5 brute force: цель " << target_hex << ", длина=" << length << ", кандидатов=" << md5_budget << "\n";
    {
        const auto start = Clock::now();
        const auto result = lab::md5_bruteforce_cpu(target, alphabet, length, length, md5_budget);
        const double ms = elapsed_ms(start);
        rows.push_back({"md5", "CPU", md5_budget, ms, result.password, result.checked, result.found});
        std::cout << "  CPU=" << std::fixed << std::setprecision(3) << ms << " ms; проверено=" << result.checked
                  << "; найдено=" << (result.found ? "да" : "нет") << "\n";
    }
#if LAB_HAS_OPENCL
    if (runtime && md5_kernel) {
        try {
            const auto start = Clock::now();
            const auto result = lab::md5_bruteforce_gpu(*runtime, *md5_kernel, target, alphabet, length, length, md5_budget);
            const double ms = elapsed_ms(start);
            rows.push_back({"md5", "GPU", md5_budget, ms, result.password, result.checked, result.found});
            std::cout << "  GPU=" << std::fixed << std::setprecision(3) << ms << " ms; work-items=" << result.checked
                      << "; найдено=" << (result.found ? "да" : "нет") << "\n";
            if (!result.found || lab::md5_hex(result.password) != target_hex) {
                throw std::runtime_error("GPU MD5 тест не прошёл сверку");
            }
        } catch (const std::exception& error) {
            std::cout << "  GPU MD5 ошибка: " << error.what() << "\n";
        }
    }
#endif
    write_csv(options.csv, rows);
    std::cout << "\nCSV записан: " << options.csv << "\n"
              << "Графики: python scripts/plot_benchmarks.py " << options.csv << "\n";
}

void run_tests(const Options& options) {
    int passed = 0;
    auto check = [&passed](bool condition, const std::string& description) {
        if (!condition) throw std::runtime_error("TEST FAILED: " + description);
        ++passed;
        std::cout << "[OK] " << description << "\n";
    };
    check(lab::md5_hex("") == "d41d8cd98f00b204e9800998ecf8427e", "MD5 пустой строки");
    check(lab::md5_hex("abc") == "900150983cd24fb0d6963f7d28e17f72", "MD5 строки abc");
    check(lab::md5_hex("message digest") == "f96b697d7cb7938d525a2f31aaf161d0", "MD5 строки message digest");
    check(lab::nearly_equal(lab::determinant_cpu({1, 2, 3, 4}, 2), -2.0), "определитель матрицы 2x2");
    check(lab::nearly_equal(lab::determinant_cpu({0, 1, 1, 0}, 2), -1.0), "частичный выбор главного элемента");
    check(lab::determinant_cpu({1, 2, 2, 4}, 2) == 0.0, "вырожденная матрица");
    check(lab::candidate_from_index(42, "0123456789", 3) == "042", "кодирование кандидата по индексу");
    const auto target = lab::md5_words_from_hex(lab::md5_hex("042"));
    const auto cpu = lab::md5_bruteforce_cpu(target, "0123456789", 3, 3, 1000);
    check(cpu.found && cpu.password == "042", "CPU MD5 brute force");

    if (options.device != DeviceMode::Cpu) {
#if LAB_HAS_OPENCL
        try {
            lab::OpenCLRuntime runtime;
            std::cout << "OpenCL GPU: " << runtime.device_name() << "\n";
            try {
                auto det_kernels = lab::build_determinant_kernels(runtime);
                check(lab::nearly_equal(lab::determinant_gpu(runtime, det_kernels, {0, 1, 1, 0}, 2), -1.0), "OpenCL determinant pivot test");
            } catch (const std::exception& error) {
                if (options.device == DeviceMode::Gpu) throw;
                std::cout << "[SKIP] OpenCL determinant: " << error.what() << "\n";
            }
            try {
                auto md5_kernel = lab::build_md5_kernel(runtime);
                const auto gpu = lab::md5_bruteforce_gpu(runtime, md5_kernel, target, "0123456789", 3, 3, 1000);
                check(gpu.found && gpu.password == "042" && lab::md5_hex(gpu.password) == lab::md5_hex("042"), "OpenCL MD5 brute force");
            } catch (const std::exception& error) {
                if (options.device == DeviceMode::Gpu) throw;
                std::cout << "[SKIP] OpenCL MD5: " << error.what() << "\n";
            }
        } catch (const std::exception& error) {
            if (options.device == DeviceMode::Gpu) throw;
            std::cout << "[SKIP] OpenCL GPU-тесты: " << error.what() << "\n";
        }
#else
        if (options.device == DeviceMode::Gpu) throw std::runtime_error("Программа собрана без OpenCL");
        std::cout << "[SKIP] OpenCL GPU-тесты: при сборке SDK не найден.\n";
#endif
    }
    std::cout << "\nПройдено проверок: " << passed << "\n";
}

} // namespace

int main(int argc, char** argv) {
    try {
        const Options options = parse_options(argc, argv);
        if (options.command == "help") {
            print_help();
        } else if (options.command == "devices") {
            show_devices();
        } else if (options.command == "det") {
            run_determinant(options);
        } else if (options.command == "md5") {
            run_md5(options);
        } else if (options.command == "demo") {
            std::cout << "=== Демонстрация лабораторной работы №3 ===\n";
            run_determinant(options, true);
            std::cout << "\n";
            run_md5(options, true);
        } else if (options.command == "benchmark") {
            run_benchmark(options);
        } else if (options.command == "test") {
            run_tests(options);
        } else {
            throw std::invalid_argument("Неизвестная команда: " + options.command + " (см. lab3 help)");
        }
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Ошибка: " << error.what() << "\n\nИспользуйте `lab3 help` для справки.\n";
        return 1;
    }
}
