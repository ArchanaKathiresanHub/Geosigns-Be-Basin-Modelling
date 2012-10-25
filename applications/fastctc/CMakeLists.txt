if (BM_PARALLEL)

add_executable(fastctc
	src/CrustalThicknessCalculator.C 
	src/CrustalThicknessCalculatorFactory.C 
	src/main.C
)

bm_include_libraries(
	DataMining
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
	${PROJECT_SOURCE_DIR}/libraries/DataAccess/src/Interface
	${HDF5_INCLUDE_DIRS}
	${PETSC_INCLUDE_DIRS}
	${FLEXLM_INCLUDE_DIRS}
)

target_link_libraries(fastctc
	DataMining
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
)

install(TARGETS fastctc
	RUNTIME DESTINATION bin
)

endif(BM_PARALLEL)
