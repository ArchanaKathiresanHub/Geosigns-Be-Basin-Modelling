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

set( BOOST_ROOT "${buildToolDirectory}/boost_1_59_0" CACHE PATH "Location of the Boost C++ libraries")

set( BM_PLATFORM_ID "MSWindows_Amd64" CACHE STRING "An identifier for the platform (OS, etc...) for this is build" )

option( BM_CSHARP_API "Build the C sharp interface" ON)
option( BM_USE_FLEXLM "Whether to require a license via FlexLM" OFF)

set( INTEL_MKL_ROOT "${CMAKE_SOURCE_DIR}/../3rdparty/PETSc_Windows-3.5.2/lib/intel64" CACHE PATH "Location of Intel MKL")

set( LSF_HOME "//europe.shell.com/tcs/ams/apps/hpc/lsfprod/9.1/linux2.6-glibc2.3-x86_64" CACHE PATH "Home dir for LSF binary files")

set(BM_CLOCK_GETTIME_LIB "")
set(BM_DL_LIB "" )

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
