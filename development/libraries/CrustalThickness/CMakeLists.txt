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

set( LIB_NAME "CrustalThickness" )

include_directories( SYSTEM
   ${HDF5_INCLUDE_DIRS}
)

create_bm_library( TARGET ${LIB_NAME}
                   LIBRARIES DataMining
				             cmbapi
                             DataAccess
                             AbstractDerivedProperties
                             DerivedProperties
                             TableIO
                             EosPack
                             CBMGenerics
                             FileSystem
                             utilities
                             DataModel
                             Utilities_Petsc
                             ${HDF5_LIBRARIES}
                             ${PETSC_LIBRARIES}
                   INSTALLTARGET )

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
   test/MockInterfaceOutput.cpp
)

set ( io_dataAccessMockFiles
   ../DataAccess/test/MockCrustalThicknessRiftingHistoryData.h
   ../DataAccess/test/MockCrustalThicknessData.h
)

set ( io_derivedPropertiesMockFiles
   ../DerivedProperties/test/MockDerivedSurfaceProperty.h
   ../DerivedProperties/test/MockDerivedSurfaceProperty.cpp
   ../DerivedProperties/test/MockGrid.h
   ../DerivedProperties/test/MockFormation.h
   ../DerivedProperties/test/MockProperty.h
   ../DerivedProperties/test/MockSurface.h
)

set ( io_dataModelMockFiles
   ../DataModel/test/MockValidator.h
   ../DataModel/test/MockValidator.cpp
)

set ( tester_interface
   test/InterfaceInputTester.h
   test/InterfaceInputTester.cpp
)

set ( tester_McKenzie
   test/McKenzieTester.h
   test/McKenzieTester.cpp
)

#
# 2. Inputs testing
#
add_gtest( NAME "${LIB_NAME}::ConfigFileCtc"
           SOURCES test/ConfigFileCtc.cpp
           LIBRARIES ${LIB_NAME} DataMining DataAccess SerialDataAccess DataModel
           ENV_VARS CTCDIR=${PROJECT_SOURCE_DIR}/geocase/misc
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
         )

add_gtest( NAME "${LIB_NAME}::InterfaceInput"
           SOURCES test/InterfaceInput.cpp ${io_ctcMockFiles} ${io_dataAccessMockFiles} ${io_dataModelMockFiles} ${tester_interface} ../utilities/test/GoogleTestMacros.h
           LIBRARIES ${LIB_NAME} DataMining DataAccess SerialDataAccess DataModel
           ENV_VARS CTCDIR=${PROJECT_SOURCE_DIR}/geocase/misc
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
         )

#
# 3. Calculator testing
#
add_gtest( NAME "${LIB_NAME}::Density"
           SOURCES test/Density.cpp ${io_ctcMockFiles}  ${io_derivedPropertiesMockFiles} ${io_dataModelMockFiles}
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../DataModel/test
           LIBRARIES ${LIB_NAME} SerialDataAccess DataMining DataAccess SerialDataAccess DataModel
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
         )

add_gtest( NAME "${LIB_NAME}::TotTectSubsidence"
           SOURCES test/TotalTectonicSubsidence.cpp ${io_ctcMockFiles}  ${io_derivedPropertiesMockFiles} ${io_dataModelMockFiles}
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../DataModel/test
           LIBRARIES ${LIB_NAME} DataMining DataAccess SerialDataAccess DataModel
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
         )

add_gtest( NAME "${LIB_NAME}::Paleowaterdepth"
           SOURCES test/Paleowaterdepth.cpp ${io_ctcMockFiles}  ${io_derivedPropertiesMockFiles} ${io_dataModelMockFiles}
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../DataModel/test
           LIBRARIES ${LIB_NAME} DataMining DataAccess SerialDataAccess DataModel
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
         )

add_gtest( NAME "${LIB_NAME}::PWDResidual"
           SOURCES test/PaleowaterdepthResidual.cpp ${io_ctcMockFiles} ${io_dataModelMockFiles} ${io_dataModelMockFiles}
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../DataModel/test
           LIBRARIES ${LIB_NAME} DataMining DataAccess SerialDataAccess DataModel
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
         )

add_gtest( NAME "${LIB_NAME}::LinearFunction"
           SOURCES test/LinearFunction.cpp
           LIBRARIES ${LIB_NAME} DataMining DataAccess SerialDataAccess
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
         )

add_gtest( NAME "${LIB_NAME}::McKenzieEquations"
           SOURCES test/McKenzieEquations.cpp ${tester_McKenzie} ${io_ctcMockFiles}  ${io_derivedPropertiesMockFiles} ${io_dataModelMockFiles}
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../DataModel/test
           LIBRARIES ${LIB_NAME} DataMining DataAccess SerialDataAccess DataModel
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
         )

add_gtest( NAME "${LIB_NAME}::McKenzieCompute"
           SOURCES test/McKenzieCompute.cpp  ${tester_McKenzie} ${io_ctcMockFiles}  ${io_derivedPropertiesMockFiles} ${io_dataModelMockFiles}
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../DataModel/test
           LIBRARIES ${LIB_NAME} DataMining DataAccess SerialDataAccess DataModel
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
         )

#
# 4. Others testing
#

add_gtest( NAME "${LIB_NAME}::MapSmootherSerial"
           SOURCES test/MapSmootherSerial.cpp
           LIBRARIES ${LIB_NAME} DataMining DataAccess SerialDataAccess
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
         )

if (UNIX)
   # In LSF environment (on LSF cluster node, when build is running as a LSF job) mpirun is trying to use job settings
   # to run mpi unit tests. Sometime it fails because build job requested just 1 cpu. To prevent this we can specify
   # machines file with localhost list only
   copy_test_file(machines)
   set( MACHINE_FILE -machinefile machines)
endif (UNIX)

add_gtest( NAME "${LIB_NAME}::MSDistributed"
           SOURCES test/MapSmootherDistributed.cpp
           LIBRARIES ${LIB_NAME} ${PETSC_LIBRARIES} Utilities_Petsc DataMining DataAccess DistributedDataAccess
           INCLUDE_DIRS ${PETSC_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/../DistributedDataAccess/src ${CMAKE_CURRENT_SOURCE_DIR}/../DataAccess/src
           LINK_FLAGS "${PETSC_LINK_FLAGS}"
           MPI_SIZE 2
           MPIRUN_PRMS ${MACHINE_FILE}
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
         )

add_gtest( NAME "${LIB_NAME}::RiftingEvent"
           SOURCES test/RiftingEvent.cpp ${io_ctcMockFiles} ${io_dataModelMockFiles}
           LIBRARIES ${LIB_NAME} DataMining DataAccess SerialDataAccess
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
         )


################################################################
########################### DOXYGEN ############################
################################################################

generate_dox(CrustalThickness.cfg)
