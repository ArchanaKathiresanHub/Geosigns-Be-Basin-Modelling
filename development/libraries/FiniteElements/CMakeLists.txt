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

set( LIB_NAME "FiniteElements" )
include_directories ( SYSTEM ${EIGEN_INCLUDE_DIRS} )
create_bm_library( TARGET ${LIB_NAME} 
                   LIBRARIES LinearAlgebra 
                             utilities )

#######################################
### Unit Tests
#######################################

#######################################
# Unit tests for PTDiagramCalculator

add_gtest ( NAME FEM::BasisCache
   SOURCES test/BasisCacheTest.cpp
   LIBRARIES ${LIB_NAME}
   FOLDER "${BASE_FOLDER}/${LIB_NAME}"
 )

add_gtest( NAME FEM::ThreeVector
   SOURCES test/ThreeVectorTest.cpp
   LIBRARIES ${LIB_NAME}
   FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME FEM::MultiComponentVector
   SOURCES test/MultiComponentVectorTest.cpp
   LIBRARIES ${LIB_NAME}
   FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)


add_gtest( NAME FEM::ElementGeometryMatrix
   SOURCES test/ElementGeometryMatrixTest.cpp
   LIBRARIES ${LIB_NAME}
   FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME FEM::FreeFunctions
   SOURCES test/FreeFunctionsTest.cpp
   LIBRARIES ${LIB_NAME}
   FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME FEM::CopyTest
   SOURCES test/CopyTest.cpp
   LIBRARIES ${LIB_NAME}
   FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME FEM::Matrix3x3
   SOURCES test/Matrix3x3Test.cpp
   LIBRARIES ${LIB_NAME}
   FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME FEM::ElementVector
   SOURCES test/ElementVectorTest.cpp
   LIBRARIES ${LIB_NAME}
   FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest ( NAME FEM::PropertiesToQuadPts
   SOURCES test/PropertiesToQuadPtsTest.cpp
   LIBRARIES ${LIB_NAME} LinearAlgebra
   FOLDER "${BASE_FOLDER}/${LIB_NAME}"
 )

add_gtest ( NAME FEM::BasisFunctionInterpolator
   SOURCES test/BasisFunctionInterpolatorTest.cpp
   LIBRARIES ${LIB_NAME}
   FOLDER "${BASE_FOLDER}/${LIB_NAME}"
 )

add_gtest ( NAME JacobianStorageTest
   SOURCES test/JacobianStorageTest.cpp
   LIBRARIES ${LIB_NAME} LinearAlgebra
   FOLDER "${BASE_FOLDER}/${LIB_NAME}"
 )

add_gtest( NAME FEM::ElementMatrix
   SOURCES test/ElementMatrixTest.cpp
   LIBRARIES ${LIB_NAME}
   FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME FEM::GradElementVector
   SOURCES test/GradElementVectorTest.cpp
   LIBRARIES ${LIB_NAME}
   FOLDER "${BASE_FOLDER}/${LIB_NAME}"
 )

add_gtest( NAME FEM::BooleanVector
   SOURCES test/BooleanVectorTest.cpp
   LIBRARIES ${LIB_NAME}
   FOLDER "${BASE_FOLDER}/${LIB_NAME}"
 )

add_gtest( NAME FEM::ArrayOfVector3_Matrix3x3
   SOURCES test/ArrayOfVector3_Matrix3x3Test.cpp
   LIBRARIES ${LIB_NAME}
   FOLDER "${BASE_FOLDER}/${LIB_NAME}"
 )

add_gtest ( NAME FEM::BasisFunction
   SOURCES test/BasisFunctionTest.cpp
   LIBRARIES ${LIB_NAME}
   FOLDER "${BASE_FOLDER}/${LIB_NAME}"
 )
