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
  src/DerivedSurfaceProperty.h
  src/SurfaceProperty.h
)

set(all_srcs
  src/DerivedSurfaceProperty.C
  src/SurfaceProperty.C
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
