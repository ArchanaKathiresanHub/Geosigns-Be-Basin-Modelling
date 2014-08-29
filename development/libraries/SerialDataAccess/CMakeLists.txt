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

FILE( GLOB all_headers src/Interface/*.h )
source_group(include FILES ${all_headers})

FILE( GLOB all_srcs src/*.C)
source_group(source FILES ${all_srcs})

add_library(SerialDataAccess 
	${all_srcs}
	${all_headers}
)

bm_include_libraries(
   DataModel
   Serial_Hdf5
   utilities
   TableIO
   CBMGenerics
   DataAccess
)

include_directories(
   ${HDF5_INCLUDE_DIRS}
)

generate_dox( src/SerialDataAccess.cfg )

target_link_libraries(SerialDataAccess
   Serial_Hdf5
   CBMGenerics
   utilities
   TableIO
   DataAccess
)

if (UNIX)
   target_link_libraries(SerialDataAccess rt)
endif(UNIX)

install(TARGETS SerialDataAccess
   RUNTIME DESTINATION bin
   LIBRARY	DESTINATION lib
   ARCHIVE DESTINATION lib
)
