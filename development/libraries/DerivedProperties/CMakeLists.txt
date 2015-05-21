#########################################################################
#                                                                       #
# Copyright (C) 2012-2013 Shell International Exploration & Production. #
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
  src/DerivedFormationMapProperty.h
  src/DerivedFormationProperty.h
  src/DerivedFormationSurfaceProperty.h
  src/DerivedPropertyManager.h
  src/DerivedSurfaceProperty.h
  src/ErosionFactorFormationMapCalculator.h
  src/FaultElementFormationMapCalculator.h
  src/FormationMapProperty.h
  src/FormationMapPropertyCalculator.h
  src/FormationProperty.h
  src/FormationPropertyAtSurface.h
  src/FormationPropertyCalculator.h
  src/FormationSurfaceProperty.h
  src/FormationSurfacePropertyAtSurface.h
  src/FormationSurfacePropertyCalculator.h
  src/FormationSurfacePropertyOffsetCalculator.h
  src/FracturePressureFormationCalculator.h
  src/HydrostaticPressureFormationCalculator.h
  src/HydrostaticPressureSurfaceCalculator.h
  src/LithostaticPressureFormationCalculator.h
  src/LithostaticPressureSurfaceCalculator.h
  src/OverpressureFormationCalculator.h
  src/OverpressureSurfaceCalculator.h
  src/PermeabilityFormationCalculator.h
  src/PermeabilityFormationSurfaceCalculator.h
  src/PorosityFormationCalculator.h
  src/PrimaryFormationMapProperty.h
  src/PrimaryFormationMapPropertyCalculator.h
  src/PrimaryFormationProperty.h
  src/PrimaryFormationPropertyCalculator.h
  src/PrimaryFormationSurfaceProperty.h
  src/PrimaryFormationSurfacePropertyCalculator.h
  src/PrimarySurfaceProperty.h
  src/PrimarySurfacePropertyCalculator.h
  src/PropertyCalculator.h
  src/PropertyErasePredicate.h
  src/PropertyRetriever.h
  src/PropertySnapshotCalculatorMap.h
  src/SurfaceProperty.h
  src/SurfacePropertyCalculator.h
  src/SurfacePropertyOffsetCalculator.h
  src/ThermalConductivityFormationCalculator.h
  src/ThicknessFormationMapCalculator.h
  src/VelocityFormationCalculator.h
  src/VesFormationCalculator.h
  src/VesSurfaceCalculator.h
)

set(all_srcs
  src/AbstractPropertyManager.C
  src/AllochthonousLithologyFormationMapCalculator.C
  src/DerivedFormationMapProperty.C
  src/DerivedFormationProperty.C
  src/DerivedFormationSurfaceProperty.C
  src/DerivedPropertyManager.C
  src/DerivedSurfaceProperty.C
  src/ErosionFactorFormationMapCalculator.C
  src/FaultElementFormationMapCalculator.C
  src/FormationMapProperty.C
  src/FormationProperty.C
  src/FormationPropertyAtSurface.C
  src/FormationSurfaceProperty.C
  src/FormationSurfacePropertyAtSurface.C
  src/FormationSurfacePropertyOffsetCalculator.C
  src/FracturePressureFormationCalculator.C
  src/HydrostaticPressureFormationCalculator.C
  src/HydrostaticPressureSurfaceCalculator.C
  src/LithostaticPressureFormationCalculator.C
  src/LithostaticPressureSurfaceCalculator.C
  src/OverpressureFormationCalculator.C
  src/OverpressureSurfaceCalculator.C
  src/PermeabilityFormationCalculator.C
  src/PermeabilityFormationSurfaceCalculator.C
  src/PorosityFormationCalculator.C
  src/PrimaryFormationMapProperty.C
  src/PrimaryFormationMapPropertyCalculator.C
  src/PrimaryFormationProperty.C
  src/PrimaryFormationPropertyCalculator.C
  src/PrimaryFormationSurfaceProperty.C
  src/PrimaryFormationSurfacePropertyCalculator.C
  src/PrimarySurfaceProperty.C
  src/PrimarySurfacePropertyCalculator.C
  src/PropertyCalculator.C
  src/PropertyRetriever.C
  src/SurfaceProperty.C
  src/SurfacePropertyOffsetCalculator.C
  src/ThermalConductivityFormationCalculator.C
  src/ThicknessFormationMapCalculator.C
  src/VelocityFormationCalculator.C
  src/VesFormationCalculator.C
  src/VesSurfaceCalculator.C
)


add_library(DerivedProperties
  ${all_srcs}
  ${all_headers}
)

target_link_libraries(DerivedProperties 
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
  ${HDF5_INCLUDE_DIRS}
)

generate_dox( DerivedProperties.cfg )

add_gtest( NAME DerivedProperties::DerivedProperties
           SOURCES test/DerivedProperties.C
           LIBRARIES DerivedProperties SerialDataAccess GeoPhysics
)

add_gtest( NAME DerivedProperties::SimplePropertyManagerTest
           SOURCES test/SimplePropertyManagerTest.C
           LIBRARIES DerivedProperties SerialDataAccess GeoPhysics
)

add_gtest( NAME DerivedProperties::CompoundPropertyManagerTest
           SOURCES test/CompoundPropertyManagerTest.C
           LIBRARIES DerivedProperties SerialDataAccess GeoPhysics
)

add_gtest( NAME DerivedProperties::MultiplePropertyManagerTest
           SOURCES test/MultiplePropertyManagerTest.C
           LIBRARIES DerivedProperties SerialDataAccess GeoPhysics
)

add_gtest( NAME DerivedProperties::MultipleFormationMapPropertyManagerTest
           SOURCES test/MultipleFormationMapPropertyManagerTest.C
           LIBRARIES DerivedProperties SerialDataAccess GeoPhysics
)

add_gtest( NAME DerivedProperties::MultipleFormationSurfacePropertyManagerTest
           SOURCES test/MultipleFormationSurfacePropertyManagerTest.C
           LIBRARIES DerivedProperties SerialDataAccess GeoPhysics
)

add_gtest( NAME DerivedProperties::MultipleFormationPropertyManagerTest
           SOURCES test/MultipleFormationPropertyManagerTest.C
           LIBRARIES DerivedProperties SerialDataAccess GeoPhysics
)

add_gtest( NAME DerivedProperties::ThicknessPropertyTest
           SOURCES test/ThicknessPropertyTest.C
           LIBRARIES DerivedProperties SerialDataAccess GeoPhysics
)

add_gtest( NAME DerivedProperties::OverpressurePropertyTest
           SOURCES test/OverpressurePropertyTest.C
           LIBRARIES DerivedProperties SerialDataAccess GeoPhysics
)

add_gtest( NAME DerivedProperties::SurfacePropertyTest
           SOURCES test/SurfacePropertyTest.C
           LIBRARIES DerivedProperties SerialDataAccess GeoPhysics
)
