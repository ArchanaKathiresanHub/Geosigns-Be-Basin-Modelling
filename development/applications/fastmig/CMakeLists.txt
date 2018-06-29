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

set( APP_NAME "fastmig" )
add_executable( ${APP_NAME} ${all_srcs} ${all_headers} )

set_target_properties( ${APP_NAME} PROPERTIES SUFFIX ".exe" )
set_target_properties( ${APP_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${APP_NAME}" )

create_application_run_script( ${APP_NAME} )

application_env_var( TARGET ${APP_NAME} VARS
     "GENEXDIR"    "$MISCDIR/genex40"
     "GENEX5DIR"   "$MISCDIR/genex50"
     "OTGCDIR"     "$MISCDIR/OTGC"
     "CTCDIR"      "$MISCDIR"
     "EOSPACKDIR"  "$MISCDIR/eospack"
)

add_definitions(-DUSEOTGC)

include_directories(
    ${PROJECT_SOURCE_DIR}/libraries
)

include_directories( SYSTEM
    ${PETSC_INCLUDE_DIRS}
    ${HDF5_INCLUDE_DIRS}
    ${MPI_INCLUDE_DIRS}
)

bm_include_libraries(
    DataModel
    DataAccess
    DistributedDataAccess
    AbstractDerivedProperties
    DerivedProperties
    FiniteElements
    GeoPhysics
    Interpolation
    LinearAlgebra
    Utilities_Petsc
    Serial_Hdf5
    Parallel_Hdf5
    EosPack    
    CBMGenerics
    genex6
    genex6_kernel
    FileSystem
    utilities
    TableIO
    functions
)


target_link_libraries(${APP_NAME}
    DataModel
    DataAccess
    AbstractDerivedProperties
    DerivedProperties
    DistributedDataAccess
    GeoPhysics
    Utilities_Petsc
    Serial_Hdf5
    Parallel_Hdf5
    EosPack
    CBMGenerics
    FiniteElements
    genex6
    genex6_kernel
    OTGC_kernel6
    FileSystem
    utilities
    TableIO
    functions
    ${PETSC_LIBRARIES}
    ${HDF5_LIBRARIES}
    ${MPI_LIBRARIES}
    ${BM_CLOCK_GETTIME_LIB}
)

set_target_properties(${APP_NAME}
      PROPERTIES LINK_FLAGS "${PETSC_LINK_FLAGS}"
)
   
if (UNIX)
   # The OneFilePerProcess library is only available on Unix
   bm_include_libraries( OneFilePerProcess )
   target_link_libraries(${APP_NAME} OneFilePerProcess)
endif ()

if (BM_USE_FLEXLM)
   include_directories(${FLEXLM_INCLUDE_DIRS})
   target_link_libraries(${APP_NAME} ${FLEXLM_LIBRARIES})
endif(BM_USE_FLEXLM)

install(TARGETS ${APP_NAME}
    RUNTIME DESTINATION bin
)

set(CFGFLS "${PROJECT_SOURCE_DIR}/geocase/misc")

### Unit Tests

add_gtest( NAME "Fastmig::Biodegrade"
   SOURCES test/BiodegradeTest.C src/Biodegrade.C
   LIBRARIES DataAccess DistributedDataAccess
   FOLDER "${BASE_FOLDER}/${APP_NAME}"
)

add_gtest( NAME "Fastmig::CapillarySealStrength"
   SOURCES test/CapillarySealStrength.C src/CapillarySealStrength.C src/Composition.C src/Biodegrade.C src/DiffusionLeak.C src/DiffusionCoefficient.C
   LIBRARIES DataAccess DistributedDataAccess CBMGenerics EosPack GeoPhysics functions
   ENV_VARS EOSPACKDIR=${CFGFLS}/eospack GENEX5DIR=${CFGFLS}/genex50 CTCDIR=${CFGFLS}/
   FOLDER "${BASE_FOLDER}/${APP_NAME}"
)

add_gtest( NAME "Fastmig::Leak"
   SOURCES test/Leak.C src/Leak.C
   LIBRARIES functions
   FOLDER "${BASE_FOLDER}/${APP_NAME}"
)

add_gtest( NAME "Fastmig::LeakGas"
   SOURCES test/LeakGas.C src/Leak.C src/LeakGas.C
   LIBRARIES functions
   FOLDER "${BASE_FOLDER}/${APP_NAME}"
)

add_gtest( NAME "Fastmig::LeakOrSpill"
   SOURCES test/LeakOrSpill.C src/Leak.C src/Spill.C src/LeakOrSpill.C
   LIBRARIES functions
   FOLDER "${BASE_FOLDER}/${APP_NAME}"
)

add_gtest( NAME "Fastmig::Spill"
   SOURCES test/Spill.C src/Spill.C 
   LIBRARIES functions
   FOLDER "${BASE_FOLDER}/${APP_NAME}"
)

add_gtest( NAME "Fastmig::SpillOilAndGas"
   SOURCES test/SpillOilAndGas.C src/Spill.C src/SpillOilAndGas.C 
   LIBRARIES functions
   FOLDER "${BASE_FOLDER}/${APP_NAME}"
)

add_gtest( NAME "Fastmig::LeakOrWaste"
           SOURCES test/LeakOrWaste.cpp src/LeakOrWaste.C src/Leak.C src/Waste.C
           LIBRARIES functions
           FOLDER "${BASE_FOLDER}/${APP_NAME}"
          )

add_gtest( NAME "Fastmig::LeakOrWasteGasAndSpillOil"
           SOURCES test/LeakOrWasteGasAndSpillOil.cpp src/LeakOrWasteGasAndSpillOil.C src/Leak.C src/Waste.C src/Spill.C src/LeakGas.C src/LeakGasAndSpillOil.C src/LeakOrSpill.C src/SpillOilAndGas.C src/WasteGasAndSpillOil.C
           LIBRARIES functions
           FOLDER "${BASE_FOLDER}/${APP_NAME}"
          )

add_gtest( NAME "Fastmig::LeakGasAndSpillOil"
           SOURCES test/LeakGasAndSpillOil.cpp src/Leak.C src/LeakGas.C src/LeakGasAndSpillOil.C src/Spill.C src/SpillOilAndGas.C
           LIBRARIES functions
           FOLDER "${BASE_FOLDER}/${APP_NAME}"
          )

endif (BM_PARALLEL)

generate_dox( src/fastmig.cfg )

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
