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
	src/errorhandling.h
	src/FormationMapOutputPropertyValue.h
	src/FormationOutputPropertyValue.h
	src/FormationSurfaceOutputPropertyValue.h
	src/OutputPropertyValue.h
	src/SurfaceOutputPropertyValue.h)

set(all_srcs
	src/FormationMapOutputPropertyValue.C
	src/FormationOutputPropertyValue.C
	src/FormationSurfaceOutputPropertyValue.C
	src/SurfaceOutputPropertyValue.C
	src/track1d.C)

source_group(include FILES ${all_headers})

source_group(source FILES ${all_srcs})

add_executable(track1d 
	${all_srcs}
	${all_headers}
)

bm_include_libraries(
	DataModel
	DataMining
	DataAccess
	SerialDataAccess
	Interpolation
	LinearAlgebra
	GeoPhysics
	DerivedProperties
	DataAccess	
	TableIO
	CBMGenerics
	Serial_Hdf5
	EosPack
	utilities
)

include_directories(
	${HDF5_INCLUDE_DIRS}
)

target_link_libraries(track1d
	DataMining
	DataAccess
	SerialDataAccess
	Interpolation
	LinearAlgebra
	GeoPhysics
	DerivedProperties
	DataAccess	
	TableIO
	CBMGenerics
	Serial_Hdf5
	EosPack
	utilities
	${HDF5_LIBRARIES}
)

install(TARGETS track1d
	RUNTIME DESTINATION bin
)
# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
