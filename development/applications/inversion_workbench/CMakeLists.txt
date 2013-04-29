add_executable(probe
    src/probe.C
)

add_executable(adjust
    src/adjust.C
    src/project.C
    src/optionparser.C
)

include_directories(
    ${PROJECT_SOURCE_DIR}/libraries/SerialDataAccess/src/Interface
    ${PROJECT_SOURCE_DIR}/libraries
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

add_executable(parameter-space-exploration-tool 
        src/parameter-space-exploration-tool.C
        src/parameter.C
        src/case.C
        src/experiment.C
        src/project.C
        src/BasementProperty.C
	${generator_source_files}
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(parameter-space-exploration-tool
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

get_target_property(PARSPEC_EXEC parspec LOCATION)


add_custom_command( OUTPUT ${generator_source_files}
	DEPENDS parspec  ParameterSpaceExplorationTool.Spec
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMAND ${PARSPEC_EXEC} 
	ARGS -schema ParameterSpaceExplorationTool -directory ${CMAKE_CURRENT_BINARY_DIR} -input ParameterSpaceExplorationTool.Spec 
)

