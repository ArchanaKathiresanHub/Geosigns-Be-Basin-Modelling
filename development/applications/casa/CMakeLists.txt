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

FILE( GLOB all_srcs src/*.cpp)
source_group(source FILES ${all_srcs})

configure_file(../../libraries/casaAPI/src/CauldronEnvConfig.h.in CauldronEnvConfig.h)

add_executable( casa
  ${all_srcs}
  ${all_headers}
)

if (UNIX)
add_dependencies(casa Boost)
endif (UNIX)

bm_include_libraries(
   TableIO
   casaAPI
   cmbAPI
   CBMGenerics
   FileSystem
   DataAccess
   utilities
)

include_directories( utilities
   ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( SYSTEM
   ${SUMPP_INCLUDE_DIR}
   ${NNLIB_INCLUDE_DIR}
)

target_link_libraries( casa
   casaapi
   cmbapi
   CBMGenerics
   DataAccess
   SerialDataAccess
   Serial_Hdf5
   TableIO
   utilities
   SUMlib
   NNlib
   ${HDF5_LIBRARIES}
   ${Boost_LIBRARIES}
   ${LSF_LIBS}
)

install( TARGETS casa
   DESTINATION bin
)
