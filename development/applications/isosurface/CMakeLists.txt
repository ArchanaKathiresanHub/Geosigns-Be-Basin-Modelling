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

set( TARGET_NAME "isosurface" )
add_executable(${TARGET_NAME}
   src/isosurface.cpp
)
set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${TARGET_NAME}" )

include_directories( SYSTEM
   ${HDF5_INCLUDE_DIRS}
)

target_link_libraries(${TARGET_NAME}
   DataAccess
   SerialDataAccess
   TableIO
   CBMGenerics
   utilities
)

install(TARGETS ${TARGET_NAME}
   RUNTIME DESTINATION bin
)
