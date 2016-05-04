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

FILE( GLOB all_headers src/*.h src/*.hpp)
source_group("Header Files" FILES ${all_headers})

FILE( GLOB all_srcs src/*.cpp)
source_group("Source Files" FILES ${all_srcs})

add_library(VisualizationIO_projectHandle ${all_srcs} ${all_headers})

include_directories(
	${HDF5_INCLUDE_DIRS}
)

bm_include_libraries(
	DataModel
	DataAccess
    utilities
	SerialDataAccess
    FileSystem 
	DataAccess
	TableIO
    VisualizationIO
)

target_link_libraries(VisualizationIO_projectHandle
	DataModel
	DataAccess
    utilities
	SerialDataAccess
    FileSystem 
	DataAccess
	TableIO
    VisualizationIO
	${HDF5_LIBRARIES}
	${Boost_LIBRARIES}
)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
