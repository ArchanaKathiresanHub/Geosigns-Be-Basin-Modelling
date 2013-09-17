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

cmake_minimum_required(VERSION 2.8)
project(matrix)


# configuration

#Compiler
set( CMAKE_CXX_COMPILER "icpc" CACHE STRING "C++ compiler")
set( CMAKE_C_COMPILER "icc" CACHE STRING "C compiler")

#MPI
set( MPI_ROOT "/nfs/rvl/apps/3rdparty/intel/impi/4.1.0.030/intel64" CACHE PATH "Path to MPI library" )
#set(MPI_ROOT "/glb/home/nlwsu2/projects/apps/openmpi-1.4.5")
find_package(MPI REQUIRED)

include_directories(${MPI_INCLUDE_DIRS})
add_definitions(-DHAVE_MPI=1)

#BLAS
set( BLA_VENDOR "Intel10_64lp_seq" CACHE STRING "Choice of BLAS/LAPACK vendor. Set to 'All' if any vendor isok. Vendors are: Goto, ATLAS PhiPACK, CXML, DXML, SunPerf, SCSL, SGIMATH, IBMESSL, Intel10_32 (intel mkl v10 32 bit), Intel10_64lp (intel mkl v10 64 bit,lp thread model, lp64 model), Intel10_64lp_seq (intel mkl v10 64 bit,sequential code, lp64 model), Intel( older versions of mkl 32 and 64 bit),ACML, ACML_MP, ACML_GPU, Apple, NAS, Generic" )
set( BLAS_ROOT "/nfs/rvl/apps/3rdparty/intel/ics2013/composer_xe_2013.3.163/mkl" CACHE PATH "Path to BLAS library" )
set( BLAS_FOUND "ON" CACHE BOOL "Whether a BLAS library is available" )
set( BLAS_LINKER_FLAGS "" CACHE STRING "Linker flags required to link with BLAS" )
set( BLAS_LIBRARIES "-Wl,--start-group;/nfs/rvl/apps/3rdparty/intel/ics2013/composer_xe_2013.3.163/mkl/lib/intel64/libmkl_intel_lp64.a;/nfs/rvl/apps/3rdparty/intel/ics2013/composer_xe_2013.3.163/mkl/lib/intel64/libmkl_sequential.a;/nfs/rvl/apps/3rdparty/intel/ics2013/composer_xe_2013.3.163/mkl/lib/intel64/libmkl_core.a;-Wl,--end-group" CACHE STRING "List of libraries that have to be linked to use BLAS" )
set( BLAS95_FOUND "OFF" CACHE BOOL "Whether Fortran 95 interface for BLAS is provided" )

set( LAPACK_ROOT "/nfs/rvl/apps/3rdparty/intel/ics2013/composer_xe_2013.3.163/mkl" CACHE PATH "Path to LAPACK library" )

list(APPEND CMAKE_PREFIX_PATH ${BLAS_ROOT} ${LAPACK_ROOT})
set(BLA_STATIC ON)
find_package(BLAS)

include_directories(${BLAS_ROOT}/include)

# Cuda
find_package(CUDA)

# Compiler flags

set( CMAKE_CXX_FLAGS "-Wall")

#

enable_testing()

# building
add_executable(tslevelset
    tslevelset.C
)

target_link_libraries(tslevelset
    matrix
)

add_executable(bench
    bench.C
    gettime.C
)

target_link_libraries(bench
    matrix
    ${MPI_LIBRARIES}
)

cuda_add_library(matrix
    generalexception.C
    petscio.C
    petscmatrix.C
    petscvector.C
    diamatrix.C
    coomatrix.C
    cudacsrmatrix.C
    cudasparse.cu
    cudaexception.cu
    cudaarray.cu
    cudamatdescr.C
    cudahybmatrix.C
    ilu.C
    cudasolveanalysisinfo.C
)

target_link_libraries(matrix    
    ${BLAS_LIBRARIES}
    ${CUDA_cusparse_LIBRARY}
)

add_executable(multiply
    mklmatrix.C
    multiply.C
    gettime.C
)
target_link_libraries(multiply
    matrix
)

add_executable(diagonals
    diagonals.C
    gettime.C
)
target_link_libraries(diagonals
    matrix
)

cuda_add_executable( cusparsemv
    cudaarray.cu
    cusparsemv.cu
    gettime.C
    cudaexception.cu
)
target_link_libraries(cusparsemv
    matrix
)

# tets


add_executable(petscio.t
    petscio.t.C
    petscio.C
    generalexception.C
)

add_executable(petscmatrix.t
    petscmatrix.t.C
    petscmatrix.C
    petscio.C
    generalexception.C
)

target_link_libraries(petscmatrix.t
    ${BLAS_LIBRARIES}
)

add_executable(diamatrix.t
    diamatrix.t.C
    diamatrix.C
    generalexception.C
)

target_link_libraries(diamatrix.t
    ${BLAS_LIBRARIES}
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

add_test(NAME DIAMatrix_creationSlow
    COMMAND diamatrix.t creationSlow
)

add_test(NAME DIAMatrix_forwardIterator
    COMMAND diamatrix.t forwardIterator 
)
add_test(NAME DIAMatrix_reverseIterator
    COMMAND diamatrix.t reverseIterator
)
