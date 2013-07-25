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

add_gtest(NAME GeoPhysics::PermeabilityImpermeable
   SOURCES test/PermeabilityImpermeable.C
   LIBRARIES GeoPhysics
)

add_gtest( NAME GeoPhysics::PermeabilitySandStone
   SOURCES test/PermeabilitySandStone.C
   LIBRARIES GeoPhysics
)

add_gtest( NAME GeoPhysics::PermeabilityMudStone
      SOURCES test/PermeabilityMudStone.C
      LIBRARIES GeoPhysics
)

add_gtest( NAME GeoPhysics::PermeabilityMultiPoint
      SOURCES test/PermeabilityMultiPoint.C
      LIBRARIES GeoPhysics
)




# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
