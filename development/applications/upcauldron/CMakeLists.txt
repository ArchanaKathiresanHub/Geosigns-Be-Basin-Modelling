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

add_executable(upcauldron src/upcauldron.cpp src/process.cpp)

target_link_libraries(upcauldron
   TableIO
   FileSystem
   utilities
)

install(TARGETS upcauldron
	RUNTIME DESTINATION bin
)
