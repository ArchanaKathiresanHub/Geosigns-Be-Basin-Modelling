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

cmake_minimum_required(VERSION 2.8)

#
# Load system-dependent configuration
#
if (UNIX AND EXISTS /apps/sss/share/getos2 )
   set(defaultPreset "ShellLinux")
elseif(WIN32)
   set(defaultPreset "Windows")
else()
   set(defaultPreset OFF)
endif()

set( BM_CONFIG_PRESET "${defaultPreset}" CACHE STRING "Reference to CMake file containing configuration presets. You can choose from: ShellLinux, Windows, and OFF") 
if ( BM_CONFIG_PRESET )
   include("cmake/Go${BM_CONFIG_PRESET}.cmake")
endif()

#
# Set location to third party components that are included in TFS
#
set(THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty")

#
# Options whether to build certain parts
#
option( BM_CSHARP_API "Build the C sharp interface (Windows only)" OFF )
option( BM_PARALLEL "Build the parallel cauldron applications" ON )
option( BM_BUILD_QT3_APPS "Build programs that need QT3" OFF)
option( BM_BUILD_PARAVIEW_PLUGIN "Also build the ParaView plugin to view cauldron projects in ParaView."  OFF)
option( BM_BUILD_SERIAL_APPS "Also build serial version of apps" ON )
option( BM_BUILD_DOCS "Build documentation" ON )
option( BM_BUILD_ENABLE_TESTING "Whether testing is enabled" ON )
option( BM_CODE_COVERAGE_ENABLE "Enable code coverage" OFF )
option( BM_INSTALL_BENCHMARK_SUITE "Install the cauldron benchmark suite" OFF )
option( BM_USE_FLEXLM "Whether to require a license via FlexLM" OFF)

#
# Variables concerning version numbers
#
set(BM_VERSION_NUMBER_MAJOR "2014" CACHE STRING "Numerical version major string. Anything like 2014")
set(BM_VERSION_NUMBER_MINOR "01" CACHE STRING "Numerical version minor string. Anything like 01")
set(BM_VERSION_NUMBER ${BM_VERSION_NUMBER_MAJOR}.${BM_VERSION_NUMBER_MINOR})
set(BM_VERSION_TAG "nightly" CACHE STRING "Version tag. E.g. nightly, cmake, etc")
set(BM_VERSION_STRING "v${BM_VERSION_NUMBER}${BM_VERSION_TAG}")
set(BM_SVNREVISION "-9999" CACHE STRING "Source control revision number (TFS or SVN)")
set(BM_BUILD_NUMBER "0" CACHE STRING "Build number (Used to number Windows DLLs)")
set(BM_REVISION "0" CACHE STRING "Revision number (Used to number Windows DLLs)")

#
# Various configurable variables
#

set( BM_THIRD_PARTY_COMPONENT_LIST_FILE  "ThirdPartyComponentsList.xml" CACHE FILEPATH "File name of third party component list")

set(BM_SSSDEV_INSTALL_DIR "${CMAKE_BINARY_DIR}/sssdev" CACHE PATH "sssdev installation path")
set(BM_TFS_BUILD_NUMBER "" CACHE STRING "TFS Build identifier (aka BuildDetail.BuildNumber)")
set(BM_TFS_SERVER_URL "" CACHE STRING "TFS Collection URL of Cauldron")
set(BM_TFS_PROJECT_NAME "" CACHE STRING "TFS Project Name (i.e. 'Basin Modeling'")


#
# Common CMake configuration flags
# 

option( BUILD_SHARED_LIBS "Build shared libraries" OFF )
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE PATH "Installation prefix" )
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "CMake build type ('Release' or 'Debug')")


#
# Determine minimum toolset: CMake & C++ Compiler
#
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
include(cmake/EnvSetup.cmake)
init_environment()
find_package(Compiler CONFIG)

# 
# Start the project description & enable the compiler
#
Project(BasinModeling)

#
# General Include files
#

include(cmake/BasinModelingUtilities.cmake)
include(cmake/ExternalProject.cmake)
include(FeatureSummary)
include(cmake/ConfigurationFlags.cmake)
include(cmake/WarningFlags.cmake)


#
# Detect and set the configuration of 3rd party components
#

configure_external_projects()

#
# Setting compile flags
#   

if (NOT BM_USE_FLEXLM)
   add_definitions(-DDISABLE_FLEXLM=1)
endif()

