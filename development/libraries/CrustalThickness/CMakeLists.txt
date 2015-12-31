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

if (NOT BM_PARALLEL)
	return()
endif()


set( all_headers
   src/DensityCalculator.h
   src/InterfaceDefs.h
	src/InterfaceInput.h
	src/InterfaceOutput.h
   src/LinearFunction.h
)

set( all_srcs
	src/DensityCalculator.C
	src/InterfaceInput.C
	src/InterfaceOutput.C
   src/LinearFunction.C
)

add_library(CrustalThickness
  ${all_srcs}
  ${all_headers}
)

bm_include_libraries(
	DataModel
	DataAccess
	DerivedProperties
	GeoPhysics
	EosPack	
	CBMGenerics
	utilities
	Interpolation
	LinearAlgebra
	Utilities_Petsc
	TableIO
	Serial_Hdf5
)

remove_definitions(-fPIC)

include_directories(
	${HDF5_INCLUDE_DIRS}
)

target_link_libraries(CrustalThickness
	DistributedDataAccess
	DataAccess
	DerivedProperties	
	EosPack	
	CBMGenerics
	utilities
	TableIO
	Utilities_Petsc
	Serial_Hdf5
	${HDF5_LIBRARIES}
) 

install(TARGETS CrustalThickness
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

generate_dox(CrustalThickness.cfg)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
