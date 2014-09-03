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
  src/SurfaceProperty.h
  src/FormationSurfaceProperty.h
  src/FormationProperty.h
  src/DerivedSurfaceProperty.h
  src/DerivedFormationSurfaceProperty.h
  src/DerivedFormationProperty.h
  src/AbstractPropertyManager.h
  src/DerivedPropertyManager.h
  src/PrimarySurfaceProperty.h
  src/PrimaryFormationSurfaceProperty.h
  src/PrimaryFormationProperty.h
)

set(all_srcs
  src/SurfaceProperty.C
  src/FormationSurfaceProperty.C
  src/FormationProperty.C
  src/DerivedSurfaceProperty.C
  src/DerivedFormationSurfaceProperty.C
  src/DerivedFormationProperty.C
  src/AbstractPropertyManager.C
  src/DerivedPropertyManager.C
  src/PrimarySurfaceProperty.C
  src/PrimaryFormationSurfaceProperty.C
  src/PrimaryFormationProperty.C
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
           LIBRARIES DerivedProperties
)

add_gtest( NAME DerivedProperties::SimplePropertyManagerTest
           SOURCES test/SimplePropertyManagerTest.C
           LIBRARIES DerivedProperties
)

add_gtest( NAME DerivedProperties::CompoundPropertyManagerTest
           SOURCES test/CompoundPropertyManagerTest.C
           LIBRARIES DerivedProperties
)

add_gtest( NAME DerivedProperties::MultiplePropertyManagerTest
           SOURCES test/MultiplePropertyManagerTest.C
           LIBRARIES DerivedProperties
)
