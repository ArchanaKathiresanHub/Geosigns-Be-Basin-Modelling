#########################################################################
#                                                                       #
# Copyright (C) 2012-2016 Shell International Exploration & Production. #
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
   #src/AbstractTableCTC.h
   #src/AbstractTableCTCRiftingHistory.h
   src/PropertyAttribute.h
)

set(all_sources
   src/PropertyAttribute.C
)


source_group(include FILES ${all_headers})
source_group(source  FILES ${all_sources})

add_library(DataModel
   ${all_sources}
   ${all_headers}
)
