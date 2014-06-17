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

if (BM_PARALLEL)

add_executable(fastctc
	src/CrustalThicknessCalculator.C 
	src/CrustalThicknessCalculatorFactory.C 
	src/main.C
)

bm_include_libraries(
	DataAccess
	DistributedDataAccess
	TableIO
	CBMGenerics
	Serial_Hdf5
	Parallel_Hdf5
	utilities
	Utilities_Petsc
	LinearAlgebra
	Interpolation
	CrustalThickness
)

include_directories(
	${HDF5_INCLUDE_DIRS}
	${MPI_INCLUDE_DIRS}
	${PETSC_INCLUDE_DIRS}
	${FLEXLM_INCLUDE_DIRS}
)

target_link_libraries(fastctc
	DataAccess
	DistributedDataAccess
	TableIO
	CBMGenerics
	Serial_Hdf5
	Parallel_Hdf5
	utilities
	Utilities_Petsc
	LinearAlgebra
	Interpolation
	CrustalThickness
	${HDF5_LIBRARIES}
	${PETSC_LIBRARIES}
	${FLEXLM_LIBRARIES}
	${BM_MPI_LOCK_LIBRARIES}
)

set_target_properties(fastctc
      PROPERTIES LINK_FLAGS "${PETSC_LINK_FLAGS}"
)


install(TARGETS fastctc
	RUNTIME DESTINATION bin
)

endif(BM_PARALLEL)
# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:

