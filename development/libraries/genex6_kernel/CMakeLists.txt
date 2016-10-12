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

add_library(genex6_kernel 
   ${all_srcs}
   ${all_headers}
)

bm_include_libraries(
  DataModel
	genex6 
	CBMGenerics 
	EosPack 
	DerivedProperties
	GeoPhysics
	Interpolation
	LinearAlgebra
	FileSystem
	utilities 
	Serial_Hdf5 
	OTGC_kernel6
	TableIO 
	DataAccess 
	DataAccessInterface
)

include_directories(
	..
	src
)

include_directories( SYSTEM
	${HDF5_INCLUDE_DIRS}
)

target_link_libraries(genex6_kernel
	genex6
	CBMGenerics
	EosPack
	DerivedProperties
	FileSystem
	utilities
	Serial_Hdf5
	OTGC_kernel6
	TableIO
	DataAccess
)

install(TARGETS genex6_kernel
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

