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

   FILE( GLOB all_headers src/*.h )
   source_group(include FILES ${all_headers})

   FILE( GLOB all_srcs src/*.C)
   source_group(source FILES ${all_srcs})

   set( LIB_NAME "Parallel_Hdf5" )
   add_library(${LIB_NAME} 
      ${all_srcs}
      ${all_headers}
   )
   set_target_properties( ${LIB_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${LIB_NAME}" )
   if (UNIX)
       add_dependencies(${LIB_NAME} PETSC HDF5)
   endif (UNIX)
   set (OFPP "")
   if (UNIX)
      set (OFPP OneFilePerProcess)
      bm_include_libraries( OneFilePerProcess )
   endif ()

   bm_include_libraries(
      Serial_Hdf5
      utilities
      FileSystem
      Utilities_Petsc
   )

   include_directories( SYSTEM
      ${HDF5_INCLUDE_DIRS}
      ${MPI_INCLUDE_DIRS}
      ${PETSC_INCLUDE_DIRS}
   )

   target_link_libraries(${LIB_NAME} 
      Utilities_Petsc
      Serial_Hdf5
      utilities
      FileSystem
      ${OFPP}
      ${PETSC_LIBRARIES}
      ${MPI_LIBRARIES}
   )
   
        set_target_properties(${LIB_NAME}
            PROPERTIES LINK_FLAGS "${PETSC_LINK_FLAGS}"
        )


   
   install(TARGETS ${LIB_NAME}
      RUNTIME DESTINATION bin
      LIBRARY DESTINATION lib
      ARCHIVE DESTINATION lib
   )


endif(BM_PARALLEL)
# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:

