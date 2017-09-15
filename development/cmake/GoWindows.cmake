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
option(BM_CSHARP_API "Build the C sharp interface" ON)

# Version of .NET for C# projects
set(DONET_TARGET_VERSION "4.5.2") 

STRING(REPLACE "." "" DONET_TARGET_VERSION_SHORT ${DONET_TARGET_VERSION})

# Check that platform was chosen correctly
if (IS_DIRECTORY "c:/Apps/3rdparty/")
   set(CBM_HOME "c:/Apps")

elseif (IS_DIRECTORY "e:/Apps/3rdparty/")
   set(CBM_HOME "e:/Apps")

elseif (IS_DIRECTORY "c:/opt/3rdparty/")
  set(CBM_HOME "c:/opt")

else()
   message(FATAL_ERROR "Can not find 3rdparty tools set in the standard places: c:/Appa, c:/opt, e:/Apps.")
endif()

set(BM_EXTERNAL_COMPONENTS_DIR "${CBM_HOME}/3rdparty" CACHE PATH "The path to the directory of prebuilt libraries")

find_program( MSTEST "mstest.exe"
       PATHS "C:/Program Files (x86)/Microsoft Visual Studio 14.0/Common7/IDE"
             "C:/Program Files (x86)/Microsoft Visual Studio 12.0/Common7/IDE"
             "C:/Program Files (x86)/Microsoft Visual Studio 11.0/Common7/IDE" 
             "C:/Program Files (x86)/Microsoft Visual Studio 10.0/Common7/IDE" 
       DOC "Location of MSTest unit test driver"
     )

#BOOST
set(BOOST_ROOT "${BM_EXTERNAL_COMPONENTS_DIR}/boost_1_65_1" CACHE PATH "Location of the Boost C++ libraries")

#PETSC 
if (IS_DIRECTORY "${BM_EXTERNAL_COMPONENTS_DIR}/PETSc_Windows-${PETSC_VERSION}")
   set(PETSC_HOME "${BM_EXTERNAL_COMPONENTS_DIR}/PETSc_Windows-${PETSC_VERSION}" CACHE PATH "PETSc home path") 
else()
   message( FATAL_ERROR "Can not find PETSc in ${BM_EXTERNAL_COMPONENTS_DIR}/PETSc_Windows-${PETSC_VERSION}")
endif()

#MKL
set(INTEL_MKL_ROOT "${PETSC_HOME}/lib/intel64" CACHE PATH "Location of Intel MKL")

# other variables
set(BM_PLATFORM_ID "MSWindows_Amd64" CACHE STRING "An identifier for the platform (OS, etc...) for this is build" )
set(BM_CLOCK_GETTIME_LIB "")
set(BM_DL_LIB "")

#HDF5
#set(ENV{HDF5_DIR} "${BM_EXTERNAL_COMPONENTS_DIR}/hdf5.win64-${HDF5_VERSION}/cmake")
if (IS_DIRECTORY "${BM_EXTERNAL_COMPONENTS_DIR}/hdf5.win64-${HDF5_VERSION}/include")
   set( HDF5_FOUND TRUE)
   set( HDF5_ROOT "${BM_EXTERNAL_COMPONENTS_DIR}/hdf5.win64-${HDF5_VERSION}" CACHE PATH "Path to the HDF5 library")
else()
   set( HDF5_FOUND FALSE )
   message( FATAL_ERROR "Can not find HDF5 in ${BM_EXTERNAL_COMPONENTS_DIR}/hdf5.win64-${HDF5_VERSION}" )
endif()

#MPI
if (IS_DIRECTORY "${BM_EXTERNAL_COMPONENTS_DIR}/MSMpi7.1")
   set(MPI_ROOT "${BM_EXTERNAL_COMPONENTS_DIR}/MSMpi7.1" CACHE PATH "Directory where MPI is installed on Windows")
   set(MPI_FOUND TRUE)
   set(MPI_VERSION "7.1" CACHE STRING "The version number MS MPI")
