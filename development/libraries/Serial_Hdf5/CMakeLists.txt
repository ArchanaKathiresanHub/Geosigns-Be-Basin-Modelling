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

set(SERIALHDF5_SOURCES
	src/buffer_readwrite.C
	src/h5_file_types.C
	src/h5_types.C
	src/readwriteobject.C
)

add_library(Serial_Hdf5 
   ${SERIALHDF5_SOURCES}
)


include_directories(${HDF5_serial_INCLUDE_DIRS})

target_link_libraries(Serial_Hdf5 ${HDF5_serial_LIBRARIES})

	
install(TARGETS Serial_Hdf5
	RUNTIME DESTINATION bin
	LIBRARY	DESTINATION lib
	ARCHIVE DESTINATION lib
)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
