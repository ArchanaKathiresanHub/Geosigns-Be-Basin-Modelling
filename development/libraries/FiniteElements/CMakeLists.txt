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

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx")
endif()

FILE( GLOB all_headers src/*.h )
source_group(include FILES ${all_headers})

FILE( GLOB all_srcs src/*.C)
source_group(source FILES ${all_srcs})

add_library(FiniteElements
	${all_srcs}
	${all_headers}
)

include_directories ( ${EIGEN_INCLUDE_DIRS} )

bm_include_libraries(LinearAlgebra DataModel utilities FiniteElements)

target_link_libraries(FiniteElements
	LinearAlgebra
	utilities
)

install(TARGETS FiniteElements
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
   LIBRARIES FiniteElements
 )

add_gtest( NAME FEM::ThreeVector
   SOURCES test/ThreeVectorTest.C
   LIBRARIES FiniteElements
)

add_gtest( NAME FEM::Matrix3x3
   SOURCES test/Matrix3x3Test.C
   LIBRARIES FiniteElements
)

add_gtest( NAME FEM::ElementVector
   SOURCES test/ElementVectorTest.C
   LIBRARIES FiniteElements
)

add_gtest ( NAME FEM::PropertiesToQuadPts
   SOURCES test/PropertiesToQuadPtsTest.C
   LIBRARIES FiniteElements LinearAlgebra
 )

add_gtest ( NAME FEM::BasisFunctionInterpolator
   SOURCES test/BasisFunctionInterpolatorTest.C
   LIBRARIES FiniteElements
 )
