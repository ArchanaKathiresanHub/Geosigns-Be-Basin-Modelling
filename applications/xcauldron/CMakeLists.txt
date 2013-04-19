add_library(projectdependencies
      src/projectdependencies.C
)

add_executable(xcauldron
	src/xcauldron.C
)

bm_include_libraries(
	TableIO
)

target_link_libraries(projectdependencies
      TableIO
)

target_link_libraries(xcauldron
        projectdependencies
	TableIO
)

install(TARGETS xcauldron
	RUNTIME DESTINATION bin
)
