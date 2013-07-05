if (BM_PARALLEL)
set(target_name fasttouch7)

add_executable(${target_name}
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

remove_definitions(-fPIC)
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

# IMPORTANT: remove -rdynamic from CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

include_directories(
	${HDF5_parallel_INCLUDE_DIRS}
	${MPI_C_INCLUDE_PATH}
	${PETSC_INCLUDE_DIRS}
	${XERCES_INCLUDE_DIRS}
	${XSD_INCLUDE_DIRS}
	${TS7_GEOCOSM_XML_INCLUDE}
	${TS7_INCLUDE_DIRS}
	${TS_INCLUDE_DIRS}
	${FLEXLM_INCLUDE_DIRS}
)

target_link_libraries(${target_name} 
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
	${HDF5_parallel_LIBRARIES}
	${FLEXLM_LIBRARIES}
	${XERCES_LIBRARIES}
	${PETSC_LIBRARIES}
	${BLAS_LIBRARIES}
	${BM_MPI_LOCK_LIBRARIES}
)

set_target_properties(fasttouch7
      PROPERTIES LINK_FLAGS "${PETSC_LINK_FLAGS}"
)


install(TARGETS fasttouch7
	RUNTIME DESTINATION bin
)

endif(BM_PARALLEL)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
