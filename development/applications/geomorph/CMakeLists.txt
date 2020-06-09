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
	src/GeoMorphAllochthonousLithology.cpp
	src/GeoMorphAllochthonousLithologyDistribution.cpp
	src/AllochthonousLithologySimulator.cpp
	src/AllochthonousModellingFactory.cpp
	src/ErrorPointSet.cpp
	src/geomorph.cpp
	src/MapIntervalInterpolator.cpp
	src/MapProcessingOperations.cpp
	src/PrescribedGridPointCondenser.cpp
	src/SubsampledGridPointCondenser.cpp
)

include_directories( SYSTEM
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
