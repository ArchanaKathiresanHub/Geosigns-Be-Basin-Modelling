add_executable(track1d
	src/track1d.C
)

add_definitions(-DUSEINTERFACE)

bm_include_libraries(
	DataMining
	DataAccess
	SerialDataAccess
	DataAccess	
	TableIO
	CBMGenerics
	Serial_Hdf5
	EosPack
	utilities
	LinearAlgebra
	Interpolation
	FiniteElements
)

include_directories(
	${HDF5_INCLUDE_DIRS}
)

target_link_libraries(track1d
	DataMining
	DataAccess
	SerialDataAccess
	DataAccess	
	TableIO
	CBMGenerics
	Serial_Hdf5
	EosPack
	utilities
	LinearAlgebra
	Interpolation
	FiniteElements
	${HDF5_LIBRARIES}
)

install(TARGETS track1d
	RUNTIME DESTINATION bin
)
