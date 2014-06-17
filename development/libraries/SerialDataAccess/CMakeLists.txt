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

add_library( SerialDataAccess 
   src/SerialApplicationGlobalOperations.C
   src/SerialGrid.C
   src/SerialGridMap.C
   src/SerialMapWriter.C
   src/SerialMessageHandler.C
   src/SerialObjectFactory.C
   src/SerialProjectHandle.C
   src/Serialhdf5funcs.C
)

bm_include_libraries(
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
