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

add_library(genex5_kernel
	src/AdsorptionFunction.C
	src/AdsorptionSimulator.C
	src/ImmobileSpecies.C
	src/Utilities.C
	src/UnitTestDataCreator.C
	src/SpeciesState.C
	src/SpeciesResult.C
	src/SpeciesProperties.C
	src/Species.C
	src/SourceRockNodeOutput.C
	src/SourceRockNodeInput.C
	src/SourceRockNode.C
	src/SimulatorState.C
	src/GeneralParametersHandler.C
	src/Simulator.C
	src/SCmathlib.C
	src/PVTCalculator.C
	src/ReactionRatio.C
	src/Reaction.C
	src/ChemicalModel.C
	src/Element.C
	src/Constants.C
)

bm_include_libraries(
	CBMGenerics 
	EosPack 
	utilities
)

target_link_libraries(genex5_kernel
	CBMGenerics
	EosPack
	utilities
)

generate_dox( src/genex5_kernel.cfg )

install(TARGETS genex5_kernel
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

