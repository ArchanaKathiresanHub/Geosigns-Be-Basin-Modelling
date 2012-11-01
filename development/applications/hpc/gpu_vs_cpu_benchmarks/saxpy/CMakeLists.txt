# GPU SAXPY
cuda_add_executable(gpu-saxpy
    saxpy.cu
)

# cuBLAS SAXPY
cuda_add_executable(cublas-saxpy
    cbsaxpy.cu
)

cuda_add_cublas_to_target(cublas-saxpy)


# CPU SAXPY
add_executable(cpu-saxpy
    saxpy.c
)

set_target_properties( cpu-saxpy
    PROPERTIES
    COMPILE_FLAGS "-g -fast -openmp -restrict"
    LINK_FLAGS "-openmp -g -fast"
)
