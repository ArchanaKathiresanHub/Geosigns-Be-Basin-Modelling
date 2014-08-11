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

add_executable(eosptdiag
	src/main.C
	src/TrapperIoTableRec.C
)

bm_include_libraries(
	DataAccess
	SerialDataAccess
	TableIO
	CBMGenerics
	EosPack
)

include_directories(
	${HDF5_INCLUDE_DIRS}
	${PROJECT_SOURCE_DIR}/libraries/DataModel/src
)

target_link_libraries(eosptdiag
	DataAccess
	SerialDataAccess
	TableIO
	CBMGenerics
	EosPack
	${HDF5_LIBRARIES}
)

install(TARGETS eosptdiag
	RUNTIME DESTINATION bin
)
