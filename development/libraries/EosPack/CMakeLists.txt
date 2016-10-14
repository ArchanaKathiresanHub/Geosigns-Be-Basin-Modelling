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

add_library(EosPack 
   ${all_srcs}
   ${all_headers}
)

include_directories(utilities)

bm_include_libraries(CBMGenerics utilities)

target_link_libraries(EosPack CBMGenerics utilities) 

generate_dox( src/EosPack.cfg )


#######################################
### Unit Tests
#######################################

#######################################
# Unit tests for PTDiagramCalculator

add_gtest( NAME PTDiagramCalculator
   SOURCES test/PTDiagramBuilderTest.C test/PVTCfgFileMgr.C
   LIBRARIES EosPack
)

######################################
# Unit test for PTDiagramBuilderTest in multithreaded environment. 
# Windows run sometimes crashed in Debug version for no obvious reason
if(UNIX)
add_gtest( NAME PTDiagramCalculator_MT
   SOURCES test/PTDiagramBuilderMTTest.C test/PVTCfgFileMgr.C
   LIBRARIES EosPack
   COMPILE_FLAGS "${OpenMP_CXX_FLAGS}"
   LINK_FLAGS "${OpenMP_CXX_FLAGS} ${OpenMP_LINK_FLAGS}"
)
endif(UNIX)

######################################
# Unit test for flasher
add_gtest( NAME pvtFlash::CAPI
   SOURCES test/EosPackTest.C test/PVTCfgFileMgr.C
   LIBRARIES EosPack
)

######################################
# Unit test for flasher in multithreaded environment
# Windows run sometimes crashed in Debug version for no obvious reason
if(UNIX)
add_gtest( NAME pvtFlash::CAPI_MT
   SOURCES test/EosPackMTTest.C test/PVTCfgFileMgr.C
   LIBRARIES EosPack
   COMPILE_FLAGS "${OpenMP_CXX_FLAGS}"
   LINK_FLAGS "${OpenMP_CXX_FLAGS} ${OpenMP_LINK_FLAGS}"
)
endif(UNIX)
