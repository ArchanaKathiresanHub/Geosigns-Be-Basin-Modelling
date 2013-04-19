add_executable(datadriller
	src/datadriller.C
)

bm_include_libraries(
	DataMining
	DataAccess
	SerialDataAccess
	CBMGenerics
	GeoPhysics
	CrustalThinkness
	Serial_Hdf5
	EosPack
	utilities
	TableIO
	LinearAlgebra
	Interpolation
	FiniteElements
)

include_directories(
	${HDF5_serial_INCLUDE_DIRS}
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
	${HDF5_serial_LIBRARIES}
)

install(TARGETS datadriller
	RUNTIME DESTINATION bin
)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
