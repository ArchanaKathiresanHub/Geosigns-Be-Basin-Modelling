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
	${HDF5_serial_INCLUDE_DIRS}
	${PROJECT_SOURCE_DIR}/libraries
	${PROJECT_SOURCE_DIR}/libraries/SerialDataAccess/src/Interface
	${PROJECT_SOURCE_DIR}/libraries/DataAccess/src/Interface
)

bm_include_libraries(
	SerialDataAccess
	DataAccess
	EosPack	
	CBMGenerics
	genex6
	OTGC_kernel6
	TableIO
	CBMGenerics
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
	utilities
	${HDF5_serial_LIBRARIES}
)

install(TARGETS UnitTestOTGC6
	RUNTIME DESTINATION bin
)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
