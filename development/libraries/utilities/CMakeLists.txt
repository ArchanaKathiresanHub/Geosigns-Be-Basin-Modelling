

add_library( utilities
	src/IBSinterpolator2d.C
	src/IBSinterpolator.C
	src/mangle.C
	src/PiecewiseInterpolator.C
	src/Quadrature.C
	src/Quadrature3D.C
	src/WallTime.C
)

#generate_export_header( utilities )
#include_directories(${CMAKE_CURRENT_BINARY_DIR})

if (MSVC)
	add_definitions( "-D_USE_MATH_DEFINES" )
endif(MSVC)

install(TARGETS utilities
	RUNTIME DESTINATION bin
	LIBRARY	DESTINATION lib
	ARCHIVE DESTINATION lib
)
