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

add_executable(upcauldron src/upcauldron.C src/process.C)

bm_include_libraries(
	TableIO
	FileSystem
   utilities
)

target_link_libraries(upcauldron
	TableIO
	FileSystem
   utilities
)

install(TARGETS upcauldron
	RUNTIME DESTINATION bin
)

# Local Variables:

# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
