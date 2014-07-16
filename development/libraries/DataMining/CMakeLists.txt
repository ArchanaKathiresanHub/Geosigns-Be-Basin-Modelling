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
FILE( GLOB all_headers src/*.h )
source_group(include FILES ${all_headers})

FILE( GLOB all_srcs src/*.C)
source_group(source FILES ${all_srcs})

add_library(DataMining
	${all_srcs}
	${all_headers}
)

bm_include_libraries( 
	FiniteElements 
	utilities 
	Interpolation 
	LinearAlgebra 
	TableIO 
	EosPack 
	DataAccess 
	GeoPhysics 
	CBMGenerics
)

include_directories(
	src
	${HDF5_INCLUDE_DIRS}
)

target_link_libraries(DataMining
	FiniteElements
	utilities
	Interpolation
	LinearAlgebra
	TableIO
	EosPack
	DataAccess
	GeoPhysics
	CBMGenerics
	${HDF5_LIBRARIES}
)

install(TARGETS DataMining
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
