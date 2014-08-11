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

add_executable(datadriller
	src/datadriller.C
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

install(TARGETS datadriller
	RUNTIME DESTINATION bin
)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
