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

set( LIB_NAME "FileSystem" )
add_library(${LIB_NAME}
   ${all_srcs}
   ${all_headers}
)
set_target_properties( ${LIB_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${LIB_NAME}" )

bm_include_libraries(
   utilities
)

include_directories( src )

target_link_libraries(${LIB_NAME}
   utilities
   ${Boost_LIBRARIES}
)

generate_dox( src/FileSystem.cfg )

add_gtest( NAME ${LIB_NAME}::Path
           SOURCES test/Path.C
           LIBRARIES FileSystem utilities ${Boost_LIBRARIES}
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::FilePath
           SOURCES test/FilePath.C
           LIBRARIES ${LIB_NAME} utilities ${Boost_LIBRARIES}
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::FolderPath
           SOURCES test/FolderPath.C
           LIBRARIES ${LIB_NAME} utilities ${Boost_LIBRARIES}
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)
