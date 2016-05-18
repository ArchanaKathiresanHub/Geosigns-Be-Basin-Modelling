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

FILE( GLOB all_headers src/*.h src/pugixml-1.7/*.hpp src/lz4/lz4.h)
source_group("Header Files" FILES ${all_headers})

FILE( GLOB all_srcs src/*.cpp src/pugixml-1.7/*.cpp src/lz4/lz4.c)
source_group("Source Files" FILES ${all_srcs})

add_library(VisualizationIO ${all_srcs} ${all_headers})

include_directories(
	${HDF5_INCLUDE_DIRS}
)

bm_include_libraries(
  FileSystem
  utilities
)

target_link_libraries(VisualizationIO
  FileSystem
  ${HDF5_LIBRARIES}
  ${Boost_LIBRARIES}
)

generate_dox( VisualizationIO.cfg )

add_gtest( NAME CauldronIO::Project
           SOURCES test/Project.cpp test/DataStore.cpp
           LIBRARIES VisualizationIO
)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
