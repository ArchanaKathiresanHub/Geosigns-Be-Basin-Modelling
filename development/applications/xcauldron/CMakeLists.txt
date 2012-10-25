add_executable(xcauldron
	src/xcauldron.C
)

bm_include_libraries(
	TableIO
)

target_link_libraries(xcauldron
	TableIO
)

install(TARGETS xcauldron
	RUNTIME DESTINATION bin
)
