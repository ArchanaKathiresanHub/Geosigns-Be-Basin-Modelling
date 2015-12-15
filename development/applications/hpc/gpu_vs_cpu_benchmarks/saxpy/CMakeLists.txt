#########################################################################
#                                                                       #
# Copyright (C) 2012-2013 Shell International Exploration & Production. #
# All rights reserved.                                                  #
#                                                                       #
# Developed under license for Shell by PDS BV.                          #
#                                                                       #
# Confidential and proprietary source code of Shell.                    #
# Do not distribute without written permission from Shell.              #
#                                                                       #
#########################################################################

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


