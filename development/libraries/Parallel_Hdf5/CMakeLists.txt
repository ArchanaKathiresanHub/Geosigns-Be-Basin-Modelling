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
   set( LIB_NAME "Parallel_Hdf5" )

   set (OPP "")
   if (UNIX)
      set (OPP OtherParallelProcess)
   endif ()

   include_directories( SYSTEM
      ${HDF5_INCLUDE_DIRS}
      ${MPI_INCLUDE_DIRS}
      ${PETSC_INCLUDE_DIRS}
   )

   create_bm_library( TARGET ${LIB_NAME}
                      LIBRARIES Utilities_Petsc
                                Serial_Hdf5
                                FileSystem
                                utilities
                                ${OPP}
                                ${PETSC_LIBRARIES}
                                ${MPI_LIBRARIES}
                      INSTALLTARGET )
 
 
   if (UNIX)
      add_dependencies(${LIB_NAME} PETSC HDF5)
   endif()
   
   set_target_properties(${LIB_NAME}
       PROPERTIES LINK_FLAGS "${PETSC_LINK_FLAGS}"
   )

endif(BM_PARALLEL)
