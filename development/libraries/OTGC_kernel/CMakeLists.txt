add_library( OTGC_kernel
	src/Utilities.C
	src/SpeciesState.C
	src/SpeciesProperties.C
	src/Species.C
	src/Input.C
	src/SimulatorState.C
	src/GeneralParametersHandler.C
	src/Simulator.C
	src/SCmathlib.C
	src/ReactionRatio.C
	src/Reaction.C
	src/ChemicalModel.C
	src/LinearInterpolator.C
)

bm_include_libraries( CBMGenerics ) 

target_link_libraries(OTGC_kernel
	CBMGenerics
)

generate_dox( src/OTGC_kernel.cfg )

install(TARGETS OTGC_kernel
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

