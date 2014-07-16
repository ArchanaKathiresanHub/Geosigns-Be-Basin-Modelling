#########################################################################
#                                                                       #
# Copyright (C) 2012-2013 Shell International Exploration & Production. #
# All rights reserved.                                                  #
#                                                                       #
# Developed under license for Shell by PDS BV.                          #
#                                                                       #
# Confidential and proprietary source code of Shell.                    #
# Do not distribute without written permission from Shell.              #
#                                                                       #
#########################################################################

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
	src/RichardsonIteration.C
	src/Vector.C
)

bm_include_libraries(
	utilities
)

target_link_libraries(LinearAlgebra
	utilities
)

install(TARGETS LinearAlgebra
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

