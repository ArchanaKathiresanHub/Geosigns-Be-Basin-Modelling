add_executable(isosurface
	src/isosurface.C
)

add_definitions(-DUSEINTERFACE)

include_directories(
	${HDF5_serial_INCLUDE_DIRS}
)

bm_include_libraries(
	SerialDataAccess
	DataAccess
	TableIO
	utilities
	CBMGenerics
)

target_link_libraries(isosurface
	DataAccess
	SerialDataAccess
	TableIO
	CBMGenerics
	utilities
)

install(TARGETS isosurface
	RUNTIME DESTINATION bin
)
# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
