
set(EOSPACK_SOURCES 
	src/EosPack.C
	src/EosCauldron.C
	src/EosPvtModel.C
	src/Parse.C
	src/polynomials.C
)

if (WIN32)
	set(EOSPACK_EXTRA_SOURCES src/EosPackWin.c)
endif(WIN32)

add_library(EosPack 
	${EOSPACK_SOURCES}
	${EOSPACK_EXTRA_SOURCES}
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

bm_include_libraries(CBMGenerics)

target_link_libraries(EosPack CBMGenerics)

generate_dox( src/EosPack.cfg )

install(TARGETS EosPack
	RUNTIME DESTINATION bin
	LIBRARY	DESTINATION lib
	ARCHIVE DESTINATION lib
)
