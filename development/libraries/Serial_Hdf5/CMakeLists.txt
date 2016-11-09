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

add_library(Serial_Hdf5 
	${all_srcs}
	${all_headers}
)

include_directories( SYSTEM ${HDF5_INCLUDE_DIRS} )

target_link_libraries(Serial_Hdf5 ${HDF5_LIBRARIES})

	
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
