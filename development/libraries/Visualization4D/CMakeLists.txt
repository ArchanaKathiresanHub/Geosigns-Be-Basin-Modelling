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

FILE( GLOB all_srcs src/*.cpp)
source_group(source FILES ${all_srcs})

add_library(Visualization4D ${all_srcs})

include_directories(
	$ENV{OIVHOME}/include
	${HDF5_INCLUDE_DIRS})

bm_include_libraries(
	DataAccess
	DataModel
	TableIO
	VisualizationIO)

target_link_libraries(Visualization4D
	DataAccess
	VisualizationIO)

