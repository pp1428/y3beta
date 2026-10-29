cmake_minimum_required(VERSION 3.20)
project(gpu_lab3_opencl LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenCL QUIET)

add_executable(lab3
    src/main.cpp
    src/md5.cpp
    src/opencl_runtime.cpp
    src/tasks.cpp
)

target_include_directories(lab3 PRIVATE src)
target_compile_definitions(lab3 PRIVATE LAB_KERNEL_DIR="${CMAKE_CURRENT_SOURCE_DIR}/kernels")

if(OpenCL_FOUND)
    target_link_libraries(lab3 PRIVATE OpenCL::OpenCL)
    target_compile_definitions(lab3 PRIVATE LAB_HAS_OPENCL=1 CL_TARGET_OPENCL_VERSION=120 CL_USE_DEPRECATED_OPENCL_1_2_APIS)
    message(STATUS "OpenCL найден: собираются CPU и GPU/OpenCL реализации")
else()
    target_compile_definitions(lab3 PRIVATE LAB_HAS_OPENCL=0)
    message(WARNING "OpenCL SDK не найден. Будет собрана CPU-версия; для GPU установите OpenCL SDK и пересоберите проект.")
endif()

if(MSVC)
    target_compile_options(lab3 PRIVATE /W4 /permissive- /utf-8)
else()
    target_compile_options(lab3 PRIVATE -Wall -Wextra -Wpedantic)
endif()
