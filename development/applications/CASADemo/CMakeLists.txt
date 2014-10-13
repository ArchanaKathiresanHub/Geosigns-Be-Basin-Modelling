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

add_executable( CASADemo
	${all_srcs}
	${all_headers}
) 

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
