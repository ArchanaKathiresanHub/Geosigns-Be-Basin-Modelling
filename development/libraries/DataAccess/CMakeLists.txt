set (SOURCE_FILES
	src/AllochthonousLithology.C
	src/AllochthonousLithologyDistribution.C
	src/AllochthonousLithologyInterpolation.C
	src/AttributeValue.C
	src/BasementFormation.C
	src/BasementSurface.C
	src/BiodegradationParameters.C
	src/Child.C
	src/ConstrainedOverpressureInterval.C
	src/CrustFormation.C
	src/CrustalThicknessData.C
	src/DAObject.C
    src/DiffusionLeakageParameters.C
	src/FaultElementCalculator.C
	src/FaultFileReader.C
	src/FaultFileReaderFactory.C
	src/Faulting.C
	src/FaultCollection.C
    src/FluidType.C
	src/FluidHeatCapacitySample.C
	src/FluidThermalConductivitySample.C
	src/FluidDensitySample.C
	src/Formation.C
    src/FracturePressureFunctionParameters.C
	src/GridMap.C
	src/IBSFaultFileReader.C
	src/InputValue.C
	src/IrreducibleWaterSaturationSample.C
	src/LandmarkFaultFileReader.C
	src/LangmuirAdsorptionIsothermSample.C
	src/LangmuirAdsorptionTOCEntry.C
	src/Lead.C
	src/LeadTrap.C
	src/LithoType.C
	src/LithologyHeatCapacitySample.C
	src/LithologyThermalConductivitySample.C
	src/MD5.C
	src/MantleFormation.C
	src/MobileLayer.C
	src/ObjectFactory.C
	src/OutputProperty.C
	src/PaleoFormationProperty.C
	src/PaleoProperty.C
	src/PaleoSurfaceProperty.C
	src/Parent.C
	src/PointAdsorptionHistory.C
    src/ProjectHandle.C
	src/Property.C
	src/PropertyValue.C
	src/ReconModelFaultFileReader.C
	src/RelatedProject.C
	src/Reservoir.C
	src/RunParameters.C
	src/ProjectData.C
	src/SGDensitySample.C
	src/Snapshot.C
	src/SourceRock.C
	src/Surface.C
	src/TouchstoneMap.C
	src/Trap.C
	src/Trapper.C
	src/Migration.C
	src/TrapPhase.C
	src/ZycorFaultFileReader.C
	src/auxiliaryfaulttypes.C
	src/hdf5funcs.C
	src/wildMatch.C
	)

add_library(DataAccess STATIC ${SOURCE_FILES})

bm_include_libraries(
	Serial_Hdf5
	CBMGenerics
	utilities 
	TableIO
	EosPack
	CrustalThickness
)

include_directories( 
	src
	${HDF5_serial_INCLUDE_DIRS}
)

target_link_libraries(DataAccess 
	Serial_Hdf5
	CBMGenerics 
	utilities
	TableIO
)


# DataAccess library should be position independent because it is linked later
# with SerialDataAccess
set_target_properties(DataAccess PROPERTIES POSITION_INDEPENDENT_CODE ON)

generate_dox( src/DataAccess.cfg )

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
