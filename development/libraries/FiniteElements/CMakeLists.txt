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

add_library(FiniteElements
	src/BasisFunction.C 
	src/BasisFunctionCache.C 
	src/FiniteElement.C
	src/FiniteElementTypes.C
)

include_directories (${EIGEN_INCLUDE_DIRS})

bm_include_libraries(utilities FiniteElements)

target_link_libraries(FiniteElements
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

add_gtest ( NAME  BasisCacheTest
   SOURCES test/BasisCacheTest.C
   LIBRARIES FiniteElements 
 )


add_gtest( NAME FiniteElementMethod::ThreeVector
   SOURCES test/ThreeVectorTest.C
   LIBRARIES FiniteElements
)

add_gtest( NAME FiniteElementMethod::Matrix3x3
   SOURCES test/Matrix3x3Test.C
   LIBRARIES FiniteElements
)

add_gtest( NAME FiniteElementMethod::ElementVector 
   SOURCES test/ElementVectorTest.C
   LIBRARIES FiniteElements
)

