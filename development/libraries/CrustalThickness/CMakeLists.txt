#########################################################################
#                                                                       #
# Copyright (C) 2012-2018 Shell International Exploration & Production. #
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
)

#
# 3. CrustalThickness library
#       Used by fastctc application
#
set( LIB_NAME "CrustalThickness" )
add_library(${LIB_NAME}
   ${all_srcs}
   ${all_headers}
)
set_target_properties( ${LIB_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${LIB_NAME}" )

################################################################
################### SET DEPENDENCIES/LINKS #####################
################################################################

bm_include_libraries(
   DataModel
   DataAccess
   DataMining
   GeoPhysics
   AbstractDerivedProperties
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

include_directories( SYSTEM
   ${HDF5_INCLUDE_DIRS}
)

target_link_libraries(${LIB_NAME}
   DataModel
   DataAccess
   DataMining
   AbstractDerivedProperties
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

install(TARGETS ${LIB_NAME}
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
)

set ( io_dataAccessMockFiles
   ../DataAccess/test/MockCrustalThicknessRiftingHistoryData.h
   ../DataAccess/test/MockCrustalThicknessData.h
)

set ( io_derivedPropertiesMockFiles
   ../DerivedProperties/test/MockDerivedSurfaceProperty.h
   ../DerivedProperties/test/MockDerivedSurfaceProperty.C
   ../DerivedProperties/test/MockGrid.h
   ../DerivedProperties/test/MockFormation.h
   ../DerivedProperties/test/MockProperty.h
   ../DerivedProperties/test/MockSurface.h
)

set ( io_dataModelMockFiles
   ../DataModel/test/MockValidator.h
   ../DataModel/test/MockValidator.C
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
add_gtest( NAME "${LIB_NAME}::ConfigFileCtc"
           SOURCES test/ConfigFileCtc.C
           LIBRARIES ${LIB_NAME} DataMining DataAccess SerialDataAccess DataModel
           ENV_VARS CTCDIR=${PROJECT_SOURCE_DIR}/geocase/misc
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
         )

add_gtest( NAME "${LIB_NAME}::InterfaceInput"
           SOURCES test/InterfaceInput.C ${io_ctcMockFiles} ${io_dataAccessMockFiles} ${io_dataModelMockFiles} ${tester_interface} ../utilities/test/GoogleTestMacros.h
           LIBRARIES ${LIB_NAME} DataMining DataAccess SerialDataAccess DataModel
           ENV_VARS CTCDIR=${PROJECT_SOURCE_DIR}/geocase/misc
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
         )

#
# 3. Calculator testing
#
add_gtest( NAME "${LIB_NAME}::Density"
           SOURCES test/Density.C ${io_ctcMockFiles}  ${io_derivedPropertiesMockFiles} ${io_dataModelMockFiles}
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../DataModel/test
           LIBRARIES ${LIB_NAME} SerialDataAccess DataMining DataAccess SerialDataAccess DataModel
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
         )

add_gtest( NAME "${LIB_NAME}::TotalTectonicSubsidence"
           SOURCES test/TotalTectonicSubsidence.C ${io_ctcMockFiles}  ${io_derivedPropertiesMockFiles} ${io_dataModelMockFiles}
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../DataModel/test
           LIBRARIES ${LIB_NAME} DataMining DataAccess SerialDataAccess DataModel
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
         )

add_gtest( NAME "${LIB_NAME}::Paleowaterdepth"
           SOURCES test/Paleowaterdepth.C ${io_ctcMockFiles}  ${io_derivedPropertiesMockFiles} ${io_dataModelMockFiles}
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../DataModel/test
           LIBRARIES ${LIB_NAME} DataMining DataAccess SerialDataAccess DataModel
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
         )

add_gtest( NAME "${LIB_NAME}::PaleowaterdepthResidual"
           SOURCES test/PaleowaterdepthResidual.C ${io_ctcMockFiles} ${io_dataModelMockFiles} ${io_dataModelMockFiles}
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../DataModel/test
           LIBRARIES ${LIB_NAME} DataMining DataAccess SerialDataAccess DataModel
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
         )

add_gtest( NAME "${LIB_NAME}::LinearFunction"
           SOURCES test/LinearFunction.C
           LIBRARIES ${LIB_NAME} DataMining DataAccess SerialDataAccess
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
         )

add_gtest( NAME "${LIB_NAME}::McKenzieEquations"
           SOURCES test/McKenzieEquations.C ${tester_McKenzie} ${io_ctcMockFiles}  ${io_derivedPropertiesMockFiles} ${io_dataModelMockFiles}
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../DataModel/test
           LIBRARIES ${LIB_NAME} DataMining DataAccess SerialDataAccess DataModel
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
         )

add_gtest( NAME "${LIB_NAME}::McKenzieCompute"
           SOURCES test/McKenzieCompute.C  ${tester_McKenzie} ${io_ctcMockFiles}  ${io_derivedPropertiesMockFiles} ${io_dataModelMockFiles}
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../DataModel/test
           LIBRARIES ${LIB_NAME} DataMining DataAccess SerialDataAccess DataModel
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
         )

#
# 4. Others testing
#

add_gtest( NAME "${LIB_NAME}::MapSmootherSerial"
           SOURCES test/MapSmootherSerial.C
           LIBRARIES ${LIB_NAME} DataMining DataAccess SerialDataAccess
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
         )

if (UNIX)
   # In LSF environment (on LSF cluster node, when build is running as a LSF job) mpirun is trying to use job settings
   # to run mpi unit tests. Sometime it fails because build job requested just 1 cpu. To prevent this we can specify
   # machines file with localhost list only
   configure_file(test/machines machines COPYONLY )
   set( MACHINE_FILE -machinefile machines)
endif (UNIX)

add_gtest( NAME "${LIB_NAME}::MapSmootherDistributed"
           SOURCES test/MapSmootherDistributed.C
           LIBRARIES ${LIB_NAME} ${PETSC_LIBRARIES} Utilities_Petsc DataMining DataAccess DistributedDataAccess
           INCLUDE_DIRS ${PETSC_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/../DistributedDataAccess/src ${CMAKE_CURRENT_SOURCE_DIR}/../DataAccess/src
           LINK_FLAGS "${PETSC_LINK_FLAGS}"
           MPI_SIZE 2
           MPIRUN_PRMS ${MACHINE_FILE}
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
         )

add_gtest( NAME "${LIB_NAME}::RiftingEvent"
           SOURCES test/RiftingEvent.C ${io_ctcMockFiles} ${io_dataModelMockFiles}
           LIBRARIES ${LIB_NAME} DataMining DataAccess SerialDataAccess
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
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
