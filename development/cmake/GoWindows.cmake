find_program( MSTEST "mstest.exe"
       PATHS "C:/Program Files (x86)/Microsoft Visual Studio 10.0/Common7/IDE"
             "C:/Program Files (x86)/Microsoft Visual Studio 11.0/Common7/IDE"
       DOC "Location of MSTest unit test driver"
     )

set( BOOST_ROOT "c:/opt/boost_1_55_0" CACHE PATH "Location of the Boost C++ libraries")

set( BM_PLATFORM_ID "MSWindows" CACHE STRING "An identifier for the platform (OS, etc...) for this is build" )

option( BM_CSHARP_API "Build the C sharp interface" ON)

