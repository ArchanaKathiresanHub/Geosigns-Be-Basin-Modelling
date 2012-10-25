if (BM_PARALLEL)

add_executable(fasttouch7
	src/main.C
	src/FastTouch.C
	src/MasterTouch.C
	src/ObjectFactory.C
	src/TsLibLoader.C
	src/LibLoaderLinux.C
)

bm_include_libraries(
	DistributedDataAccess
	DataAccess
	Utilities_Petsc
	Serial_Hdf5
	Parallel_Hdf5
	TableIO
	utilities
	CBMGenerics
)

include_directories(
	${HDF5_INCLUDE_DIRS}
	${PETSC_INCLUDE_DIRS}
	${XERCES_INCLUDE_DIRS}
	${XSD_INCLUDE_DIRS}
	${TS7_GEOCOSM_XML_INCLUDE}
	${TS7_INCLUDE_DIRS}
	${TS_INCLUDE_DIRS}
	${FLEXLM_INCLUDE_DIRS}
)
	
target_link_libraries(fasttouch7
	DataAccess
	DistributedDataAccess
	Utilities_Petsc
	Serial_Hdf5
	Parallel_Hdf5
	TableIO
	utilities
	CBMGenerics
	${TS7_LIBRARIES}
	${TS_LIBRARIES}
	${HDF5_LIBRARIES}
	${FLEXLM_LIBRARIES}
	${XERCES_LIBRARIES}
	${PETSC_LIBRARIES}
	${BLAS_LIBRARIES}
)

install(TARGETS fasttouch7
	RUNTIME DESTINATION bin
)

endif(BM_PARALLEL)
