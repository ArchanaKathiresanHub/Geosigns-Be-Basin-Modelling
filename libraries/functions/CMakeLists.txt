add_library(functions
	src/MonotonicIncreasingPiecewiseLinearInvertableFunction.C
        src/enableGDBBreak.C
)


install(TARGETS functions
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

