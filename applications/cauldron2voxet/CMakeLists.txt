add_executable(cauldron2voxet
	src/cauldron2voxet.C
	src/CauldronProperty.C
	src/DepthInterpolator.C
	src/DerivedProperty.C
	src/DerivedPropertyFormationFunction.C
	src/DerivedPropertyFunction.C
	src/GridDescription.C
	src/LayerInterpolator.C
	src/VoxetCalculator.C
	src/VoxetDomainInterpolator.C
	src/VoxetProjectHandle.C
	src/VoxetPropertyGrid.C
	${PROJECT_BINARY_DIR}/applications/cauldron2voxet/voxetschemafuncs.C
	${PROJECT_BINARY_DIR}/applications/cauldron2voxet/voxetschema.C
)

bm_include_libraries(
	SerialDataAccess
	DataAccess
	utilities
	TableIO
	CBMGenerics
)

include_directories(
	${PROJECT_SOURCE_DIR}/libraries/SerialDataAccess/src/Interface
	${PROJECT_SOURCE_DIR}/libraries/DataAccess/src/Interface
	${FP_INCLUDE_DIRS}
	${HDF5_INCLUDE_DIRS}
	${PROJECT_BINARY_DIR}/applications/cauldron2voxet
)

target_link_libraries(cauldron2voxet
	DataAccess
	SerialDataAccess
	TableIO
	${FP_LIBRARIES}
	Serial_Hdf5
	CBMGenerics
	utilities
	${HDF5_LIBRARIES}
)

get_target_property(PARSPEC_EXEC parspec LOCATION)
add_custom_command(OUTPUT 
		${PROJECT_BINARY_DIR}/applications/cauldron2voxet/voxetschemafuncs.h
		${PROJECT_BINARY_DIR}/applications/cauldron2voxet/voxetschemafuncs.C
		${PROJECT_BINARY_DIR}/applications/cauldron2voxet/voxetschema.h
		${PROJECT_BINARY_DIR}/applications/cauldron2voxet/voxetschema.C

	DEPENDS parspec
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/applications/cauldron2voxet
	COMMAND ${PARSPEC_EXEC}
	ARGS -schema VoxetSchema -directory ${PROJECT_BINARY_DIR}/applications/cauldron2voxet -input src/VoxetProperty.Spec
)

install(TARGETS cauldron2voxet
	RUNTIME DESTINATION bin
)
