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


set (voxetProjectFormatSources
	${PROJECT_BINARY_DIR}/applications/cauldron2voxet/voxetschemafuncs.h
	${PROJECT_BINARY_DIR}/applications/cauldron2voxet/voxetschemafuncs.C
	${PROJECT_BINARY_DIR}/applications/cauldron2voxet/voxetschema.h
	${PROJECT_BINARY_DIR}/applications/cauldron2voxet/voxetschema.C
	)

set( mainSources
	src/cauldron2voxet.C
	src/CauldronProperty.C
	src/DepthInterpolator.C
	src/GridDescription.C
	src/LayerInterpolator.C
	src/VoxetCalculator.C
	src/VoxetDomainInterpolator.C
	src/VoxetProjectHandle.C
	src/VoxetPropertyGrid.C
    )

add_executable( cauldron2voxet
   ${mainSources}
   ${voxetProjectFormatSources}
)

bm_include_libraries(
	DataModel
	SerialDataAccess
	DataAccess
	utilities
	TableIO
	CBMGenerics
)

include_directories(
	${HDF5_INCLUDE_DIRS}
	${PROJECT_BINARY_DIR}/applications/cauldron2voxet
)

target_link_libraries(cauldron2voxet
	DataAccess
	SerialDataAccess
	TableIO
	Serial_Hdf5
	CBMGenerics
	utilities
	${HDF5_LIBRARIES}
)

get_target_property(PARSPEC_EXEC parspec LOCATION)
add_custom_command(OUTPUT ${voxetProjectFormatSources}
	DEPENDS parspec
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/applications/cauldron2voxet
	COMMAND ${PARSPEC_EXEC}
	ARGS -schema VoxetSchema -directory ${PROJECT_BINARY_DIR}/applications/cauldron2voxet -input src/VoxetProperty.Spec
)

install(TARGETS cauldron2voxet
	RUNTIME DESTINATION bin
)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
