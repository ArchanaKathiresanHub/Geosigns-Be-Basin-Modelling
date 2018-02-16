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


set(all_headers
  src/AbstractPropertyManager.h
  src/AbstractPropertyValues.h
  src/AllochthonousLithologyFormationMapCalculator.h
  src/BrineDensityCalculator.h
  src/BrineViscosityCalculator.h
  src/BulkDensityFormationCalculator.h
  src/DepthHighResFormationCalculator.h
  src/DerivedFormationMapProperty.h
  src/DerivedFormationProperty.h
  src/DerivedFormationSurfaceProperty.h
  src/DerivedPropertyManager.h
  src/DerivedSurfaceProperty.h
  src/FaultElementFormationMapCalculator.h
  src/FluidVelocityFormationCalculator.h
  src/FormationMapOutputPropertyValue.h 
  src/FormationMapProperty.h
  src/FormationMapPropertyCalculator.h
  src/FormationOutputPropertyValue.h
  src/FormationSurfaceOutputPropertyValue.h
  src/FormationProperty.h
  src/FormationPropertyAtSurface.h
  src/FormationPropertyCalculator.h
  src/FormationSurfaceProperty.h
  src/FormationSurfacePropertyAtSurface.h
  src/FormationSurfacePropertyCalculator.h
  src/FormationSurfacePropertyOffsetCalculator.h
  src/FracturePressureFormationCalculator.h
  src/GammaRayFormationCalculator.h
  src/HeatFlowFormationCalculator.h
  src/HydrostaticPressureFormationCalculator.h
  src/HydrostaticPressureSurfaceCalculator.h
  src/IndirectFormationProperty.h
  src/Interpolate2DProperty.h
  src/LithostaticPressureFormationCalculator.h
  src/LithostaticPressureSurfaceCalculator.h
  src/MaxVesHighResFormationCalculator.h
  src/OverpressureFormationCalculator.h
  src/OverpressureSurfaceCalculator.h
  src/OutputUtilities.h
  src/OutputPropertyValue.h
  src/PermeabilityFormationCalculator.h
  src/PermeabilityFormationSurfaceCalculator.h
  src/PorosityFormationCalculator.h
  src/PressureFormationCalculator.h
  src/PrimaryFormationMapProperty.h
  src/PrimaryFormationMapPropertyCalculator.h
  src/PrimaryFormationProperty.h
  src/PrimaryFormationPropertyCalculator.h
  src/PrimaryFormationSurfaceProperty.h
  src/PrimaryFormationSurfacePropertyCalculator.h
  src/PrimaryReservoirProperty.h
  src/PrimaryReservoirPropertyCalculator.h
  src/PrimarySurfaceProperty.h
  src/PrimarySurfacePropertyCalculator.h
  src/PropertyCalculator.h
  src/PropertyErasePredicate.h
  src/PropertyRetriever.h
  src/PropertySnapshotCalculatorMap.h
  src/ReflectivitySurfaceCalculator.h
  src/ReservoirProperty.h
  src/ReservoirPropertyCalculator.h
  src/SonicFormationCalculator.h
  src/SurfaceOutputPropertyValue.h
  src/SurfaceProperty.h
  src/SurfacePropertyCalculator.h
  src/SurfacePropertyOffsetCalculator.h
  src/ThermalConductivityFormationCalculator.h
  src/ThermalDiffusivityFormationCalculator.h
  src/ThicknessFormationMapCalculator.h
  src/TwoWayTimeFormationCalculator.h
  src/TwoWayTimeResidualSurfaceCalculator.h
  src/VelocityFormationCalculator.h
  src/VesFormationCalculator.h
  src/VesHighResFormationCalculator.h
  src/VesSurfaceCalculator.h
)

