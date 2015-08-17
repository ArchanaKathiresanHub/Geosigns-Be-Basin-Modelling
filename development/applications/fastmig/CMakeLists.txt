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
	${PETSC_INCLUDE_DIRS}
	${HDF5_INCLUDE_DIRS}
	${MPI_INCLUDE_DIRS}
)

bm_include_libraries(
	DataModel
	DataAccess
	DistributedDataAccess
	DerivedProperties
	GeoPhysics
	Interpolation
	LinearAlgebra
	Utilities_Petsc
	Serial_Hdf5
	Parallel_Hdf5
	EosPack	
	CBMGenerics
	genex6
	genex6_kernel
	utilities
	TableIO
	functions
)


target_link_libraries(fastmig
	DataModel
	DataAccess
	DerivedProperties
	DistributedDataAccess
	GeoPhysics
	Utilities_Petsc
	Serial_Hdf5
	Parallel_Hdf5
	EosPack	
	CBMGenerics
	genex6
	genex6_kernel
	OTGC_kernel6
	utilities
	TableIO
	functions
	${PETSC_LIBRARIES}
	${HDF5_LIBRARIES}
	${MPI_LIBRARIES}
)

set_target_properties(fastmig
      PROPERTIES LINK_FLAGS "${PETSC_LINK_FLAGS}"
)
   
if (UNIX)
   # The OneFilePerProcess library is only available on Unix
   bm_include_libraries( OneFilePerProcess )
   target_link_libraries(fastmig OneFilePerProcess)
endif ()

if (BM_USE_FLEXLM)
   include_directories(${FLEXLM_INCLUDE_DIRS})
   target_link_libraries(fastmig ${FLEXLM_LIBRARIES})
endif(BM_USE_FLEXLM)

install(TARGETS fastmig
	RUNTIME DESTINATION bin
)


### Unit Tests

add_gtest( NAME "Fastmig::Biodegrade"
		   SOURCES test/BiodegradeTest.C src/Biodegrade.C
		  )

endif (BM_PARALLEL)

generate_dox( src/fastmig.cfg )

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
