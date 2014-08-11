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

add_executable(cauldron2bhf
	src/cauldron2bhf.C
)

include_directories(
	${HDF5_INCLUDE_DIRS}
)

bm_include_libraries(
	DataModel
	SerialDataAccess
	DataAccess
	TableIO
	CBMGenerics
)

target_link_libraries(cauldron2bhf
	DataAccess
	SerialDataAccess
	TableIO
	${HDF5_LIBRARIES}
)

install(TARGETS cauldron2bhf
	RUNTIME DESTINATION bin
)
# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:

