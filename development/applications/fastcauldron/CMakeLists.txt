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

set( all_headers,
     src/AllochthonousLithologyMapCalculator.h
     src/auxiliary_operations.h
     src/BaseElement.h
     src/BasementPropertyCalculator.h
     src/BiomarkersAdapter.h
     src/BiomarkersCalculator.h
     src/BoundaryConditions.h
     src/BoundaryId.h
     src/BrineProperties.h
     src/BrooksCorey.h
     src/BulkDensityCalculator.h
     src/CapillaryPressureCalculator.h
     src/ChemicalCompactionGrid.h
     src/ChemicalCompactionSchneiderGrid.h
     src/ChemicalCompactionWalderhaugGrid.h
     src/ComponentConcentrationCalculator.h
     src/CompositeElementActivityPredicate.h
     src/ComputationalDomain.h
     src/ComputedPropertyValue.h
     src/ConstrainedArray.h
     src/CrustFormation.h
     src/DarcyCalculations.h
     src/DerivedOutputPropertyMapFactory.h
     src/DerivedOutputPropertyMap.h
     src/ElementActivityPredicate.h
     src/ElementAlwaysActivePredicate.h
     src/element_contributions.h
     src/ElementContributions.h
     src/ElementDofsMixin.h
     src/ElementFaceValues.h
     src/ElementGrid.h
     src/ElementNonZeroPorosityActivityPredicate.h
     src/ElementThicknessActivityPredicate.h
     src/ElementVolumeGrid.h
     src/ErosionFactorCalculator.h
     src/ExplicitMultiComponentFlowSolver.h
     src/FaceAreaInterpolatorCalculator.h
     src/FacePermeabilityInterpolatorCalculator.h
     src/FastcauldronFactory.h
     src/FastcauldronSimulator.h
     src/FastcauldronStartup.h
     src/FaultElementMapCalculator.h
     src/fctcalc.h
     src/FCTCorrectionCalculator.h
     src/fem_grid_auxiliary_functions.h
     src/fem_grid.h
     src/fileio.h
     src/filterwizard.h
     src/FissionTrackCalculator.h
     src/FluidPropertyCalculator.h
     src/FluidVelocityCalculator.h
     src/FormationElementGrid.h
     src/FormationMapping.h
     src/FormationSubdomainElementGrid.h
     src/FracturePressureMapCalculator.h
     src/FracturePressureVolumeCalculator.h
     src/GeneralElement.h
     src/GenericFortranMatrix.h
     src/GeometricLoopPressureSolver.h
     src/ghost_array.h
     src/globaldefs.h
     src/HcFluidVelocityCalculator.h
     src/HeatFlowCalculator.h
     src/history.h
     src/HydraulicFracturingManager.h
     src/ImmobileSpeciesValues.h
     src/LayerElement.h
     src/layer.h
     src/layer_iterators.h
     src/LinearPropertyInterpolator.h
     src/Lithology.h
     src/LithologyIdCalculator.h
     src/LithologyVolumeCalculator.h
     src/MantleFormation.h
     src/MapElement.h
     src/mapinterpolator.h
     src/MatrixInterpolator.h
     src/MatrixNonZeroCalculator.h
     src/MaxVesCalculator.h
     src/MemoryChecker.h
     src/Mesh3DIndex.h
     src/milestones.h
     src/MultiComponentFlowHandler.h
     src/MultiPropertyTemporalInterpolator.h
     src/Nodal3DIndexArray.h
     src/NodalGrid.h
     src/NodalVolumeGrid.h
     src/NonGeometricLoopPressureSolver.h
     src/OptionalGenexPropertyCalculator.h
     src/Output.h
     src/OutputPropertyMap.h
     src/PermeabilityCalculator.h
     src/PetscBlockVector.h
     src/PetscLogStages.h
     src/PetscObjectAllocator.h
     src/PetscSolver.h
     src/PoreVolumeInterpolatorCalculator.h
     src/PorosityCalculator.h
     src/PressureSolver.h
     src/PrimaryOutputPropertyMap.h
     src/PrimaryOutputPropertyVolume.h
     src/PropertiesIoTbl.h
     src/Property.h
     src/property_manager.h
     src/PropertyManager.h
     src/PropertyOutputConstraints.h
     src/PropertyValue.h
     src/propinterface.h
     src/PVTCalculator.h
     src/PvtHcProperties.h
     src/readproperties.h
     src/ReflectivityCalculator.h
     src/RelatedProject.h
     src/RelativePermeabilityCalculator.h
     src/RequiredGenex5PropertyCalculator.h
     src/RunParameters.h
     src/SaturationCalculator.h
     src/Saturation.h
     src/SedimentElementActivityPredicate.h
     src/SimpleBoundedArray.h
     src/SmectiteIlliteAdapter.h
     src/SmectiteIlliteCalculator.h
     src/snapshotdata.h
     src/snapshotinterpolator.h
     src/SolutionVectorMapping.h
     src/SonicCalculator.h
     src/SourceRocksTemporalInterpolator.h
     src/SourceTermInterpolatorCalculator.h
     src/SourceTermTemporalInterpolator.h
     src/StatisticsHandler.h
     src/StratigraphicColumn.h
     src/StratigraphicGrids.h
     src/SubdomainElement.h
     src/Subdomain.h
     src/System.h
     src/TemperatureForVreInputGrid.h
     src/temperature_solver.h
     src/TemporalPropertyInterpolator.h
     src/ThermalConductivityCalculator.h
     src/ThermalDiffusivityCalculator.h
     src/ThicknessCalculator.h
     src/ThicknessErrorMapCalculator.h
     src/timefilter.h
     src/TimeOfElementInvasionCalculator.h
     src/TransportedVolumeCalculator.h
     src/TwoWayTimeCalculator.h
     src/utils.h
     src/VectorInterpolator.h
     src/VelocityCalculator.h
     src/VitriniteReflectanceCalculator.h
     src/VolumeCalculator.h
     src/VreGrid.h
     src/VreOutputGrid.h
     src/VtkMeshWriter.h
     src/Well.h
)

