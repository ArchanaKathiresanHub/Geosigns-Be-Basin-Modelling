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

add_library(genex6_kernel
	src/AdsorptionFunction.C
	src/AdsorptionFunctionFactory.C
	src/AdsorptionSimulator.C
	src/AdsorptionSimulatorFactory.C
	src/C1AdsorptionSimulator.C
	src/C1NodeAdsorptionHistory.C
	src/IrreducibleWaterSaturationFunction.C
	src/LangmuirAdsorptionFunction.C
	src/LangmuirAdsorptionIsothermInterval.C
	src/LangmuirAdsorptionTOCFunction.C
	src/LinearGridInterpolator.C
	src/ConstantGridInterpolator.C
	src/OTGCC1AdsorptionSimulator.C
	src/OTGCNodeAdsorptionHistory.C
	src/PVTCalculator.C
	src/SpeciesResult.C
	src/SourceRock.C
	src/SourceRockAdsorptionHistory.C
	src/SourceRockNode.C
	src/SimulatorState.C
	src/SnapshotInterval.C
	src/UnitTestDataCreator.C
	src/GenexHistory.C
)

bm_include_libraries(
	genex6 
	CBMGenerics 
	EosPack 
	utilities 
	Serial_Hdf5 
	OTGC_kernel6
	TableIO 
	DataAccess 
)

include_directories(
	..
	src
	${HDF5_serial_INCLUDE_DIRS}
)

target_link_libraries(genex6_kernel
	genex6
	CBMGenerics
	EosPack
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

