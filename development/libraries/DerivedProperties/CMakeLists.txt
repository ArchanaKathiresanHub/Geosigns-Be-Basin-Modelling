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
  src/DerivedFormationProperty.h
  src/DerivedFormationSurfaceProperty.h
  src/DerivedPropertyManager.h
  src/DerivedSurfaceProperty.h
  src/FormationPropertyCalculator.h
  src/FormationProperty.h
  src/FormationSurfacePropertyCalculator.h
  src/FormationSurfaceProperty.h
  src/PrimaryFormationPropertyCalculator.h
  src/PrimaryFormationProperty.h
  src/PrimaryFormationSurfacePropertyCalculator.h
  src/PrimaryFormationSurfaceProperty.h
  src/PrimarySurfacePropertyCalculator.h
  src/PrimarySurfaceProperty.h
  src/SurfacePropertyCalculator.h
  src/SurfaceProperty.h
)

set(all_srcs
  src/AbstractPropertyManager.C
  src/DerivedFormationProperty.C
  src/DerivedFormationSurfaceProperty.C
  src/DerivedPropertyManager.C
  src/DerivedSurfaceProperty.C
  src/FormationProperty.C
  src/FormationSurfaceProperty.C
  src/PrimaryFormationProperty.C
  src/PrimaryFormationPropertyCalculator.C
  src/PrimaryFormationSurfaceProperty.C
  src/PrimaryFormationSurfacePropertyCalculator.C
  src/PrimarySurfaceProperty.C
  src/PrimarySurfacePropertyCalculator.C
  src/SurfaceProperty.C
)


add_library(DerivedProperties
  ${all_srcs}
  ${all_headers}
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
           LIBRARIES DerivedProperties SerialDataAccess
)

add_gtest( NAME DerivedProperties::SimplePropertyManagerTest
           SOURCES test/SimplePropertyManagerTest.C
           LIBRARIES DerivedProperties SerialDataAccess
)

add_gtest( NAME DerivedProperties::CompoundPropertyManagerTest
           SOURCES test/CompoundPropertyManagerTest.C
           LIBRARIES DerivedProperties SerialDataAccess
)

add_gtest( NAME DerivedProperties::MultiplePropertyManagerTest
           SOURCES test/MultiplePropertyManagerTest.C
           LIBRARIES DerivedProperties SerialDataAccess
)
