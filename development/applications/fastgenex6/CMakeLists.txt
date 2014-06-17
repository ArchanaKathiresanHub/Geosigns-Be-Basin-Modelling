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

add_executable( fastgenex6
	src/main.C
	src/GenexSimulator.C
	src/GenexSimulatorFactory.C
)

include_directories(
	${HDF5_INCLUDE_DIRS}
	${MPI_INCLUDE_DIRS}
	${FLEXLM_INCLUDE_DIRS}
	${PETSC_INCLUDE_DIRS}
)

bm_include_libraries(
	DataAccess
	DistributedDataAccess
	Utilities_Petsc
	Serial_Hdf5
	OneFilePerProcess
	Parallel_Hdf5
	genex6
	genex6_kernel
	CBMGenerics
	EosPack
	OTGC_kernel6
	utilities
	TableIO
)

target_link_libraries( fastgenex6
	DataAccess
	DistributedDataAccess
	Utilities_Petsc
	Serial_Hdf5
	OneFilePerProcess
	Parallel_Hdf5
	genex6
	genex6_kernel
	CBMGenerics
	EosPack
	OTGC_kernel6
	utilities
	TableIO
	${HDF5_LIBRARIES}
	${FLEXLM_LIBRARIES}
	${PETSC_LIBRARIES}
	${BLAS_LIBRARIES}
	${BM_MPI_LOCK_LIBRARIES}
)

set_target_properties(fastgenex6
      PROPERTIES LINK_FLAGS "${PETSC_LINK_FLAGS}"
)


install(TARGETS fastgenex6
	RUNTIME DESTINATION bin
)

endif(BM_PARALLEL)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
