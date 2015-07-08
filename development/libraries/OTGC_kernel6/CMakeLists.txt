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

FILE( GLOB all_headers src/*.h )
source_group(include FILES ${all_headers})

FILE( GLOB all_srcs src/*.C)
source_group(source FILES ${all_srcs})

add_library(OTGC_kernel6 
   ${all_srcs}
   ${all_headers}
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

