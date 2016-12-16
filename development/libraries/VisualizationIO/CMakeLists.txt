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

add_library(VisualizationIO ${all_srcs} ${all_headers})

include_directories( SYSTEM
  ${HDF5_INCLUDE_DIRS}
  ${PUGIXML_INCLUDE_DIR}
  ${LZ_INCLUDE_DIR}
)

bm_include_libraries(
  utilities
  FileSystem
)

target_link_libraries(VisualizationIO
  utilities
  FileSystem
  PugiXMLlib
  LZlib
  ${HDF5_LIBRARIES}
  ${Boost_LIBRARIES}
)

generate_dox( VisualizationIO.cfg )

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
