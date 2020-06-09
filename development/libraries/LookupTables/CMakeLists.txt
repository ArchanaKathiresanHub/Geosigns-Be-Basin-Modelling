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

include_directories( 
    ${PROJECT_SOURCE_DIR}/libraries/EosPack/src
    ${PROJECT_SOURCE_DIR}/libraries/CBMGenerics/src )

set( LIB_NAME "LookupTables" )

create_bm_library( TARGET ${LIB_NAME} )

add_gtest( NAME TSRlookupTest:GetValue
           SOURCES test/lookup.cpp
           LIBRARIES ${LIB_NAME}
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)
add_gtest( NAME TSRlookupTest:GetTableName
           SOURCES test/lookup.cpp
           LIBRARIES ${LIB_NAME}
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME TSRlookupTest:TablesCheck
           SOURCES test/lookup.cpp
           LIBRARIES ${LIB_NAME}
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME TSRlookupTest:GetValueForSalinity
           SOURCES test/lookup.cpp
           LIBRARIES ${LIB_NAME}
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)
