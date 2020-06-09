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

set( LIB_NAME "LinearAlgebra" )
create_bm_library( TARGET ${LIB_NAME}
                   LIBRARIES utilities ${BLAS_LIBRARIES}
                   INSTALLTARGET)

add_gtest ( NAME SimdTraits::TraitsTest
            SOURCES test/TraitsTest.cpp
            LIBRARIES ${LIB_NAME}
            FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest ( NAME SimdTraits::InstTest01
            SOURCES test/SimdInstTest01.cpp
            LIBRARIES ${LIB_NAME}
            FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest ( NAME SimdTraits::InstTest02
            SOURCES test/SimdInstTest02.cpp
            LIBRARIES ${LIB_NAME}
            FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest ( NAME MatrixTests::ConstructionTest
            SOURCES test/MatrixTest01.cpp
            LIBRARIES ${LIB_NAME}
            FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest ( NAME MatrixTests::MultTest
            SOURCES test/MatrixMultTest01.cpp
            LIBRARIES ${LIB_NAME}
            FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest ( NAME MatrixTests::SimpleMultTest
            SOURCES test/MatrixMultTest03.cpp
            LIBRARIES ${LIB_NAME}
            FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest ( NAME MatrixTests::SimpleMultAddTest
            SOURCES test/MatrixAddMultTest01.cpp
            LIBRARIES ${LIB_NAME}
            FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest ( NAME MatrixTests::GeneralMultTest
            SOURCES test/MatrixMultTest04.cpp
            LIBRARIES ${LIB_NAME}
            FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)
