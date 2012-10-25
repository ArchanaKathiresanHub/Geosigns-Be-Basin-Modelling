add_executable(UnitTestGenex6 src/main.C)

bm_include_libraries(
	DistributedDataAccess
	DataAccess	
	genex6
	TableIO
	genex6_kernel
	CBMGenerics
	EosPack
	utilities
)

include_directories(
	${HDF5_INCLUDE_DIRS}
)

target_link_libraries(UnitTestGenex6
	DistributedDataAccess
	TableIO
	genex6_kernel
	genex6
	CBMGenerics
	EosPack
	utilities
	${MPI_C_LIBRARIES}
)

		
