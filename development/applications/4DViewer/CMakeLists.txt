if (UNIX)
	add_compile_options(-std=c++11 -Wno-literal-suffix)
endif (UNIX)

if (BM_BUILD_4DVIEWER)
	add_subdirectory(4DRenderServer)
	add_subdirectory(4DViewer)
endif (BM_BUILD_4DVIEWER)