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

add_executable(isosurface
	src/isosurface.C
)

include_directories(
	${HDF5_INCLUDE_DIRS}
	${PROJECT_SOURCE_DIR}/libraries/DataModel/src
)

bm_include_libraries(
	SerialDataAccess
	DataAccess
	TableIO
	utilities
	CBMGenerics
)

target_link_libraries(isosurface
	DataAccess
	SerialDataAccess
	TableIO
	CBMGenerics
	utilities
)

install(TARGETS isosurface
	RUNTIME DESTINATION bin
)
# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
