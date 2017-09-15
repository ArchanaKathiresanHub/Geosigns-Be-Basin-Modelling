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

add_library(Interpolation
   ${all_srcs}
   ${all_headers}
)

bm_include_libraries(
   utilities
   LinearAlgebra
)

include_directories( src )

target_link_libraries(Interpolation
   utilities
   LinearAlgebra
)

install(TARGETS Interpolation
   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)

add_gtest( NAME Interpolation::Point
           SOURCES test/Point.C
           LIBRARIES Interpolation
)

add_gtest( NAME Interpolation::GeometryVector
           SOURCES test/GeometryVector.C
           LIBRARIES Interpolation
)

add_gtest( NAME Interpolation::GenericTwoDArray
           SOURCES test/GenericTwoDArray.C
           LIBRARIES Interpolation
)

add_gtest( NAME Interpolation::RBF
           SOURCES test/RBF.C
           LIBRARIES Interpolation
)

add_gtest( NAME Interpolation::RBFInterpolant
           SOURCES test/RBFInterpolant.C
           LIBRARIES Interpolation
)

add_gtest( NAME Interpolation::RBFGeneralOperations
           SOURCES test/RBFGeneralOperations.C
           LIBRARIES Interpolation
)
