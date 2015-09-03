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

add_library(utilities 
	${all_srcs}
	${all_headers}
)

# under Linux we are building Boost that is why this target exist
if (UNIX)
   add_dependencies(utilities Boost)
endif (UNIX)

if (MSVC)
	add_definitions( "-D_USE_MATH_DEFINES" )
endif(MSVC)


target_link_libraries(utilities
   ${Boost_LIBRARIES}
)

install(TARGETS utilities
	RUNTIME DESTINATION bin
	LIBRARY	DESTINATION lib
	ARCHIVE DESTINATION lib
)

### Unit Tests

add_gtest( NAME utilities::IBSinterpolator 
			SOURCES test/IBSinterpolator.C
			LIBRARIES utilities)
 			
add_gtest( NAME utilities::IBSinterpolator2d 
			SOURCES test/IBSinterpolator2d.C
			LIBRARIES utilities)
