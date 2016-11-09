#########################################################################
#                                                                       #
# Copyright (C) 2012-2016 Shell International Exploration & Production. #
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

add_definitions(-DNNLIB_EXPORT)

include_directories(SYSTEM "./include")

add_library( NNlib
   ${all_srcs}
   ${all_headers}
)

if (UNIX)
add_dependencies(NNlib Boost)
endif (UNIX)

install(TARGETS NNlib
   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)