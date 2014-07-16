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

add_library(Interpolation
	src/ApproximateCardinalFunction.C
	src/IntervalInterpolator.C
	src/Point.C
	src/RBFGeneralOperations.C
)

bm_include_libraries(
	utilities
	LinearAlgebra
)

target_link_libraries(Interpolation
	${BLAS_LIBRARIES}
	utilities
	LinearAlgebra
)

install(TARGETS Interpolation
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

