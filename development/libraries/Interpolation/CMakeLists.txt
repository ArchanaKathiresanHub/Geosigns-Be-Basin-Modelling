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

set( LIB_NAME "Interpolation" )
create_bm_library( TARGET ${LIB_NAME}
                   LIBRARIES LinearAlgebra
                             utilities
                   INSTALLTARGET )

add_gtest( NAME ${LIB_NAME}::Point
           SOURCES test/Point.cpp
           LIBRARIES ${LIB_NAME}
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::GeometryVector
           SOURCES test/GeometryVector.cpp
           LIBRARIES ${LIB_NAME}
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::GenericTwoDArray
           SOURCES test/GenericTwoDArray.cpp
           LIBRARIES ${LIB_NAME}
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::RBF
           SOURCES test/RBF.cpp
           LIBRARIES ${LIB_NAME}
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::RBFInterpolant
           SOURCES test/RBFInterpolant.cpp
           LIBRARIES ${LIB_NAME}
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::RBFGeneralOperations
           SOURCES test/RBFGeneralOperations.cpp
           LIBRARIES ${LIB_NAME}
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)
