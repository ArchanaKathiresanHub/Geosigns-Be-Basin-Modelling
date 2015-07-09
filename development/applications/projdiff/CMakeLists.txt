#########################################################################
#                                                                       #
# Copyright (C) 2012-2015 Shell International Exploration & Production. #
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

add_executable( projdiff
   ${all_srcs}
   ${all_headers}
)

bm_include_libraries(
   cmbAPI
   CBMGenerics
   FileSystem
   DataAccess
   utilities
)

include_directories(
   ${SUMPP_INCLUDE_DIR}
)

target_link_libraries( projdiff
   cmbapi
   CBMGenerics
   DataAccess
   SerialDataAccess
   Serial_Hdf5
   TableIO
   utilities
   ${HDF5_LIBRARIES}
   ${Boost_LIBRARIES}
)

install( TARGETS projdiff
         DESTINATION bin
)
