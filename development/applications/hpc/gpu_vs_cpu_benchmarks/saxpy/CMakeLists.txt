include_directories(${PROJECT_SOURCE_DIR}/gettime)

# GPU SAXPY
cuda_add_executable(gpu-saxpy
    saxpy.cu
)
target_link_libraries(gpu-saxpy gettime)

# cuBLAS SAXPY
cuda_add_executable(cublas-saxpy
    cbsaxpy.cu
)
cuda_add_cublas_to_target(cublas-saxpy)
target_link_libraries(cublas-saxpy gettime)


# CPU SAXPY
add_executable(cpu-saxpy
    saxpy.c
)
target_link_libraries(cpu-saxpy gettime)
target_enable_openmp(cpu-saxpy)


