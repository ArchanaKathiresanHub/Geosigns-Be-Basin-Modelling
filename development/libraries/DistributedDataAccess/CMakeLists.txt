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

   FILE( GLOB all_headers src/Interface/*.h )
   source_group(include FILES ${all_headers})

   FILE( GLOB all_srcs src/*.C)
   source_group(source FILES ${all_srcs})

   add_library(DistributedDataAccess 
      ${all_srcs}
      ${all_headers}
   )
   if (UNIX)
      add_dependencies(DistributedDataAccess PETSC)
   endif (UNIX)

   set (OFPP "")
   if (UNIX)
      set (OFPP OneFilePerProcess)
   endif ()

   target_link_libraries( DistributedDataAccess 
      DataAccess
      Utilities_Petsc
      Serial_Hdf5
      Parallel_Hdf5
      TableIO
      FileSystem
      utilities
      CBMGenerics
      ${HDF5_LIBRARIES}
      ${MPI_LIBRARIES}
      ${PETSC_LIBRARIES}
      ${OFPP}
   )

   bm_include_libraries(
      DataModel
      DataAccessInterface 
      DataAccess 
      Utilities_Petsc 
      Serial_Hdf5 
      Parallel_Hdf5
      TableIO
      FileSystem
      utilities 
      CBMGenerics
      )

   if (UNIX)
   bm_include_libraries(
      OneFilePerProcess
      )
   endif (UNIX)

   include_directories(
      src
   )

   include_directories( SYSTEM
      ${PETSC_INCLUDE_DIRS}
      ${HDF5_INCLUDE_DIRS}
      ${MPI_INCLUDE_DIRS}
   )

   set_target_properties(DistributedDataAccess
       PROPERTIES LINK_FLAGS "${PETSC_LINK_FLAGS}"
   )


   install(TARGETS DistributedDataAccess
      RUNTIME DESTINATION bin
      LIBRARY DESTINATION lib
      ARCHIVE DESTINATION lib
   )

   add_gtest( NAME DistributedGridMap_MPInp4
              SOURCES test/DistributedGridMap.C
              COMPILE_FLAGS "-DNO_ASSERT_DEATH"
              LIBRARIES Utilities_Petsc ${PETSC_LIBRARIES} DistributedDataAccess DataAccess
              LINK_FLAGS "${PETSC_LINK_FLAGS}"
              MPI_SIZE 4
   )

   add_gtest( NAME DistributedGridMap_MPInp2
              SOURCES test/DistributedGridMap.C
              COMPILE_FLAGS "-DNO_ASSERT_DEATH"
              LIBRARIES Utilities_Petsc ${PETSC_LIBRARIES} DistributedDataAccess DataAccess
              LINK_FLAGS "${PETSC_LINK_FLAGS}"
              MPI_SIZE 2
   )

   # MPI death tests need special care, separate binary is required
   add_gtest( NAME DistributedGridMap_MPInp2::AssertDeath1
              SOURCES test/DistributedGridMap.C
              COMPILE_FLAGS "-DASSERT_DEATH_CTR_1"
              LIBRARIES Utilities_Petsc ${PETSC_LIBRARIES} DistributedDataAccess DataAccess
              LINK_FLAGS "${PETSC_LINK_FLAGS}"
              MPI_SIZE 2
   )

   # MPI death tests need special care, separate binary is required
   add_gtest( NAME DistributedGridMap_MPInp2::AssertDeath2
              SOURCES test/DistributedGridMap.C
              COMPILE_FLAGS "-DASSERT_DEATH_CTR_2"
              LIBRARIES Utilities_Petsc ${PETSC_LIBRARIES} DistributedDataAccess DataAccess
              LINK_FLAGS "${PETSC_LINK_FLAGS}"
              MPI_SIZE 2
   )

   # MPI death tests need special care, separate binary is required
   add_gtest( NAME DistributedGridMap_MPInp2::AssertDeath3
              SOURCES test/DistributedGridMap.C
              COMPILE_FLAGS "-DASSERT_DEATH_CTR_3"
              LIBRARIES Utilities_Petsc ${PETSC_LIBRARIES} DistributedDataAccess DataAccess
              LINK_FLAGS "${PETSC_LINK_FLAGS}"
              MPI_SIZE 2
   )

   # MPI death tests need special care, separate binary is required
   add_gtest( NAME DistributedGridMap_MPInp2::AssertDeathNotRetrieved
              SOURCES test/DistributedGridMap.C
              COMPILE_FLAGS "-DASSERT_DEATH_NOT_RETRIEVED"
              LIBRARIES Utilities_Petsc ${PETSC_LIBRARIES} DistributedDataAccess DataAccess
              LINK_FLAGS "${PETSC_LINK_FLAGS}"
              MPI_SIZE 2
   )

   # MPI death tests need special care, separate binary is required
   add_gtest( NAME DistributedGridMap_MPInp2::HangingCommunication
              SOURCES test/DistributedGridMap.C
              COMPILE_FLAGS "-DASSERT_THROW_ON_RESTORE"
              LIBRARIES Utilities_Petsc ${PETSC_LIBRARIES} DistributedDataAccess DataAccess
              LINK_FLAGS "${PETSC_LINK_FLAGS}"
              MPI_SIZE 2
   )
endif(BM_PARALLEL)

generate_dox( src/DistributedDataAccess.cfg )
# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
