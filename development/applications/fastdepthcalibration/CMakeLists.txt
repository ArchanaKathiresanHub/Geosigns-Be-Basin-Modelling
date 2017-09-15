#########################################################################
#                                                                       #
# Copyright (C) 2012-2016 Shell International Exploration & Production. #
# All rights reserved.                                                  #
#                                                                       #
# Developed under license for Shell by PDS BV.                          #
#                                                                       #
# Confidential and proprietary source code of Shell.                    #
# Do not distribute without written permission from Shell.              #
#                                                                       #
#########################################################################

if (BM_PARALLEL)

################################################################
######################### SET TARGETS ##########################
################################################################

FILE( GLOB all_headers src/*.h )
source_group(include FILES ${all_headers})

FILE( GLOB all_srcs src/*.cpp)
source_group(source FILES ${all_srcs})

set( APP_NAME "fastdepthcalibration" )

add_executable( ${APP_NAME} ${all_srcs} ${all_headers} )

set_target_properties( ${APP_NAME} PROPERTIES SUFFIX ".exe" )

create_application_run_script( ${APP_NAME} )

application_env_var( TARGET ${APP_NAME} VARS
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

if (UNIX)
add_dependencies(${APP_NAME} FastcauldronApplication TableIO Boost cmbapi PETSC )
else (WIN32)
add_dependencies(${APP_NAME} FastcauldronApplication TableIO )
endif (UNIX)

include_directories(
   src
   ${PROJECT_SOURCE_DIR}/applications/fastcauldron/src
   ${PROJECT_SOURCE_DIR}/libraries/cmbAPI/src
)

include_directories( SYSTEM
   ${HDF5_INCLUDE_DIRS}
   ${MPI_INCLUDE_DIRS}
   ${PETSC_INCLUDE_DIRS}
   ${EIGEN_INCLUDE_DIRS}
)

bm_include_libraries(
    FastcauldronApplication
    DataModel
    DataAccess
    DerivedProperties
    DistributedDataAccess
    utilities
    Utilities_Petsc
    Serial_Hdf5
    Parallel_Hdf5
    EosPack
    TableIO
    LinearAlgebra
    Interpolation
    FiniteElements
    CBMGenerics
    genex6
    genex6_kernel
    GeoPhysics
    FileSystem
    calibration_kernel
    cmbapi
)

target_link_libraries(${APP_NAME}
    FastcauldronApplication
    DataAccess
    DistributedDataAccess
    )

target_link_libraries( ${APP_NAME}
    DataAccess
    DerivedProperties
    DistributedDataAccess
    utilities
    Utilities_Petsc
    Serial_Hdf5
    Parallel_Hdf5
    EosPack
    TableIO
    LinearAlgebra
    Interpolation
    FiniteElements
    CBMGenerics
    genex6
    genex6_kernel
    GeoPhysics
    FileSystem
    calibration_kernel
    OTGC_kernel6
    cmbapi
    ${HDF5_LIBRARIES}
    ${PETSC_LIBRARIES}
    ${MPI_LIBRARIES}
    ${Boost_LIBRARIES}
    ${BM_CLOCK_GETTIME_LIB}
)

set_target_properties(${APP_NAME}
      PROPERTIES LINK_FLAGS "${PETSC_LINK_FLAGS}"
)

if(UNIX)
   # The OneFilePerProcess library is only available on Unix
   bm_include_libraries( OneFilePerProcess )
   target_link_libraries(${APP_NAME} OneFilePerProcess)
endif()

if (WIN32)
  # Windows.h defines 'min' and 'max' macros if NOMINMAX is not defined. These
  # macros collide with std::min and std::max
  add_definitions(-DNOMINMAX)
endif()

if (BM_USE_FLEXLM)
   include_directories( ${FLEXLM_INCLUDE_DIRS})
   target_link_libraries(${APP_NAME} ${FLEXLM_LIBRARIES})
endif(BM_USE_FLEXLM)

install(TARGETS ${APP_NAME}
    RUNTIME DESTINATION bin
)
    
endif(BM_PARALLEL)

