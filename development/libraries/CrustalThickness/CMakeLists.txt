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
   src/ConfigFileParameterCtc.h
   src/DensityCalculator.h
   src/InterfaceInput.h
   src/InterfaceOutput.h
   src/LinearFunction.h
   src/MapSmoother.h
   src/McKenzieCrustCalculator.h
   src/PaleowaterdepthCalculator.h
   src/PaleowaterdepthResidualCalculator.h
   src/PropertyValue.h
   src/RiftingEvent.h
   src/TotalTectonicSubsidenceCalculator.h
   src/Validator.h
)

#
# 2. This list contains all implementation files except main.C
#
set( all_srcs
   src/ConfigFileParameterCtc.C
   src/DensityCalculator.C
   src/InterfaceInput.C
   src/InterfaceOutput.C
   src/LinearFunction.C
   src/MapSmoother.C
   src/McKenzieCrustCalculator.C
   src/PaleowaterdepthCalculator.C
   src/PaleowaterdepthResidualCalculator.C
   src/PropertyValue.C
   src/RiftingEvent.C
   src/TotalTectonicSubsidenceCalculator.C
   src/Validator.C
)

#
# 3. CrustalThickness library
#       Used by fastctc application
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
   ${PETSC_LIBRARIES}
)

install(TARGETS CrustalThickness
   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)

################################################################
######################## UNIT TESTS ############################
################################################################
# Most of the linked libraries in the unit tests are only required
# because of the cross dependencies between our libraries

#
# 1. Set mock files
#
set ( io_ctcMockFiles
   test/MockConfigFileParameterCtc.h
   test/MockInterfaceInput.h
   test/MockInterfaceOutput.h
   test/MockInterfaceOutput.C
   test/MockValidator.h
   test/MockValidator.C
)

set ( io_dataAccessMockFiles
   ../DataAccess/test/MockCrustalThicknessRiftingHistoryData.h
   ../DataAccess/test/MockCrustalThicknessData.h
)

set ( io_derivedPropertiesMockFiles
   ../DerivedProperties/test/MockDerivedSurfaceProperty.h
   ../DerivedProperties/test/MockDerivedSurfaceProperty.C
)

set ( tester_interface
   test/InterfaceInputTester.h
   test/InterfaceInputTester.C
)

set ( tester_McKenzie
   test/McKenzieTester.h
   test/McKenzieTester.C
)

#
# 2. Inputs testing
#
add_gtest( NAME "CrustalThickness::ConfigFileCtc"
           SOURCES test/ConfigFileCtc.C 
           LIBRARIES CrustalThickness DataMining DataAccess SerialDataAccess
           ENV_VARS CTCDIR=${PROJECT_SOURCE_DIR}/geocase/misc
         )

add_gtest( NAME "CrustalThickness::InterfaceInput"
           SOURCES test/InterfaceInput.C ${io_ctcMockFiles} ${io_dataAccessMockFiles} ${tester_interface} ../utilities/test/GoogleTestMacros.h
           LIBRARIES CrustalThickness DataMining DataAccess SerialDataAccess
           ENV_VARS CTCDIR=${PROJECT_SOURCE_DIR}/geocase/misc
         )

#
# 3. Calculator testing
#
add_gtest( NAME "CrustalThickness::Density"
           SOURCES test/Density.C ${io_ctcMockFiles}  ${io_derivedPropertiesMockFiles}
           LIBRARIES CrustalThickness SerialDataAccess DataMining DataAccess SerialDataAccess
         )

add_gtest( NAME "CrustalThickness::TotalTectonicSubsidence"
           SOURCES test/TotalTectonicSubsidence.C ${io_ctcMockFiles}  ${io_derivedPropertiesMockFiles}
           LIBRARIES CrustalThickness DataMining DataAccess SerialDataAccess
         )

add_gtest( NAME "CrustalThickness::Paleowaterdepth"
           SOURCES test/Paleowaterdepth.C ${io_ctcMockFiles}  ${io_derivedPropertiesMockFiles}
           LIBRARIES CrustalThickness DataMining DataAccess SerialDataAccess
         )

add_gtest( NAME "CrustalThickness::PaleowaterdepthResidual"
           SOURCES test/PaleowaterdepthResidual.C ${io_ctcMockFiles}
           LIBRARIES CrustalThickness DataMining DataAccess SerialDataAccess
         )

add_gtest( NAME "CrustalThickness::LinearFunction"
           SOURCES test/LinearFunction.C 
           LIBRARIES CrustalThickness DataMining DataAccess SerialDataAccess
         )

add_gtest( NAME "CrustalThickness::McKenzieEquations"
           SOURCES test/McKenzieEquations.C ${tester_McKenzie} ${io_ctcMockFiles}  ${io_derivedPropertiesMockFiles}
           LIBRARIES CrustalThickness DataMining DataAccess SerialDataAccess
         )

add_gtest( NAME "CrustalThickness::McKenzieCompute"
           SOURCES test/McKenzieCompute.C  ${tester_McKenzie} ${io_ctcMockFiles}  ${io_derivedPropertiesMockFiles}
           LIBRARIES CrustalThickness DataMining DataAccess SerialDataAccess
         )

#
# 4. Others testing
#
add_gtest( NAME "CrustalThickness::MapSmoother"
           SOURCES test/MapSmoother.C
           LIBRARIES CrustalThickness DataMining DataAccess SerialDataAccess
         )


add_gtest( NAME "CrustalThickness::RiftingEvent"
           SOURCES test/RiftingEvent.C ${io_ctcMockFiles}
           LIBRARIES CrustalThickness DataMining DataAccess SerialDataAccess
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
