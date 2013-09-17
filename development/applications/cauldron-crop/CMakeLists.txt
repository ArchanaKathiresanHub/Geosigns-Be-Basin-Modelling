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

add_executable(cauldron-crop
	src/crop.C
)

bm_include_libraries(
	TableIO
        DataAccess
        SerialDataAccess
        utilities
)

include_directories(
        ${HDF5_serial_INCLUDE_DIRS}
)

target_link_libraries(cauldron-crop
      projectdependencies
      TableIO
      DataAccess
      SerialDataAccess
      utilities
      ${HDF5_serial_LIBRARIES}
      ${HDF5_serial_HOME}/lib/libhdf5_hl.a
)

install(TARGETS cauldron-crop
        RUNTIME DESTINATION bin
)
