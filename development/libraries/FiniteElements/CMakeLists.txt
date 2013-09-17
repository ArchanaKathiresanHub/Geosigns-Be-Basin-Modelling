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

add_library(FiniteElements
	src/BasisFunction.C 
	src/FiniteElement.C
	src/FiniteElementTypes.C
)

bm_include_libraries(utilities)

target_link_libraries(FiniteElements
	utilities
)

install(TARGETS FiniteElements
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)
