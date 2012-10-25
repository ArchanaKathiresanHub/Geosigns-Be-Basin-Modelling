add_executable(isosurface
	src/isosurface.C
)

add_definitions(-DUSEINTERFACE)

include_directories(
	${HDF5_INCLUDE_DIRS}
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
