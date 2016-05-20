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
FILE( GLOB all_headers src/*.h)
source_group("Header Files" FILES ${all_headers})

FILE( GLOB all_srcs src/*.cpp)
source_group("Source Files" FILES ${all_srcs})

add_library(Visualization4D ${all_srcs})

set(OIVHOME $ENV{OIVHOME})
set(OIVARCH $ENV{OIVARCH})

if(DEFINED ENV{OIV_INCLUDE_DIR})
  set(OIV_INCLUDE_DIR $ENV{OIV_INCLUDE_DIR})
else()
  set(OIV_INCLUDE_DIR ${OIVHOME}/include)
endif()

include_directories(
	${OIV_INCLUDE_DIR}
	${HDF5_INCLUDE_DIRS})

bm_include_libraries(
	utilities
	FileSystem
	DataAccess
	DataModel
	TableIO
	VisualizationIO)

target_link_libraries(Visualization4D
	FileSystem
	DataAccess
	VisualizationIO)

