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

add_executable(UnitTestOTGC6
	src/TestingCode.C
	src/main.C
)

include_directories(
	${HDF5_INCLUDE_DIRS}
)

bm_include_libraries(
	DataModel
	SerialDataAccess
	DataAccess
	EosPack	
	CBMGenerics
	genex6
	OTGC_kernel6
	TableIO
	CBMGenerics
	FileSystem
	utilities
)

target_link_libraries(UnitTestOTGC6
	DataAccess
	SerialDataAccess
	EosPack	
	CBMGenerics
	genex6
	OTGC_kernel6
	TableIO
	FileSystem
	utilities
	${HDF5_LIBRARIES}
)

install(TARGETS UnitTestOTGC6
	RUNTIME DESTINATION bin
)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
