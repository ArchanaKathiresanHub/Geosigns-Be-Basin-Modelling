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

add_executable(UnitTestGenex6 src/main.C)

bm_include_libraries(
	DistributedDataAccess
	DataAccess	
	genex6
	TableIO
	genex6_kernel
	CBMGenerics
	EosPack
	utilities
)

include_directories(
	${HDF5_INCLUDE_DIRS}
)

target_link_libraries(UnitTestGenex6
	DistributedDataAccess
	TableIO
	genex6_kernel
	genex6
	CBMGenerics
	EosPack
	utilities
	${MPI_LIBRARIES}
)

		
