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
   src/AbstractFormation.h
   src/AbstractGrid.h
   src/AbstractProperty.h
   src/AbstractReservoir.h
   src/AbstractSnapshot.h
   src/AbstractSurface.h
   src/AbstractValidator.h
   src/PropertyAttribute.h
)

set(all_sources
   src/PropertyAttribute.C
)

source_group(include FILES ${all_headers})
source_group(source  FILES ${all_sources})

set( LIB_NAME "DataModel" )
add_library(${LIB_NAME}
   ${all_sources}
   ${all_headers}
)
set_target_properties( ${LIB_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${LIB_NAME}" )
