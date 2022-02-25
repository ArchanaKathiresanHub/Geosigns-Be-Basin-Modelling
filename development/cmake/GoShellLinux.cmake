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
option(BM_USE_FLEXLM "Whether to require a license via FlexLM" ON)

# Check that platform was chosen correctly
execute_process(
      COMMAND /apps/sss/share/getos2
      RESULT_VARIABLE CSCE_PLATFORM_IS_NOT_OK
      OUTPUT_VARIABLE CSCE_PLATFORM
      OUTPUT_STRIP_TRAILING_WHITESPACE
)

if (CSCE_PLATFORM_IS_NOT_OK)
   message(FATAL_ERROR "This platform is not Shell Linux!")
else()
   set( BM_PLATFORM_ID "${CSCE_PLATFORM}" CACHE STRING "An identifier for the platform (OS, etc...) for this is build" )
endif()

set(BM_EXTERNAL_COMPONENTS_DIR "${CBM_HOME}/hpc-library" CACHE PATH "The path to the directory of prebuilt libraries")
set(BM_SSSDEV_INSTALL_DIR "/apps/sssdev/ibs" CACHE PATH "sssdev installation path")

set(INTEL_CXX_VERSION "2019a" CACHE STRING "The version number of intel compiler. Used in module load command")
set(INTEL_CXX_ROOT "/glb/data/hpcrnd/easybuild/PRODUCTION/software/rhel/7/icc/2019.1.144-GCC-8.2.0-2.31.1/compilers_and_libraries_2019.1.144/linux/" CACHE PATH "Path to Intel's compiler collection")
set(INTEL_MPI_ROOT "/glb/data/hpcrnd/easybuild/PRODUCTION/software/rhel/7/impi/2018.4.274-iccifort-2019.1.144-GCC-8.2.0-2.31.1" CACHE PATH "Path to Intel MPI library" )
set(INTEL_MKL_ROOT "/glb/data/hpcrnd/easybuild/PRODUCTION/software/rhel/7/imkl/2019.1.144-iimpi-2019a/mkl" CACHE PATH "Path to Intel MKL" )

option(BM_USE_INTEL_COMPILER "Whether to use the Intel compiler" ON)
option(BM_USE_INTEL_MPI "Whether to use the Intel MPI" ON)

set(BM_CLOCK_GETTIME_LIB "")
set(BM_DL_LIB "")

set(BLA_VENDOR "MKL")

# Flexlm setup
set(FLEXLM_ROOT "/apps/3rdparty/EPTFlexLm/v9.2" CACHE PATH "Path to FlexLM directory" )
set(FLEXLM_LIBRARY "${FLEXLM_ROOT}/LinuxRHEL_i686_30WS/lib64/EPTFlexLm.o" CACHE FILEPATH "Path of FlexLM libraries" )

# Doxygen
execute_process(
    COMMAND bash "-c" "printf `which doxygen`" 
	RESULT_VARIABLE result_NEWDOXYGEN_EXECUTABLE
    OUTPUT_VARIABLE output_NEWDOXYGEN_EXECUTABLE
)
if( result_NEWDOXYGEN_EXECUTABLE MATCHES "0$")
   message(STATUS "This is the doxy exe ${output_NEWDOXYGEN_EXECUTABLE}")
   set(DOXYGEN_EXECUTABLE "${output_NEWDOXYGEN_EXECUTABLE}" CACHE PATH "Path to doxygen executable")
else()
   message(WARNING "DOXYGEN NOT FOND for BM_BUILD_DOCS!")
   set(DOXYGEN_EXECUTABLE "")
endif()
