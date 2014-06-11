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
set( MPIRUN "${MPI_ROOT64}/bin/mpirun" CACHE FILEPATH "Location of mpirun command" )

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

# Now figure out the compiler options to add
# of course, this depends on whether we want to links statically (default) or
# dynamically
option(MPI_LINKS_STATIC "Whether to link statically with the MPI" ON)

# The options that Intel MPI uses, can be found by doing 'mpiicpc -show' on
# the command line
set(MPI_LINK_FLAGS)
if (MPI_LINKS_STATIC)
  list(APPEND MPI_LINK_FLAGS "-static-intel")
else()
  list(APPEND MPI_LINK_FLAGS "-Xlinker" "-rpath")
  list(APPEND MPI_LINK_FLAGS "-Xlinker" "${MPI_ROOT64}/lib")
endif()
list(APPEND MPI_LINK_FLAGS "-Xlinker" "--enable-new-dtags")
list(APPEND MPI_LINK_FLAGS "-L${MPI_ROOT64}/lib")

string(REPLACE ";" " " MPI_LINK_FLAGS "${MPI_LINK_FLAGS}")

set(MPI_INCLUDE_DIRS "${MPI_ROOT64}/include")

find_library(MPI_DL_LIBRARY dl)
find_library(MPI_RT_LIBRARY rt)
find_library(MPI_PTHREAD_LIBRARY pthread)

set(MPI_LIBRARIES)
list(APPEND MPI_LIBRARIES "mpigc4" "mpigf" "mpi" "mpigi" ${MPI_DL_LIBRARY} ${MPI_RT_LIBRARY} ${MPI_PTHREAD_LIBRARY})

# Note: We only need C bindings. The C++ bindings sometimes give trouble
# because of SEEK_SET, etc... already being defined in stdio.h or iostream
add_definitions(-DMPICH_SKIP_MPICXX)

