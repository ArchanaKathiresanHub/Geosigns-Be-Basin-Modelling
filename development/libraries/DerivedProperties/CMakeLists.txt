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
)

set(all_srcs
  src/SurfaceProperty.C
  src/FormationSurfaceProperty.C
  src/FormationProperty.C
  src/DerivedSurfaceProperty.C
  src/DerivedFormationSurfaceProperty.C
  src/DerivedFormationProperty.C
)


add_library(DerivedProperties
  ${all_srcs}
  ${all_headers}
)


include_directories(
  ${PROJECT_SOURCE_DIR}/libraries/DataModel/src
)

add_gtest( NAME DerivedProperties::DerivedSurfaceProperty
           SOURCES test/DerivedSurfaceProperty.C
           LIBRARIES DerivedProperties
)
