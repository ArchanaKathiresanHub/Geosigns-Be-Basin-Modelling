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

add_executable(fastreq
	src/main.C
	src/FastReqProjectHandle.C
	src/FastReqObjectFactory.C
)

include_directories(
	${HDF5_INCLUDE_DIRS}
	${PROJECT_SOURCE_DIR}/libraries/DataModel/src
)

bm_include_libraries(
	SerialDataAccess
	DataAccess
	TableIO
	Serial_Hdf5
	CBMGenerics
	utilities
)

target_link_libraries( fastreq
	DataAccess
	SerialDataAccess
	TableIO
	Serial_Hdf5
	CBMGenerics
	utilities
	${HDF5_LIBRARIES}
)

install(TARGETS fastreq
	RUNTIME DESTINATION bin
)


generate_dox( src/fastreq.cfg )
# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
