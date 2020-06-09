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

set( TARGET_NAME "EosPackDemo" )
add_executable(${TARGET_NAME} src/main.cpp)
set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${TARGET_NAME}" )

target_link_libraries( ${TARGET_NAME}
   EosPack
   CBMGenerics
   utilities
)

install( TARGETS ${TARGET_NAME}
   DESTINATION   bin
)
