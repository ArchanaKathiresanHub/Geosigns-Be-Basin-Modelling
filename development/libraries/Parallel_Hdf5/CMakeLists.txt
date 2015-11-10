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

	FILE( GLOB all_headers src/*.h )
	source_group(include FILES ${all_headers})

	FILE( GLOB all_srcs src/*.C)
	source_group(source FILES ${all_srcs})

	add_library(Parallel_Hdf5 
	   ${all_srcs}
	   ${all_headers}
	)
	if (UNIX)
	    add_dependencies(Parallel_Hdf5 PETSC HDF5)
	endif (UNIX)
	set (OFPP "")
	if (UNIX)
		set (OFPP OneFilePerProcess)
		bm_include_libraries( OneFilePerProcess )
	endif ()

	bm_include_libraries(
		Serial_Hdf5
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
		${OFPP}
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

