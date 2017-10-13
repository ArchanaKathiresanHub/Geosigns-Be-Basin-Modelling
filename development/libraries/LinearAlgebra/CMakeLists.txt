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

FILE( GLOB all_headers src/*.h )
source_group(include FILES ${all_headers})

FILE( GLOB all_srcs src/*.C)
source_group(source FILES ${all_srcs})

set( LIB_NAME "LinearAlgebra" )
add_library(${LIB_NAME}
   ${all_srcs}
   ${all_headers}
)
set_target_properties( ${LIB_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${LIB_NAME}" )

bm_include_libraries(
   utilities
)

target_link_libraries(${LIB_NAME}
   utilities
        ${BLAS_LIBRARIES}
)

install(TARGETS ${LIB_NAME}
   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)

add_gtest ( NAME SimdTraits::TraitsTest
            SOURCES test/TraitsTest.C
            LIBRARIES ${LIB_NAME}
            FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest ( NAME SimdTraits::InstTest01
            SOURCES test/SimdInstTest01.C
            LIBRARIES ${LIB_NAME}
            FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest ( NAME SimdTraits::InstTest02
            SOURCES test/SimdInstTest02.C
            LIBRARIES ${LIB_NAME}
            FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest ( NAME MatrixTests::ConstructionTest
            SOURCES test/MatrixTest01.C
            LIBRARIES ${LIB_NAME}
            FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest ( NAME MatrixTests::MultTest
            SOURCES test/MatrixMultTest01.C
            LIBRARIES ${LIB_NAME}
            FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest ( NAME MatrixTests::SimpleMultTest
            SOURCES test/MatrixMultTest03.C
            LIBRARIES ${LIB_NAME}
            FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest ( NAME MatrixTests::SimpleMultAddTest
            SOURCES test/MatrixAddMultTest01.C
            LIBRARIES ${LIB_NAME}
            FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest ( NAME MatrixTests::GeneralMultTest
            SOURCES test/MatrixMultTest04.C
            LIBRARIES ${LIB_NAME}
            FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)
