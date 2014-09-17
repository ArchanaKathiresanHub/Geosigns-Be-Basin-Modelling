find_program( MSTEST "mstest.exe"
       PATHS "C:/Program Files (x86)/Microsoft Visual Studio 10.0/Common7/IDE"
             "C:/Program Files (x86)/Microsoft Visual Studio 11.0/Common7/IDE"
       DOC "Location of MSTest unit test driver"
     )

set( BOOST_ROOT "c:/opt/boost_1_55_0" CACHE PATH "Location of the Boost C++ libraries")

set( BM_PLATFORM_ID "MSWindows_Amd64" CACHE STRING "An identifier for the platform (OS, etc...) for this is build" )

option( BM_CSHARP_API "Build the C sharp interface" ON)
option( BM_USE_FLEXLM "Whether to require a license via FlexLM" OFF)

set( INTEL_MKL_ROOT "${CMAKE_SOURCE_DIR}/../3rdparty/PETSc_Windows-3.4.4/lib/intel64" CACHE PATH "Location of Intel MKL")

