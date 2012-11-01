
# GPU bifa
cuda_add_executable(gpu-bifa
    bifa.cu
)


add_executable(cpu-ji-bifa
    bifa.c
)

set_target_properties( cpu-ji-bifa
    PROPERTIES
    COMPILE_FLAGS "-g -fast -openmp -DORDER_JI"
    LINK_FLAGS "-g -fast -openmp"
)

add_executable(cpu-ij-bifa
    bifa.c
)

set_target_properties( cpu-ij-bifa
    PROPERTIES
    COMPILE_FLAGS "-g -fast -openmp -DORDER_IJ"
    LINK_FLAGS "-g -fast -openmp"
)

add_executable(cpu-ij-unrolled-bifa
    bifa.c
)

set_target_properties( cpu-ij-unrolled-bifa
    PROPERTIES
    COMPILE_FLAGS "-g -fast -openmp -DORDER_IJ -DUNROLL"
    LINK_FLAGS "-g -fast -openmp"
)
