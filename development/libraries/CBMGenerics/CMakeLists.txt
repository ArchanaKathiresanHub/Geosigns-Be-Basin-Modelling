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

#
# ATTENTION: The file is used in Basin Modeling and GEOCASE projects
#
set( LIB_NAME "CBMGenerics" )
create_bm_library( TARGET ${LIB_NAME} LIBRARIES utilities INSTALLTARGET )

add_gtest( NAME "${LIB_NAME}::Polyfunction"
           SOURCES test/Polyfunction.cpp
           LIBRARIES ${LIB_NAME}
           FOLDER "${BASE_FOLDER}/${LIB_NAME}" )

add_gtest( NAME "${LIB_NAME}::capillarySealStrength"
           SOURCES test/capillarySealStrength.cpp
           LIBRARIES ${LIB_NAME}
           FOLDER "${BASE_FOLDER}/${LIB_NAME}" )
