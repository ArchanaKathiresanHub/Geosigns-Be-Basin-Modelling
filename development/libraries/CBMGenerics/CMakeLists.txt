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

set( LIB_NAME "CBMGenerics" )
add_library(${LIB_NAME}
   ${all_srcs}
   ${all_headers}
)
set_target_properties( ${LIB_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${LIB_NAME}" )

add_dependencies( ${LIB_NAME}
   utilities
)

bm_include_libraries( ${LIB_NAME}
   utilities 
)

target_link_libraries( ${LIB_NAME}
   utilities
)

#Doc. generation is commented because most of the classes in CBMGeneric have no doxygen documentation and generate warnings
#generate_dox( src/CBMGenerics.cfg )

install(TARGETS ${LIB_NAME}
   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)

### Unit Tests

add_gtest( NAME ${LIB_NAME}::Polyfunction 
           SOURCES test/Polyfunction.cpp
           LIBRARIES ${LIB_NAME}
           FOLDER "${BASE_FOLDER}/${LIB_NAME}" )

add_gtest( NAME ${LIB_NAME}::capillarySealStrength 
           SOURCES test/capillarySealStrength.cpp
           LIBRARIES ${LIB_NAME}
           FOLDER "${BASE_FOLDER}/${LIB_NAME}" )
