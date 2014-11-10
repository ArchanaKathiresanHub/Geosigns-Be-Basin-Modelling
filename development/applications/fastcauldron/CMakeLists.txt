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
        src
        ${PROJECT_SOURCE_DIR}/libraries
	${HDF5_INCLUDE_DIRS}
	${MPI_INCLUDE_DIRS}
	${PETSC_INCLUDE_DIRS}
	${EIGEN_INCLUDE_DIRS}
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
	${HDF5_LIBRARIES}
	${PETSC_LIBRARIES}
	${MPI_LIBRARIES}
	${Boost_LIBRARIES}
)

set_target_properties(fastcauldron
      PROPERTIES LINK_FLAGS "${PETSC_LINK_FLAGS}"
)

if(UNIX)
   # The OneFilePerProcess library is only available on Unix
   bm_include_libraries( OneFilePerProcess )
   target_link_libraries(fastcauldron OneFilePerProcess)
endif()

if (BM_USE_FLEXLM)
   include_directories( ${FLEXLM_INCLUDE_DIRS})
   target_link_libraries(fastcauldron ${FLEXLM_LIBRARIES})
endif(BM_USE_FLEXLM)

install(TARGETS fastcauldron
	RUNTIME DESTINATION bin
)

add_gtest( NAME "VectorInterpolator"
           SOURCES test/InterpolationTest.C 
           LIBRARIES LinearAlgebra 
           )

add_gtest( NAME "MatrixInterpolator"
           SOURCES test/MatrixInterpolationTest.C
           LIBRARIES LinearAlgebra )

add_gtest( NAME "BrooksCorey::computeCapillaryEntryPressure"
	   SOURCES test/PceTest.C )

configure_file( 
      ${CMAKE_CURRENT_SOURCE_DIR}/test/MemCheck.project3d
      MemCheck.project3d
      COPYONLY
)

add_memcheck( NAME "Fastcauldron_SimpleItcoupled"
             EXECUTABLE fastcauldron
             PARAMETERS "-project" "MemCheck.project3d" "-itcoupled"
      )
	
endif(BM_PARALLEL)	

generate_dox( src/fastcauldron.cfg )

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
