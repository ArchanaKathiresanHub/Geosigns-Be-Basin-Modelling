#
#
#
set(target_name RubyGeneralIO)

file(GLOB SOURCE_FILES "${BB_ROOT}/${target_name}/src/*.[Ch]")

include_directories(${BB_ROOT}/RubyUtilities/src)
include_directories("/nfs/rvl/groups/ept-sg/SWEast/Cauldron/hpc/hdf5/1.6.6/LinuxRHEL64_x86_64_57WS/include")
include_directories("/nfs/rvl/users/ibs/geocase_64/prod/v2.4/include/dds")
include_directories("/nfs/rvl/users/ibs/geocase_64/prod/v2.4/include/gospel")

add_definitions(-DHDF5_ENABLED)

add_library(${target_name} STATIC ${SOURCE_FILES})
