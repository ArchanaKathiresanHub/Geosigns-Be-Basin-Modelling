#########################################################################
#                                                                       #
# Copyright (C) 2018-2018 Shell International Exploration & Production. #
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
   src/FormationMapProperty.h
   src/FormationMapPropertyCalculator.h
   src/FormationProperty.h
   src/FormationPropertyAtSurface.h
   src/FormationPropertyCalculator.h
   src/FormationSurfaceProperty.h
   src/FormationSurfacePropertyAtSurface.h
   src/FormationSurfacePropertyCalculator.h
   src/FormationSurfacePropertyOffsetCalculator.h
   src/Interpolate2DProperty.h
   src/PropertyCalculator.h
   src/PropertyErasePredicate.h
   src/PropertySnapshotCalculatorMap.h
   src/ReservoirProperty.h
   src/ReservoirPropertyCalculator.h
   src/SurfaceProperty.h
   src/SurfacePropertyCalculator.h
   src/SurfacePropertyOffsetCalculator.h
)

set(all_srcs
   src/AbstractPropertyManager.C
   src/FormationMapProperty.C
   src/FormationMapPropertyCalculator.C
   src/FormationProperty.C
   src/FormationPropertyAtSurface.C
   src/FormationPropertyCalculator.C
   src/FormationSurfaceProperty.C
   src/FormationSurfacePropertyAtSurface.C
   src/FormationSurfacePropertyCalculator.C
   src/FormationSurfacePropertyOffsetCalculator.C
   src/PropertyCalculator.C
   src/ReservoirProperty.C
   src/ReservoirPropertyCalculator.C
   src/SurfaceProperty.C
   src/SurfacePropertyCalculator.C
   src/SurfacePropertyOffsetCalculator.C
)


set( LIB_NAME "AbstractDerivedProperties" )
add_library(${LIB_NAME}
  ${all_srcs}
  ${all_headers}
)
set_target_properties( ${LIB_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${LIB_NAME}" )

target_link_libraries(${LIB_NAME}
  DataModel
  utilities
)

include_directories(
  ${PROJECT_SOURCE_DIR}/libraries/DataModel/src
  ${PROJECT_SOURCE_DIR}/libraries/utilities/src
)

generate_dox( AbstractDerivedProperties.cfg )

