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

set( all_srcs    src/HDF5VirtualFileDriver.c src/RewriteFileName.c )
set( all_headers src/HDF5VirtualFileDriver.h src/RewriteFileName.h )

add_definitions(-DH5_HAVE_PARALLEL -std=c99)
add_library(OneFilePerProcess
	${all_srcs}
	${all_headers}
)

include_directories( 
	src
        ${MPI_INCLUDE_DIRS}
        ${HDF5_parallel_INCLUDE_DIRS}
        ${HDF5_SOURCE_DIR}/src
)

target_link_libraries(OneFilePerProcess
      ${HDF5_parallel_LIBRARIES}
      ${MPI_LIBRARIES}
)

add_gtest( NAME OneFilePerProcess::rewriteFileName
           SOURCES test/RewriteFileName.C
           LIBRARIES OneFilePerProcess
)

add_gtest( NAME OneFilePerProcess::HDF5VirtualFileDriver
           SOURCES test/HDF5VirtualFileDriver.C
           LIBRARIES OneFilePerProcess
)

add_gtest( NAME OneFilePerProcess::HDF5VirtualFileDriver-MPIPROCS=2
           SOURCES test/HDF5VirtualFileDriver.C
           LIBRARIES OneFilePerProcess
           MPI_SIZE 2
)

