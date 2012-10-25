if (BM_PARALLEL)

add_library(CrustalThickness
	src/LinearFunction.C
	src/DensityCalculator.C
	src/InterfaceInput.C
	src/InterfaceOutput.C
)

bm_include_libraries( 
	DistributedDataAccess
	DataAccess 
	TableIO 
	Utilities 
	Serial_Hdf5
	Parallel_Hdf5 
	utilities 
	CBMGenerics
)

include_directories(
	${HDF5_INCLUDE_DIRS}
	${PETSC_INCLUDE_DIRS}
	${MPI_C_INCLUDE_PATH}
)

target_link_libraries(CrustalThickness
	DistributedDataAccess
	DataAccess
	TableIO
	Utilities_Petsc
	Serial_Hdf5
	Parallel_Hdf5
	utilities
	${HDF5_LIBRARIES}
	${PETSC_LIBRARIES}
	${MPI_C_LIRBARIES}
)

install(TARGETS CrustalThickness
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

endif(BM_PARALLEL)
