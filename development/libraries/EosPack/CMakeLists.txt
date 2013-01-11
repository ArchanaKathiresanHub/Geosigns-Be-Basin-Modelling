
set(EOSPACK_SOURCES 
	src/EosPack.C
	src/EosCauldron.C
	src/EosPvtModel.C
	src/EosPvtTable.C
	src/EosUtils.C
	src/EosWater.C
	src/Parse.C
	src/polynomials.C
	src/PTDiagCalculator.C
)

#if (WIN32)
#	set(EOSPACK_EXTRA_SOURCES src/EosPackWin.c)
#endif(WIN32)

add_library(EosPack 
	${EOSPACK_SOURCES}
	${EOSPACK_EXTRA_SOURCES}
)

bm_include_libraries(CBMGenerics)

target_link_libraries(EosPack CBMGenerics)

generate_dox( src/EosPack.cfg )

install(TARGETS EosPack
	RUNTIME DESTINATION bin
	LIBRARY	DESTINATION lib
	ARCHIVE DESTINATION lib
)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End: