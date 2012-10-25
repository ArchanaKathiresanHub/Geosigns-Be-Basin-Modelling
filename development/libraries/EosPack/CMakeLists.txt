
set(EOSPACK_SOURCES 
	src/EosPack.C
	src/EosCauldron.C
	src/EosPvtModel.C
	src/Parse.C
	src/polynomials.C
)

#if (WIN32)
#	set(EOSPACK_EXTRA_SOURCES src/EosPackWin.c)
#endif(WIN32)

if(UNIX)
	message(STATUS "CMAKE_CXX_FLAGS_RELEASE  = ${CMAKE_CXX_FLAGS_RELEASE}")
	# The library to be compiled with -O3 is EosPack, all others,
	# including the applications are to be compiled with -O2.
	# Also we want assert(3) in Release configuration
	set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()

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

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End: