add_executable(cauldron2eclipse
	src/cauldron2eclipse.C
)

include_directories(
	${HDF5_INCLUDE_DIRS}
	${PROJECT_SOURCE_DIR}/libraries/SerialDataAccess/src/Interface
	${PROJECT_SOURCE_DIR}/libraries/DataAccess/src/Interface
)

bm_include_libraries(
	SerialDataAccess
	DataAccess
	TableIO
	CBMGenerics
)

target_link_libraries(cauldron2eclipse
	DataAccess
	SerialDataAccess
	TableIO
	${HDF5_LIBRARIES}
)

install(TARGETS cauldron2eclipse
	RUNTIME DESTINATION bin
)
