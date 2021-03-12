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
copy_test_file(Acquifer.HDF)
copy_test_file(AcquiferScale1_CauldronOutputDir/HydrostaticTemperature_Results.HDF)

copy_test_file(TestGenex0dSimulator.project3d)



add_gtest( NAME "Genex0dInputManager"
    SOURCES src/Genex0dInputManager.cpp test/TestGenex0dInputManager.cpp
    LIBRARIES ${GENEX0D_LIB_NAME}
    FOLDER "${BASE_FOLDER}/${GENEX0D_LIB_NAME}"
)

add_gtest( NAME "Genex0dFormationManager"
    SOURCES src/Genex0dFormationManager.cpp test/TestGenex0dFormationManager.cpp
    LIBRARIES ${GENEX0D_LIB_NAME} DataAccess SerialDataAccess
    FOLDER "${BASE_FOLDER}/${GENEX0D_LIB_NAME}"
)

add_gtest( NAME "Genex0dProjectManager"
    SOURCES src/Genex0dProjectManager.cpp test/TestGenex0dProjectManager.cpp
    LIBRARIES ${GENEX0D_LIB_NAME} utilities cmbapi DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES} FileSystem
    FOLDER "${BASE_FOLDER}/${GENEX0D_LIB_NAME}"
)

add_gtest( NAME "Genex0dSimulator"
    SOURCES src/Genex0dSimulator.cpp test/TestGenex0dSimulator.cpp
    LIBRARIES ${GENEX0D_LIB_NAME} utilities cmbapi DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES} FileSystem
    FOLDER "${BASE_FOLDER}/${GENEX0D_LIB_NAME}"
)

#add_gtest( NAME "Genex0dSourceRock"
#    SOURCES src/Genex0dSourceRock.cpp test/TestGenex0dSourceRock.cpp
#    LIBRARIES ${GENEX0D_LIB_NAME} utilities cmbapi DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES} FileSystem
#    FOLDER "${BASE_FOLDER}/${GENEX0D_LIB_NAME}"
#)
