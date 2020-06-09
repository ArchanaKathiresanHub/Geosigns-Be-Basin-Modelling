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

if (BM_PARALLEL)
   set( LIB_NAME "DistributedDataAccess" )

   set (OFPP "")
   if (UNIX)
      set (OFPP OneFilePerProcess)
   endif ()

   include_directories( SYSTEM
      ${PETSC_INCLUDE_DIRS}
      ${HDF5_INCLUDE_DIRS}
      ${MPI_INCLUDE_DIRS}
   )

   create_bm_library( TARGET ${LIB_NAME}
                      LIBRARIES DataAccess
                                Parallel_Hdf5
                                Utilities_Petsc
                                Serial_Hdf5
                                TableIO
                                FileSystem
                                CBMGenerics
                                utilities
                                MapSmoothing
                                ${HDF5_LIBRARIES}
                                ${MPI_LIBRARIES}
                                ${PETSC_LIBRARIES}
                                ${OFPP}
                      INSTALLTARGET )

   if (UNIX)
      add_dependencies(${LIB_NAME} PETSC)
   endif (UNIX)

   set_target_properties(${LIB_NAME}
       PROPERTIES LINK_FLAGS "${PETSC_LINK_FLAGS}"
   )
   if (UNIX)
      # In LSF environment (on LSF cluster node, when build is running as a LSF job) mpirun is trying to use job settings
      # to run mpi unit tests. Sometime it fails because build job requested just 1 cpu. To prevent this we can specify
      # machines file with localhost list only
      copy_test_file(machines)
      set( MACHINE_FILE -machinefile machines)
   endif (UNIX)


   add_gtest( NAME DistributedGridMap_MPInp4
              SOURCES test/DistributedGridMap.cpp
              COMPILE_FLAGS "-DNO_ASSERT_DEATH"
              LIBRARIES Utilities_Petsc ${PETSC_LIBRARIES} ${LIB_NAME} DataAccess
              LINK_FLAGS "${PETSC_LINK_FLAGS}"
              MPI_SIZE 4
              MPIRUN_PRMS ${MACHINE_FILE}
              FOLDER "${BASE_FOLDER}/${LIB_NAME}"
   )

   add_gtest( NAME DistributedGridMap_MPInp2
              SOURCES test/DistributedGridMap.cpp
              COMPILE_FLAGS "-DNO_ASSERT_DEATH"
              LIBRARIES Utilities_Petsc ${PETSC_LIBRARIES} ${LIB_NAME} DataAccess
              LINK_FLAGS "${PETSC_LINK_FLAGS}"
              MPI_SIZE 2
              MPIRUN_PRMS ${MACHINE_FILE}
              FOLDER "${BASE_FOLDER}/${LIB_NAME}"
   )

   # MPI death tests need special care, separate binary is required
   add_gtest( NAME DistributedGridMap_MPInp2::AssertDeath1
              SOURCES test/DistributedGridMap.cpp
              COMPILE_FLAGS "-DASSERT_DEATH_CTR_1"
              LIBRARIES Utilities_Petsc ${PETSC_LIBRARIES} ${LIB_NAME} DataAccess
              LINK_FLAGS "${PETSC_LINK_FLAGS}"
              MPI_SIZE 2
              MPIRUN_PRMS ${MACHINE_FILE}
              FOLDER "${BASE_FOLDER}/${LIB_NAME}"
   )

   # MPI death tests need special care, separate binary is required
   add_gtest( NAME DistributedGridMap_MPInp2::AssertDeath2
              SOURCES test/DistributedGridMap.cpp
              COMPILE_FLAGS "-DASSERT_DEATH_CTR_2"
              LIBRARIES Utilities_Petsc ${PETSC_LIBRARIES} ${LIB_NAME} DataAccess
              LINK_FLAGS "${PETSC_LINK_FLAGS}"
              MPI_SIZE 2
              MPIRUN_PRMS ${MACHINE_FILE}
              FOLDER "${BASE_FOLDER}/${LIB_NAME}"
   )

   # MPI death tests need special care, separate binary is required
   add_gtest( NAME DistributedGridMap_MPInp2::AssertDeath3
              SOURCES test/DistributedGridMap.cpp
              COMPILE_FLAGS "-DASSERT_DEATH_CTR_3"
              LIBRARIES Utilities_Petsc ${PETSC_LIBRARIES} ${LIB_NAME} DataAccess
              LINK_FLAGS "${PETSC_LINK_FLAGS}"
              MPI_SIZE 2
              MPIRUN_PRMS ${MACHINE_FILE}
              FOLDER "${BASE_FOLDER}/${LIB_NAME}"
   )

   # MPI death tests need special care, separate binary is required
   add_gtest( NAME DistributedGridMap_MPInp2::AssertDeathNotRetrieved
              SOURCES test/DistributedGridMap.cpp
              COMPILE_FLAGS "-DASSERT_DEATH_NOT_RETRIEVED"
              LIBRARIES Utilities_Petsc ${PETSC_LIBRARIES} ${LIB_NAME} DataAccess
              LINK_FLAGS "${PETSC_LINK_FLAGS}"
              MPI_SIZE 2
              MPIRUN_PRMS ${MACHINE_FILE}
              FOLDER "${BASE_FOLDER}/${LIB_NAME}"
   )

   # MPI death tests need special care, separate binary is required
   add_gtest( NAME DistributedGridMap_MPInp2::HangingCommunication
              SOURCES test/DistributedGridMap.cpp
              COMPILE_FLAGS "-DASSERT_THROW_ON_RESTORE"
              LIBRARIES Utilities_Petsc ${PETSC_LIBRARIES} ${LIB_NAME} DataAccess
              LINK_FLAGS "${PETSC_LINK_FLAGS}"
              MPI_SIZE 2
              MPIRUN_PRMS ${MACHINE_FILE}
              FOLDER "${BASE_FOLDER}/${LIB_NAME}"
   )
endif(BM_PARALLEL)

generate_dox( src/DistributedDataAccess.cfg )