else()
   message( FATAL_ERROR "Can not find MS MPI ${BM_EXTERNAL_COMPONENTS_DIR}/MSMpi7.1" )
   set(MPI_FOUND FALSE)
endif()

#Flexlm
#set( FLEXLM_ROOT "${BM_EXTERNAL_COMPONENTS_DIR}/flexlm_11.12.1_Windows_VS11.0" CACHE PATH "Path to FlexLM directory" )
#set( FLEXLM_LIBRARY "${FLEXLM_ROOT}/X64/EPTFlexLM_MDd.lib" CACHE FILEPATH "Path to Multi-threaded debug FlexLM libraries" )

#SWIG
if (IS_DIRECTORY "${BM_EXTERNAL_COMPONENTS_DIR}/swigwin-${SWIG_VERSION}" )
   set(SWIG_DIR "${BM_EXTERNAL_COMPONENTS_DIR}/swigwin-${SWIG_VERSION}" CACHE PATH "The directory where SWIG is installed")
   set(SWIG_FOUND TRUE)
else()
   message( FATAL_ERROR "Can not find Swig in ${BM_EXTERNAL_COMPONENTS_DIR}/swigwin-${SWIG_VERSION}" )
endif()

# LSF setup
if (IS_DIRECTORY "${BM_EXTERNAL_COMPONENTS_DIR}/LSF/include" )
   set(LSF_INCLUDE_DIR "${BM_EXTERNAL_COMPONENTS_DIR}/LSF/include")
   set(LSF_LIB "${BM_EXTERNAL_COMPONENTS_DIR}/LSF/lib/liblsf.lib")
   set(LSBATCH_LIB "${BM_EXTERNAL_COMPONENTS_DIR}/LSF/lib/libbat.lib")
   set(LSF_HOME "//europe.shell.com/tcs/ams/apps/hpc/lsfprod/9.1/linux2.6-glibc2.3-x86_64" CACHE PATH "Home dir for LSF binary files")
   mark_as_advanced(LSF_INCLUDE_DIR)
   mark_as_advanced(LSBATCH_LIB)
   mark_as_advanced(LSF_LIB)
   set(LSF_FOUND TRUE)
else()
   set(LSF_FOUND FALSE)
   message( WARNING "Can not find LSF in ${BM_EXTERNAL_COMPONENTS_DIR}/LSF." )
endif()

# TSLib Geocosm library setup
set( TSLIB_ROOT "${BM_EXTERNAL_COMPONENTS_DIR}/geocosm" CACHE PATH "Path to Geocosm's TsLib" )
set( TSLIB_LIBRARY_DIR "${TSLIB_ROOT}/lib/x64")
set( TSLIB_GEOCOSMBASECPP "${TSLIB_LIBRARY_DIR}/geocosmxmllibbasecppD.lib" CACHE FILEPATH "Path to geocosm basecpp library")
set( TSLIB_GEOCOSMEXCEPTION "${TSLIB_LIBRARY_DIR}/GeocosmExceptionD.lib" CACHE FILEPATH "Path to geocosm exception library")

set( MCR_ROOT "${TSLIB_ROOT}/3rdparty/matlabmcr/matlab/v80" CACHE PATH "Location of Matlab Compiler Runtime" )
set( MCR_VERSION "80" CACHE STRING "Versio number of Matlab Compiler Runtime")

set( XERCES_ROOT "${TSLIB_ROOT}/3rdparty/Xerces/xerces-c-3.1.0-x86_64-windows-vc-9.0" CACHE PATH "Path to Xerces-C library" )
set( XERCES_LIBRARIES "${XERCES_ROOT}/lib/xerces-c_3D.lib" CACHE FILEPATH "Xerces-C library")
set( XSD_ROOT "${TSLIB_ROOT}/3rdparty/xsd/xsd-3.3.0-i686-windows/libxsd" CACHE PATH "Path to Codesynthesis's XSD library")

