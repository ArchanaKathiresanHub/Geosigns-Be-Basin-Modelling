
if (BM_PARALLEL)

	add_library(Utilities_Petsc ${LIBRARY_TYPE}
		src/PetscVectors.C
	)

	include_directories(
		${PETSC_INCLUDE_DIRS}
		 ${MPI_C_INCLUDE_PATH}
	)

	target_link_libraries(Utilities_Petsc 
		${MPI_C_LIBRARIES}
		${PETSC_LIBRARIES}
	)

	install(TARGETS Utilities_Petsc
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
	)

else (BM_PARALLEL)
	message("Utilities_Petsc will not be compiled, because MPI is unavailable")
endif (BM_PARALLEL)
