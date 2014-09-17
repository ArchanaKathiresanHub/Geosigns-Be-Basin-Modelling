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

add_executable(geomorph
	src/AllochthonousLithology.C
	src/AllochthonousLithologyDistribution.C
	src/AllochthonousLithologySimulator.C
	src/AllochthonousModellingFactory.C
	src/ErrorPointSet.C
	src/geomorph.C
	src/MapIntervalInterpolator.C
	src/MapProcessingOperations.C
	src/PrescribedGridPointCondenser.C
	src/SubsampledGridPointCondenser.C
)

bm_include_libraries(
	DataModel
	SerialDataAccess
	DataAccess
	TableIO
	CBMGenerics
	Serial_Hdf5
	utilities
	LinearAlgebra
	Interpolation
)

include_directories(
	${HDF5_INCLUDE_DIRS}
)

target_link_libraries(geomorph
	DataAccess
	SerialDataAccess
	TableIO
	CBMGenerics
	Serial_Hdf5
	utilities
	LinearAlgebra
	Interpolation
	${HDF5_LIBRARIES}
)

if (BM_USE_FLEXLM)
   include_directories(${FLEXLM_INCLUDE_DIRS})
   target_link_libraries(geomorph ${FLEXLM_LIBRARIES})
endif(BM_USE_FLEXLM)

generate_dox( src/geomorph.cfg )

install( TARGETS geomorph
	RUNTIME DESTINATION bin
)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
