#########################################################################
#                                                                       #
# Copyright (C) 2012-2014 Shell International Exploration & Production. #
# All rights reserved.                                                  #
#                                                                       #
# Developed under license for Shell by PDS BV.                          #
#                                                                       #
# Confidential and proprietary source code of Shell.                    #
# Do not distribute without written permission from Shell.              #
#                                                                       #
#########################################################################

execute_process(
      COMMAND /apps/sss/share/getos2
      RESULT_VARIABLE CSCE_PLATFORM_IS_NOT_OK
      OUTPUT_VARIABLE CSCE_PLATFORM
      OUTPUT_STRIP_TRAILING_WHITESPACE
)

if (NOT CSCE_PLATFORM_IS_NOT_OK)
   set(platformId "${CSCE_PLATFORM}")
else()
   message(WARNING "This platform is not Shell Linux!")
   set(platformId "Unknown_Linux")
endif()

set( BM_PLATFORM_ID "${platformId}" CACHE STRING "An identifier for the platform (OS, etc...) for this is build" )

set(CBM_HOME "/nfs/rvl/groups/ept-sg/SWEast/Cauldron" CACHE PATH "The path the shared drive of the development team")

option( BM_CSHARP_API "Build the C sharp interface (Windows only)" OFF )
option( BM_USE_FLEXLM "Whether to require a license via FlexLM" ON)
option( BM_EXTERNAL_COMPONENTS_REBUILD "Whether or not to rebuild external components" OFF)
set(BM_EXTERNAL_COMPONENTS_DIR "/nfs/rvl/groups/ept-sg/SWEast/Cauldron/hpc-library" CACHE PATH "The path to the directory of prebuilt libraries")
set(BM_SSSDEV_INSTALL_DIR "/apps/sssdev/ibs" CACHE PATH "sssdev installation path")

set(INTEL_CXX_ROOT "/apps/3rdparty/intel/ics2013/composer_xe_2013.5.192" CACHE PATH "Path to Intel's compiler collection")
set(INTEL_MPI_ROOT "/apps/3rdparty/intel/impi/4.1.1.036" CACHE PATH "Path to Intel MPI library" )
option(BM_USE_INTEL_COMPILER "Whether to use the Intel compiler (UNIX only)" ON)
option(BM_USE_INTEL_MPI "Whether to use the Intel MPI (UNIX only)" ON)

set(DOXYGEN_EXECUTABLE "${CBM_HOME}/Tools/bin/doxygen" CACHE PATH "Path to doxygen executable")
set(DOT_EXECUTABLE "${CBM_HOME}/Tools/bin/dot" CACHE PATH "Path to dot executable")

set( FLEXLM_ROOT "/apps/3rdparty/EPTFlexLm/v9.2" CACHE PATH "Path to FlexLM directory" )
set( FLEXLM_LIBRARY "${FLEXLM_ROOT}/LinuxRHEL_i686_30WS/lib64/EPTFlexLm.o" CACHE FILEPATH "Path of FlexLM libraries" )

set( LSF_HOME "/glb/apps/hpc/lsfprod/9.1/linux2.6-glibc2.3-x86_64" CACHE PATH "Home dir for LSF binary files")

set( TS7_ROOT "/nfs/rvl/groups/ept-sg/SWEast/Cauldron/hpc/tslib/tslib-7.3" CACHE PATH "Path to Geocosm's TsLib 7.3" )

