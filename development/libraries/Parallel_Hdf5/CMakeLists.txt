if (BM_PARALLEL)

	add_library( Parallel_Hdf5 
		src/h5_vector_conversions.C
		src/petscvector_readwrite.C
	)

	bm_include_libraries(
		Serial_Hdf5
		Utilities_Petsc
	)

	add_definitions(${HDF5_parallel_DEFINITIONS})

	include_directories(
		${HDF5_parallel_INCLUDE_DIRS}
		${MPI_C_INCLUDE_PATH}
		${PETSC_INCLUDE_DIRS}
	)

	target_link_libraries(Parallel_Hdf5 
		Utilities_Petsc
		Serial_Hdf5
		${PETSC_LIBRARIES}
		${MPI_C_LIBRARIES}
	)
	
        set_target_properties(Parallel_Hdf5
            PROPERTIES LINK_FLAGS "${PETSC_LINK_FLAGS}"
        )


	
	install(TARGETS Parallel_Hdf5
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
	)


endif(BM_PARALLEL)
# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:

