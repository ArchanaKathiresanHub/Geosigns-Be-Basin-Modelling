#########################################################################
#                                                                       #
# Copyright (C) 2012-2016 Shell International Exploration & Production. #
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

################################################################
######################### SET TARGETS ##########################
################################################################

#
# 1. This list contains all header files
#
set( all_headers
   src/AbstractInterfaceOutput.h
   src/AbstractValidator.h
   src/DensityCalculator.h
   src/InterfaceDefs.h
	src/InterfaceInput.h
	src/InterfaceOutput.h
   src/LinearFunction.h
   src/MapSmoother.h
   src/PaleowaterdepthCalculator.h
   src/PaleowaterdepthResidualCalculator.h
   src/PropertyValue.h
   src/TotalTectonicSubsidenceCalculator.h
   src/Validator.h
)

#
# 2. This list contains all implementation files except main.C
#
set( all_srcs
	src/DensityCalculator.C
	src/InterfaceInput.C
	src/InterfaceOutput.C
   src/LinearFunction.C
   src/MapSmoother.C
   src/PaleowaterdepthCalculator.C
   src/PaleowaterdepthResidualCalculator.C
   src/PropertyValue.C
   src/TotalTectonicSubsidenceCalculator.C
   src/Validator.C
)

#
# 3. CrustalThickness library
#       Used byt fastctc application
#
add_library(CrustalThickness
  ${all_srcs}
  ${all_headers}
)

################################################################
################### SET DEPENDENCIES/LINKS #####################
################################################################

bm_include_libraries(
	DataModel
	DataAccess
   DataMining
	GeoPhysics
   DerivedProperties
	EosPack	
	CBMGenerics
   FileSystem
	utilities
	Interpolation
	LinearAlgebra
	Utilities_Petsc
	TableIO
	Serial_Hdf5
)

#What is that?
remove_definitions(-fPIC)

include_directories(
	${HDF5_INCLUDE_DIRS}
)

target_link_libraries(CrustalThickness
	DistributedDataAccess
	DataAccess
   DataMining
	DerivedProperties	
	EosPack	
	CBMGenerics
   FileSystem
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

################################################################
######################## UNIT TESTS ############################
################################################################

add_gtest( NAME "CrustalThickness::Density"
           SOURCES test/Density.C test/MockInterfaceOutput.h test/MockValidator.h test/MockInterfaceOutput.C test/MockValidator.C ../DerivedProperties/test/MockDerivedSurfaceProperty.h ../DerivedProperties/test/MockDerivedSurfaceProperty.C
           LIBRARIES CrustalThickness
         )

add_gtest( NAME "CrustalThickness::TotalTectonicSubsidence"
           SOURCES test/TotalTectonicSubsidence.C test/MockInterfaceOutput.h test/MockInterfaceOutput.C test/MockValidator.h test/MockValidator.C ../DerivedProperties/test/MockDerivedSurfaceProperty.h ../DerivedProperties/test/MockDerivedSurfaceProperty.C
           LIBRARIES CrustalThickness SerialDataAccess DataAccess
         )

add_gtest( NAME "CrustalThickness::Paleowaterdepth"
           SOURCES test/Paleowaterdepth.C test/MockInterfaceOutput.h test/MockInterfaceOutput.C test/MockValidator.h test/MockValidator.C ../DerivedProperties/test/MockDerivedSurfaceProperty.h ../DerivedProperties/test/MockDerivedSurfaceProperty.C
           LIBRARIES CrustalThickness SerialDataAccess DataAccess
         )

add_gtest( NAME "CrustalThickness::PaleowaterdepthResidual"
           SOURCES test/PaleowaterdepthResidual.C test/MockInterfaceOutput.h test/MockInterfaceOutput.C test/MockValidator.h test/MockValidator.C
           LIBRARIES CrustalThickness SerialDataAccess DataAccess
         )

add_gtest( NAME "CrustalThickness::MapSmoother"
           SOURCES test/MapSmoother.C
           LIBRARIES CrustalThickness SerialDataAccess DataAccess
         )

################################################################
########################### DOXYGEN ############################
################################################################

generate_dox(CrustalThickness.cfg)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
