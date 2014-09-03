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

add_executable(fastcauldron ${all_srcs} ${all_headers})

include_directories(
        ${PROJECT_SOURCE_DIR}/libraries
	${HDF5_INCLUDE_DIRS}
	${MPI_INCLUDE_DIRS}
	${PETSC_INCLUDE_DIRS}
	${FLEXLM_INCLUDE_DIRS}
	${EIGEN_INCLUDE_DIRS}
        ${BLAS_INCLUDE_DIRS}
)

bm_include_libraries(
	DataModel
	DataAccess
	DistributedDataAccess
	utilities
	Utilities_Petsc
	Serial_Hdf5
	Parallel_Hdf5
	EosPack
	TableIO
	LinearAlgebra
	Interpolation
	FiniteElements
	CBMGenerics
	genex6
	genex6_kernel	
	GeoPhysics
	calibration_kernel	
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

target_link_libraries(fastcauldron
	DataAccess
	DistributedDataAccess
	utilities
	Utilities_Petsc
	Serial_Hdf5
	Parallel_Hdf5
	EosPack
	TableIO
	LinearAlgebra
	Interpolation
	FiniteElements
	CBMGenerics
	genex6
	genex6_kernel	
	GeoPhysics
	calibration_kernel	
	OTGC_kernel6
	${targetLibraries}
)

set_target_properties(fastcauldron
      PROPERTIES LINK_FLAGS "${PETSC_LINK_FLAGS}"
)

install(TARGETS fastcauldron
	RUNTIME DESTINATION bin
)

include_directories ( src ${EIGEN_INCLUDE_DIRS} )

# no 'dl' library on Windows
if (UNIX)
add_gtest( NAME VectorInterpolator
           SOURCES test/InterpolationTest.C 
           LIBRARIES LinearAlgebra ${BLAS_LIBRARIES} dl
           )

add_gtest( NAME MatrixInterpolator
           SOURCES test/MatrixInterpolationTest.C
           LIBRARIES LinearAlgebra ${BLAS_LIBRARIES} dl )
endif (UNIX)

add_gtest( NAME "BrooksCorey::computeCapillaryEntryPressure"
	   SOURCES test/PceTest.C )

	
endif(BM_PARALLEL)	

generate_dox( src/fastcauldron.cfg )

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