#
# This list contains all implementation files except main.C
#
set( all_srcs_except_main
     src/AllochthonousLithologyMapCalculator.C
     src/BasementPropertyCalculator.C
     src/BiomarkersAdapter.C
     src/BiomarkersCalculator.C
     src/BoundaryConditions.C
     src/BoundaryId.C
     src/BrineProperties.C
     src/BrooksCorey.C
     src/BulkDensityCalculator.C
     src/CapillaryPressureCalculator.C
     src/ChemicalCompactionGrid.C
     src/ChemicalCompactionSchneiderGrid.C
     src/ChemicalCompactionWalderhaugGrid.C
     src/ComponentConcentrationCalculator.C
     src/CompositeElementActivityPredicate.C
     src/ComputationalDomain.C
     src/CrustFormation.C
     src/DarcyCalculations.C
     src/DerivedOutputPropertyMapFactory.C
     src/element_contributions.C
     src/ElementContributions.C
     src/ElementDofsMixin.C
     src/ElementFaceValues.C
     src/ElementGrid.C
     src/ElementVolumeGrid.C
     src/ErosionFactorCalculator.C
     src/ExplicitMultiComponentFlowSolver.C
     src/FaceAreaInterpolatorCalculator.C
     src/FacePermeabilityInterpolatorCalculator.C
     src/FastcauldronFactory.C
     src/FastcauldronSimulator.C
     src/FastcauldronStartup.C
     src/FaultElementMapCalculator.C
     src/fctcalc.C
     src/FCTCorrectionCalculator.C
     src/fem_grid_auxiliary_functions.C
     src/fem_grid.C
     src/fileio.C
     src/filterwizard.C
     src/FissionTrackCalculator.C
     src/FluidPropertyCalculator.C
     src/FluidVelocityCalculator.C
     src/FormationSubdomainElementGrid.C
     src/FracturePressureMapCalculator.C
     src/FracturePressureVolumeCalculator.C
     src/GeometricLoopPressureSolver.C
     src/ghost_array.C
     src/globaldefs.C
     src/HcFluidVelocityCalculator.C
     src/HeatFlowCalculator.C
     src/history.C
     src/HydraulicFracturingManager.C
     src/ImmobileSpeciesValues.C
     src/layer.C
     src/LayerElement.C
     src/layer_iterators.C
     src/LinearPropertyInterpolator.C
     src/Lithology.C
     src/LithologyIdCalculator.C
     src/LithologyVolumeCalculator.C
     src/MantleFormation.C
     src/MapElement.C
     src/mapinterpolator.C
     src/MatrixNonZeroCalculator.C
     src/MaxVesCalculator.C
     src/MemoryChecker.C
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
     src/PetscLogStages.C
     src/PetscObjectAllocator.C
     src/PetscSolver.C
     src/PoreVolumeInterpolatorCalculator.C
     src/PorosityCalculator.C
     src/PressureSolver.C
     src/PrimaryOutputPropertyMap.C
     src/PrimaryOutputPropertyVolume.C
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
     src/SolutionVectorMapping.C
     src/SonicCalculator.C
     src/SourceRocksTemporalInterpolator.C
     src/SourceTermInterpolatorCalculator.C
     src/SourceTermTemporalInterpolator.C
     src/StatisticsHandler.C
     src/StratigraphicColumn.C
     src/StratigraphicGrids.C
     src/Subdomain.C
     src/System.C
     src/TemperatureForVreInputGrid.C
     src/temperature_solver.C
     src/TemporalPropertyInterpolator.C
     src/ThermalConductivityCalculator.C
     src/ThermalDiffusivityCalculator.C
     src/ThicknessCalculator.C
     src/ThicknessErrorMapCalculator.C
     src/timefilter.C
     src/TimeOfElementInvasionCalculator.C
     src/TransportedVolumeCalculator.C
     src/TwoWayTimeCalculator.C
     src/utils.C
     src/VelocityCalculator.C
     src/VitriniteReflectanceCalculator.C
     src/VolumeCalculator.C
     src/VreGrid.C
     src/VreOutputGrid.C
     src/VtkMeshWriter.C
     src/Well.C
)

