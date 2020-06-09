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

set( APP_NAME "tracktraps" )

add_executable(${APP_NAME}
   src/TrackProjectHandle.cpp
   src/PersistentTrap.cpp
   src/PersistentTrapFactory.cpp
   src/TrackReservoir.cpp
   src/TrackTrap.cpp
   src/generics.cpp
   src/main.cpp
)

set_target_properties( ${APP_NAME} PROPERTIES SUFFIX ".exe" )
set_target_properties( ${APP_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${APP_NAME}" )

create_application_run_script( ${APP_NAME} )

application_env_var( TARGET ${APP_NAME} VARS
     "EOSPACKDIR"  "$MISCDIR/eospack"
)

include_directories( SYSTEM
   ${HDF5_INCLUDE_DIRS}
)


target_link_libraries(${APP_NAME}
   EosPack
   DataAccess
   SerialDataAccess
   TableIO
   utilities
   CBMGenerics
   ${HDF5_LIBRARIES}
)

install(TARGETS ${APP_NAME}
   RUNTIME DESTINATION bin
)
