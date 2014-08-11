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

add_subdirectory(TableIO)
add_subdirectory(utilities)
add_subdirectory(DataModel)
add_subdirectory(CBMGenerics)
add_subdirectory(Serial_Hdf5)
add_subdirectory(EosPack)
add_subdirectory(DataAccess)
add_subdirectory(DataMining)
add_subdirectory(SerialDataAccess)
add_subdirectory(GeoPhysics)
add_subdirectory(casaAPI)
add_subdirectory(cmbAPI)
add_subdirectory(FiniteElements)
add_subdirectory(Interpolation)
add_subdirectory(LinearAlgebra)

if(UNIX)
	# Libraries that can only be built on Unix at the moment.
	add_subdirectory(calibration_kernel)
	add_subdirectory(Utilities_Petsc)
	add_subdirectory(Parallel_Hdf5)
	add_subdirectory(DerivedProperties)
	add_subdirectory(DistributedDataAccess)
	add_subdirectory(genex5_kernel)
	add_subdirectory(genex6)
	add_subdirectory(OTGC_kernel6)
	add_subdirectory(genex6_kernel)
	add_subdirectory(OTGC_kernel)
	add_subdirectory(CrustalThickness)
	add_subdirectory(functions)
        add_subdirectory(ParaViewPlugin)
        add_subdirectory(OneFilePerProcess)
else(UNIX)
	add_subdirectory(CSharpAPI)
endif(UNIX)
