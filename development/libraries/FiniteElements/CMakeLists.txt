add_library(FiniteElements
	src/BasisFunction.C 
	src/FiniteElement.C
	src/FiniteElementTypes.C
)

bm_include_libraries(utilities)

target_link_libraries(FiniteElements
	utilities
)

install(TARGETS FiniteElements
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)
