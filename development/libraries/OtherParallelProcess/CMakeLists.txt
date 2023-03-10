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

# Note: The .c source file are compiled with a c compiler and the .cpp source files with the c++ compiler. Those are then packed into a single archive

set( all_srcs    src/RewriteFileName.c src/h5merge.cpp src/fileHandler.cpp src/fileHandlerReuse.cpp src/fileHandlerAppend.cpp )
set( all_headers src/RewriteFileName.h src/h5merge.h src/fileHandler.h src/fileHandlerReuse.h src/fileHandlerAppend.h )

set( LIB_NAME "OtherParallelProcess" )
add_library(${LIB_NAME}
   ${all_srcs}
   ${all_headers}
)
target_include_directories( ${LIB_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src" )
set_target_properties( ${LIB_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${LIB_NAME}" )

include_directories( SYSTEM
  ${MPI_INCLUDE_DIRS}
  ${HDF5_INCLUDE_DIRS}
  ${HDF5_SOURCE_DIR}/src
)

target_link_libraries(${LIB_NAME}
      ${HDF5_LIBRARIES}
      ${MPI_LIBRARIES}
)

if (UNIX)
add_dependencies(${LIB_NAME} HDF5)
endif (UNIX)

if (UNIX)
   # In LSF environment (on LSF cluster node, when build is running as a LSF job) mpirun is trying to use job settings
   # to run mpi unit tests. Sometime it fails because build job requested just 1 cpu. To prevent this we can specify
   # machines file with localhost list only
   copy_test_file(machines)
   set( MACHINE_FILE -machinefile machines)
endif (UNIX)


add_gtest( NAME ${LIB_NAME}::rewriteFileName
           SOURCES test/RewriteFileName.cpp
           LIBRARIES ${LIB_NAME}
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::h5mergeTest-MPIPROCS=2
           SOURCES test/readDataset.cpp
           LIBRARIES ${LIB_NAME}
           MPI_SIZE 2
           MPIRUN_PRMS ${MACHINE_FILE}
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

