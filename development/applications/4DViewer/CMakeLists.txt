if (UNIX)
  add_compile_options(-std=c++11 -Wno-literal-suffix)
endif (UNIX)

set(OIVHOME $ENV{OIVHOME})
set(OIVARCH $ENV{OIVARCH})

if(DEFINED ENV{OIV_INCLUDE_DIR})
  set(OIV_INCLUDE_DIR $ENV{OIV_INCLUDE_DIR})
else()
  set(OIV_INCLUDE_DIR ${OIVHOME}/include)
endif()

set(OIV_LIB_DIR_REL ${OIVHOME}/${OIVARCH}/lib)
string(REGEX REPLACE Release Debug OIV_LIB_DIR_DBG ${OIV_LIB_DIR_REL})

add_subdirectory(4DRenderServer)
add_subdirectory(4DViewer)

