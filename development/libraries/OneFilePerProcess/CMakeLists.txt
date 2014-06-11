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

set( all_srcs    src/HDF5VirtualFileDriver.c src/RewriteFileName.c src/h5merge.C )
set( all_headers src/HDF5VirtualFileDriver.h src/RewriteFileName.h src/h5merge.h )

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

set_target_properties(OneFilePerProcess
      PROPERTIES LINK_FLAGS "${MPI_LINK_FLAGS}"
)

add_gtest( NAME OneFilePerProcess::rewriteFileName
           SOURCES test/RewriteFileName.C
           LINK_FLAGS "${MPI_LINK_FLAGS}"
           LIBRARIES OneFilePerProcess
)

add_gtest( NAME OneFilePerProcess::HDF5VirtualFileDriver
           SOURCES test/HDF5VirtualFileDriver.C
           LINK_FLAGS "${MPI_LINK_FLAGS}"
           LIBRARIES OneFilePerProcess
)

add_gtest( NAME OneFilePerProcess::HDF5VirtualFileDriver-MPIPROCS=2
           SOURCES test/HDF5VirtualFileDriver.C
           LINK_FLAGS "${MPI_LINK_FLAGS}"
           LIBRARIES OneFilePerProcess
           MPI_SIZE 2
)
add_gtest( NAME OneFilePerProcess::h5mergeTest-MPIPROCS=2
           SOURCES test/readDataset.C
           LINK_FLAGS "${MPI_LINK_FLAGS}"
           LIBRARIES OneFilePerProcess
           MPI_SIZE 2
)

