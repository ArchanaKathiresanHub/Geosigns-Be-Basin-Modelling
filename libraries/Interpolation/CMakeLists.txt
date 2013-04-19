add_library(Interpolation
	src/ApproximateCardinalFunction.C
	src/IntervalInterpolator.C
	src/Point.C
	src/PThreadsApproximateCardinalPreconditioner.C
	src/RBFGeneralOperations.C
)

bm_include_libraries(
	utilities
	ThreadUtilities
	LinearAlgebra
)

include_directories(
	${BLAS_INCLUDE_DIRS}
)

target_link_libraries(Interpolation
	${BLAS_LIBRARIES}
	utilities
	ThreadUtilities
	LinearAlgebra
)

install(TARGETS Interpolation
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

