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

add_library(OTGC_kernel6
	src/SimulatorState.C
)

bm_include_libraries(
	genex6
	utilities
	CBMGenerics
	EosPack
)

target_link_libraries(OTGC_kernel6
	utilities
	EosPack
	CBMGenerics
	genex6
)

add_definitions(-DUSEOTGC)

install(TARGETS OTGC_kernel6
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

