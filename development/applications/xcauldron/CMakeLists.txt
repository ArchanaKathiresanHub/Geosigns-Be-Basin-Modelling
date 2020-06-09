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

add_library(projectdependencies
      src/projectdependencies.cpp
)

add_executable(xcauldron
	src/xcauldron.cpp
)


target_link_libraries(projectdependencies
        TableIO
  	FileSystem
	utilities
)

target_link_libraries(xcauldron
        projectdependencies
	TableIO
)

install(TARGETS xcauldron
	RUNTIME DESTINATION bin
)
