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
   set( BM_PLATFORM_ID "${CSCE_PLATFORM}" CACHE STRING "An identifier for the platform (OS, etc...) for this is build" )
else()
   message(WARNING "This platform is not Shell Linux!")
endif()


set(CBM_HOME "/nfs/rvl/groups/ept-sg/SWEast/Cauldron" CACHE PATH "The path the shared drive of the development team")

option( BM_BUILD_QT3_APPS "Build programs that need QT3" ON)
option( BM_CSHARP_API "Build the C sharp interface (Windows only)" OFF )
option( BM_USE_FLEXLM "Whether to require a license via FlexLM" ON)
option( BM_EXTERNAL_COMPONENTS_REBUILD "Whether or not to rebuild external components" OFF)
set(BM_EXTERNAL_COMPONENTS_DIR "/nfs/rvl/groups/ept-sg/SWEast/Cauldron/hpc-library" CACHE PATH "The path to the directory of prebuilt libraries")
set(BM_SSSDEV_INSTALL_DIR "/apps/sssdev/ibs" CACHE PATH "sssdev installation path")

set(INTEL_CXX_ROOT "/apps/3rdparty/intel/ics2013/composer_xe_2013.5.192" CACHE PATH "Path to Intel's compiler collection")
set(INTEL_MPI_ROOT "/apps/3rdparty/intel/impi/4.1.1.036" CACHE PATH "Path to Intel MPI library" )
set(INTEL_MKL_ROOT "${INTEL_CXX_ROOT}/mkl" CACHE PATH "Path to Intel MKL" )

option(BM_USE_INTEL_COMPILER "Whether to use the Intel compiler (UNIX only)" ON)
option(BM_USE_INTEL_MPI "Whether to use the Intel MPI (UNIX only)" ON)

set(DOXYGEN_EXECUTABLE "${CBM_HOME}/Tools/bin/doxygen" CACHE PATH "Path to doxygen executable")
set(DOT_EXECUTABLE "${CBM_HOME}/Tools/bin/dot" CACHE PATH "Path to dot executable")

set( FLEXLM_ROOT "/apps/3rdparty/EPTFlexLm/v9.2" CACHE PATH "Path to FlexLM directory" )
set( FLEXLM_LIBRARY "${FLEXLM_ROOT}/LinuxRHEL_i686_30WS/lib64/EPTFlexLm.o" CACHE FILEPATH "Path of FlexLM libraries" )

set( LSF_HOME "/glb/apps/hpc/lsfprod/9.1/linux2.6-glibc2.3-x86_64" CACHE PATH "Home dir for LSF binary files")

set(PARAVIEW_CONFIG_ROOT "${CBM_HOME}/Tools/paraview/Paraview-4.1.0-Release/Linux64x_26/lib/cmake/paraview-4.1"
      CACHE PATH "Path where ParaViewConfig.cmake can be found")
	
set( TSLIB_VERSION "7.4")	
set( TSLIB_ROOT "/nfs/rvl/groups/ept-sg/SWEast/Cauldron/hpc/tslib/tslib-7.4.2" CACHE PATH "Path to Geocosm's TsLib" )
set( TSLIB_LIBRARY_DIR "${TSLIB_ROOT}/bin/x86_64_linux/Release" CACHE PATH "Path to Geocosm's TsLib library directory")

set( MCR_ROOT "/apps/sss/matlab/8.0" CACHE PATH "Path to Matlab")
set( MCR_VERSION "8.0" CACHE STRING "Matlab Version")

set( XERCES_ROOT "${TSLIB_ROOT}/3rdparty/Xerces/xerces-c-3.1.0-x86_64-linux-gcc-3.4" CACHE PATH "Path to Xerces-C library" )
set( XERCES_LIBRARIES "${XERCES_ROOT}/lib/libxerces-c-3.1.so" )

set( XSD_ROOT "${TSLIB_ROOT}/3rdparty/xsd/xsd-3.3.0-x86_64-linux-gnu/libxsd" CACHE PATH "Path to Codesynthesis's XSD library")