source_group(include FILES ${all_headers})
source_group(source FILES ${all_srcs_except_main} src/main.C)

add_library( FastcauldronApplication
	${all_srcs_except_main}
	${all_headers}
)

add_executable(fastcauldron src/main.C ${all_headers})

if (UNIX)
add_dependencies(FastcauldronApplication TableIO Boost PETSC)
else (WIN32)
add_dependencies(FastcauldronApplication TableIO)
endif (UNIX)


include_directories(
        src
        ${PROJECT_SOURCE_DIR}/libraries
	${HDF5_INCLUDE_DIRS}
	${MPI_INCLUDE_DIRS}
	${PETSC_INCLUDE_DIRS}
	${EIGEN_INCLUDE_DIRS}
)

bm_include_libraries(
	FastcauldronApplication
	DataModel
	DataAccess
	DistributedDataAccess
	utilities
	Utilities_Petsc
	Serial_Hdf5
	Parallel_Hdf5
	EosPack
	TableIO
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
	FastcauldronApplication
	DataAccess
	DistributedDataAccess
	utilities
	Utilities_Petsc
	Serial_Hdf5
	Parallel_Hdf5
	EosPack
	TableIO
	LinearAlgebra
	Interpolation
	FiniteElements
	CBMGenerics
	genex6
	genex6_kernel	
	GeoPhysics
	calibration_kernel
	OTGC_kernel6
	${HDF5_LIBRARIES}
	${PETSC_LIBRARIES}
	${MPI_LIBRARIES}
	${Boost_LIBRARIES}
     ${BM_CLOCK_GETTIME_LIB}
)

set_target_properties(fastcauldron
      PROPERTIES LINK_FLAGS "${PETSC_LINK_FLAGS}"
)

if(UNIX)
   # The OneFilePerProcess library is only available on Unix
   bm_include_libraries( OneFilePerProcess )
   target_link_libraries(fastcauldron OneFilePerProcess)
endif()

if (WIN32)
  # Windows.h defines 'min' and 'max' macros if NOMINMAX is not defined. These
  # macros collide with std::min and std::max
  add_definitions(-DNOMINMAX)
endif()

if (BM_USE_FLEXLM)
   include_directories( ${FLEXLM_INCLUDE_DIRS})
   target_link_libraries(fastcauldron ${FLEXLM_LIBRARIES})
endif(BM_USE_FLEXLM)

install(TARGETS fastcauldron
	RUNTIME DESTINATION bin
)

