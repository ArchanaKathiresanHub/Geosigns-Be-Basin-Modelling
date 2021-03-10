#########################################################################
#                                                                       #
# Copyright (C) 2012-2018 Shell International Exploration & Production. #
# All rights reserved.                                                  #
#                                                                       #
# Confidential and proprietary source code of Shell.                    #
# Do not distribute without written permission from Shell.              #
#                                                                       #
#########################################################################

if (BM_PARALLEL)

################################################################
######################### SET TARGETS ##########################
################################################################

set( FDC_TARGET_NAME "fastdepthconversion" )
add_executable( ${FDC_TARGET_NAME} src/main.cpp ${all_headers} )
set_target_properties( ${FDC_TARGET_NAME} PROPERTIES SUFFIX ".exe" )
set_target_properties( ${FDC_TARGET_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${FDC_TARGET_NAME}" )

create_application_run_script( ${FDC_TARGET_NAME} )

application_env_var( TARGET ${FDC_TARGET_NAME} VARS
     "GENEXDIR"    "$MISCDIR/genex40"
     "GENEX5DIR"   "$MISCDIR/genex50"
     "GENEX6DIR"   "$MISCDIR/genex60"
     "OTGCDIR"     "$MISCDIR/OTGC"
     "CTCDIR"      "$MISCDIR"
     "EOSPACKDIR"  "$MISCDIR/eospack"
)

################################################################
################### SET DEPENDENCIES/LINKS #####################
################################################################

include_directories(
   ${PROJECT_SOURCE_DIR}/libraries/FastDepthConversion/src
)

include_directories( SYSTEM
   ${CMAKE_CURRENT_BINARY_DIR}
   ${HDF5_INCLUDE_DIRS}
   ${MPI_INCLUDE_DIRS}
   ${PETSC_INCLUDE_DIRS}
   ${EIGEN_INCLUDE_DIRS}
)

target_link_libraries(${FDC_TARGET_NAME}
    FastDepthConversionLib
    FastcauldronApplication
    DataAccess
    DistributedDataAccess
    )

set_target_properties(${FDC_TARGET_NAME}
      PROPERTIES LINK_FLAGS "${PETSC_LINK_FLAGS}"
)

install(TARGETS ${FDC_TARGET_NAME}
    RUNTIME DESTINATION bin
)

endif(BM_PARALLEL)

