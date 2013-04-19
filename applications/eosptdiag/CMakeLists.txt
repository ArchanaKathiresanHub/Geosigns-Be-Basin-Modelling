add_executable(eosptdiag
	src/main.C
	src/TrapperIoTableRec.C
)

bm_include_libraries(
	DataAccess
	SerialDataAccess
	TableIO
	CBMGenerics
	EosPack
)

include_directories(
	${HDF5_serial_INCLUDE_DIRS}
)

target_link_libraries(eosptdiag
	DataAccess
	SerialDataAccess
	TableIO
	CBMGenerics
	EosPack
	${HDF5_serial_LIBRARIES}
)

install(TARGETS eosptdiag
	RUNTIME DESTINATION bin
)
