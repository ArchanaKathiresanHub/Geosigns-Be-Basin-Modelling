#########################################################################
#                                                                       #
# Copyright (C) 2012-2019 Shell International Exploration & Production. #
# All rights reserved.                                                  #
#                                                                       #
# Confidential and proprietary source code of Shell.                    #
# Do not distribute without written permission from Shell.              #
#                                                                       #
#########################################################################

set( LIB_NAME "MapSmoothing" )

include_directories( SYSTEM
      ${HDF5_INCLUDE_DIRS}
)

create_bm_library( TARGET ${LIB_NAME}
                   LIBRARIES
    DataAccess
    SerialDataAccess
    Serial_Hdf5
    ${HDF5_LIBRARIES}
)

################################################################
######################## UNIT TESTS ############################
################################################################

add_gtest( NAME "MapSmoothingTests"
           SOURCES test/mapSmoothingTest.cpp
           LIBRARIES ${LIB_NAME} DataAccess SerialDataAccess
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
         )

