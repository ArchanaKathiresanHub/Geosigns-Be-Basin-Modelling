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

add_executable(fastgenex6 ${all_srcs} ${all_headers})

include_directories(
	${HDF5_INCLUDE_DIRS}
	${MPI_INCLUDE_DIRS}
	${PETSC_INCLUDE_DIRS}
)

bm_include_libraries(
	DataModel
	DataAccess
	DerivedProperties
	GeoPhysics
	Interpolation
	LinearAlgebra
	DistributedDataAccess
	Utilities_Petsc
	Serial_Hdf5
	Parallel_Hdf5
	genex6
	genex6_kernel
	CBMGenerics
	EosPack
	OTGC_kernel6
	utilities
	TableIO
)

target_link_libraries( fastgenex6
	DataAccess
	DerivedProperties
	GeoPhysics
	DistributedDataAccess
	Utilities_Petsc
	Serial_Hdf5
	Parallel_Hdf5
	genex6
	genex6_kernel
	CBMGenerics
	EosPack
	OTGC_kernel6
	utilities
	TableIO
	${HDF5_LIBRARIES}
	${PETSC_LIBRARIES}
)

# OFPP only for Linux
if (UNIX)
	target_link_libraries(fastgenex6 OneFilePerProcess)
	bm_include_libraries( OneFilePerProcess )
endif (UNIX)

set_target_properties(fastgenex6
      PROPERTIES LINK_FLAGS "${PETSC_LINK_FLAGS}"
)

if (BM_USE_FLEXLM)
   include_directories( ${FLEXLM_INCLUDE_DIRS})
   target_link_libraries(fastgenex6 ${FLEXLM_LIBRARIES})
endif(BM_USE_FLEXLM)


install(TARGETS fastgenex6
	RUNTIME DESTINATION bin
)

endif(BM_PARALLEL)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
