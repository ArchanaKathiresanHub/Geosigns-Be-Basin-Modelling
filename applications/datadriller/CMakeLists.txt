add_executable(datadriller
	src/datadriller.C
)

bm_include_libraries(
	DataMining
	DataAccess
	SerialDataAccess
	CBMGenerics
	GeoPhysics
	Serial_Hdf5
	EosPack
	utilities
	TableIO
	LinearAlgebra
	Interpolation
	FiniteElements
)

include_directories(
	${HDF5_INCLUDE_DIRS}
)

target_link_libraries(datadriller
	DataMining
	DataAccess
	SerialDataAccess
	CBMGenerics
	GeoPhysics
	Serial_Hdf5
	EosPack
	utilities
	TableIO
	LinearAlgebra
	Interpolation
	FiniteElements
)

install(TARGETS datadriller
	RUNTIME DESTINATION bin
)
