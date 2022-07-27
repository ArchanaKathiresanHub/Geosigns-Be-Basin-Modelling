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

FILE( GLOB all_headers src/*.h )
FILE( GLOB all_srcs src/*.cpp)

set( FDC_LIB_NAME "FastDepthConversionLib" )
add_library( ${FDC_LIB_NAME}
             ${all_srcs}
             ${all_headers} )
set_target_properties( ${FDC_LIB_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${FDC_LIB_NAME}" )

################################################################
################### SET DEPENDENCIES/LINKS #####################
################################################################

if (UNIX)
add_dependencies(${FDC_LIB_NAME} FastcauldronLib TableIO Boost cmbapi PETSC )
else (WIN32)
add_dependencies(${FDC_LIB_NAME} FastcauldronLib TableIO )
endif (UNIX)

include_directories(
   src
   ${PROJECT_SOURCE_DIR}/libraries/FastCauldron/src
)


include_directories( SYSTEM
   ${CMAKE_CURRENT_BINARY_DIR}
   ${HDF5_INCLUDE_DIRS}
   ${MPI_INCLUDE_DIRS}
   ${PETSC_INCLUDE_DIRS}
   ${EIGEN_INCLUDE_DIRS}
)

target_link_libraries( ${FDC_LIB_NAME}
    DataAccess
    AbstractDerivedProperties
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
    OTGC_kernel6
    cmbapi
    ${HDF5_LIBRARIES}
    ${PETSC_LIBRARIES}
    ${MPI_LIBRARIES}
    ${Boost_LIBRARIES}
    ${BM_CLOCK_GETTIME_LIB}
)

if(UNIX)
   # The OneFilePerProcess library is only available on Unix
   target_link_libraries(${FDC_LIB_NAME} OneFilePerProcess)
endif()

if (WIN32)
  # Windows.h defines 'min' and 'max' macros if NOMINMAX is not defined. These
  # macros collide with std::min and std::max
  add_definitions(-DNOMINMAX)
endif()

if (BM_USE_FLEXLM)
   include_directories( ${FLEXLM_INCLUDE_DIRS})
   target_link_libraries(${FDC_LIB_NAME} ${FLEXLM_LIBRARIES})
endif(BM_USE_FLEXLM)

# Unit tests

configure_file(test/Project.project3d ${CMAKE_CURRENT_BINARY_DIR}/Project.project3d COPYONLY)
configure_file(test/Input.HDF ${CMAKE_CURRENT_BINARY_DIR}/Input.HDF COPYONLY)
configure_file(test/Inputs.HDF ${CMAKE_CURRENT_BINARY_DIR}/Inputs.HDF COPYONLY)
configure_file(test/increasedDepthsSurf3.txt ${CMAKE_CURRENT_BINARY_DIR}/increasedDepthsSurf3.txt COPYONLY)

add_gtest( NAME "FDCProjectManager"
    SOURCES src/FDCProjectManager.cpp test/TestsFDCProjectManager.cpp
    LIBRARIES ${FDC_LIB_NAME} ${PETSC_LIBRARIES} utilities cmbapi DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES} FileSystem
    LINK_FLAGS "${PETSC_LINK_FLAGS}"
    FOLDER "${BASE_FOLDER}/${FDC_TARGET_NAME}"
)

add_gtest( NAME "FDCMapFieldProperties"
    SOURCES src/FDCMapFieldProperties.cpp test/TestsFDCMapFieldProperties.cpp
    LIBRARIES ${FDC_LIB_NAME} utilities cmbapi DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES} FileSystem
    FOLDER "${BASE_FOLDER}/${FDC_TARGET_NAME}"
)

add_gtest( NAME "FDCVectorFieldProperties"
    SOURCES src/FDCVectorFieldProperties.cpp test/TestsFDCVectorFieldProperties.cpp
    LIBRARIES ${FDC_LIB_NAME} utilities cmbapi DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES} FileSystem
    FOLDER "${BASE_FOLDER}/${FDC_TARGET_NAME}"
)

add_gtest( NAME "FDCLithoProperties"
    SOURCES src/FDCLithoProperties.cpp test/TestsFDCLithoProperties.cpp
    LIBRARIES ${FDC_LIB_NAME} utilities cmbapi DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES} FileSystem
    FOLDER "${BASE_FOLDER}/${FDC_TARGET_NAME}"
)

endif(BM_PARALLEL)

