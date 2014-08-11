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

add_executable(tracktraps
	src/ProjectHandle.C
	src/PersistentTrap.C
	src/PersistentTrapFactory.C
	src/Reservoir.C
	src/Trap.C
	src/generics.C
	src/main.C
	src/utils.C
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

include_directories(
	${HDF5_INCLUDE_DIRS}
)


target_link_libraries(tracktraps
	EosPack
	DataAccess
	SerialDataAccess
	TableIO
	utilities
	CBMGenerics
	${HDF5_LIBRARIES}
)

install(TARGETS tracktraps
	RUNTIME DESTINATION bin
)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
