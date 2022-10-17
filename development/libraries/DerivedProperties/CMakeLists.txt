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


set( LIB_NAME "DerivedProperties" )
include_directories( SYSTEM
  ${HDF5_INCLUDE_DIRS}
  ${PETSC_INCLUDE_DIRS} 
)


create_bm_library( TARGET ${LIB_NAME}
                   LIBRARIES AbstractDerivedProperties
                             GeoPhysics
                             FiniteElements
                             LinearAlgebra
                             DataAccess
                             DataModel
                             utilities
                             ${HDF5_LIBRARIES}
                             ${Boost_LIBRARIES} )

generate_dox( DerivedProperties.cfg )

copy_test_file(DBMProject.project3d)
copy_test_file(01_simple_50km_HF_test.project3d)
copy_test_file(01_simple_50km_HF_test.HDF)


################################################################
######################## UNIT TESTS ############################
################################################################

#
# 1. Set files
#
set ( MockGRPropertyManagerFiles 
    test/MockGRPropertyManager.h
    test/MockGRPropertyManager.cpp
)

set ( MockPorosityCalculatorFiles 
    test/MockPorosityCalculator.h
    test/MockPorosityCalculator.cpp
)

add_gtest( NAME ${LIB_NAME}::${LIB_NAME}
           SOURCES test/DerivedProperties.cpp
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../DataModel/test
           LIBRARIES ${LIB_NAME} GeoPhysics DataModel DataAccess SerialDataAccess
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::SimplePropertyManagerTest
           SOURCES test/SimplePropertyManagerTest.cpp
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../DataModel/test
           LIBRARIES ${LIB_NAME} GeoPhysics DataModel DataAccess SerialDataAccess
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::CompoundFormationMapPropertyManagerTest
           SOURCES test/CompoundFormationMapPropertyManagerTest.cpp
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../DataModel/test
           LIBRARIES ${LIB_NAME} GeoPhysics DataModel DataAccess SerialDataAccess
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::CompoundContinuousPropertyManagerTest
           SOURCES test/CompoundContinuousPropertyManagerTest.cpp
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../DataModel/test
           LIBRARIES ${LIB_NAME} GeoPhysics DataModel DataAccess SerialDataAccess
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::CompoundDiscontinuousPropertyManagerTest
           SOURCES test/CompoundDiscontinuousPropertyManagerTest.cpp
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../DataModel/test
           LIBRARIES ${LIB_NAME} GeoPhysics DataModel DataAccess SerialDataAccess
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::MultiplePropertyManagerTest
           SOURCES test/MultiplePropertyManagerTest.cpp
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../DataModel/test
           LIBRARIES ${LIB_NAME} GeoPhysics DataModel DataAccess SerialDataAccess
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::MultipleFormationMapPropertyManagerTest
           SOURCES test/MultipleFormationMapPropertyManagerTest.cpp
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../DataModel/test
           LIBRARIES ${LIB_NAME} GeoPhysics DataModel DataAccess SerialDataAccess
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::MultipleFormationSurfacePropertyManagerTest
           SOURCES test/MultipleFormationSurfacePropertyManagerTest.cpp
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../DataModel/test
           LIBRARIES ${LIB_NAME} GeoPhysics DataModel DataAccess SerialDataAccess
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::MultipleFormationPropertyManagerTest
           SOURCES test/MultipleFormationPropertyManagerTest.cpp
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../DataModel/test
           LIBRARIES ${LIB_NAME} GeoPhysics DataModel DataAccess SerialDataAccess
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::ThicknessPropertyTest
           SOURCES test/ThicknessPropertyTest.cpp
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../DataModel/test
           LIBRARIES ${LIB_NAME} GeoPhysics DataModel DataAccess SerialDataAccess
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::OverpressurePropertyTest
           SOURCES test/OverpressurePropertyTest.cpp
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../DataModel/test
           LIBRARIES ${LIB_NAME} GeoPhysics DataModel DataAccess SerialDataAccess
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::SurfacePropertyTest
           SOURCES test/SurfacePropertyTest.cpp
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../DataModel/test
           LIBRARIES ${LIB_NAME} GeoPhysics DataModel DataAccess SerialDataAccess
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::GammaRayFormationCalculatorTest
           SOURCES test/GammaRayFormationCalculatorTest.cpp ${MockGRPropertyManagerFiles} ${MockPorosityCalculatorFiles} 
           INCLUDE_DIRS  ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/../DataModel/test ${CMAKE_CURRENT_SOURCE_DIR}/../DataAccess/src ${CMAKE_CURRENT_SOURCE_DIR}/../DistributedDataAccess/src ${PETSC_INCLUDE_DIRS}
           LIBRARIES ${LIB_NAME} ${PETSC_LIBRARIES} Utilities_Petsc GeoPhysics DataAccess DistributedDataAccess
           LINK_FLAGS "${PETSC_LINK_FLAGS}"
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::DepthBelowMudlinePropertyTest
           SOURCES test/DepthBelowMudlinePropertyTest.cpp
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../DataModel/test ${CMAKE_CURRENT_SOURCE_DIR}/../GeoPhysics/src ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/../DataAccess/src ${CMAKE_CURRENT_SOURCE_DIR}/../DistributedDataAccess/src
           LIBRARIES ${LIB_NAME} GeoPhysics DataModel DataAccess DistributedDataAccess
	       ENV_VARS CTCDIR=${PROJECT_SOURCE_DIR}/geocase/misc
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::HeatFlowPropertyTest
           SOURCES test/HeatFlowPropertyTest.cpp test/MockHeatFlowPropertyManager.cpp
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/test ${CMAKE_CURRENT_SOURCE_DIR}/../DataModel/test ${CMAKE_CURRENT_SOURCE_DIR}/../GeoPhysics/src ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/../DataAccess/src ${CMAKE_CURRENT_SOURCE_DIR}/../DistributedDataAccess/src
           LIBRARIES ${LIB_NAME} GeoPhysics DataModel DataAccess DistributedDataAccess
	       ENV_VARS CTCDIR=${PROJECT_SOURCE_DIR}/geocase/misc
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

if (UNIX)
   # In LSF environment (on LSF cluster node, when build is running as a LSF job) mpirun is trying to use job settings
   # to run mpi unit tests. Sometime it fails because build job requested just 1 cpu. To prevent this we can specify
   # machines file with localhost list only
   copy_test_file(machines)
   set( MACHINE_FILE -machinefile machines)
endif (UNIX)

add_gtest( NAME ${LIB_NAME}::GammaRayFormationCalculatorDistributedTest
           SOURCES test/GammaRayFormationCalculatorDistributedTest.cpp ${MockGRPropertyManagerFiles} ${MockPorosityCalculatorFiles} 
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/../DataModel/test ${CMAKE_CURRENT_SOURCE_DIR}/../DataAccess/src ${CMAKE_CURRENT_SOURCE_DIR}/../DistributedDataAccess/src ${PETSC_INCLUDE_DIRS}
           LIBRARIES ${LIB_NAME} ${PETSC_LIBRARIES} Utilities_Petsc GeoPhysics DataAccess DistributedDataAccess
           LINK_FLAGS "${PETSC_LINK_FLAGS}"
           MPI_SIZE 2
           MPIRUN_PRMS ${MACHINE_FILE}
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

