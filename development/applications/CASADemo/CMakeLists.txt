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

add_executable(CASADemo src/main.C src/CfgFileParser.C)

bm_include_libraries(
	API
	CBMGenerics
	utilities
)

target_link_libraries( CASADemo
   api
   CBMGenerics
   DataAccess
   SerialDataAccess
   Serial_Hdf5
   TableIO
   utilities
   SUMlib
   ${HDF5_LIBRARIES}
   ${Boost_LIBRARIES}
   ${LSF_LIBS}
)

install( TARGETS CASADemo
   DESTINATION	bin
)
