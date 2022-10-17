#########################################################################
#                                                                       #
# Copyright (C) 2012-2014 Shell International Exploration & Production. #
# All rights reserved.                                                  #
#                                                                       #
# Developed under license for Shell by PDS BV.                          #
#                                                                       #
# Confidential and proprietary source code of Shell.                    #
# Do not distribute without written permission from Shell.              #
#                                                                       #
#########################################################################

## std::bind2nd used in this version was removed from c++17
set (CMAKE_CXX_STANDARD 14)
set (CMAKE_C_STANDARD 14)

FILE( GLOB all_headers include/*.h )
source_group(include FILES ${all_headers})

FILE( GLOB all_srcs src/*.cpp)
source_group(source FILES ${all_srcs})

add_definitions(-DSUMLIB_EXPORT)

include_directories( SYSTEM
      ${SUMPP_INCLUDE_DIR}
      ${Boost_INCLUDE_DIRS}
)

set( LIB_NAME "SUMlib" )
add_library( ${LIB_NAME}
   ${all_srcs}
   ${all_headers}
)
set_target_properties( ${LIB_NAME} PROPERTIES FOLDER "3rdparty/${LIB_NAME}" )

if (UNIX)
   add_dependencies(${LIB_NAME} Boost)
endif (UNIX)

install(TARGETS ${LIB_NAME}
   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
