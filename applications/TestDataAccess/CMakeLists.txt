
add_executable(TestDataAccess src/TestDataAccess.C)
bm_include_libraries(
	SerialDataAccess
	DataAccess
	TableIO
	CBMGenerics
)
include_directories(
	${HDF5_INCLUDE_DIRS}
)
target_link_libraries(TestDataAccess
	SerialDataAccess
	DataAccess
	TableIO
	CBMGenerics
)


