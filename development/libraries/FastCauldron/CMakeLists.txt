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

################################################################
######################### SET TARGETS ##########################
################################################################


FILE( GLOB all_headers src/*.h )
source_group(include FILES ${all_headers})

FILE( GLOB all_srcs src/*.cpp)
source_group(source FILES ${all_srcs})

#
# FastcauldronApplication library, contains all code except main.c
#     The main purpose of this library is to be able to run mesh unit tests (see below)
#     without having to rebuild all the application every time
#     since the FastcauldronApplication library is linked to the unit tests projects.
#
set( FASTCAULDRONAPP_TARGET_NAME "FastcauldronLib" )
add_library( ${FASTCAULDRONAPP_TARGET_NAME}
    ${all_srcs}
    ${all_headers}
)
set_target_properties( ${FASTCAULDRONAPP_TARGET_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${FASTCAULDRONAPP_TARGET_NAME}" )

################################################################
################### SET DEPENDENCIES/LINKS #####################
################################################################

if (UNIX)
add_dependencies(${FASTCAULDRONAPP_TARGET_NAME} TableIO Boost PETSC)
else (WIN32)
add_dependencies(${FASTCAULDRONAPP_TARGET_NAME} TableIO)
endif (UNIX)

include_directories(
   src
   ${PROJECT_SOURCE_DIR}/libraries
)

include_directories( SYSTEM
    ${HDF5_INCLUDE_DIRS}
    ${MPI_INCLUDE_DIRS}
    ${PETSC_INCLUDE_DIRS}
    ${EIGEN_INCLUDE_DIRS}
)

#
# FastcauldronApplication links to IBS libraries
#
target_link_libraries(${FASTCAULDRONAPP_TARGET_NAME}
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
    ${HDF5_LIBRARIES}
    ${PETSC_LIBRARIES}
    ${MPI_LIBRARIES}
    ${Boost_LIBRARIES}
    ${BM_CLOCK_GETTIME_LIB}
)

if(UNIX)
   # The OtherParallelProcess library is only available on Unix
   target_link_libraries(${FASTCAULDRONAPP_TARGET_NAME} OtherParallelProcess)
endif()

if (WIN32)
  # Windows.h defines 'min' and 'max' macros if NOMINMAX is not defined. These
  # macros collide with std::min and std::max
  add_definitions(-DNOMINMAX)
endif()

if (BM_USE_FLEXLM)
   include_directories( ${FLEXLM_INCLUDE_DIRS})
   target_link_libraries(${FASTCAULDRONAPP_TARGET_NAME} ${FLEXLM_LIBRARIES})
endif(BM_USE_FLEXLM)

################################################################
######################## UNIT TESTS ############################
################################################################


################## INTERPOLATOR UNIT TESTS #####################
#
#
add_gtest( NAME "VectorInterpolator"
           SOURCES test/InterpolationTest.cpp
           LIBRARIES LinearAlgebra
           FOLDER "${BASE_FOLDER}/${FASTCAULDRONAPP_TARGET_NAME}"
         )

add_gtest( NAME "MatrixInterpolator"
           SOURCES test/MatrixInterpolationTest.cpp
           LIBRARIES LinearAlgebra
           FOLDER "${BASE_FOLDER}/${FASTCAULDRONAPP_TARGET_NAME}"
         )

################## PETSC SOLVER UNIT TESTS #####################
#
#
add_gtest( NAME "PetscSolver"
           SOURCES src/PetscSolver test/PetscSolver.cpp
           LIBRARIES ${PETSC_LIBRARIES}
           LINK_FLAGS "${PETSC_LINK_FLAGS}"
           ENV_VARS LD_LIBRARY_PATH=$ENV{LD_LIBRARY_PATH}:${HYPRE_LIBRARY_DIR}
           FOLDER "${BASE_FOLDER}/${FASTCAULDRONAPP_TARGET_NAME}"
         )

################## PETSC OBJECTS IO UNIT TESTS #################
#
#
add_gtest( NAME "PetscObjectsIO"
           SOURCES src/PetscObjectsIO test/PetscObjectsIO.cpp
           LIBRARIES ${PETSC_LIBRARIES} ${Boost_LIBRARIES} FileSystem
           LINK_FLAGS "${PETSC_LINK_FLAGS}"
           FOLDER "${BASE_FOLDER}/${FASTCAULDRONAPP_TARGET_NAME}"
         )

###################### MESH UNIT TESTS #########################
#
# 1. Copy project files and input maps for dof counting unit tests
#
configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/test/Acquifer.project3d
      Acquifer.project3d
      COPYONLY
)

configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/test/AcquiferFull.project3d
      AcquiferFull.project3d
      COPYONLY
)

configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/test/Acquifer.HDF
      Acquifer.HDF
      COPYONLY
)

configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/test/MeshWithHoles01.project3d
      MeshWithHoles01.project3d
      COPYONLY
)

configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/test/MeshWithHoles02.project3d
      MeshWithHoles02.project3d
      COPYONLY
)

configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/test/MeshWithMixedHolesStripes.project3d
      MeshWithMixedHolesStripes.project3d
      COPYONLY
)

configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/test/chessBoard1.hdf5
      chessBoard1.hdf5
      COPYONLY
)

configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/test/chessBoard2.hdf5
      chessBoard2.hdf5
      COPYONLY
)

configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/test/thickStripes.hdf5
      thickStripes.hdf5
      COPYONLY
)

#
# 2. Copy valid vtk mesh files for dof counting unit tests
#
configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/test/valid_noholes_seds_only_0.vtk
      valid_noholes_seds_only_0.vtk
      COPYONLY
)

configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/test/valid_noholes_seds_only_10.vtk
      valid_noholes_seds_only_10.vtk
      COPYONLY
)

configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/test/valid_noholes_with_mantle_0.vtk
      valid_noholes_with_mantle_0.vtk
      COPYONLY
)

configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/test/valid_noholes_with_mantle_10.vtk
      valid_noholes_with_mantle_10.vtk
      COPYONLY
)

configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/test/valid_seds_holes01_0.vtk
      valid_seds_holes01_0.vtk
      COPYONLY
)

configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/test/valid_seds_holes02_0.vtk
      valid_seds_holes02_0.vtk
      COPYONLY
)

configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/test/valid_mixed_holes_0.vtk
      valid_mixed_holes_0.vtk
      COPYONLY
)

#
# 3. Add unit tests
#
set(CFGFLS "${PROJECT_SOURCE_DIR}/geocase/misc")

add_gtest( NAME "MeshUnitTesterFull"
           SOURCES test/MeshUnitTesterFull.cpp test/MeshUnitTester.cpp
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/test
           LIBRARIES  ${FASTCAULDRONAPP_TARGET_NAME} DataAccess DistributedDataAccess utilities Utilities_Petsc Serial_Hdf5 Parallel_Hdf5 EosPack TableIO LinearAlgebra Interpolation FiniteElements CBMGenerics genex6_kernel GeoPhysics FileSystem OTGC_kernel6 ${HDF5_LIBRARIES} ${PETSC_LIBRARIES} ${MPI_LIBRARIES} ${Boost_LIBRARIES}
           LINK_FLAGS "${PETSC_LINK_FLAGS}"
           ENV_VARS EOSPACKDIR=${CFGFLS}/eospack GENEX5DIR=${CFGFLS}/genex50
           FOLDER "${BASE_FOLDER}/${FASTCAULDRONAPP_TARGET_NAME}"
         )

add_gtest( NAME "MeshUnitTesterSediments"
           SOURCES test/MeshUnitTesterSediments.cpp test/MeshUnitTester.cpp
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/test
           LIBRARIES  ${FASTCAULDRONAPP_TARGET_NAME} DataAccess DistributedDataAccess utilities Utilities_Petsc Serial_Hdf5 Parallel_Hdf5 EosPack TableIO LinearAlgebra Interpolation FiniteElements CBMGenerics genex6_kernel GeoPhysics OTGC_kernel6 ${HDF5_LIBRARIES} ${PETSC_LIBRARIES} ${MPI_LIBRARIES} ${Boost_LIBRARIES}
           LINK_FLAGS "${PETSC_LINK_FLAGS}"
           ENV_VARS EOSPACKDIR=${CFGFLS}/eospack GENEX5DIR=${CFGFLS}/genex50
           FOLDER "${BASE_FOLDER}/${FASTCAULDRONAPP_TARGET_NAME}"
         )

add_gtest( NAME "MeshUnitTesterHoles01"
           SOURCES test/MeshUnitTesterHoles01.cpp test/MeshUnitTester.cpp
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/test
           LIBRARIES  ${FASTCAULDRONAPP_TARGET_NAME} DataAccess DistributedDataAccess utilities Utilities_Petsc Serial_Hdf5 Parallel_Hdf5 EosPack TableIO LinearAlgebra Interpolation FiniteElements CBMGenerics genex6_kernel GeoPhysics OTGC_kernel6 ${HDF5_LIBRARIES} ${PETSC_LIBRARIES} ${MPI_LIBRARIES} ${Boost_LIBRARIES}
           LINK_FLAGS "${PETSC_LINK_FLAGS}"
           ENV_VARS EOSPACKDIR=${CFGFLS}/eospack GENEX5DIR=${CFGFLS}/genex50
           FOLDER "${BASE_FOLDER}/${FASTCAULDRONAPP_TARGET_NAME}"
         )

add_gtest( NAME "MeshUnitTesterHoles02"
           SOURCES test/MeshUnitTesterHoles02.cpp test/MeshUnitTester.cpp
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/test
           LIBRARIES  ${FASTCAULDRONAPP_TARGET_NAME} DataAccess DistributedDataAccess utilities Utilities_Petsc Serial_Hdf5 Parallel_Hdf5 EosPack TableIO LinearAlgebra Interpolation FiniteElements CBMGenerics genex6_kernel GeoPhysics OTGC_kernel6 ${HDF5_LIBRARIES} ${PETSC_LIBRARIES} ${MPI_LIBRARIES} ${Boost_LIBRARIES}
           LINK_FLAGS "${PETSC_LINK_FLAGS}"
           ENV_VARS EOSPACKDIR=${CFGFLS}/eospack GENEX5DIR=${CFGFLS}/genex50
           FOLDER "${BASE_FOLDER}/${FASTCAULDRONAPP_TARGET_NAME}"
         )

add_gtest( NAME "MeshUnitTesterMixedHoles"
           SOURCES test/MeshUnitTesterMixedHoles.cpp test/MeshUnitTester.cpp
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/test
           LIBRARIES  ${FASTCAULDRONAPP_TARGET_NAME} DataAccess DistributedDataAccess utilities Utilities_Petsc Serial_Hdf5 Parallel_Hdf5 EosPack TableIO LinearAlgebra Interpolation FiniteElements CBMGenerics genex6_kernel GeoPhysics OTGC_kernel6 ${HDF5_LIBRARIES} ${PETSC_LIBRARIES} ${MPI_LIBRARIES} ${Boost_LIBRARIES}
           LINK_FLAGS "${PETSC_LINK_FLAGS}"
           ENV_VARS EOSPACKDIR=${CFGFLS}/eospack GENEX5DIR=${CFGFLS}/genex50
           FOLDER "${BASE_FOLDER}/${FASTCAULDRONAPP_TARGET_NAME}"
         )


endif(BM_PARALLEL)

################################################################
########################### DOXYGEN ############################
################################################################

generate_dox(src/fastcauldron.cfg)
