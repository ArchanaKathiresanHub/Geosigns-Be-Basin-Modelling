FILE( GLOB all_headers src/*.h )
source_group(include FILES ${all_headers})

FILE( GLOB all_srcs src/*.C)
source_group(source FILES ${all_srcs})


add_library( GeoPhysics STATIC
	${all_srcs}
	${all_headers}
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


add_executable(PermeabilityMultiPoint
      test/PermeabilityMultiPoint.C
)

target_link_libraries(PermeabilityMultiPoint
      GeoPhysics
)      

add_test(NAME GeoPhysics::PermeabilityMultiPoint::permeability
      COMMAND PermeabilityMultiPoint permeability
)      

add_test(NAME GeoPhysics::PermeabilityMultiPoint::permeabilityDerivative
      COMMAND PermeabilityMultiPoint permeabilityDerivative
)      

add_test(NAME GeoPhysics::PermeabilityMultiPoint::depoPerm
      COMMAND PermeabilityMultiPoint depoPerm
)      

add_test(NAME GeoPhysics::PermeabilityMultiPoint::model
      COMMAND PermeabilityMultiPoint model
)      



# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
