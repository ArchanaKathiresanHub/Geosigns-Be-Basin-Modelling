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

#
# ATTENTION: The file is used in Basin Modeling and GEOCASE projects
#

add_library(CBMGenerics
	src/ComponentManager.C
	src/GenexResultManager.C
	src/capillarySealStrength.C
	src/fracturePressure.C
	src/waterDensity.C
	src/methaneSolubility.C
	src/Polyfunction.C
)

#Doc. generation is commented because most of the classes in CBMGeneric have no doxygen documentation and generate warnings
#generate_dox( src/CBMGenerics.cfg )

install(TARGETS CBMGenerics
	RUNTIME DESTINATION bin
	LIBRARY	DESTINATION lib
	ARCHIVE DESTINATION lib
)

