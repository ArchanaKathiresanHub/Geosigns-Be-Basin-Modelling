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

set(CBM_HOME "/nfs/rvl/groups/ept-sg/SWEast/Cauldron" CACHE PATH "The path the shared drive of the development team")
set(BM_EXTERNAL_COMPONENTS_DIR "${CBM_HOME}/hpc-library" CACHE PATH "The path to the directory of prebuilt libraries")
set(BM_SSSDEV_INSTALL_DIR "/apps/sssdev/ibs" CACHE PATH "sssdev installation path")

set(INTEL_CXX_ROOT "/glb/data/hpcrnd/easybuild/PRODUCTION/software/rhel/6/icc/2016.1.150-GCC-4.9.3-2.25/compilers_and_libraries_2016.1.150/linux" CACHE PATH "Path to Intel's compiler collection")
set(INTEL_MPI_ROOT "/glb/data/hpcrnd/easybuild/PRODUCTION/software/rhel/6/impi/5.1.2.150-iccifort-2016.1.150-GCC-4.9.3-2.25" CACHE PATH "Path to Intel MPI library" )
set(INTEL_MKL_ROOT "/glb/data/hpcrnd/easybuild/PRODUCTION/software/rhel/6/imkl/11.3.1.150-iimpi-2016.01-GCC-4.9.3-2.25/mkl" CACHE PATH "Path to Intel MKL" )

option(BM_USE_INTEL_COMPILER "Whether to use the Intel compiler" ON)
option(BM_USE_INTEL_MPI "Whether to use the Intel MPI" ON)

set(BM_CLOCK_GETTIME_LIB "")
set(BM_DL_LIB "")

set(BLA_VENDOR "MKL")

# Flexlm setup
set(FLEXLM_ROOT "/apps/3rdparty/EPTFlexLm/v9.2" CACHE PATH "Path to FlexLM directory" )
set(FLEXLM_LIBRARY "${FLEXLM_ROOT}/LinuxRHEL_i686_30WS/lib64/EPTFlexLm.o" CACHE FILEPATH "Path of FlexLM libraries" )

set(DOXYGEN_EXECUTABLE "${CBM_HOME}/Tools/bin/doxygen" CACHE PATH "Path to doxygen executable")
set(DOT_EXECUTABLE "${CBM_HOME}/Tools/bin/dot" CACHE PATH "Path to dot executable")

# LSF setup
if (DEFINED ENV{LSF_BINDIR})
   set(LSF_HOME "$ENV{LSF_BINDIR}/.." CACHE PATH "Home dir for LSF binary files")
else ()
   set(LSF_HOME "/glb/apps/hpc/lsfprod/9.1/linux2.6-glibc2.3-x86_64" CACHE PATH "Home dir for LSF binary files")
endif ()
set(LSF_CAULDRON_PROJECT_NAME               "cldrn"     CACHE STRING "Project name for submitting jobs to LSF (for billing purpose)" )

# Paraview plugin setup
set(PARAVIEW_CONFIG_ROOT "${CBM_HOME}/Tools/paraview/Paraview-4.1.0-Release/Linux64x_26/lib/cmake/paraview-4.1"
      CACHE PATH "Path where ParaViewConfig.cmake can be found")

# TSLib Geocosm library setup
set( TSLIB_ROOT "${CBM_HOME}/hpc/tslib/tslib-${TSLIB_VERSION}" CACHE PATH "Path to Geocosm's TsLib" )
set( TSLIB_LIBRARY_DIR "${TSLIB_ROOT}/bin/x86_64_linux/Release" CACHE PATH "Path to Geocosm's TsLib library directory")

set( MCR_ROOT "/apps/sss/matlab/8.0" CACHE PATH "Path to Matlab")
set( MCR_VERSION "8.0" CACHE STRING "Matlab Version")

set( XERCES_ROOT "${TSLIB_ROOT}/3rdparty/Xerces/xerces-c-3.1.0-x86_64-linux-gcc-3.4" CACHE PATH "Path to Xerces-C library" )
set( XERCES_LIBRARIES "${XERCES_ROOT}/lib/libxerces-c-3.1.so" )
set( XSD_ROOT "${TSLIB_ROOT}/3rdparty/xsd/xsd-3.3.0-x86_64-linux-gnu/libxsd" CACHE PATH "Path to Codesynthesis's XSD library")





