#########################################################################
#                                                                       #
# Copyright (C) 2012-2018 Shell International Exploration & Production. #
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

set( APP_NAME "fastgenex6" )

add_executable( ${APP_NAME} ${all_srcs} ${all_headers} )

set_target_properties( ${APP_NAME} PROPERTIES SUFFIX ".exe" )
set_target_properties( ${APP_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${APP_NAME}" )

create_application_run_script( ${APP_NAME} )
application_env_var( TARGET ${APP_NAME} VARS
     "GENEX5DIR"   "$MISCDIR/genex50"
     "GENEX6DIR"   "$MISCDIR/genex60"
     "OTGCDIR"     "$MISCDIR/OTGC"
     "CTCDIR"      "$MISCDIR"
     "EOSPACKDIR"  "$MISCDIR/eospack"
)

include_directories( SYSTEM
    ${HDF5_INCLUDE_DIRS}
    ${MPI_INCLUDE_DIRS}
    ${PETSC_INCLUDE_DIRS}
)

bm_include_libraries(
    DataModel
    DataAccess
    AbstractDerivedProperties
    DerivedProperties
    GeoPhysics
    Interpolation
    LinearAlgebra
    DistributedDataAccess
    Utilities_Petsc
    Serial_Hdf5
    Parallel_Hdf5
    genex6
    genex6_kernel
    CBMGenerics
    EosPack
    OTGC_kernel6
    FileSystem
    utilities
    TableIO
)

target_link_libraries( ${APP_NAME}
    DataAccess
    AbstractDerivedProperties
    DerivedProperties
    GeoPhysics
    DistributedDataAccess
    Utilities_Petsc
    Serial_Hdf5
    Parallel_Hdf5
    genex6
    genex6_kernel
    CBMGenerics
    EosPack
    OTGC_kernel6
    FileSystem
    utilities
    TableIO
    ${HDF5_LIBRARIES}
    ${PETSC_LIBRARIES}
)

# OFPP only for Linux
if (UNIX)
    target_link_libraries(${APP_NAME} OneFilePerProcess)
    bm_include_libraries( OneFilePerProcess )
endif (UNIX)

set_target_properties(${APP_NAME}
      PROPERTIES LINK_FLAGS "${PETSC_LINK_FLAGS}"
)

if (BM_USE_FLEXLM)
   include_directories( ${FLEXLM_INCLUDE_DIRS})
   target_link_libraries(${APP_NAME} ${FLEXLM_LIBRARIES})
endif(BM_USE_FLEXLM)


install(TARGETS ${APP_NAME}
    RUNTIME DESTINATION bin
)

endif(BM_PARALLEL)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
