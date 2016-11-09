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

set( all_srcs    src/HDF5VirtualFileDriver.c src/RewriteFileName.c src/h5merge.C src/fileHandler.C src/fileHandlerReuse.C src/fileHandlerAppend.C )
set( all_headers src/HDF5VirtualFileDriver.h src/RewriteFileName.h src/h5merge.h src/fileHandler.h src/fileHandlerReuse.h src/fileHandlerAppend.h )

set_source_files_properties( src/HDF5VirtualFileDriver.c 
      PROPERTIES COMPILE_FLAGS "-std=c99"
)

add_library(OneFilePerProcess
	${all_srcs}
	${all_headers}
)

include_directories(
	src
)

include_directories( SYSTEM
  ${MPI_INCLUDE_DIRS}
  ${HDF5_INCLUDE_DIRS}
  ${HDF5_SOURCE_DIR}/src
)

target_link_libraries(OneFilePerProcess
      ${HDF5_LIBRARIES}
      ${MPI_LIBRARIES}
)

if (UNIX)
add_dependencies(OneFilePerProcess HDF5)
endif (UNIX)

if (UNIX)
   # In LSF environment (on LSF cluster node, when build is running as a LSF job) mpirun is trying to use job settings
   # to run mpi unit tests. Sometime it fails because build job requested just 1 cpu. To prevent this we can specify
   # machines file with localhost list only
   configure_file(test/machines machines COPYONLY )
   set( MACHINE_FILE -machinefile machines)
endif (UNIX)
                                    
            
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
           MPIRUN_PRMS ${MACHINE_FILE}
)
add_gtest( NAME OneFilePerProcess::h5mergeTest-MPIPROCS=2
           SOURCES test/readDataset.C
           LIBRARIES OneFilePerProcess
           MPI_SIZE 2
           MPIRUN_PRMS ${MACHINE_FILE}
)

