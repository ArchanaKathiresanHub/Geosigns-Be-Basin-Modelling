#########################################################################
#                                                                       #
# Copyright (C) 2012-2015 Shell International Exploration & Production. #
# All rights reserved.                                                  #
#                                                                       #
# Developed under license for Shell by PDS BV.                          #
#                                                                       #
# Confidential and proprietary source code of Shell.                    #
# Do not distribute without written permission from Shell.              #
#                                                                       #
#########################################################################

if (BM_PARALLEL)

set( all_headers
   src/CrustalThicknessCalculator.h
	src/CrustalThicknessCalculatorFactory.h
)

set( all_srcs
   src/CrustalThicknessCalculator.C 
	src/CrustalThicknessCalculatorFactory.C 
	src/main.C
)

source_group(include FILES ${all_headers})
source_group(source FILES ${all_srcs})

add_executable(fastctc ${all_srcs} ${all_headers})

bm_include_libraries(
	DataModel
	DataAccess
	DistributedDataAccess
	DerivedProperties
	GeoPhysics
	Interpolation
	LinearAlgebra
   TableIO
   EosPack
	CBMGenerics
	Serial_Hdf5
   OneFilePerProcess
	Parallel_Hdf5
	utilities
	Utilities_Petsc
	LinearAlgebra
	Interpolation
	CrustalThickness
)

include_directories(
	${HDF5_INCLUDE_DIRS}
	${MPI_INCLUDE_DIRS}
	${PETSC_INCLUDE_DIRS}
	${FLEXLM_INCLUDE_DIRS}
)

target_link_libraries(fastctc
	DataModel
	DataAccess
	DistributedDataAccess
	DerivedProperties
	GeoPhysics
	Interpolation
	LinearAlgebra
   TableIO
   EosPack
	CBMGenerics
	Serial_Hdf5
  	Parallel_Hdf5
	utilities
	Utilities_Petsc
	LinearAlgebra
	Interpolation
	CrustalThickness
   ${HDF5_LIBRARIES}
	${PETSC_LIBRARIES}
)


# OFPP only for Linux
if (UNIX)
	target_link_libraries(fastctc OneFilePerProcess)
	bm_include_libraries( OneFilePerProcess )
endif (UNIX)

# Add FLEXLM if enabled
if (BM_USE_FLEXLM)
   include_directories( ${FLEXLM_INCLUDE_DIRS})
   target_link_libraries(fastctc ${FLEXLM_LIBRARIES})
endif(BM_USE_FLEXLM)

set_target_properties(fastctc
      PROPERTIES LINK_FLAGS "${PETSC_LINK_FLAGS}"
)

install(TARGETS fastctc
	RUNTIME DESTINATION bin
)

generate_dox(fastctc.cfg)

endif(BM_PARALLEL)
# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:

