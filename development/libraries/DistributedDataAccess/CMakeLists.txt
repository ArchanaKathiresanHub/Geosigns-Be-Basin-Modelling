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
	add_library( DistributedDataAccess
		src/DistributedApplicationGlobalOperations.C
		src/DistributedMapWriter.C
		src/DistributedGridMap.C
		src/DistributedGrid.C
		src/DistributedMessageHandler.C
		src/DistributedObjectFactory.C
		src/DistributedProjectHandle.C
		src/GlobalGrid.C
		src/Distributedhdf5funcs.C
	)	

	target_link_libraries( DistributedDataAccess 
		DataAccess
		Utilities_Petsc
		Serial_Hdf5
		Parallel_Hdf5
		TableIO
		utilities
		CBMGenerics
		${HDF5_parallel_LIBRARIES}
		${MPI_LIBRARIES}
		${PETSC_LIBRARIES}
	)

	bm_include_libraries(
		DataAccess 
		Utilities_Petsc 
		Serial_Hdf5 
		Parallel_Hdf5
		TableIO
		utilities 
		CBMGenerics
		)
	
	remove_definitions(-fPIC)
	add_definitions(${HDF5_parallel_DEFINITIONS})

	include_directories( 
		src
		${PETSC_INCLUDE_DIRS}
		${HDF5_parallel_INCLUDE_DIRS}
		${MPI_INCLUDE_DIRS}
	)

        set_target_properties(DistributedDataAccess
            PROPERTIES LINK_FLAGS "${PETSC_LINK_FLAGS}"
        )


	install(TARGETS DistributedDataAccess
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
	)
endif(BM_PARALLEL)

generate_dox( src/DistributedDataAccess.cfg )
# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
