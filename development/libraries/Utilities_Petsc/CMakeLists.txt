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

	add_library(Utilities_Petsc STATIC
		src/PetscVectors.C
	)

	include_directories(
		${PETSC_INCLUDE_DIRS}
		${MPI_INCLUDE_DIRS}
	)

	target_link_libraries(Utilities_Petsc 
		${PETSC_LIBRARIES}
		${MPI_LIBRARIES}
	)

        set_target_properties(Utilities_Petsc
            PROPERTIES LINK_FLAGS "${PETSC_LINK_FLAGS}"
        )


	install(TARGETS Utilities_Petsc
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
	)

else (BM_PARALLEL)
	message("Utilities_Petsc will not be compiled, because MPI is unavailable")
endif (BM_PARALLEL)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
