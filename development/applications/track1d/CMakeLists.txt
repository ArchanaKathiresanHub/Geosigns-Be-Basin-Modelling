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

add_executable(track1d
	src/track1d.C
)

bm_include_libraries(
	DataModel
	DataMining
	DataAccess
	SerialDataAccess
	DataAccess	
	TableIO
	CBMGenerics
	Serial_Hdf5
	EosPack
	utilities
)

include_directories(
	${HDF5_INCLUDE_DIRS}
)

target_link_libraries(track1d
	DataMining
	DataAccess
	SerialDataAccess
	DataAccess	
	TableIO
	CBMGenerics
	Serial_Hdf5
	EosPack
	utilities
	${HDF5_LIBRARIES}
)

install(TARGETS track1d
	RUNTIME DESTINATION bin
)
# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
