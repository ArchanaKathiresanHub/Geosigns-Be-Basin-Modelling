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

add_executable(eosptdiag
   ${all_srcs}
   ${all_headers}
)

bm_include_libraries(
   DataModel
   DataAccess
   SerialDataAccess
   TableIO
   CBMGenerics
   EosPack
   utilities
)

include_directories( SYSTEM
   ${HDF5_INCLUDE_DIRS}
)

target_link_libraries(eosptdiag
   DataAccess
   SerialDataAccess
   TableIO
   CBMGenerics
   EosPack
   utilities
   ${HDF5_LIBRARIES}
)

install(TARGETS eosptdiag
   RUNTIME DESTINATION bin
)
