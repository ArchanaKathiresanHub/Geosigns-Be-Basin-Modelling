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
	src/ProjectHandle.C
	src/PersistentTrap.C
	src/PersistentTrapFactory.C
	src/Reservoir.C
	src/Trap.C
	src/generics.C
	src/main.C
)

set_target_properties( ${APP_NAME} PROPERTIES SUFFIX ".exe" )

create_application_run_script( ${APP_NAME} )

application_env_var( TARGET ${APP_NAME} VARS
     "EOSPACKDIR"  "$MISCDIR/eospack"
)

bm_include_libraries(
	DataModel
	EosPack
	DataAccess
	SerialDataAccess
	TableIO
	utilities
	CBMGenerics
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

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
