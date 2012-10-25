if (BM_PARALLEL)

add_executable( fastgenex6
	src/main.C
	src/GenexSimulator.C
	src/GenexSimulatorFactory.C
)

include_directories(
	${HDF5_INCLUDE_DIRS}
	${FLEXLM_INCLUDE_DIRS}
	${PETSC_INCLUDE_DIRS}
)

bm_include_libraries(
	DataAccess
	DistributedDataAccess
	Utilities_Petsc
	Serial_Hdf5
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
)

install(TARGETS fastgenex6
	RUNTIME DESTINATION bin
)

endif(BM_PARALLEL)
