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


add_library(genex6
	src/Utilities.C
	src/SpeciesManager.C
	src/SpeciesProperties.C
	src/Species.C
	src/SpeciesState.C
	src/GeneralParametersHandler.C
	src/ImmobileSpecies.C
	src/Input.C
	src/SimulatorStateBase.C
	src/Simulator.C
	src/SCmathlib.C
	src/ReactionRatio.C
	src/Reaction.C
	src/ChemicalModel.C
)

bm_include_libraries(
	CBMGenerics 
	EosPack 
	utilities
)

target_link_libraries(genex6
	CBMGenerics
	utilities
	EosPack
)

install(TARGETS genex6
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

