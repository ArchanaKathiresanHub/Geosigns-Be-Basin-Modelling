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

add_executable(gempis_recv
	src/gempis_recv.cpp
)

target_link_libraries(gempis_recv
	pthread
	rt
)

install(TARGETS gempis_recv
	RUNTIME DESTINATION bin
)
