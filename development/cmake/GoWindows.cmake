find_program( MSTEST "mstest.exe"
       PATHS "C:/Program Files (x86)/Microsoft Visual Studio 10.0/Common7/IDE"
             "C:/Program Files (x86)/Microsoft Visual Studio 11.0/Common7/IDE" 
             "C:/Program Files (x86)/Microsoft Visual Studio 12.0/Common7/IDE" 
       DOC "Location of MSTest unit test driver"
     )

set(buildToolDirectory "c:/Apps")
if (IS_DIRECTORY "c:/opt")
  set(buildToolDirectory "c:/opt")
endif()
if (IS_DIRECTORY "E:/APPS")
  set(buildToolDirectory "E:/APPS")
endif()

#BOOST
set( BOOST_ROOT "${buildToolDirectory}/3rdparty/boost_1_59_0" CACHE PATH "Location of the Boost C++ libraries")

#PETSC 
set( PETSC_VERSION "3.5.2" CACHE STRING "PETSC Version")
set( PETSC_HOME "${buildToolDirectory}/3rdparty/PETSc_Windows-3.5.2" CACHE PATH "PETSc home path") 
set( INTEL_MKL_ROOT "${PETSC_HOME}/lib/intel64" CACHE PATH "Location of Intel MKL")

#HDF5
set( HDF5_ROOT "${buildToolDirectory}/3rdparty/hdf5.win64-1.8.11/hdf5" CACHE PATH "Path to the HDF5 library")
set( HDF5_FOUND TRUE)

#MPI
set( MPI_ROOT "${buildToolDirectory}/3rdparty/MicrosoftMPI-HPC-Pack-2012-R2" CACHE PATH "Directory where MPI is installed on Windows")

#Flexlm
set( FLEXLM_ROOT "${buildToolDirectory}/3rdparty/flexlm_11.12.1_Windows_VS11.0" CACHE PATH "Path to FlexLM directory" )
set( FLEXLM_LIBRARY "${FLEXLM_ROOT}/X64/EPTFlexLM_MDd.lib" CACHE FILEPATH "Path to Multi-threaded debug FlexLM libraries" )

#SWIG
set(SWIG_VERSION "3.0.2" CACHE STRING "The version number of the swig executable")
set(SWIG_DIR "${buildToolDirectory}/3rdparty/swigwin-${SWIG_VERSION}" CACHE PATH "The directory where SWIG is installed")
set(SWIG_FOUND TRUE)

#LSF libraries
set (LSF_INCLUDE_DIR "${buildToolDirectory}/3rdparty/LSF/include")
set (LSF_LIB "${buildToolDirectory}/3rdparty/LSF/lib/liblsf.lib")
set (LSBATCH_LIB "${buildToolDirectory}/3rdparty/LSF/lib/libbat.lib")
set( LSF_HOME "//europe.shell.com/tcs/ams/apps/hpc/lsfprod/9.1/linux2.6-glibc2.3-x86_64" CACHE PATH "Home dir for LSF binary files")
mark_as_advanced(LSF_INCLUDE_DIR)
mark_as_advanced(LSBATCH_LIB)
mark_as_advanced(LSF_LIB)

# Required libraries for touchstone (tslib, Matlab mcr, xerces and xsd)
set( TSLIB_VERSION "7.4.2" CACHE STRING "Tslib version")
set( TSLIB_ROOT "${buildToolDirectory}/geocosm" CACHE PATH "Path to Geocosm's TsLib" )
set( TSLIB_LIBRARY_DIR "${TSLIB_ROOT}/lib/x64")
set( TSLIB_GEOCOSMBASECPP "${TSLIB_LIBRARY_DIR}/geocosmxmllibbasecppD.lib" CACHE FILEPATH "Path to geocosm basecpp library")
set( TSLIB_GEOCOSMEXCEPTION "${TSLIB_LIBRARY_DIR}/GeocosmExceptionD.lib" CACHE FILEPATH "Path to geocosm exception library")
set( MCR_ROOT "${TSLIB_ROOT}/3rdparty/matlabmcr/matlab/v80" CACHE PATH "Location of Matlab Compiler Runtime" )
set( MCR_VERSION "80" CACHE STRING "Versio number of Matlab Compiler Runtime")
set( XERCES_ROOT "${TSLIB_ROOT}/3rdparty/Xerces/xerces-c-3.1.0-x86_64-windows-vc-9.0" CACHE PATH "Path to Xerces-C library" )
set( XERCES_LIBRARIES "${XERCES_ROOT}/lib/xerces-c_3D.lib" CACHE FILEPATH "Xerces-C library")
set( XSD_ROOT "${TSLIB_ROOT}/3rdparty/xsd/xsd-3.3.0-i686-windows/libxsd" CACHE PATH "Path to Codesynthesis's XSD library")

# other variables
set( BM_PLATFORM_ID "MSWindows_Amd64" CACHE STRING "An identifier for the platform (OS, etc...) for this is build" )
option( BM_CSHARP_API "Build the C sharp interface" ON)
option( BM_USE_FLEXLM "Whether to require a license via FlexLM" OFF)
set(BM_CLOCK_GETTIME_LIB "")
set(BM_DL_LIB "" )
