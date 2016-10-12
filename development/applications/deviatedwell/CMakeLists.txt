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

add_executable(deviatedwell
	src/DeviatedWellData.C
	src/deviatedwell.C
	src/GlobalDefs.C
	src/WellTrajectoryLocation.C
	src/CSVWellWriter.C
	src/MatlabWellWriter.C
	src/StdOutWellWriter.C
	src/StreamWellWriter.C
	src/WellWriter.C
	src/WellWriterFactory.C
)

bm_include_libraries(
	DataModel
	DataMining
	GeoPhysics
	DataAccess
	SerialDataAccess
	DerivedProperties
	EosPack
	utilities
	Serial_Hdf5
	TableIO
	LinearAlgebra
	Interpolation
	FiniteElements
	CBMGenerics
	calibration_kernel
)

include_directories( SYSTEM
	${HDF5_INCLUDE_DIRS}
)

target_link_libraries(deviatedwell
	DataMining
	GeoPhysics
	DerivedProperties
	DataAccess
	SerialDataAccess
	EosPack
	utilities
	Serial_Hdf5
	TableIO
	LinearAlgebra
	Interpolation
	FiniteElements
	CBMGenerics
	calibration_kernel
	${HDF5_LIBRARIES}
)

install(TARGETS deviatedwell
	RUNTIME DESTINATION bin
)
# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
