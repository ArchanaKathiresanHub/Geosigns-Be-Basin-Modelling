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

set(CBM_HOME "/opt/cauldron" CACHE PATH "The path the shared drive of the development team")

option( BM_BUILD_QT3_APPS "Build programs that need QT3" ON)
option( BM_CSHARP_API "Build the C sharp interface (Windows only)" OFF )
option( BM_USE_FLEXLM "Whether to require a license via FlexLM" ON)
option( BM_EXTERNAL_COMPONENTS_REBUILD "Whether or not to rebuild external components" OFF)

set(BM_EXTERNAL_COMPONENTS_DIR "${CBM_HOME}/hpc-library" CACHE PATH "The path to the directory of prebuilt libraries")

set(BM_SSSDEV_INSTALL_DIR "/opt/ibs" CACHE PATH "sssdev installation path")

set(INTEL_CXX_ROOT "INTEL_CXX_ROOT-NOTFOUND" CACHE PATH "Path to Intel's compiler collection")
set(INTEL_MPI_ROOT "INTEL_MPI_ROOT-NOTFOUND" CACHE PATH "Path to Intel MPI library" )
set(INTEL_MKL_ROOT "INTEL_MKL_ROOT-NOTFOUND" CACHE PATH "Path to Intel MKL" )

option(BM_USE_INTEL_COMPILER "Whether to use the Intel compiler (UNIX only)" OFF)
option(BM_USE_INTEL_MPI "Whether to use the Intel MPI (UNIX only)" OFF)

set( FLEXLM_ROOT "FLEXLM_ROOT-NOTFOUND" CACHE PATH "Path to FlexLM directory" )
set( FLEXLM_LIBRARY "FLEXLM_LIBRARY-NOTFOUND" CACHE FILEPATH "Path of FlexLM libraries" )

set( LSF_HOME "LSF_HOME-NOTFOUND" CACHE PATH "Home dir for LSF binary files")

set(PARAVIEW_CONFIG_ROOT "PARAVIEW_CONFIG_ROOT-NOTFOUND" CACHE PATH "Path where ParaViewConfig.cmake can be found")
	
set( TSLIB_VERSION "7.4")	
set( TSLIB_ROOT "${BM_EXTERNAL_COMPONENTS_DIR}/tslib/tslib-7.4.2" CACHE PATH "Path to Geocosm's TsLib" )
set( TSLIB_LIBRARY_DIR "${TSLIB_ROOT}/bin/x86_64_linux/Release" CACHE PATH "Path to Geocosm's TsLib library directory")

set( MCR_ROOT "${BM_EXTERNAL_COMPONENTS_DIR}/matlab/8.0" CACHE PATH "Path to Matlab")
set( MCR_VERSION "8.0" CACHE STRING "Matlab Version")

set( XERCES_ROOT "${TSLIB_ROOT}/3rdparty/Xerces/xerces-c-3.1.0-x86_64-linux-gcc-3.4" CACHE PATH "Path to Xerces-C library" )
set( XERCES_LIBRARIES "${XERCES_ROOT}/lib/libxerces-c-3.1.so" )

set( XSD_ROOT "${TSLIB_ROOT}/3rdparty/xsd/xsd-3.3.0-x86_64-linux-gnu/libxsd" CACHE PATH "Path to Codesynthesis's XSD library")





