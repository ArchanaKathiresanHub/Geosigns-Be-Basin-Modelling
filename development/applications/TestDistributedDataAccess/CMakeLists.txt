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
	
	
