add_executable(fastmiglogger
	src/fastmiglogger.C
)

bm_include_libraries(
	EosPack	
	CBMGenerics
	SerialDataAccess
	DataAccess
	TableIO
)

include_directories(
	${HDF5_INCLUDE_DIRS}
	${PROJECT_SOURCE_DIR}/libraries/DataAccess/src/Interface
)

target_link_libraries(fastmiglogger
	EosPack	
	CBMGenerics
	DataAccess
	SerialDataAccess
	TableIO
	${HDF5_LIBRARIES}
)


install(TARGETS fastmiglogger
	RUNTIME DESTINATION bin
)