add_gtest( NAME "VectorInterpolator"
           SOURCES test/InterpolationTest.C 
           LIBRARIES LinearAlgebra 
           )

add_gtest( NAME "MatrixInterpolator"
           SOURCES test/MatrixInterpolationTest.C
           LIBRARIES LinearAlgebra )

add_gtest( NAME "BrooksCorey::computeCapillaryEntryPressure"
	   SOURCES test/PceTest.C )

add_gtest( NAME "PetscSolver"
           SOURCES src/PetscSolver.C test/PetscSolver.C
           LIBRARIES ${PETSC_LIBRARIES}
           LINK_FLAGS "${PETSC_LINK_FLAGS}"
         )

configure_file( 
      ${CMAKE_CURRENT_SOURCE_DIR}/test/MemCheck.project3d
      MemCheck.project3d
      COPYONLY
)

add_memcheck( NAME "Fastcauldron_SimpleItcoupled"
             EXECUTABLE fastcauldron
             PARAMETERS "-project" "MemCheck.project3d" "-itcoupled"
      )



#
# Copy project files and input maps for dof counting unit tests
#
configure_file( 
      ${CMAKE_CURRENT_SOURCE_DIR}/test/Acquifer.project3d
      Acquifer.project3d
      COPYONLY
)

configure_file( 
      ${CMAKE_CURRENT_SOURCE_DIR}/test/Acquifer.HDF
      Acquifer.HDF
      COPYONLY
)

configure_file( 
      ${CMAKE_CURRENT_SOURCE_DIR}/test/MeshWithHoles01.project3d
      MeshWithHoles01.project3d
      COPYONLY
)

configure_file( 
      ${CMAKE_CURRENT_SOURCE_DIR}/test/MeshWithHoles02.project3d
      MeshWithHoles02.project3d
      COPYONLY
)

configure_file( 
      ${CMAKE_CURRENT_SOURCE_DIR}/test/MeshWithMixedHolesStripes.project3d
      MeshWithMixedHolesStripes.project3d
      COPYONLY
)

configure_file( 
      ${CMAKE_CURRENT_SOURCE_DIR}/test/chessBoard1.hdf5
      chessBoard1.hdf5
      COPYONLY
)

configure_file( 
      ${CMAKE_CURRENT_SOURCE_DIR}/test/chessBoard2.hdf5
      chessBoard2.hdf5
      COPYONLY
)

configure_file( 
      ${CMAKE_CURRENT_SOURCE_DIR}/test/thickStripes.hdf5
      thickStripes.hdf5
      COPYONLY
)


#
# Copy valid vtk mesh files for dof counting unit tests
#
configure_file( 
      ${CMAKE_CURRENT_SOURCE_DIR}/test/valid_noholes_seds_only_0.vtk
      valid_noholes_seds_only_0.vtk
      COPYONLY
)

configure_file( 
      ${CMAKE_CURRENT_SOURCE_DIR}/test/valid_noholes_seds_only_10.vtk
      valid_noholes_seds_only_10.vtk
      COPYONLY
)

configure_file( 
      ${CMAKE_CURRENT_SOURCE_DIR}/test/valid_noholes_with_mantle_0.vtk
      valid_noholes_with_mantle_0.vtk
      COPYONLY
)

configure_file( 
      ${CMAKE_CURRENT_SOURCE_DIR}/test/valid_noholes_with_mantle_10.vtk
      valid_noholes_with_mantle_10.vtk
      COPYONLY
)

configure_file( 
      ${CMAKE_CURRENT_SOURCE_DIR}/test/valid_seds_holes01_0.vtk
      valid_seds_holes01_0.vtk
      COPYONLY
)

configure_file( 
      ${CMAKE_CURRENT_SOURCE_DIR}/test/valid_seds_holes02_0.vtk
      valid_seds_holes02_0.vtk
      COPYONLY
)

configure_file( 
      ${CMAKE_CURRENT_SOURCE_DIR}/test/valid_mixed_holes_0.vtk
      valid_mixed_holes_0.vtk
      COPYONLY
)

set(CFGFLS "${PROJECT_SOURCE_DIR}/geocase/misc")

