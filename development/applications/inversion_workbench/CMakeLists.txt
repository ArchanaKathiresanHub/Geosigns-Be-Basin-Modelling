add_executable(probe
    src/probe.C
    src/generalexception.C
)

add_executable(adjust
    src/adjust.C
    src/project.C
    src/optionparser.C
    src/generalexception.C
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

