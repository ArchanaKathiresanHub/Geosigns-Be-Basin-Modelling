#########################################################################
#                                                                       #
# Copyright (C) 2012-2014 Shell International Exploration & Production. #
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

add_executable( datadriller
	${all_srcs}
	${all_headers}
) 

bm_include_libraries(
	DataModel
	DataMining
	DataAccess
	SerialDataAccess
	CBMGenerics
	GeoPhysics
	CrustalThinkness
	Serial_Hdf5
	EosPack
	utilities
	TableIO
	LinearAlgebra
	Interpolation
	FiniteElements
)

include_directories(
	${HDF5_INCLUDE_DIRS}
)

target_link_libraries(datadriller
	DataMining
	DataAccess
	SerialDataAccess
	CBMGenerics
	GeoPhysics
	Serial_Hdf5
	EosPack
	utilities
	TableIO
	LinearAlgebra
	Interpolation
	FiniteElements
	${HDF5_LIBRARIES}
)

set_target_properties( datadriller
      PROPERTIES LINK_FLAGS "${PETSC_LINK_FLAGS}"
)

install(TARGETS datadriller
	RUNTIME DESTINATION bin
)
