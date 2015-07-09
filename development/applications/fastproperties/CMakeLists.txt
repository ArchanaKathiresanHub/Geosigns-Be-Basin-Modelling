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

#FILE( GLOB all_headers src/*.h )
#FILE( GLOB all_srcs src/*.C)
set (all_headers,
	src/PropertiesCalculator.h
	src/FormationMapOutputPropertyValue.h
	src/FormationOutputPropertyValue.h
	src/OutputPropertyValue.h)

set(all_srcs
	src/FormationMapOutputPropertyValue.C
	src/FormationOutputPropertyValue.C
	src/PropertiesCalculator.C
	src/main.C)

source_group(include FILES ${all_headers})

source_group(source FILES ${all_srcs})

add_executable(fastproperties
	${all_srcs}
	${all_headers}
)

bm_include_libraries(
	DataModel
	DataMining
	DataAccess
	DistributedDataAccess
	Interpolation
	LinearAlgebra
	GeoPhysics
	DerivedProperties
	DataAccess	
	TableIO
	CBMGenerics
	Serial_Hdf5
	Parallel_Hdf5
	EosPack
	utilities
	Utilities_Petsc
)

include_directories(
	${HDF5_INCLUDE_DIRS}
	${MPI_INCLUDE_DIRS}
	${PETSC_INCLUDE_DIRS}
)

target_link_libraries(fastproperties
	DataMining
	DataAccess
	DistributedDataAccess
	Interpolation
	LinearAlgebra
	GeoPhysics
	DerivedProperties
	DataAccess	
	TableIO
	CBMGenerics
	Serial_Hdf5
	Parallel_Hdf5
	EosPack
	utilities
	${HDF5_LIBRARIES}
	${PETSC_LIBRARIES}
)

install(TARGETS fastproperties
	RUNTIME DESTINATION bin
)
# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
