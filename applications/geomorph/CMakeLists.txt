add_executable(geomorph
	src/AllochthonousLithology.C
	src/AllochthonousLithologyDistribution.C
	src/AllochthonousLithologySimulator.C
	src/AllochthonousModellingFactory.C
	src/ErrorPointSet.C
	src/geomorph.C
	src/MapIntervalInterpolator.C
	src/MapProcessingOperations.C
	src/PrescribedGridPointCondenser.C
	src/PThreadsAllochthonousLithology.C
	src/PThreadsAllochthonousModellingFactory.C
	src/PThreadsMapIntervalInterpolator.C
	src/SubsampledGridPointCondenser.C
)

bm_include_libraries(
	SerialDataAccess
	DataAccess
	TableIO
	CBMGenerics
	Serial_Hdf5
	utilities
	LinearAlgebra
	Interpolation
	ThreadUtilities
)

include_directories(
	${HDF5_INCLUDE_DIRS}
	${FLEXLM_INCLUDE_DIRS}
)

target_link_libraries(geomorph
	DataAccess
	SerialDataAccess
	TableIO
	CBMGenerics
	Serial_Hdf5
	utilities
	LinearAlgebra
	Interpolation
	ThreadUtilities
	${HDF5_LIBRARIES}
	${FLEXLM_LIBRARIES}
)

add_definitions(-DSVNREVISION=-9999)

generate_dox( src/geomorph.cfg )

install( TARGETS geomorph
	RUNTIME DESTINATION bin
)
