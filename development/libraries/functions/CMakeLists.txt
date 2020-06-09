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


set( LIB_NAME "functions" )
create_bm_library( TARGET ${LIB_NAME} INSTALLTARGET )

add_gtest( NAME ${LIB_NAME}::Tuple2
           SOURCES test/Tuple2.cpp
           LIBRARIES ${LIB_NAME}
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME Functions::MonoIncrPwLinInvertFun
           SOURCES test/MonotonicIncreasingPiecewiseLinearInvertableFunction.cpp
           LIBRARIES ${LIB_NAME}
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)
