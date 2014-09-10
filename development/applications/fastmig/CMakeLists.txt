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

add_executable(fastmig ${all_srcs} ${all_headers})

add_definitions(-DUSEOTGC)

include_directories(
    ${PROJECT_SOURCE_DIR}/libraries
	${FLEXLM_INCLUDE_DIRS}
	${PETSC_INCLUDE_DIRS}
	${HDF5_INCLUDE_DIRS}
	${MPI_INCLUDE_DIRS}
)

bm_include_libraries(
	DataModel
	DataAccess
	DistributedDataAccess
	Utilities_Petsc
	Serial_Hdf5
	Parallel_Hdf5
	EosPack	
	CBMGenerics
	genex6
	OTGC_kernel6
	utilities
	TableIO
	functions
)

# OFPP only for Linux
if (UNIX)
	bm_include_libraries( OneFilePerProcess )
endif (UNIX)

set (targetLibraries
	${HDF5_LIBRARIES}
	${PETSC_LIBRARIES}
	${BLAS_LIBRARIES}
	${MPI_LIBRARIES}
)

#only add flexlm for Linux
if (UNIX)
	set (targetLibraries
		${targetLibraries}
		${FLEXLM_LIBRARIES}
		OneFilePerProcess
	)
else ()
	# add a link flag to resolve a conflict between an Intel and MSVCRT library (multiple definition of floor/ceil)
	set( CMAKE_EXE_LINKER_FLAGS  
		"${CMAKE_EXE_LINKER_FLAGS} /FORCE:MULTIPLE" 
	)
endif (UNIX)

target_link_libraries(fastmig
	DataAccess
	DistributedDataAccess
	Utilities_Petsc
	Serial_Hdf5
	Parallel_Hdf5
	EosPack	
	CBMGenerics
	genex6
	OTGC_kernel6
	utilities
	TableIO
	functions
	${PETSC_LIBRARIES}
	${HDF5_LIBRARIES}
	${BLAS_LIBRARIES}
	${BM_MPI_LOCK_LIBRARIES}
	${targetLibraries}
)

set_target_properties(fastmig
      PROPERTIES LINK_FLAGS "${PETSC_LINK_FLAGS}"
)

install(TARGETS fastmig
	RUNTIME DESTINATION bin
)

endif (BM_PARALLEL)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
