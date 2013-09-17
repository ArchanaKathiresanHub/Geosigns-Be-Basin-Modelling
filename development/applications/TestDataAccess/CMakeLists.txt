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

add_executable(TestDataAccess src/TestDataAccess.C)

include_directories(
	${HDF5_serial_INCLUDE_DIRS}
	${PROJECT_SOURCE_DIR}/libraries/SerialDataAccess/src/Interface
	${PROJECT_SOURCE_DIR}/libraries/DataAccess/src/Interface
)

bm_include_libraries(
	SerialDataAccess
	DataAccess
	TableIO
	CBMGenerics
)

target_link_libraries(TestDataAccess
	DataAccess
	SerialDataAccess
	TableIO
	${HDF5_serial_LIBRARIES}
)

install(TARGETS TestDataAccess
	RUNTIME DESTINATION bin
)

# Local Variables:

# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
