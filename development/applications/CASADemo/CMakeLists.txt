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

add_executable(CASADemo src/main.C src/CfgFileParser.C src/MatlabExporter.C)

message( "Bla bla bla: " ${SUMPP_INCLUDE_DIR} )

bm_include_libraries(
   casaAPI
   cmbAPI
   CBMGenerics
   ${SUMPP_INCLUDE_DIR}
   utilities
)

include_directories(
   ${SUMPP_INCLUDE_DIR}
)

target_link_libraries( CASADemo
   casaapi
   cmbapi
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
