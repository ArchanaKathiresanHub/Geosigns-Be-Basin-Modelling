#
#
#

set(target_name grid2grid)

file(GLOB SOURCE_FILES "${BB_ROOT}/${target_name}/src/*.[Ch]")

add_definitions(-DMODULENAME="${target_name}" -DMODULEVER="1.0")
add_definitions(-Dgocad_defined_bool)

include_directories(${BB_ROOT}/RubyGeneralIO/src)
include_directories(${BB_ROOT}/RubyUtilities/src)
include_directories(${GEOCASE_BASE_DIR}/include/gospel)
include_directories(${GEOCASE_BASE_DIR}/include/dds)

link_directories(
  /nfs/rvl/users/ibs/geocase_64/prod/v2.4/lib/Linux64g/static
)


add_executable(${target_name} ${SOURCE_FILES})

target_link_libraries(${target_name} 
  RubyGeneralIO
  RubyUtilities
  Serial_Hdf5
  gospel dds dds
  Xt
  X11
)

# --- Linking debug executable ...
#/usr/bin/g++34 -m64 -Wno-deprecated -m64  grid2grid.o       
#-L../../lib/Linux64 -lRBYgeneralio_d -lRBYutilities_d 
#../../lib/Linux64/libserialhdf5.a 
#-L/usr/X11R6/lib64 
#-lXt 
#-lX11 
#/nfs/rvl/groups/ept-sg/SWEast/Cauldron/hpc/hdf5/1.6.6/LinuxRHEL64_x86_64_57WS/lib/libhdf5.a 
#-lz 
#/apps/3rdparty/EPTFlexLm/v9.2/LinuxRHEL64_x86_64_30WS/lib64/EPTFlexLm.o  
#-L/nfs/rvl/users/ibs/geocase_64/prod/v2.4/lib/Linux64g/static 
#-lgospel -ldds -ldds -lc -lm  
#-o grid2grid_test

# end of file