set(all_srcs
  src/AbstractPropertyManager.C
  src/AllochthonousLithologyFormationMapCalculator.C
  src/BrineDensityCalculator.C
  src/BrineViscosityCalculator.C
  src/BulkDensityFormationCalculator.C
  src/DepthHighResFormationCalculator.C
  src/DerivedFormationMapProperty.C
  src/DerivedFormationProperty.C
  src/DerivedFormationSurfaceProperty.C
  src/DerivedPropertyManager.C
  src/DerivedSurfaceProperty.C
  src/FaultElementFormationMapCalculator.C
  src/FluidVelocityFormationCalculator.C
  src/FormationMapOutputPropertyValue.C
  src/FormationMapProperty.C
  src/FormationMapPropertyCalculator.C
  src/FormationOutputPropertyValue.C
  src/FormationSurfaceOutputPropertyValue.C	
  src/FormationProperty.C
  src/FormationPropertyAtSurface.C
  src/FormationPropertyCalculator.C
  src/FormationSurfaceProperty.C
  src/FormationSurfacePropertyCalculator.C
  src/FormationSurfacePropertyAtSurface.C
  src/FormationSurfacePropertyOffsetCalculator.C
  src/FracturePressureFormationCalculator.C
  src/GammaRayFormationCalculator.C
  src/HeatFlowFormationCalculator.C
  src/HydrostaticPressureFormationCalculator.C
  src/HydrostaticPressureSurfaceCalculator.C
  src/IndirectFormationProperty.C
  src/LithostaticPressureFormationCalculator.C
  src/LithostaticPressureSurfaceCalculator.C
  src/MaxVesHighResFormationCalculator.C
  src/OverpressureFormationCalculator.C
  src/OverpressureSurfaceCalculator.C
  src/OutputPropertyValue.C
  src/OutputUtilities.C
  src/PermeabilityFormationCalculator.C
  src/PermeabilityFormationSurfaceCalculator.C
  src/PorosityFormationCalculator.C
  src/PressureFormationCalculator.C
  src/PrimaryFormationMapProperty.C
  src/PrimaryFormationMapPropertyCalculator.C
  src/PrimaryFormationProperty.C
  src/PrimaryFormationPropertyCalculator.C
  src/PrimaryFormationSurfaceProperty.C
  src/PrimaryFormationSurfacePropertyCalculator.C
  src/PrimaryReservoirProperty.C
  src/PrimaryReservoirPropertyCalculator.C
  src/PrimarySurfaceProperty.C
  src/PrimarySurfacePropertyCalculator.C
  src/PropertyCalculator.C
  src/PropertyRetriever.C
  src/ReflectivitySurfaceCalculator.C
  src/ReservoirProperty.C
  src/ReservoirPropertyCalculator.C
  src/SonicFormationCalculator.C
  src/SurfaceOutputPropertyValue.C
  src/SurfaceProperty.C
  src/SurfacePropertyCalculator.C
  src/SurfacePropertyOffsetCalculator.C
  src/ThermalConductivityFormationCalculator.C
  src/ThermalDiffusivityFormationCalculator.C
  src/ThicknessFormationMapCalculator.C
  src/TwoWayTimeFormationCalculator.C
  src/TwoWayTimeResidualSurfaceCalculator.C
  src/VelocityFormationCalculator.C
  src/VesFormationCalculator.C
  src/VesHighResFormationCalculator.C
  src/VesSurfaceCalculator.C
)


