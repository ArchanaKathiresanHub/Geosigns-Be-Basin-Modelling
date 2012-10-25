if (BM_PARALLEL)

	add_library( Parallel_Hdf5 
		src/h5_vector_conversions.C
		src/petscvector_readwrite.C
	)

	bm_include_libraries(
		Serial_Hdf5
		Utilities_Petsc
	)

	include_directories(
		${HDF5_INCLUDE_DIRS} 
		${MPI_C_INCLUDE_PATH}
		${PETSC_INCLUDE_DIRS}
	)

	target_link_libraries(Parallel_Hdf5 
		Utilities_Petsc
		Serial_Hdf5
		${PETSC_LIBRARIES}
		${MPI_C_LIBRARIES}
	)
	
	
	install(TARGETS Parallel_Hdf5
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
	)


endif(BM_PARALLEL)
