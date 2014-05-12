#########################################################################
#                                                                       #
# Copyright (C) 2012-2013 Shell International Exploration & Production. #
# All rights reserved.                                                  #
#                                                                       #
# Developed under license for Shell by PDS BV.                          #
#                                                                       #
# Confidential and proprietary source code of Shell.                    #
# Do not distribute without written permission from Shell.              #
#                                                                       #
#########################################################################

if (BM_PARALLEL)

add_executable(fastcauldron
	src/AllochthonousLithologyMapCalculator.C
	src/BasementPropertyCalculator.C
	src/BiomarkersAdapter.C
	src/BiomarkersCalculator.C
	src/BoundaryId.C
	src/BrineProperties.C
	src/BulkDensityCalculator.C
	src/CapillaryPressureCalculator.C
	src/CauldronCalculator.C
	src/ComponentConcentrationCalculator.C
	src/CrustFormation.C
	src/DarcyCalculations.C
	src/DerivedOutputPropertyMapFactory.C
	src/ElementContributions.C
	src/element_contributions.C
	src/ElementFaceValues.C
	src/ElementGrid.C
	src/ElementVolumeGrid.C
	src/ErosionFactorCalculator.C
	src/ExplicitMultiComponentFlowSolver.C
	src/FastcauldronFactory.C
	src/FastcauldronSimulator.C
	src/FaultElementMapCalculator.C
	src/fctcalc.C
	src/FCTCorrectionCalculator.C
	src/FaceAreaInterpolatorCalculator.C
	src/FacePermeabilityInterpolatorCalculator.C
	src/fem_grid_auxiliary_functions.C
	src/fem_grid.C
	src/fileio.C
	src/filterwizard.C
	src/FissionTrackCalculator.C
	src/FluidPropertyCalculator.C
	src/FluidVelocityCalculator.C
	src/FormationMapping.C
	src/GeometricLoopPressureSolver.C
	src/ghost_array.C
	src/globaldefs.C
	src/HcFluidVelocityCalculator.C
	src/HeatFlowCalculator.C
	src/history.C
	src/HydraulicFracturingManager.C
	src/ImmobileSpeciesValues.C
	src/layer.C
	src/layer_iterators.C
	src/LinearPropertyInterpolator.C
	src/Lithology.C
	src/LithologyIdCalculator.C
	src/LithostaticPressureCalculator.C
	src/main.C
	src/MantleFormation.C
	src/mapinterpolator.C
	src/MapElement.C
	src/MaxVesCalculator.C
	src/Mesh3DIndex.C
	src/milestones.C
	src/MultiComponentFlowHandler.C
	src/NodalGrid.C
	src/NodalVolumeGrid.C
	src/NonGeometricLoopPressureSolver.C
	src/OptionalGenexPropertyCalculator.C
	src/Output.C
	src/OutputPropertyMap.C
	src/PermeabilityCalculator.C
	src/PoreVolumeInterpolatorCalculator.C
	src/PorosityCalculator.C
	src/PressureSolver.C
	src/PrimaryOutputPropertyMap.C
	src/PrimaryOutputPropertyVolume.C
	src/profile.C
	src/PropertiesIoTbl.C
	src/Property.C
	src/property_manager.C
	src/PropertyManager.C
	src/PropertyOutputConstraints.C
	src/PropertyValue.C
	src/propinterface.C
	src/PVTCalculator.C
	src/PvtHcProperties.C
	src/readproperties.C
	src/ReflectivityCalculator.C
	src/RelatedProject.C
	src/RelativePermeabilityCalculator.C
	src/RequiredGenex5PropertyCalculator.C
	src/RunParameters.C
	src/Saturation.C
	src/SaturationCalculator.C
	src/SmectiteIlliteAdapter.C
	src/SmectiteIlliteCalculator.C
	src/snapshotdata.C
	src/snapshotinterpolator.C
	src/SourceRocksTemporalInterpolator.C
	src/SourceTermInterpolatorCalculator.C
	src/SourceTermTemporalInterpolator.C
	src/SonicCalculator.C
	src/Subdomain.C
	src/SubdomainElement.C
	src/StatisticsHandler.C
	src/FormationSubdomainElementGrid.C
	src/System.C
	src/temperature_solver.C
	src/TemporalPropertyInterpolator.C
	src/ThermalConductivityCalculator.C
	src/ThermalDiffusivityCalculator.C
	src/ThicknessCalculator.C
	src/ThicknessErrorMapCalculator.C
	src/TimeOfElementInvasionCalculator.C
	src/timefilter.C
	src/TransportedVolumeCalculator.C
	src/utils.C
	src/VelocityCalculator.C
	src/VitriniteReflectanceCalculator.C
	src/VolumeCalculator.C
	src/LayerElement.C
	src/Well.C
        src/PetscLogStages.C
        src/TemperatureForVreInputGrid.C
        src/VreGrid.C
        src/VreOutputGrid.C
)

add_definitions(${HDF5_parallel_DEFINITIONS})
remove_definitions(-fPIC)

include_directories(
	${PROJECT_SOURCE_DIR}/libraries/SerialDataAccess/src/Interface
	${PROJECT_SOURCE_DIR}/libraries
	${HDF5_parallel_INCLUDE_DIRS}
	${MPI_INCLUDE_DIRS}
	${PETSC_INCLUDE_DIRS}
	${FLEXLM_INCLUDE_DIRS}
	${EIGEN_INCLUDE_DIRS}
)

bm_include_libraries(
	DataAccess
	DistributedDataAccess
	utilities
	Utilities_Petsc
	Serial_Hdf5
	OneFilePerProcess
	Parallel_Hdf5
	EosPack
	Gempis_Utilities	
	TableIO
	ThreadUtilities
	LinearAlgebra
	Interpolation
	FiniteElements
	CBMGenerics
	genex6
	genex6_kernel	
	GeoPhysics
	calibration_kernel	
)

target_link_libraries(fastcauldron
	DataAccess
	DistributedDataAccess
	utilities
	Utilities_Petsc
	Serial_Hdf5
	OneFilePerProcess
	Parallel_Hdf5
	EosPack
	Gempis_Utilities	
	TableIO
	ThreadUtilities
	LinearAlgebra
	Interpolation
	FiniteElements
	CBMGenerics
	genex6
	genex6_kernel	
	GeoPhysics
	calibration_kernel	
	OTGC_kernel6
	${HDF5_parallel_LIBRARIES}
	${FLEXLM_LIBRARIES}
	${PETSC_LIBRARIES}
	${BLAS_LIBRARIES}
	${BM_MPI_LOCK_LIBRARIES}
	${MPI_LIBRARIES}
	z
)

set_target_properties(fastcauldron
      PROPERTIES LINK_FLAGS "${PETSC_LINK_FLAGS}"
)

install(TARGETS fastcauldron
	RUNTIME DESTINATION bin
)

include_directories ( src ${EIGEN_INCLUDE_DIRS} )

add_gtest( NAME VectorInterpolator
           SOURCES test/InterpolationTest.C 
           LIBRARIES LinearAlgebra ${BLAS_LIBRARIES}
           )

add_gtest( NAME MatrixInterpolator
           SOURCES test/MatrixInterpolationTest.C
           LIBRARIES LinearAlgebra ${BLAS_LIBRARIES} )

add_gtest( NAME "BrooksCorey::computeCapillaryEntryPressure"
	   SOURCES test/PceTest.C )

	
endif(BM_PARALLEL)	

generate_dox( src/fastcauldron.cfg )

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
