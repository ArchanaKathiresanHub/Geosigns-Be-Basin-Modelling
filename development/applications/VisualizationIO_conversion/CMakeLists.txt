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
source_group("Header Files" FILES ${all_headers})

FILE( GLOB all_srcs src/*.cpp)
source_group("Source Files" FILES ${all_srcs})

add_executable(VisualizationIO_convert ${all_srcs} ${all_headers})

include_directories(
	${HDF5_INCLUDE_DIRS}
)

bm_include_libraries(
	DataModel
	DataAccess
	SerialDataAccess
	DataAccess
	TableIO
    VisualizationIO
    VisualizationIO_projectHandle
)

target_link_libraries(
    VisualizationIO_convert
	DataModel
	DataAccess
	SerialDataAccess
	DataAccess
	TableIO
    VisualizationIO
    VisualizationIO_projectHandle
	${HDF5_LIBRARIES}
	${Boost_LIBRARIES}
)

install(TARGETS VisualizationIO_convert
	RUNTIME DESTINATION bin
)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
