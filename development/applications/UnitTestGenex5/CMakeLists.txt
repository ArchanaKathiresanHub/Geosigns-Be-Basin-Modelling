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

add_executable(UnitTestGenex5 src/main.C)

bm_include_libraries(
	genex5_kernel
	CBMGenerics
	EosPack
)
target_link_libraries(UnitTestGenex5
	genex5_kernel
	CBMGenerics
	EosPack
)
