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

set( LIB_NAME "FileSystem" )
create_bm_library( TARGET "${LIB_NAME}"
                   LIBRARIES utilities ${Boost_LIBRARIES}
                 )

generate_dox( src/FileSystem.cfg )

add_gtest( NAME "${LIB_NAME}::Path"
           SOURCES test/Path.C
           LIBRARIES FileSystem utilities ${Boost_LIBRARIES}
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME "${LIB_NAME}::FilePath"
           SOURCES test/FilePath.C
           LIBRARIES ${LIB_NAME} utilities ${Boost_LIBRARIES}
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME "${LIB_NAME}::FolderPath"
           SOURCES test/FolderPath.C
           LIBRARIES ${LIB_NAME} utilities ${Boost_LIBRARIES}
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)
