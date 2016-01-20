if (UNIX)
	add_compile_options(-std=c++11 -Wno-literal-suffix)
endif (UNIX)

add_subdirectory(4DRenderServer)
add_subdirectory(4DViewer)
