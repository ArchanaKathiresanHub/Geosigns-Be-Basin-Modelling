if (BM_PARALLEL)

add_executable(fasttouch
	src/main.C
	src/FastTouch.C
	src/BurialHistory.C
	src/MasterTouch.C
	src/ObjectFactory.C
)

include_directories(
	${TS_INCLUDE_DIRS}
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
	TableIO
	utilities
	CBMGenerics
)

target_link_libraries(fasttouch
	DataAccess
	DistributedDataAccess
	Utilities_Petsc
	Serial_Hdf5
	Parallel_Hdf5
	TableIO
	utilities
	CBMGenerics
	${TS_LIBRARIES}
	${HDF5_LIBRARIES}
	${PETSC_LIBRARIES}
	${FLEXLM_LIBRARIES}
	${BLAS_LIBRARIES}
)

install(TARGETS fasttouch
	RUNTIME DESTINATION bin
)

endif (BM_PARALLEL)
