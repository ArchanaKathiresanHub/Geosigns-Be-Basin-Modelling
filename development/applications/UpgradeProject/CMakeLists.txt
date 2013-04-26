add_executable(upcauldron src/upcauldron.C)

bm_include_libraries(
	TableIO
)

target_link_libraries(upcauldron
	TableIO
)

install(TARGETS upcauldron
	RUNTIME DESTINATION bin
)

# Local Variables:

# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
