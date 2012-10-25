add_executable(cauldron2bhf
	src/cauldron2bhf.C
)

add_definitions(-DUSEINTERFACE)

include_directories(
	${HDF5_serial_INCLUDE_DIRS}
	${PROJECT_SOURCE_DIR}/libraries/SerialDataAccess/src/Interface
	${PROJECT_SOURCE_DIR}/libraries/DataAccess/src/Interface
)

bm_include_libraries(
	SerialDataAccess
	DataAccess
	TableIO
	CBMGenerics
)

target_link_libraries(cauldron2bhf
	DataAccess
	SerialDataAccess
	TableIO
	${HDF5_serial_LIBRARIES}
)

install(TARGETS cauldron2bhf
	RUNTIME DESTINATION bin
)
# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:

