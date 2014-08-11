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

add_executable(probe
    src/probe.C
    src/ProjectResultsReader.C
)

add_executable(adjust
    src/adjust.C
    src/project.C
    src/optionparser.C
    src/Crust.C
)

include_directories(
    ${SUMPP_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/applications/xcauldron/src
    ${HDF5_INCLUDE_DIRS}
    ${PETSC_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/libraries/DataModel/src
)

bm_include_libraries(
    DataMining
    DataAccess
    SerialDataAccess
    TableIO
    Interpolation
    LinearAlgebra
    GeoPhysics
    utilities
    CBMGenerics
    EosPack
)

target_link_libraries(probe
    DataMining
    Interpolation
    LinearAlgebra
    FiniteElements
    GeoPhysics
    genex6_kernel
    genex6
    Interpolation
    LinearAlgebra
    OTGC_kernel6
    DataAccess
    SerialDataAccess
    TableIO
    CBMGenerics
    Serial_Hdf5
    utilities
    EosPack
)

target_link_libraries(adjust
    DataAccess
    SerialDataAccess
    TableIO
    CBMGenerics
    Serial_Hdf5
    utilities
    EosPack
)

set (generator_source_files
  ${CMAKE_CURRENT_BINARY_DIR}/parameterspaceexplorationtool.C 
  ${CMAKE_CURRENT_BINARY_DIR}/parameterspaceexplorationtoolfuncs.C
  ${CMAKE_CURRENT_BINARY_DIR}/parameterspaceexplorationtool.h
  ${CMAKE_CURRENT_BINARY_DIR}/parameterspaceexplorationtoolfuncs.h
  )

add_executable(cauldronexplorer
        src/BasementProperty.C
        src/BasementParameter.C
        src/InitialCrustalThicknessProperty.C
        src/InitialCrustalThicknessParameter.C
        src/Scenario.C
        src/MultiDimRange.C
        src/Crust.C
        src/CrustalThinningParameter.C
        src/CrustalThinningProperty.C
        src/UnconformityLithologyProperty.C
        src/UnconformityLithologyParameter.C
        src/UnconformityProperty.C
        src/UnconformityParameter.C
        src/DatadrillerProperty.C
        src/experiment.C
        src/cauldronexplorer.C
        src/project.C
        src/ProjectResultsReader.C
        src/system.C
        src/SurfaceTempProperty.C
        src/SurfaceTempParameter.C
        src/ExperimentResultsTablePrinter.C
        src/ExperimentResultsTable.C
	${generator_source_files}
)

set( CAULDRON_EXPLORER_LINK_FLAGS "${OpenMP_CXX_FLAGS} ${OpenMP_LINK_FLAGS}")

set_target_properties( cauldronexplorer
      PROPERTIES COMPILE_FLAGS "${OpenMP_CXX_FLAGS}" LINK_FLAGS "${CAULDRON_EXPLORER_LINK_FLAGS}"
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(cauldronexplorer
    DataMining
    Interpolation
    LinearAlgebra
    FiniteElements
    GeoPhysics
    genex6_kernel
    genex6
    Interpolation
    LinearAlgebra
    OTGC_kernel6
    DataAccess
    SerialDataAccess
    TableIO
    CBMGenerics
    Serial_Hdf5
    utilities
    EosPack
    projectdependencies
    SUMlib
)

get_target_property(PARSPEC_EXEC parspec LOCATION)


add_custom_command( OUTPUT ${generator_source_files}
	DEPENDS parspec  ParameterSpaceExplorationTool.Spec
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMAND ${PARSPEC_EXEC} 
	ARGS -schema ParameterSpaceExplorationTool -directory ${CMAKE_CURRENT_BINARY_DIR} -input ParameterSpaceExplorationTool.Spec 
)

install(TARGETS cauldronexplorer
        RUNTIME DESTINATION bin
)

install(TARGETS adjust
        RUNTIME DESTINATION bin
)

########## UNIT TESTS ############
include_directories(src)
add_gtest(NAME ScalarRange
          SOURCES test/ScalarRange.C
)

add_gtest(NAME MultiDimRange
          SOURCES test/ScalarRange.C
)

add_gtest(NAME ExperimentResultsTablePrinter
          SOURCES src/ExperimentResultsTablePrinter.C
                  test/ExperimentResultsTablePrinter.C
)

add_gtest(NAME ExperimentResultsTable
          SOURCES src/ExperimentResultsTable.C
                  src/ExperimentResultsTablePrinter.C
                  test/ExperimentResultsTable.C
)


add_gtest(NAME Experiment
          SOURCES test/Experiment.C
                  src/Scenario.C
                  src/DatadrillerProperty.C
                  src/experiment.C
                  src/project.C
                  src/ProjectResultsReader.C
                  src/system.C
                  src/Crust.C
                  src/ExperimentResultsTablePrinter.C
                  src/ExperimentResultsTable.C
          LIBRARIES
                   DataMining
                   Interpolation
                   LinearAlgebra
                   FiniteElements
                   GeoPhysics
                   genex6_kernel
                   genex6
                   Interpolation
                   LinearAlgebra
                   OTGC_kernel6
                   DataAccess
                   SerialDataAccess
                   TableIO
                   CBMGenerics
                   Serial_Hdf5
                   utilities
                   EosPack
                   SUMlib
                   projectdependencies
)

add_gtest(NAME SUMPPDoE
          SOURCES test/SUMPPDoE.C
                  src/Scenario.C
                  src/DatadrillerProperty.C
                  src/experiment.C
                  src/project.C
                  src/ProjectResultsReader.C
                  src/system.C
                  src/Crust.C
                  src/ExperimentResultsTablePrinter.C
                  src/ExperimentResultsTable.C
          LIBRARIES
                   DataMining
                   Interpolation
                   LinearAlgebra
                   FiniteElements
                   GeoPhysics
                   genex6_kernel
                   genex6
                   Interpolation
                   LinearAlgebra
                   OTGC_kernel6
                   DataAccess
                   SerialDataAccess
                   TableIO
                   CBMGenerics
                   Serial_Hdf5
                   utilities
                   EosPack
                   SUMlib
                   projectdependencies
)

add_gtest(NAME Crust 
          SOURCES test/Crust.C
                  src/Crust.C
          LIBRARIES utilities
)

