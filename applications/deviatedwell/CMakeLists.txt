add_executable(deviatedwell
	src/DeviatedWellData.C
	src/deviatedwell.C
	src/GlobalDefs.C
	src/WellTrajectoryLocation.C
	src/CSVWellWriter.C
	src/MatlabWellWriter.C
	src/StdOutWellWriter.C
	src/StreamWellWriter.C
	src/WellWriter.C
	src/WellWriterFactory.C
)

bm_include_libraries(
	DataMining
	GeoPhysics
	DataAccess
	SerialDataAccess
	EosPack
	utilities
	Serial_Hdf5
	TableIO
	ThreadUtilities
	LinearAlgebra
	Interpolation
	FiniteElements
	CBMGenerics
	calibration_kernel
)

include_directories(
	${HDF5_INCLUDE_DIRS}
	${BLAS_INCLUDE_DIRS}
)

target_link_libraries(deviatedwell
	DataMining
	GeoPhysics
	DataAccess
	SerialDataAccess
	EosPack
	utilities
	Serial_Hdf5
	TableIO
	ThreadUtilities
	LinearAlgebra
	Interpolation
	FiniteElements
	CBMGenerics
	calibration_kernel
	${HDF5_LIBRARIES}
	${BLAS_LIBRARIES}
)

install(TARGETS deviatedwell
	RUNTIME DESTINATION bin
)
