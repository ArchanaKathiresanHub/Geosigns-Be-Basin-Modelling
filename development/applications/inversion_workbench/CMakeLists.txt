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
    ${PROJECT_SOURCE_DIR}/libraries/SerialDataAccess/src/Interface
    ${PROJECT_SOURCE_DIR}/libraries
    ${PROJECT_SOURCE_DIR}/applications/xcauldron/src
    ${HDF5_serial_INCLUDE_DIRS}
    ${BLAS_INCLUDE_DIRS}
    ${PETSC_INCLUDE_DIRS}
)

bm_include_libraries(
    DataMining
    DataAccess
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
    CrustalThickness
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
	${generator_source_files}
)

set( CAULDRON_EXPLORER_LINK_FLAGS "${OpenMP_CXX_FLAGS}")
if (UNIX)
   set( CAULDRON_EXPLORER_LINK_FLAGS "${CAULDRON_EXPLORER_LINK_FLAGS} -openmp-link static")
endif()


set_target_properties(cauldronexplorer
      PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS}
                 LINK_FLAGS "${CAULDRON_EXPLORER_LINK_FLAGS}"
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
    CrustalThickness
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

########## UNIT TESTS ############
include_directories(src)

add_executable(ScalarRange
      test/ScalarRange.C
)

add_test(NAME ScalarRange.singlePass
      COMMAND ScalarRange singlePass
      )      

add_test(NAME ScalarRange.multiPass
      COMMAND ScalarRange multiPass
      )      

add_executable(MultiDimRange
      test/MultiDimRange.C
      src/MultiDimRange.C
)

add_test(NAME MultiDimRange.singlePass
      COMMAND MultiDimRange singlePass
      )      

add_test(NAME MultiDimRange.multiPass
      COMMAND MultiDimRange multiPass
      )      


add_executable(Experiment
      test/Experiment.C
      src/Scenario.C
      src/DatadrillerProperty.C
      src/experiment.C
      src/project.C
      src/ProjectResultsReader.C
      src/system.C
      src/Crust.C
    )

target_link_libraries(Experiment
    DataMining
    Interpolation
    LinearAlgebra
    FiniteElements
    GeoPhysics
    genex6_kernel
    genex6
    Interpolation
    CrustalThickness
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
)

add_test(NAME Experiment::Experiment
      COMMAND Experiment Experiment
      )      


add_executable(Crust
      src/Crust.C
      test/Crust.C
)

target_link_libraries(Crust
    utilities
)

set_target_properties(Crust
      PROPERTIES COMPILE_FLAGS "-Dprivate=public"
)

add_test(NAME Crust::overlap
   COMMAND Crust overlap
)

add_test(NAME Crust::addThinningEvent
   COMMAND Crust addThinningEvent
)

add_test(NAME Crust::getThicknessHistory
   COMMAND Crust getThicknessHistory
)

