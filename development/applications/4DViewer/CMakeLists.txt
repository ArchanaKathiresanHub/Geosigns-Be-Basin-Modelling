if (UNIX)
	add_compile_options(-std=c++11 -Wno-literal-suffix)
endif (UNIX)

if (BM_BUILD_4DVIEWER)
	set(OIVHOME $ENV{OIVHOME})
	set(OIVARCH $ENV{OIVARCH})

	set(OIV_INCLUDE_DIR ${OIVHOME}/include)
	set(OIV_LIB_DIR_REL ${OIVHOME}/arch-Windows-x86_64-msvc12-Release/lib)
	set(OIV_LIB_DIR_DBG ${OIVHOME}/arch-Windows-x86_64-msvc12-Debug/lib)

	add_subdirectory(4DRenderServer)
	add_subdirectory(4DViewer)
endif (BM_BUILD_4DVIEWER)