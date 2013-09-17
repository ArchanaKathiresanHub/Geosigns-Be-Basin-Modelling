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

FILE( GLOB all_headers src/*.h src/Interface/*.h )
source_group(include FILES ${all_headers})

FILE( GLOB all_srcs src/*.C)
source_group(source FILES ${all_srcs})

add_library(DataAccess STATIC
	${all_srcs}
	${all_headers}
)

bm_include_libraries(
	Serial_Hdf5
	CBMGenerics
	utilities 
	TableIO
	EosPack
	CrustalThickness
)

include_directories( 
	src
	${HDF5_serial_INCLUDE_DIRS}
)

target_link_libraries(DataAccess 
	Serial_Hdf5
	CBMGenerics 
	utilities
	TableIO
)

generate_dox( src/DataAccess.cfg )

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
