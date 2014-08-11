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

add_executable(TestDistributedDataAccess src/TestDataAccess.C)

bm_include_libraries(
	DataAccess
	DistributedDataAccess
	Utilities_Petsc
	genex5_kernel
	Serial_Hdf5
	Parallel_Hdf5
	CBMGenerics
	TableIO
	utilities
)

include_directories(
	${HDF5_INCLUDE_DIRS}
	${PETSC_INCLUDE_DIRS}
	${FLEXLM_INCLUDE_DIRS}
	${PROJECT_SOURCE_DIR}/libraries/DataModel/src
)

target_link_libraries(TestDistributedDataAccess
	DataAccess
	DistributedDataAccess
	Utilities_Petsc
	Serial_Hdf5
	Parallel_Hdf5
	TableIO
	${FLEXLM_LIBRARIES}
)
	
	
