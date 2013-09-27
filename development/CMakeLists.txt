#########################################################################
#                                                                       #
# Copyright (C) 2012-2013 Shell International Exploration & Production. #
# All rights reserved.                                                  #
#                                                                       #
# Developed under license for Shell by PDS BV.                          #
#                                                                       #
# Confidential and proprietary source code of Shell.                    #
# Do not distribute without written permission from Shell.              #
#                                                                       #
#########################################################################

#
# Determine minimum toolset: CMake & C++ Compiler
#
cmake_minimum_required(VERSION 2.8)
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}")
include(cmake/EnvSetup.cmake)
init_environment()
find_package(Compiler CONFIG)

# 
# Start the project description
#
Project(BasinModeling)

#
# General Include files
#

include(cmake/PredefinedVariables.cmake)
include(cmake/BasinModelingUtilities.cmake)

include(FeatureSummary)

#
# Configurable options
#
option( BM_CSHARP_API "Build the C sharp interface (Windows only)" ${WIN32} )
option( BM_PARALLEL "Build the parallel cauldron applications" ${UNIX} )
option( BM_USE_OWN_MPICH_LOCK "Use our custom ADIOI_Set_lock functions in MPI library (Linux only)" ON )
option( BM_BUILD_SERIAL_APPS "Also build serial version of apps" ON )
option( BUILD_SHARED_LIBS "Build shared libraries" OFF )
option( BM_PARALLEL "Build the parallel basin modeling applications" ON )
option( BM_BUILD_DOCS "Build documentation" OFF )
option( BM_BUILD_ENABLE_TESTING "Whether testing is enabled" ON )
option( BM_CODE_COVERAGE_ENABLE "Enable code coverage" OFF )
if(BM_CODE_COVERAGE_ENABLE)
	set( BM_CODE_COVERAGE_DIR "${PROJECT_BINARY_DIR}/profiled" CACHE PATH "Code coverage directory" )
endif()

option( BM_ENABLE_CAULDRON2VOXET_DERIVED_PROPERTIES "Enable ability to compute derived properties. This adds a dependency FunctionParser which is an LGPL licensed open source package." OFF)
option( BM_INSTALL_BENCHMARK_SUITE "Install the cauldron benchmark suite" OFF )
set( BM_THIRD_PARTY_COMPONENT_LIST_FILE  "ThirdPartyComponentsList.xml" CACHE FILEPATH "File name of third party component list")
set(CMAKE_INSTALL_PREFIX "" CACHE PATH "Installation prefix" )
set(CMAKE_BUILD_TYPE "" CACHE STRING "CMake build type ('Release' or 'Debug')")
set(SSSDEV_INSTALL_DIR "/apps/sssdev/ibs" CACHE PATH "sssdev installation path")

set(BM_VERSION_NUMBER_MAJOR "2013" CACHE STRING "Numerical version major string. Anything like 2013")
set(BM_VERSION_NUMBER_MINOR "01" CACHE STRING "Numerical version minor string. Anything like 01")
set(BM_VERSION_NUMBER ${BM_VERSION_NUMBER_MAJOR}.${BM_VERSION_NUMBER_MINOR})
set(BM_VERSION_TAG "nightly" CACHE STRING "Version tag. E.g. nightly, cmake, etc")
set(BM_SVNREVISION "-9999" CACHE STRING "Source control revision number (TFS or SVN)")

set(BM_TFS_BUILD_NUMBER "" CACHE STRING "TFS Build identifier (aka BuildDetail.BuildNumber)")
set(BM_TFS_SERVER_URL "" CACHE STRING "TFS Collection URL of Cauldron")
set(BM_TFS_PROJECT_NAME "" CACHE STRING "TFS Project Name (i.e. 'Basin Modeling'")

if(NOT BM_BUILD_NUMBER)
    set(BM_BUILD_NUMBER 0)
endif()

if(NOT BM_REVISION)
	set(BM_REVISION 0)
endif()

set(BM_VERSION_STRING "v${BM_VERSION_NUMBER}${BM_VERSION_TAG}")

if(NOT CMAKE_BUILD_TYPE)
	set (CMAKE_BUILD_TYPE Release)
endif()

#
# Setting compile flags
#   

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
   option( BM_USE_MSVC_DYNAMIC_LIBS "Use the dynamic C runtime" ON )
#  if (BM_USE_MSVC_DYNAMIC_LIBS)
#     configure_msvc_runtime("dynamic")
#  else()
#    configure_msvc_runtime("static")
#  endif()

   if (NOT BUILD_SHARED_LIBS)
      add_definitions(-DCBMGENERICS_STATIC)
      add_definitions(-DUTILITIES_STATIC)
      add_definitions(-DSERIALHDF5_STATIC)
      add_definitions(-DTABLEIO_STATIC)
   endif()
endif()

if(UNIX)
    # The library to be compiled with -O3 is EosPack, all others, 
    # including the applications are to be compiled with -O2.
    # Also we want assert(3) in Release configuration
    set(CMAKE_CXX_FLAGS_RELEASE "-O2")
    # IMPORTANT: somehow -fPIC affects regression test results
    add_definitions(-fPIC)
    if(BM_CODE_COVERAGE_ENABLE)
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

if (BM_PARALLEL)
   find_package(MPI REQUIRED CONFIG)
   if (BM_USE_OWN_MPICH_LOCK)
      find_package(MpichLock REQUIRED CONFIG)
   endif()
endif()

find_package(Hdf5 REQUIRED CONFIG)
find_package(OpenMP REQUIRED CONFIG)   

if (UNIX)
   find_package(BLAS REQUIRED CONFIG)
   find_package(PETSC REQUIRED CONFIG)
   find_package(FlexLM REQUIRED CONFIG)
   find_package(Touchstone REQUIRED CONFIG)
   find_package(Qt3 REQUIRED CONFIG)
   find_package(Boost REQUIRED CONFIG)

   if (BM_ENABLE_CAULDRON2VOXET_DERIVED_PROPERTIES)
      find_package(FunctionParser REQUIRED CONFIG)
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
   set(script_suffix ".csh")
endif()

configure_file(InstallAll${script_suffix}.cmake InstallAll${script_suffix} @ONLY)
configure_file(MoveInstalls${script_suffix}.cmake MoveInstalls${script_suffix} @ONLY)

# generate environment set-up file
include(cmake/GoCBM.cmake)


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
