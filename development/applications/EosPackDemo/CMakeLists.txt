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

add_executable(EosPackDemo src/main.C)

bm_include_libraries(
	EosPack
	CBMGenerics
        utilities
)

target_link_libraries( EosPackDemo
	EosPack
	CBMGenerics
        utilities
)

install( TARGETS EosPackDemo
   DESTINATION	bin
)
