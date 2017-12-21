#########################################################################
#                                                                       #
# Copyright (C) 2012-2016 Shell International Exploration & Production. #
# All rights reserved.                                                  #
#                                                                       #
# Developed under license for Shell by PDS BV.                          #
#                                                                       #
# Confidential and proprietary source code of Shell.                    #
# Do not distribute without written permission from Shell.              #
#                                                                       #
#########################################################################

# Check that platform was chosen correctly
execute_process(
      COMMAND "uname" "-io"
      COMMAND "sed" "s,\/,_,g"
      COMMAND "sed" "s,\ ,_,g"
      RESULT_VARIABLE PDS_PLATFORM_IS_NOT_OK
      OUTPUT_VARIABLE PDS_PLATFORM
      OUTPUT_STRIP_TRAILING_WHITESPACE
)

if (PDS_PLATFORM_IS_NOT_OK)
   message( FATAL_ERROR "This platform is not PDS Linux!" )
else()
   set( BM_PLATFORM_ID "${PDS_PLATFORM}" CACHE STRING "An identifier for the platform (OS, etc...) for this is build" )
endif()

set(CBM_HOME "/glb/data/cauldron" CACHE PATH "The path the shared drive of the development team")
set(BM_EXTERNAL_COMPONENTS_DIR "${CBM_HOME}/hpc-library" CACHE PATH "The path to the directory of prebuilt libraries")
set(BM_SSSDEV_INSTALL_DIR "${CBM_HOME}/ibs" CACHE PATH "sssdev installation path")

set(INTEL_CXX_VERSION "2017.05" CACHE STRING "The version number of intel compiler. Used in module load command")
set(INTEL_CXX_ROOT "/glb/data/opt/intel-2017/compilers_and_libraries_2017.5.239/linux" CACHE PATH "Path to Intel's compiler collection")
set(INTEL_MPI_ROOT "/glb/data/opt/intel-2017/compilers_and_libraries_2017.5.239/linux/mpi" CACHE PATH "Path to Intel MPI library" )
set(INTEL_MKL_ROOT "/glb/data/opt/intel-2017/compilers_and_libraries_2017.5.239/linux/mkl" CACHE PATH "Path to Intel MKL" )

option(BM_USE_INTEL_COMPILER "Whether to use the Intel compiler" ON)
option(BM_USE_INTEL_MPI "Whether to use the Intel MPI" ON)

set(BM_CLOCK_GETTIME_LIB "")
set(BM_DL_LIB "" )

set(BLA_VENDOR "MKL")

# Flexlm setup
if (EXISTS "/opt/cauldron/hpc-library/flexlm/EPTFlexLm_v11.11.1" )
   set( FLEXLM_ROOT "/opt/cauldron/hpc-library/flexlm/EPTFlexLm_v11.11.1" CACHE PATH "Path to FlexLM directory" )
   set( FLEXLM_LIBRARY "${FLEXLM_ROOT}/lib64/EPTFlexLm.o" CACHE FILEPATH "Path of FlexLM libraries" )
   option( BM_USE_FLEXLM "Whether to require a license via FlexLM" ON)
else()
   option( BM_USE_FLEXLM "Whether to require a license via FlexLM" OFF)
   set( FLEXLM_ROOT "" CACHE PATH "Path to FlexLM directory" )
   set( FLEXLM_LIBRARY "" CACHE FILEPATH "Path of FlexLM libraries" )
   message( WARNING "WARNING!!! No FlexLM library was found, code will be built without license checking!" )
endif()

# LSF setup
set( LSF_HOME "LSF_HOME-NOTFOUND" CACHE PATH "Home dir for LSF binary files")

# Paraview plugin setup
set(PARAVIEW_CONFIG_ROOT "PARAVIEW_CONFIG_ROOT-NOTFOUND" CACHE PATH "Path where ParaViewConfig.cmake can be found")

# Matlab MCR (needed by TSLib Geocosm library)
set( MCR_ROOT "/glb/data/opt/matlab/v80/" CACHE PATH "Path to Matlab")
set( MCR_VERSION "8.0" CACHE STRING "Matlab Version")
