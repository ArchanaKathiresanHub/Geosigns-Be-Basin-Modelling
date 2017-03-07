#########################################################################
#                                                                       #
# Copyright (C) 2012-2014 Shell International Exploration & Production. #
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

add_library(FileSystem
   ${all_srcs}
   ${all_headers}
)

bm_include_libraries(
   utilities
)

include_directories( src )

target_link_libraries(FileSystem
   utilities
   ${Boost_LIBRARIES}
)

generate_dox( src/FileSystem.cfg )

add_gtest( NAME FileSystem::Path
           SOURCES test/Path.C
           LIBRARIES FileSystem utilities ${Boost_LIBRARIES}
)

add_gtest( NAME FileSystem::FilePath
           SOURCES test/FilePath.C
           LIBRARIES FileSystem utilities ${Boost_LIBRARIES}
)

add_gtest( NAME FileSystem::FolderPath
           SOURCES test/FolderPath.C
           LIBRARIES FileSystem utilities ${Boost_LIBRARIES}
)
