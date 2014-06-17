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

if (NOT BM_PARALLEL)
	return()
endif()

add_library(CrustalThickness
	src/LinearFunction.C
	src/DensityCalculator.C
	src/InterfaceInput.C
	src/InterfaceOutput.C
)

bm_include_libraries(
	DataAccess
	TableIO
	Serial_Hdf5
)

remove_definitions(-fPIC)

include_directories(
	${HDF5_INCLUDE_DIRS}
)

target_link_libraries(CrustalThickness
	DataAccess
	TableIO
	Serial_Hdf5
	${HDF5_LIBRARIES}
)

install(TARGETS CrustalThickness
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
