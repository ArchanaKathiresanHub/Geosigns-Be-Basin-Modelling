cmake_minimum_required(VERSION 2.8)
project(gpu-vs-cpu-benchmarks)

find_package(CUDA)

add_subdirectory(saxpy)
add_subdirectory(fractal)
