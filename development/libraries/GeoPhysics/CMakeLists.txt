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
        src/Permeability.C
        src/PermeabilityMudStone.C
        src/PermeabilityMultiPoint.C
        src/PermeabilitySandStone.C
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


### Unit Tests

add_executable(PermeabilityImpermeable
      test/PermeabilityImpermeable.C
)

target_link_libraries(PermeabilityImpermeable
      GeoPhysics
)      

add_test(NAME GeoPhysics::PermeabilityImpermeable::permeability
      COMMAND PermeabilityImpermeable permeability
)      

add_test(NAME GeoPhysics::PermeabilityImpermeable::permeabilityDerivative
      COMMAND PermeabilityImpermeable permeabilityDerivative
)      

add_test(NAME GeoPhysics::PermeabilityImpermeable::depoPerm
      COMMAND PermeabilityImpermeable depoPerm
)      

add_test(NAME GeoPhysics::PermeabilityImpermeable::model
      COMMAND PermeabilityImpermeable model
)      

add_executable(PermeabilitySandStone
      test/PermeabilitySandStone.C
)

target_link_libraries(PermeabilitySandStone
      GeoPhysics
)      

add_test(NAME GeoPhysics::PermeabilitySandStone::permeability
      COMMAND PermeabilitySandStone permeability
)      

add_test(NAME GeoPhysics::PermeabilitySandStone::permeabilityDerivative
      COMMAND PermeabilitySandStone permeabilityDerivative
)      

add_test(NAME GeoPhysics::PermeabilitySandStone::depoPerm
      COMMAND PermeabilitySandStone depoPerm
)      

add_test(NAME GeoPhysics::PermeabilitySandStone::model
      COMMAND PermeabilitySandStone model
)      

add_executable(PermeabilityMudStone
      test/PermeabilityMudStone.C
)

target_link_libraries(PermeabilityMudStone
      GeoPhysics
)      

add_test(NAME GeoPhysics::PermeabilityMudStone::permeability
      COMMAND PermeabilityMudStone permeability
)      

add_test(NAME GeoPhysics::PermeabilityMudStone::permeabilityDerivative
      COMMAND PermeabilityMudStone permeabilityDerivative
)      

add_test(NAME GeoPhysics::PermeabilityMudStone::depoPerm
      COMMAND PermeabilityMudStone depoPerm
)      

add_test(NAME GeoPhysics::PermeabilityMudStone::model
      COMMAND PermeabilityMudStone model
)      



# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
