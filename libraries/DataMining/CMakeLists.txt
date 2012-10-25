add_library(DataMining
        src/BasinTemperatureGradientCalculator.C
	src/BasementHeatFlowCalculator.C
	src/BrineDensityCalculator.C
	src/BrineViscosityCalculator.C
	src/CauldronDomain.C
	src/CauldronWell.C
	src/ConstantDomainFormationProperty.C
	src/DataMiner.C
	src/DeviatedWell.C
	src/DomainFormationProperty.C
	src/DomainProperty.C
	src/DomainPropertyCollection.C
	src/DomainPropertyFactory.C
	src/DomainReservoirProperty.C
	src/DomainSurfaceProperty.C
	src/ElementFunctions.C
	src/ElementPosition.C
	src/FluidVelocityCalculator.C
	src/FracturePressureCalculator.C
	src/HeatFlowCalculator.C
	src/HeatFlowMagnitudeCalculator.C
	src/InterpolatedPropertyValues.C
	src/PieceWiseInterpolator1D.C
	src/PermeabilityCalculator.C
	src/PorosityCalculator.C
	src/RadiogenicHeatProductionCalculator.C
	src/TemperatureGradientCalculator.C
	src/ThermalConductivityCalculator.C
	src/DataMiningProjectHandle.C
	src/PropertyInterpolator2D.C
	src/PropertyInterpolator3D.C
	src/VerticalWell.C
)


bm_include_libraries( 
	FiniteElements 
	utilities 
	ThreadUtilities
	Interpolation 
	LinearAlgebra 
	TableIO 
	EosPack 
	SerialDataAccess
	DataAccess 
	GeoPhysics 
	CBMGenerics
)

include_directories(
	src
	../SerialDataAccess/src/Interface
	../DataAccess/src/Interface
	${HDF5_INCLUDE_DIRS}
)

target_link_libraries(DataMining
	FiniteElements
	utilities
	ThreadUtilities
	Interpolation
	LinearAlgebra
	TableIO
	EosPack
	SerialDataAccess
	DataAccess
	GeoPhysics
	CBMGenerics
	${HDF5_LIBRARIES}
)

install(TARGETS DataMining
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

