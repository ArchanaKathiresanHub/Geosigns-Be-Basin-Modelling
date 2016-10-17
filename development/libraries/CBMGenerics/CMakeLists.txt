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

#
# ATTENTION: The file is used in Basin Modeling and GEOCASE projects
#

FILE( GLOB all_headers src/*.h )
source_group(include FILES ${all_headers})

FILE( GLOB all_srcs src/*.cpp )
source_group(source FILES ${all_srcs})

add_library(CBMGenerics
   ${all_srcs}
   ${all_headers}
)

add_dependencies( CBMGenerics
   utilities
)

bm_include_libraries( CBMGenerics
   GeoPhysics utilities 
)

target_link_libraries( CBMGenerics
   utilities
)

#Doc. generation is commented because most of the classes in CBMGeneric have no doxygen documentation and generate warnings
#generate_dox( src/CBMGenerics.cfg )

install(TARGETS CBMGenerics
   RUNTIME DESTINATION bin
   LIBRARY	DESTINATION lib
   ARCHIVE DESTINATION lib
)

### Unit Tests

add_gtest( NAME CBMGenerics::Polyfunction 
           SOURCES test/Polyfunction.cpp
           LIBRARIES CBMGenerics )
