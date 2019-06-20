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
   set( LIB_NAME "Utilities_Petsc" )

   include_directories( SYSTEM
      ${PETSC_INCLUDE_DIRS}
      ${MPI_INCLUDE_DIRS}
   )

   create_bm_library( TARGET ${LIB_NAME}
                      LIBRARIES ${PETSC_LIBRARIES} ${MPI_LIBRARIES}
                      INSTALLTARGET )

   set_target_properties(${LIB_NAME}
       PROPERTIES LINK_FLAGS "${PETSC_LINK_FLAGS}"
   )

   if (UNIX)
       add_dependencies(${LIB_NAME} PETSC)
   endif (UNIX)
else (BM_PARALLEL)
   message("Utilities_Petsc will not be compiled, because MPI is unavailable")
endif (BM_PARALLEL)
