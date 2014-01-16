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

include(cmake/AddPackage.cmake)
include(cmake/EnvSetup.cmake)

set( MPI_ROOT "/apps/3rdparty/intel/impi/4.1.1.036" CACHE PATH "Path to MPI library" )
set( MPI_ROOT64 "${MPI_ROOT}/intel64" )

add_environment_source_script(CSHELL "${MPI_ROOT64}/bin/mpivars.csh")
add_environment_source_script(BOURNE "${MPI_ROOT64}/bin/mpivars.sh")

add_external_package_info(
       CAPABILITY MPI
       NAME         "MPI"
       VENDOR       "Intel"
       VERSION      "4.1.1.036"
       LICENSE_TYPE "Commercial"
       LICENSE_FILE "${MPI_ROOT}/mpiEULA.txt"
       URL          "http://software.intel.com/en-us/intel-mpi-library"
       DESCRIPTION  "Intel's MPI implementation"
       REQUIRED_AT  "Runtime"
       COUNTRY_OF_ORIGIN "USA"
       SHIPPED      "Yes"
       INCLUSION_TYPE "Static Link"
       USEABLE_STAND_ALONE "No"
       CONTAINS_CRYPTO "No"
       ECCN         "Unknown"
)

set(MPI_FOUND TRUE)

# The options that Intel MPI uses, can be found by doing 'mpiicpc -show' on
# the command line
set(MPI_INCLUDE_DIRS "${MPI_ROOT64}/include")
set(MPI_LIBRARIES)
foreach(LIB mpigc4 mpigf mpi mpigi)
   list(APPEND MPI_LIBRARIES "${MPI_ROOT64}/lib/lib${LIB}.a")
endforeach()

find_library(MPI_DL_LIBRARY dl)
find_library(MPI_RT_LIBRARY rt)
find_library(MPI_PTHREAD_LIBRARY pthread)

list(APPEND MPI_LIBRARIES ${MPI_DL_LIBRARY} ${MPI_RT_LIBRARY} ${MPI_PTHREAD_LIBRARY})

# Uncomment the following if linked with dynamic libraries
#set(MPI_LINK_FLAGS)
#  list(APPEND MPI_LINK_FLAGS "--enable-new-dtags")
#  list(APPEND MPI_LINK_FLAGS "-rpath")
#  list(APPEND MPI_LINK_FLAGS "${MPI_ROOT64}/lib")

# Note: We only need C bindings. The C++ bindings sometimes give trouble
# because of SEEK_SET, etc... already being defined in stdio.h or iostream
# add_definitions(-DMPICH_SKIP_MPICXX)

