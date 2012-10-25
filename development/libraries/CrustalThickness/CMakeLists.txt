if (NOT BM_PARALLEL)
	return()
endif()

add_library(CrustalThickness
	src/LinearFunction.C
	src/DensityCalculator.C
	src/InterfaceInput.C
	src/InterfaceOutput.C
)

bm_include_libraries( 
	DataAccess 
	TableIO 
	Utilities 
	Serial_Hdf5
	utilities 
	CBMGenerics
)

include_directories(
	${HDF5_serial_INCLUDE_DIRS}
)

target_link_libraries(CrustalThickness
	DistributedDataAccess
	DataAccess
	TableIO
	Utilities_Petsc
	Serial_Hdf5
	utilities
	${HDF5_serial_LIBRARIES}
)

install(TARGETS CrustalThickness
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
