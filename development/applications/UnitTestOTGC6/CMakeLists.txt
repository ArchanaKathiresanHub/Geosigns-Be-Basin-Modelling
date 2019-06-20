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

include_directories( SYSTEM
	${HDF5_INCLUDE_DIRS}
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