if(MSVC)
   if (CMAKE_CL_64)
      set(BM_WINDOWS_PLATFORM "x64")
   else (CMAKE_CL_64)
      set(BM_WINDOWS_PLATFORM "Win32")
   endif (CMAKE_CL_64)
   # remove  /Zm1000 from command line options
   string(REPLACE "/Zm1000" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
   string(REPLACE "/Zm1000" "" CMAKE_C_FLAGS ${CMAKE_CXX_FLAGS})

   message(STATUS "BM_WINDOWS_PLATFORM = ${BM_WINDOWS_PLATFORM}")

   add_definitions( "-Dand=&&" "-Dor=||" "-Dnot=!" )
#if (BUILD_SHARED_LIBS)
#     configure_msvc_runtime("dynamic")
#   else()
#     configure_msvc_runtime("static")
#   endif()

   if (NOT BUILD_SHARED_LIBS)
      add_definitions(-DCBMGENERICS_STATIC)
      add_definitions(-DUTILITIES_STATIC)
      add_definitions(-DSERIALHDF5_STATIC)
      add_definitions(-DTABLEIO_STATIC)
      add_definitions(-DSUMLIB_STATIC)
   endif()
endif()

if(UNIX)
    # The library to be compiled with -O3 is EosPack, all others, 
    # including the applications are to be compiled with -O2.
    # Also we want assert(3) in Release configuration
    set(CMAKE_CXX_FLAGS_RELEASE "-O2")
    set(CMAKE_C_FLAGS_RELEASE "-O2")

    # Depending on whether we want to link statically or dynamicaly, ...
    if (BUILD_SHARED_LIBS)
       # Then we want the search paths for the Shared Objects (.so files) to
       # be included in the binaries
       set( CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)
    else()
       # Search for static libraries only.
       set( CMAKE_FIND_LIBRARY_SUFFIXES ".a" ".so")

       # NOTE: We don't force pure static linking with the -static flag.
       # Pure static linking is a bad idea as is reported by many people over
       # the Internet, because:
       # 1) Glibc is LGPL licensed. Linking statically with it means that you
       #    have to distribute your object file with it. This is impractical
       # 2) Even when you link with Glibc statically, it is still required
       #    that a matching version of the libc.so shared library file is
       #    available on the system, because its NSS functions (gethostbyname,
       #    etc...) depend on it.
       # 3) Linking statically with libgcc is also a bad idea if you link with
       #    3rd party libraries that link with it dynamically. 
       # 4) Linking statically with libstdc++ is considered to be a good idea,
       #    but with GCC versions < 4.5 this is quite cumbersome.

    endif()

    # Extra stuff if want code coverage instrumentation in the build
    if(BM_CODE_COVERAGE_ENABLE)
       set( BM_CODE_COVERAGE_DIR "${PROJECT_BINARY_DIR}/profiled" CACHE PATH "Code coverage directory" )
       mark_as_advanced( BM_CODE_COVERAGE_DIR )
       file(MAKE_DIRECTORY ${BM_CODE_COVERAGE_DIR})
       set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -prof-gen=srcpos -prof-dir ${BM_CODE_COVERAGE_DIR}")
       set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g")
    endif()
endif()

add_definitions(-DIBSFLEXLMVERSION="2008.04")
add_definitions(-DIBSLONGVERSION="v${BM_VERSION_NUMBER}")
add_definitions(-DIBSSHORTVERSION="v${BM_VERSION_NUMBER}")
add_definitions(-DIBSLONGVERSION="v${BM_VERSION_NUMBER}")
add_definitions(-DSVNREVISION="${BM_SVNREVISION}")


#
# Include 3rd party packages
#

if(BM_BUILD_ENABLE_TESTING)
   enable_testing()
   find_package(TestFramework REQUIRED CONFIG)
endif()

if (BM_CSHARP_API)
   find_package(Swig REQUIRED CONFIG)
endif()

find_package(OpenMP REQUIRED CONFIG)   
find_package(Hdf5 REQUIRED CONFIG)
find_package(Eigen REQUIRED CONFIG)
find_package(SUMlib REQUIRED CONFIG)
find_package(Boost REQUIRED CONFIG)
find_package(BLAS REQUIRED CONFIG)
find_package(PETSC REQUIRED CONFIG)
   
if (UNIX)
   find_package(FlexLM REQUIRED CONFIG)
   find_package(Touchstone REQUIRED CONFIG)
   find_package(LSF REQUIRED CONFIG)
   
   if (BM_BUILD_QT3_APPS)
      find_package(Qt3 REQUIRED CONFIG)
   endif()

   if (BM_BUILD_PARAVIEW_PLUGIN)
      find_package(ParaView REQUIRED CONFIG)
      include(${PARAVIEW_USE_FILE})
   endif()
endif(UNIX)

if(BM_BUILD_DOCS)
   find_package(Doxygen CONFIG)
endif()

#
# Include the subdirectories that have to be built
#

add_subdirectory(libraries)
add_subdirectory(applications)
if(BM_BUILD_REGRESSION_TESTS)
	add_subdirectory(regression_tests)
endif()

#
# Install configuration files
# 

install(DIRECTORY geocase/misc/ # last backslach is MANDATORY
	DESTINATION misc
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ
    DIRECTORY_PERMISSIONS 
        OWNER_READ OWNER_WRITE OWNER_EXECUTE 
        GROUP_READ GROUP_WRITE GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
	PATTERN "geocase/misc/gempis/*"
	    PERMISSIONS
		    OWNER_READ OWNER_WRITE OWNER_EXECUTE 
		    GROUP_READ GROUP_WRITE GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE
    )

#
# Generate scripts
#
if (WIN32)
   set(script_suffix ".bat")
else ()
   set(script_suffix ".sh")
endif()

configure_file(InstallAll${script_suffix}.cmake InstallAll${script_suffix} @ONLY)
configure_file(MoveInstalls${script_suffix}.cmake MoveInstalls${script_suffix} @ONLY)

#
# Write additional environment variables to build environment set-up file
#
include(cmake/BuildEnvironment.cmake)


#
# Generate Thirdy Party Component List XML file
# 
write_external_package_info_file(${CMAKE_BINARY_DIR}/${BM_THIRD_PARTY_COMPONENT_LIST_FILE})

install( FILES "${CMAKE_BINARY_DIR}/${BM_THIRD_PARTY_COMPONENT_LIST_FILE}"
               "${CMAKE_BINARY_DIR}/ThirdPartyComponentsListStyle.xsl"
	     DESTINATION share/doc
)		 

#
# Reporting
#
feature_summary(WHAT ALL)


# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
