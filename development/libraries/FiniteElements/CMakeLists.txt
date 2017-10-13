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

set( LIB_NAME "FiniteElements" )
add_library(${LIB_NAME}
   ${all_srcs}
   ${all_headers}
)
set_target_properties( ${LIB_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${LIB_NAME}" )

include_directories ( SYSTEM ${EIGEN_INCLUDE_DIRS} )

bm_include_libraries(LinearAlgebra DataModel utilities ${LIB_NAME})

target_link_libraries(${LIB_NAME}
   LinearAlgebra
   utilities
)

install(TARGETS ${LIB_NAME}
   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)

#######################################
### Unit Tests
#######################################

#######################################
# Unit tests for PTDiagramCalculator

add_gtest ( NAME FEM::BasisCache
   SOURCES test/BasisCacheTest.C
   LIBRARIES ${LIB_NAME}
   FOLDER "${BASE_FOLDER}/${LIB_NAME}"
 )

add_gtest( NAME FEM::ThreeVector
   SOURCES test/ThreeVectorTest.C
   LIBRARIES ${LIB_NAME}
   FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME FEM::Matrix3x3
   SOURCES test/Matrix3x3Test.C
   LIBRARIES ${LIB_NAME}
   FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME FEM::ElementVector
   SOURCES test/ElementVectorTest.C
   LIBRARIES ${LIB_NAME}
   FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest ( NAME FEM::PropertiesToQuadPts
   SOURCES test/PropertiesToQuadPtsTest.C
   LIBRARIES ${LIB_NAME} LinearAlgebra
   FOLDER "${BASE_FOLDER}/${LIB_NAME}"
 )

add_gtest ( NAME FEM::BasisFunctionInterpolator
   SOURCES test/BasisFunctionInterpolatorTest.C
   LIBRARIES ${LIB_NAME}
   FOLDER "${BASE_FOLDER}/${LIB_NAME}"
 )

add_gtest ( NAME JacobianStorageTest
   SOURCES test/JacobianStorageTest.C
   LIBRARIES ${LIB_NAME} LinearAlgebra
   FOLDER "${BASE_FOLDER}/${LIB_NAME}"
 )
