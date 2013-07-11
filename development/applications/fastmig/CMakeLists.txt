if (BM_PARALLEL)

add_executable( fastmig
	  src/Barrier.C
	  src/Biodegrade.C
	  src/CapillarySealStrength.C
	  src/Column.C
	  src/Composition.C
	  src/depthToVolume_Column.C
	  src/DiffusionCoefficient.C
	  src/DiffusionLeak.C
	  src/Formation.C
	  src/Immobiles.C
	  src/Interpolator.C
	  src/LeakAllGasAndOilDistributor.C
	  src/LeakGasAndSpillOil.C
	  src/LeakGas.C
	  src/Leak.C
	  src/LeakOrSpill.C
	  src/LeakOrWasteGasAndSpillOil.C
	  src/LeakOrWaste.C
	  src/LeakWasteAndSpillDistributor.C
	  src/main.C
	  src/MassBalance.C
	  src/MassBalance_ofstream.C
	  src/MassBalance_ostream.C
	  src/migration.C
	  src/Migrator.C
	  src/ObjectFactory.C
	  src/OilToGasCracker.C
	  src/overburden_MPI.C
	  src/overburden.C
	  src/rankings.C
	  src/RequestDefs.C
	  src/RequestHandling.C
	  src/Reservoir.C
	  src/SpillAllGasAndOilDistributor.C
	  src/Spill.C
	  src/SpillOilAndGas.C
	  src/Surface.C
	  src/SurfaceGridMapContainer.C
	  src/SurfaceGridMap.C
	  src/Trap.C
	  src/utils.C
	  src/WasteGasAndSpillOil.C
	  src/Waste.C
)

add_definitions(-DUSEOTGC)

include_directories(
	${PROJECT_SOURCE_DIR}/libraries
	${PROJECT_SOURCE_DIR}/libraries/DataAccess/src/Interface
	${FLEXLM_INCLUDE_DIRS}
	${PETSC_INCLUDE_DIRS}
	${HDF5_parallel_INCLUDE_DIRS}
	${MPI_C_INCLUDE_PATH}
)
	

bm_include_libraries(
	DataAccess
	DistributedDataAccess
	Utilities_Petsc
	Serial_Hdf5
	Parallel_Hdf5
	EosPack	
	CBMGenerics
	genex6
	OTGC_kernel6
	utilities
	TableIO
	functions
)

target_link_libraries(fastmig
	DataAccess
	DistributedDataAccess
	Utilities_Petsc
	Serial_Hdf5
	Parallel_Hdf5
	EosPack	
	CBMGenerics
	genex6
	OTGC_kernel6
	utilities
	TableIO
	functions
	${FLEXLM_LIBRARIES}
	${PETSC_LIBRARIES}
	${HDF5_parallel_LIBRARIES}
	${BLAS_LIBRARIES}
	${BM_MPI_LOCK_LIBRARIES}
)

set_target_properties(fastmig
      PROPERTIES LINK_FLAGS "${PETSC_LINK_FLAGS}"
)


install(TARGETS fastmig
	RUNTIME DESTINATION bin
)

endif (BM_PARALLEL)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
