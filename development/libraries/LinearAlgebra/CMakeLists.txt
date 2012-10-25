add_library(LinearAlgebra
	src/BiCG.C
	src/DenseMatrix.C
	src/DenseRowMatrix.C
	src/FortranMatrix.C
	src/FortranMatrixOperations.C
	src/GMRes.C
	src/GMResRestart.C
	src/LinearSolver.C
	src/Preconditioner.C
	src/PThreadsDenseRowMatrix.C
	src/RichardsonIteration.C
	src/Vector.C
)

bm_include_libraries(
	utilities
	ThreadUtilities
)

target_link_libraries(LinearAlgebra
	ThreadUtilities
	utilities
)

install(TARGETS LinearAlgebra
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