set( LIB_NAME "DerivedProperties" )
add_library(${LIB_NAME}
  ${all_srcs}
  ${all_headers}
)
set_target_properties( ${LIB_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${LIB_NAME}" )

target_link_libraries(${LIB_NAME} 
  DataModel
  DataAccess
  GeoPhysics
  FiniteElements
  LinearAlgebra
  ${HDF5_LIBRARIES}
  ${Boost_LIBRARIES}
)


include_directories(
  ${PROJECT_SOURCE_DIR}/libraries/DataModel/src
  ${PROJECT_SOURCE_DIR}/libraries/TableIO/src
  ${PROJECT_SOURCE_DIR}/libraries/DataAccess/src
  ${PROJECT_SOURCE_DIR}/libraries/CBMGenerics/src
  ${PROJECT_SOURCE_DIR}/libraries/utilities/src
  ${PROJECT_SOURCE_DIR}/libraries/EosPack/src
  ${PROJECT_SOURCE_DIR}/libraries/GeoPhysics/src
  ${PROJECT_SOURCE_DIR}/libraries/LinearAlgebra/src
  ${PROJECT_SOURCE_DIR}/libraries/Interpolation/src
  ${PROJECT_SOURCE_DIR}/libraries/FiniteElements/src
)

include_directories( SYSTEM
  ${HDF5_INCLUDE_DIRS}
  ${PETSC_INCLUDE_DIRS} 
)

generate_dox( DerivedProperties.cfg )


################################################################
######################## UNIT TESTS ############################
################################################################

#
# 1. Set files
#
set ( MockGRPropertyManagerFiles 
    test/MockGRPropertyManager.h
    test/MockGRPropertyManager.C
)

set ( MockPorosityCalculatorFiles 
    test/MockPorosityCalculator.h
    test/MockPorosityCalculator.C
)

add_gtest( NAME ${LIB_NAME}::${LIB_NAME}
           SOURCES test/DerivedProperties.C
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../DataModel/test
           LIBRARIES ${LIB_NAME} SerialDataAccess GeoPhysics DataModel
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::SimplePropertyManagerTest
           SOURCES test/SimplePropertyManagerTest.C
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../DataModel/test
           LIBRARIES ${LIB_NAME} SerialDataAccess GeoPhysics DataModel
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::CompoundFormationMapPropertyManagerTest
           SOURCES test/CompoundFormationMapPropertyManagerTest.C
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../DataModel/test
           LIBRARIES ${LIB_NAME} SerialDataAccess GeoPhysics DataModel
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::CompoundContinuousPropertyManagerTest
           SOURCES test/CompoundContinuousPropertyManagerTest.C
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../DataModel/test
           LIBRARIES ${LIB_NAME} SerialDataAccess GeoPhysics DataModel
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::CompoundDiscontinuousPropertyManagerTest
           SOURCES test/CompoundDiscontinuousPropertyManagerTest.C
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../DataModel/test
           LIBRARIES ${LIB_NAME} SerialDataAccess GeoPhysics DataModel
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::MultiplePropertyManagerTest
           SOURCES test/MultiplePropertyManagerTest.C
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../DataModel/test
           LIBRARIES ${LIB_NAME} SerialDataAccess GeoPhysics DataModel
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::MultipleFormationMapPropertyManagerTest
           SOURCES test/MultipleFormationMapPropertyManagerTest.C
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../DataModel/test
           LIBRARIES ${LIB_NAME} SerialDataAccess GeoPhysics DataModel
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::MultipleFormationSurfacePropertyManagerTest
           SOURCES test/MultipleFormationSurfacePropertyManagerTest.C
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../DataModel/test
           LIBRARIES ${LIB_NAME} SerialDataAccess GeoPhysics DataModel
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::MultipleFormationPropertyManagerTest
           SOURCES test/MultipleFormationPropertyManagerTest.C
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../DataModel/test
           LIBRARIES ${LIB_NAME} SerialDataAccess GeoPhysics DataModel
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::ThicknessPropertyTest
           SOURCES test/ThicknessPropertyTest.C
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../DataModel/test
           LIBRARIES ${LIB_NAME} SerialDataAccess GeoPhysics DataModel
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::OverpressurePropertyTest
           SOURCES test/OverpressurePropertyTest.C
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../DataModel/test
           LIBRARIES ${LIB_NAME} SerialDataAccess GeoPhysics DataModel
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::SurfacePropertyTest
           SOURCES test/SurfacePropertyTest.C
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../DataModel/test
           LIBRARIES ${LIB_NAME} SerialDataAccess GeoPhysics DataModel
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::GammaRayFormationCalculatorTest
           SOURCES test/GammaRayFormationCalculatorTest.C ${MockGRPropertyManagerFiles} ${MockPorosityCalculatorFiles} 
           INCLUDE_DIRS  ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/../DataModel/test ${CMAKE_CURRENT_SOURCE_DIR}/../DataAccess/src ${CMAKE_CURRENT_SOURCE_DIR}/../DistributedDataAccess/src ${PETSC_INCLUDE_DIRS}
           LIBRARIES ${LIB_NAME} ${PETSC_LIBRARIES} Utilities_Petsc GeoPhysics DataAccess DistributedDataAccess
           LINK_FLAGS "${PETSC_LINK_FLAGS}"
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

if (UNIX)
   # In LSF environment (on LSF cluster node, when build is running as a LSF job) mpirun is trying to use job settings
   # to run mpi unit tests. Sometime it fails because build job requested just 1 cpu. To prevent this we can specify
   # machines file with localhost list only
   configure_file(test/machines machines COPYONLY )
   set( MACHINE_FILE -machinefile machines)
endif (UNIX)

add_gtest( NAME ${LIB_NAME}::GammaRayFormationCalculatorDistributedTest
           SOURCES test/GammaRayFormationCalculatorDistributedTest.C ${MockGRPropertyManagerFiles} ${MockPorosityCalculatorFiles} 
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/../DataModel/test ${CMAKE_CURRENT_SOURCE_DIR}/../DataAccess/src ${CMAKE_CURRENT_SOURCE_DIR}/../DistributedDataAccess/src ${PETSC_INCLUDE_DIRS}
           LIBRARIES ${LIB_NAME} ${PETSC_LIBRARIES} Utilities_Petsc GeoPhysics DataAccess DistributedDataAccess
           LINK_FLAGS "${PETSC_LINK_FLAGS}"
           MPI_SIZE 2
           MPIRUN_PRMS ${MACHINE_FILE}
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

