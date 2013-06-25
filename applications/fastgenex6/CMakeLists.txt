if (BM_PARALLEL)

add_executable( fastgenex6
	src/main.C
	src/GenexSimulator.C
	src/GenexSimulatorFactory.C
)

add_definitions(${HDF5_parallel_DEFINITIONS})

include_directories(
	${HDF5_parallel_INCLUDE_DIRS}
	${MPI_C_INCLUDE_PATH}
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
	${HDF5_parallel_LIBRARIES}
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
