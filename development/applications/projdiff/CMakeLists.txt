#########################################################################
#                                                                       #
# Copyright (C) 2012-2018 Shell International Exploration & Production. #
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

FILE( GLOB all_srcs src/*.cpp)
source_group(source FILES ${all_srcs})

set( TARGET_NAME "projdiff" )
add_executable( ${TARGET_NAME}
   ${all_srcs}
   ${all_headers}
)
set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${TARGET_NAME}" )

include_directories( SYSTEM
   ${SUMPP_INCLUDE_DIR}
   ${HDF5_INCLUDE_DIRS}
)

target_link_libraries( ${TARGET_NAME}
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

install( TARGETS ${TARGET_NAME}
         DESTINATION bin
)
