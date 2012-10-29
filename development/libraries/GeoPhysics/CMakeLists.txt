add_library( GeoPhysics
	src/AllochthonousLithologyInterpolator.C
	src/AllochthonousLithologyManager.C
	src/BasementLithology.C
	src/CauldronGridDescription.C
	src/CompoundLithologyArray.C
	src/CompoundLithology.C
	src/CompoundLithologyComposition.C
	src/CompoundProperty.C
	src/FracturePressureCalculator.C
	src/GeoPhysicalFunctions.C
	src/GeoPhysicsCrustFormation.C
	src/GeoPhysicsFluidType.C
	src/GeoPhysicsFormation.C
	src/GeoPhysicsMantleFormation.C
	src/GeoPhysicsObjectFactory.C
	src/GeoPhysicsProjectHandle.C
	src/GeoPhysicsSourceRock.C
	src/HCDensity.C
	src/LithologyManager.C
	src/SimpleLithology.C
	src/TimeDependantLithology.C
)

bm_include_libraries(
	utilities 
	LinearAlgebra 
	Interpolation 
	DataAccess
	genex6 
	genex6_kernel 
	TableIO
	CBMGenerics
	EosPack
	CrustalThickness
)

include_directories(
	${HDF5_serial_INCLUDE_DIRS}
)

target_link_libraries(GeoPhysics
	utilities
	LinearAlgebra
	Interpolation
	DataAccess
	genex6
	genex6_kernel
	TableIO
	CBMGenerics
	EosPack
	CrustalThickness
)
	
	
install(TARGETS GeoPhysics	
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
