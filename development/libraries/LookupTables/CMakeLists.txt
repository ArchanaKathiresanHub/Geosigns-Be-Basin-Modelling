#########################################################################
#                                                                       #
# Copyright (C) 2012-2014 Shell International Exploration & Production. #
# All rights reserved.                                                  #
#                                                                       #
# Developed under license for Shell by PDS BV.                          #
#                                                                       #
# Confidential and proprietary source code of Shell.                    #
# Do not distribute without written permission from Shell.              #
#                                                                       #
#########################################################################

set(all_headers
  src/CO2-H2O.h
  src/CO2-H2O-NaCl1.h
  src/CO2-H2O-NaCl2.h
  src/CO2-H2O-NaCl4.h
  src/CO2-H2O-NaCl6.h
  src/H2S-H2O.h
  src/H2S-H2O-NaCl1.h
  src/H2S-H2O-NaCl2.h
  src/H2S-H2O-NaCl4.h
  src/H2S-H2O-NaCl6.h
  src/LookUpDirectory.h
  src/LookUp.h

)

set(all_sources
  src/LookUpDirectory.C
  src/LookUp.C
)

include_directories( 
    ${PROJECT_SOURCE_DIR}/libraries/EosPack/src
    ${PROJECT_SOURCE_DIR}/libraries/CBMGenerics/src )


source_group(include FILES ${all_headers})
source_group(source  FILES ${all_sources})

set( LIB_NAME "LookupTables" )
add_library(${LIB_NAME}
   ${all_sources}
   ${all_headers}
)
set_target_properties( ${LIB_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${LIB_NAME}" )

add_gtest( NAME TSRlookupTest:GetValue
           SOURCES test/lookup.C
           LIBRARIES ${LIB_NAME}
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)
add_gtest( NAME TSRlookupTest:GetTableName
           SOURCES test/lookup.C
           LIBRARIES ${LIB_NAME}
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME TSRlookupTest:TablesCheck
           SOURCES test/lookup.C
           LIBRARIES ${LIB_NAME}
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME TSRlookupTest:GetValueForSalinity
           SOURCES test/lookup.C
           LIBRARIES ${LIB_NAME}
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)
