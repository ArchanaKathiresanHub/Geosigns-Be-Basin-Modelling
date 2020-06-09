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

add_executable(UnitTestGenex6
        src/main.cpp
)

include_directories( SYSTEM
	${HDF5_INCLUDE_DIRS}
)

target_link_libraries(UnitTestGenex6
	DataAccess
	SerialDataAccess
	EosPack	
	CBMGenerics
	genex6
	genex6_kernel
	GeoPhysics
	TableIO
	utilities
	${HDF5_LIBRARIES}
)

install(TARGETS UnitTestGenex6
	RUNTIME DESTINATION bin
)
