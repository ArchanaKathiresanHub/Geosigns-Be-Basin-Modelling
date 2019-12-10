#########################################################################
#                                                                       #
# Copyright (C) 2012-2019 Shell International Exploration & Production. #
# All rights reserved.                                                  #
#                                                                       #
# Confidential and proprietary source code of Shell.                    #
# Do not distribute without written permission from Shell.              #
#                                                                       #
#########################################################################

################################################################
######################### SET TARGETS ##########################
################################################################


set( TARGET_NAME "mapSmoother" )
add_executable( ${TARGET_NAME} src/main.cpp )

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${TARGET_NAME}" )

################################################################
################### SET DEPENDENCIES/LINKS #####################
################################################################

include_directories(
   ${HDF5_INCLUDE_DIRS}
)

target_link_libraries( ${TARGET_NAME}
   cmbapi
   TableIO
   DataModel
   DataAccess
   SerialDataAccess
   ${HDF5_LIBRARIES}
)

install( TARGETS ${TARGET_NAME}
         DESTINATION bin
)

################################################################
######################## UNIT TESTS ############################
################################################################

