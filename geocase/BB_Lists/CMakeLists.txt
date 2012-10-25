#
#
#

cmake_minimum_required(VERSION 2.8)

project(geocase)

option(GEOCASE_GOCAD_ENABLED "Enable Gocad" OFF)
option(GEOCASE_HDF5_ENABLED "Enable HDF5" ON)
option(GEOCASE_BUILD_IBSLINK "Build ibslink" ON)
option(GEOCASE_BUILD_123KEY "Build 123key" ON)
option(GEOCASE_BUILD_GRID2GRID "Build grid2grid" ON)
option(GEOCASE_BUILD_PROJECT2HDF "Build project2hdf" ON)
option(GEOCASE_BUILD_VIEW4D "Build view4D" ON)
option(GEOCASE_BUILD_DOCUMENTATION "Build documentation" ON)

get_filename_component(BB_ROOT "../BB" ABSOLUTE)

macro(generate_dox dox)
  if(GEOCASE_BUILD_DOCUMENTATION)
#    message (WARNING "TODO: macro generate_dox is not defined yet (${dox})")
  endif()
endmacro()

# Redefine MACRO from Basin Modeling Utilities & Functions
macro( bm_include_libraries )
        foreach(library ${ARGN})
                include_directories(
                        ${BB_ROOT}/../../libraries/${library}/src
                        ${PROJECT_BINARY_DIR}/libraries/${library}
                )
        endforeach(library)
endmacro( bm_include_libraries )

set(GEOCASE_BASE_DIR "/nfs/rvl/users/ibs/geocase_64/prod/v2.4")

if(GEOCASE_HDF5_ENABLED)
  set(ENV{HDF5_ROOT} "/nfs/rvl/groups/ept-sg/SWEast/Cauldron/hpc/hdf5/1.6.6/LinuxRHEL64_x86_64_57WS")
  find_package(HDF5 REQUIRED)
endif()

# libraries
add_subdirectory(RubyUtilities)
add_subdirectory(RubyGeneralIO)

if(GEOCASE_HDF5_ENABLED)
# the same CMakeLists.txt is used for Basin modeling and for BB
  add_subdirectory(../../libraries/Serial_Hdf5 Serial_Hdf5)
endif()

add_subdirectory(../../libraries/TableIO TableIO)
add_subdirectory(../../libraries/CBMGenerics CBMGenerics)
add_subdirectory(../../libraries/EosPack EosPack)

# applications

if(GEOCASE_BUILD_IBSLINK)
  add_subdirectory(ibslink)
endif()

if(GEOCASE_BUILD_123KEY)
  add_subdirectory(123key)
endif()

if(GEOCASE_BUILD_GRID2GRID)
  add_subdirectory(grid2grid)
endif()

if(GEOCASE_BUILD_VIEW4D)
  add_subdirectory(view4D)
endif()

if(GEOCASE_BUILD_PROJECT2HDF)
#  add_subdirectory(project2hdf)
endif()
