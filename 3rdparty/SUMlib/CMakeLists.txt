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

FILE( GLOB all_headers include/*.h )
source_group(include FILES ${all_headers})

FILE( GLOB all_srcs src/*.cpp)
source_group(source FILES ${all_srcs})

add_definitions(-DSUMLIB_EXPORT)

include_directories(
      ${SUMPP_INCLUDE_DIR}
      ${Boost_INCLUDE_DIRS}
)

add_library( SUMlib
   ${all_srcs}
   ${all_headers}
)

if (UNIX)
   add_dependencies(SUMlib Boost)
endif (UNIX)

install(TARGETS SUMlib
   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