# add_gtest( NAME "MeshUnitTesterFull"
#            SOURCES test/MeshUnitTester.C test/MeshUnitTesterFull.C
#            LIBRARIES  FastcauldronApplication DataAccess DistributedDataAccess utilities Utilities_Petsc Serial_Hdf5 Parallel_Hdf5 EosPack TableIO LinearAlgebra Interpolation FiniteElements CBMGenerics genex6_kernel GeoPhysics calibration_kernel OTGC_kernel6 ${HDF5_LIBRARIES} ${PETSC_LIBRARIES} ${MPI_LIBRARIES} ${Boost_LIBRARIES}
#            LINK_FLAGS "${PETSC_LINK_FLAGS}"
#            ENV_VARS EOSPACKDIR=${CFGFLS}/eospack GENEX5DIR=${CFGFLS}/genex50 
#          )

# add_gtest( NAME "MeshUnitTesterSediments"
#            SOURCES test/MeshUnitTester.C test/MeshUnitTesterSediments.C
#            LIBRARIES  FastcauldronApplication DataAccess DistributedDataAccess utilities Utilities_Petsc Serial_Hdf5 Parallel_Hdf5 EosPack TableIO LinearAlgebra Interpolation FiniteElements CBMGenerics genex6_kernel GeoPhysics calibration_kernel OTGC_kernel6 ${HDF5_LIBRARIES} ${PETSC_LIBRARIES} ${MPI_LIBRARIES} ${Boost_LIBRARIES}
#            LINK_FLAGS "${PETSC_LINK_FLAGS}"
#            ENV_VARS EOSPACKDIR=${CFGFLS}/eospack GENEX5DIR=${CFGFLS}/genex50 
#          )

# add_gtest( NAME "MeshUnitTesterHoles01"
#            SOURCES test/MeshUnitTester.C test/MeshUnitTesterHoles01.C
#            LIBRARIES  FastcauldronApplication DataAccess DistributedDataAccess utilities Utilities_Petsc Serial_Hdf5 Parallel_Hdf5 EosPack TableIO LinearAlgebra Interpolation FiniteElements CBMGenerics genex6_kernel GeoPhysics calibration_kernel OTGC_kernel6 ${HDF5_LIBRARIES} ${PETSC_LIBRARIES} ${MPI_LIBRARIES} ${Boost_LIBRARIES}
#            LINK_FLAGS "${PETSC_LINK_FLAGS}"
#            ENV_VARS EOSPACKDIR=${CFGFLS}/eospack GENEX5DIR=${CFGFLS}/genex50 
#          )

# add_gtest( NAME "MeshUnitTesterHoles02"
#            SOURCES test/MeshUnitTester.C test/MeshUnitTesterHoles02.C
#            LIBRARIES  FastcauldronApplication DataAccess DistributedDataAccess utilities Utilities_Petsc Serial_Hdf5 Parallel_Hdf5 EosPack TableIO LinearAlgebra Interpolation FiniteElements CBMGenerics genex6_kernel GeoPhysics calibration_kernel OTGC_kernel6 ${HDF5_LIBRARIES} ${PETSC_LIBRARIES} ${MPI_LIBRARIES} ${Boost_LIBRARIES}
#            LINK_FLAGS "${PETSC_LINK_FLAGS}"
#            ENV_VARS EOSPACKDIR=${CFGFLS}/eospack GENEX5DIR=${CFGFLS}/genex50 
#          )

# add_gtest( NAME "MeshUnitTesterMixedHoles"
#            SOURCES test/MeshUnitTester.C test/MeshUnitTesterMixedHoles.C
#            LIBRARIES  FastcauldronApplication DataAccess DistributedDataAccess utilities Utilities_Petsc Serial_Hdf5 Parallel_Hdf5 EosPack TableIO LinearAlgebra Interpolation FiniteElements CBMGenerics genex6_kernel GeoPhysics calibration_kernel OTGC_kernel6 ${HDF5_LIBRARIES} ${PETSC_LIBRARIES} ${MPI_LIBRARIES} ${Boost_LIBRARIES}
#            LINK_FLAGS "${PETSC_LINK_FLAGS}"
#            ENV_VARS EOSPACKDIR=${CFGFLS}/eospack GENEX5DIR=${CFGFLS}/genex50 
#          )

	
endif(BM_PARALLEL)	

generate_dox(src/fastcauldron.cfg )

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:



