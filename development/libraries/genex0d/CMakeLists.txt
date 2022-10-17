set( GENEX0D_LIB_NAME "genex0dLib" )

include_directories(
    src
    utilities
)

include_directories( SYSTEM
    ${CMAKE_CURRENT_BINARY_DIR}
    ${HDF5_INCLUDE_DIRS}
)

create_bm_library( TARGET ${GENEX0D_LIB_NAME}
                   LIBRARIES cmbapi
                   DataExtraction
                   DataMining
                   genex6
                   genex6_kernel
                   DataAccess
                   SerialDataAccess
                   EosPack
                   CBMGenerics
                   TableIO
                   FileSystem
                   utilities
                   Serial_Hdf5
                   GeoPhysics
                   ${HDF5_LIBRARIES}
)

if (WIN32)
  # Windows.h defines 'min' and 'max' macros if NOMINMAX is not defined. These
  # macros collide with std::min and std::max
  add_definitions(-DNOMINMAX)
endif()

if (BM_USE_FLEXLM)
   include_directories( ${FLEXLM_INCLUDE_DIRS})
   target_link_libraries(${GENEX0D_LIB_NAME} ${FLEXLM_LIBRARIES})
endif(BM_USE_FLEXLM)

# Add unit tests

set(CFGFLS "${PROJECT_SOURCE_DIR}/geocase/misc")

copy_test_file(AcquiferScale1.project3d)
copy_test_file(AcquiferScale1_Genex0dSourceRock.project3d)
copy_test_file(AcquiferScale1_Genex0dFormation.project3d)

copy_test_file(Acquifer.HDF)
copy_test_file(Acquifer_Genex0dFormation.HDF)
copy_test_file(Acquifer_Genex0dSourceRock.HDF)

copy_test_file(AcquiferScale1_CauldronOutputDir/HydrostaticTemperature_Results.HDF)
copy_test_file(AcquiferScale1_Genex0dFormation_CauldronOutputDir/HydrostaticTemperature_Results.HDF)
copy_test_file(AcquiferScale1_Genex0dSourceRock_CauldronOutputDir/HydrostaticTemperature_Results.HDF)

copy_test_file(TestGenex0dSimulator.project3d)

add_gtest( NAME "Genex0dInputManager"
    SOURCES test/TestGenex0dInputManager.cpp
    LIBRARIES ${GENEX0D_LIB_NAME} DataAccess SerialDataAccess
    FOLDER "${BASE_FOLDER}/${GENEX0D_LIB_NAME}"
)

add_gtest( NAME "Genex0dFormationManager"
    SOURCES test/TestGenex0dFormationManager.cpp
    LIBRARIES ${GENEX0D_LIB_NAME} DataAccess SerialDataAccess
    FOLDER "${BASE_FOLDER}/${GENEX0D_LIB_NAME}"
)

add_gtest( NAME "Genex0dProjectManager"
    SOURCES test/TestGenex0dProjectManager.cpp
    LIBRARIES ${GENEX0D_LIB_NAME} utilities cmbapi DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES} FileSystem
    FOLDER "${BASE_FOLDER}/${GENEX0D_LIB_NAME}"
)

add_gtest( NAME "Genex0dSourceRock"
    SOURCES test/TestGenex0dSourceRock.cpp
    LIBRARIES ${GENEX0D_LIB_NAME} utilities cmbapi DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES} FileSystem
    ENV_VARS EOSPACKDIR=${CFGFLS}/eospack GENEX5DIR=${CFGFLS}/genex50 CTCDIR=${CFGFLS}/
    FOLDER "${BASE_FOLDER}/${GENEX0D_LIB_NAME}"
)

add_gtest( NAME "Genex0dSimulator"
    SOURCES test/TestGenex0dSimulator.cpp
    LIBRARIES ${GENEX0D_LIB_NAME} utilities cmbapi DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES} FileSystem
    FOLDER "${BASE_FOLDER}/${GENEX0D_LIB_NAME}"
)
