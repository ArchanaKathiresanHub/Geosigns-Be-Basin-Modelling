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

set( LIB_NAME "SerialDataAccess" )

include_directories( SYSTEM
   ${HDF5_INCLUDE_DIRS}
)

create_bm_library( TARGET ${LIB_NAME}
LIBRARIES
   ${HDF5_LIBRARIES}
   DataAccess
   Serial_Hdf5
   TableIO
   CBMGenerics
   FileSystem
   utilities
INSTALLTARGET
)

generate_dox( src/SerialDataAccess.cfg )

if (UNIX)
   target_link_libraries(${LIB_NAME} rt)
endif(UNIX)

add_gtest( NAME SerialGridMap
           SOURCES test/SerialGridMap.cpp
           LIBRARIES ${LIB_NAME} DataAccess
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME SerialMapWriter
           SOURCES test/SerialMapWriter.cpp
           LIBRARIES ${LIB_NAME} DataAccess Serial_Hdf5 FileSystem
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)
