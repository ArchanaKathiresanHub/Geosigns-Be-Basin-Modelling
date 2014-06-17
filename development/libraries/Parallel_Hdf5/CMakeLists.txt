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

	add_library( Parallel_Hdf5
		src/h5_vector_conversions.C
		src/h5_parallel_file_types.C
		src/petscvector_readwrite.C
	)

	bm_include_libraries(
		Serial_Hdf5
		OneFilePerProcess
		Utilities_Petsc
	)

	include_directories(
		${HDF5_INCLUDE_DIRS}
		${MPI_INCLUDE_DIRS}
		${PETSC_INCLUDE_DIRS}
	)

	target_link_libraries(Parallel_Hdf5 
		Utilities_Petsc
		Serial_Hdf5
		OneFilePerProcess
		${PETSC_LIBRARIES}
		${MPI_LIBRARIES}
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

