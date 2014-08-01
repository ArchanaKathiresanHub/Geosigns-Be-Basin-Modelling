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



add_library( utilities
	src/IBSinterpolator2d.C
	src/IBSinterpolator.C
	src/mangle.C
	src/PiecewiseInterpolator.C
	src/Quadrature.C
	src/Quadrature3D.C
	src/WallTime.C
	src/TimeToComplete.C
	src/ProgressManager.C
    src/formattingexception.C
)

#generate_export_header( utilities )
#include_directories(${CMAKE_CURRENT_BINARY_DIR})

if (MSVC)
	add_definitions( "-D_USE_MATH_DEFINES" )
endif(MSVC)

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
