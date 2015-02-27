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

add_library(LinearAlgebra
	${all_srcs}
	${all_headers}
)

bm_include_libraries(
	utilities
)

target_link_libraries(LinearAlgebra
	utilities
        ${BLAS_LIBRARIES}
)

install(TARGETS LinearAlgebra
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

add_gtest ( NAME SimdTraits::TraitsTest
            SOURCES test/TraitsTest.C
            LIBRARIES LinearAlgebra
)

add_gtest ( NAME SimdTraits::InstTest01
            SOURCES test/SimdInstTest01.C
            LIBRARIES LinearAlgebra
)

add_gtest ( NAME SimdTraits::InstTest02
            SOURCES test/SimdInstTest02.C
            LIBRARIES LinearAlgebra
)

add_gtest ( NAME MatrixTests::ConstructionTest
            SOURCES test/MatrixTest01.C
            LIBRARIES LinearAlgebra
)
