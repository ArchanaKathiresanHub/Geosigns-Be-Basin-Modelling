include_directories(${PROJECT_SOURCE_DIR}/gettime)

# GPU bifa
cuda_add_executable(gpu-bifa
    bifa.cu
)
target_link_libraries(gpu-bifa gettime)

add_executable(cpu-ji-bifa
    bifa.c
)
target_link_libraries(cpu-ji-bifa gettime)
target_enable_openmp(cpu-ji-bifa)
target_add_definitions(cpu-ji-bifa ORDER_JI)

add_executable(cpu-ij-bifa
    bifa.c
)
target_link_libraries(cpu-ij-bifa gettime)
target_enable_openmp(cpu-ij-bifa)
target_add_definitions(cpu-ij-bifa ORDER_IJ)

add_executable(cpu-ij-unrolled-bifa
    bifa.c
)
target_link_libraries(cpu-ij-unrolled-bifa gettime)
target_enable_openmp(cpu-ij-unrolled-bifa)
target_add_definitions(cpu-ij-unrolled-bifa ORDER_IJ UNROLL)


