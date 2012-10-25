add_executable(fastreq
	src/main.C
	src/FastReqProjectHandle.C
	src/FastReqObjectFactory.C
)

add_definitions(-DUSEINTERFACE)

include_directories(
	${HDF5_serial_INCLUDE_DIRS}
)

bm_include_libraries(
	SerialDataAccess
	DataAccess
	TableIO
	Serial_Hdf5
	CBMGenerics
	utilities
)

target_link_libraries( fastreq
	DataAccess
	SerialDataAccess
	TableIO
	Serial_Hdf5
	CBMGenerics
	utilities
	${HDF5_serial_LIBRARIES}
)

install(TARGETS fastreq
	RUNTIME DESTINATION bin
)


generate_dox( src/fastreq.cfg )
# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
