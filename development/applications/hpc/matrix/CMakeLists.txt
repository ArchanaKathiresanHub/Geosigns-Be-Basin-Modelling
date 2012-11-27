cmake_minimum_required(VERSION 2.8)
project(matrix)


# configuration
set( BLA_VENDOR "Intel10_64lp_seq" CACHE STRING "Choice of BLAS/LAPACK vendor. Set to 'All' if any vendor isok. Vendors are: Goto, ATLAS PhiPACK, CXML, DXML, SunPerf, SCSL, SGIMATH, IBMESSL, Intel10_32 (intel mkl v10 32 bit), Intel10_64lp (intel mkl v10 64 bit,lp thread model, lp64 model), Intel10_64lp_seq (intel mkl v10 64 bit,sequential code, lp64 model), Intel( older versions of mkl 32 and 64 bit),ACML, ACML_MP, ACML_GPU, Apple, NAS, Generic" )
set( BLAS_ROOT "/nfs/rvl/apps/3rdparty/intel/ics2011/composerxe-2011.3.174/mkl" CACHE PATH "Path to BLAS library" )
set( BLAS_FOUND "ON" CACHE BOOL "Whether a BLAS library is available" )
set( BLAS_LINKER_FLAGS "" CACHE STRING "Linker flags required to link with BLAS" )
set( BLAS_LIBRARIES "-Wl,--start-group;/nfs/rvl/apps/3rdparty/intel/ics2011/composerxe-2011.3.174/mkl/lib/intel64/libmkl_intel_lp64.a;/nfs/rvl/apps/3rdparty/intel/ics2011/composerxe-2011.3.174/mkl/lib/intel64/libmkl_sequential.a;/nfs/rvl/apps/3rdparty/intel/ics2011/composerxe-2011.3.174/mkl/lib/intel64/libmkl_core.a;-Wl,--end-group" CACHE STRING "List of libraries that have to be linked to use BLAS" )
set( BLAS95_FOUND "OFF" CACHE BOOL "Whether Fortran 95 interface for BLAS is provided" )

set( LAPACK_ROOT "/nfs/rvl/apps/3rdparty/intel/ics2011/composerxe-2011.3.174/mkl" CACHE PATH "Path to LAPACK library" )

list(APPEND CMAKE_PREFIX_PATH ${BLAS_ROOT} ${LAPACK_ROOT})
set(BLA_STATIC ON)
find_package(BLAS)

include_directories(${BLAS_ROOT}/include)

#

set( CMAKE_CXX_FLAGS "-Wall")

#

enable_testing()

# building
add_executable(multiply
    petscio.C
    generalexception.C
    petscmatrix.C
    petscvector.C
    mklmatrix.C
    multiply.C
    gettime.C
)
target_link_libraries(multiply
    ${BLAS_LIBRARIES}
)

add_executable(petscio.t
    petscio.t.C
    petscio.C
    generalexception.C
)

set_target_properties(petscio.t
    PROPERTIES COMPILE_FLAGS "-fno-access-control"
)

add_executable(petscmatrix.t
    petscmatrix.t.C
    petscmatrix.C
    petscio.C
    generalexception.C
)

set_target_properties(petscmatrix.t
    PROPERTIES COMPILE_FLAGS "-fno-access-control"
)

add_test(NAME petscio::convertHostToNetwork
    COMMAND petscio.t convertHostToNetwork
)

add_test(NAME petscio::convertNetworkToHost
    COMMAND petscio.t convertNetworkToHost
)

add_test(NAME petscio::readIndices
    COMMAND petscio.t readIndices
)

add_test(NAME petscio::readReal
    COMMAND petscio.t readReal
)

add_test(NAME petscio::writeReal
    COMMAND petscio.t writeReal
)

add_test(NAME petscio::writeIndices
    COMMAND petscio.t writeIndices
)

add_test(NAME PetscMatrix::emptyMatrixConstruction
    COMMAND petscmatrix.t emptyMatrixConstruction
)

add_test(NAME PetscMatrix::load
    COMMAND petscmatrix.t load
)

add_test(NAME PetscMatrix::save
    COMMAND petscmatrix.t save
)

add_test(NAME PetscMatrix::iterator
    COMMAND petscmatrix.t iterator
)

add_test(NAME PetscMatrix::insert
    COMMAND petscmatrix.t insert
)
