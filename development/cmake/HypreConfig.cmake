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

include(cmake/AddPackage.cmake)

if (UNIX)

add_external_project_to_repository(
      NAME Hypre
      VERSION 2.9.1
      ARCHIVE "${THIRD_PARTY_DIR}/sources/hypre-2.9.1a.tar.gz"
      ARCHIVE_MD5 "e1e9bb31db0c5863a83dc10828daa60d"
      CONFIGURE_COMMAND 
         "./configure" 
            "--prefix={ROOT}" 
            "CC={CC}" 
            "CXX={CXX}" 
            "--with-blas=yes"
            "--with-blas-libs=mkl_intel_lp64\ mkl_core\ mkl_sequential"
            "--with-blas-lib-dirs=${INTEL_MKL_ROOT}/lib/intel64"
            "--with-lapack=yes"
            "--with-lapack-libs=" 
            "--with-lapack-lib-dirs=" 
            "--with-fmangle-blas=one-underscore" 
            "--with-fmangle-lapack=one-underscore" 
            "--without-babel" 
            "--without-mli" 
            "--without-fei" 
            "--with-superlu" 
            "--enable-fortran=no"
            "--enable-shared"

      BUILD_COMMAND   "make"
      TEST_COMMAND    "make" "test"
      INSTALL_COMMAND "make" "install"
      CONFIGURE_OPTIONS 
        COMPILER "{CurrentCompiler}" 
        MPI      "{CurrentMPI}"      "--with-MPI"
        SPEED    "Release"           "CFLAGS=-g -O3" "CXXFLAGS=-g -O3"
        SPEED    "Debug"             "CFLAGS=-g -O3" "CXXFLAGS=-g -O3" "--enable-debug"
        SPEED    "DebugAll"          "--enable-debug"
        SPEED    "MemCheck"          
        SPEED    "CodeCoverage"      "CFLAGS=-g -O3" "CXXFLAGS=-g -O3"
        OS       "{CurrentPlatform}" 
        LINK     "Dynamic"           
        LINK     "Static"            
      YIELD_LIBRARIES "HYPRE"
)


# Set the path to the include directory
set( HYPRE_ROOT "${Hypre_ROOT}" CACHE PATH "Path to HYPRE library" )
set( Hypre_FOUND TRUE )
set( HYPRE_INCLUDE_DIRS "${HYPRE_ROOT}/include" )
set( HYPRE_LIBRARY_DIR "${HYPRE_ROOT}/lib" )
set( HYPRE_LIBRARIES "${HYPRE_LIBRARY_DIR}/libHYPRE-2.9.1a.so" )


add_external_package_info( 
    CAPABILITY  HypreLib
    NAME         "Hypre"
    VENDOR       "Lawrence Livermore National Laboratory"
    VERSION      "2.9.1"
    LICENSE_TYPE "LGPL 2.1"
    LICENSE_FILE "${THIRD_PARTY_DIR}/licenses/Hypre-2.9.1a.txt"
    URL          "http://computation.llnl.gov/projects/hypre-scalable-linear-solvers-multigrid-methods"
    DESCRIPTION  "Library of linear solvers"
    REQUIRED_AT  "Runtime"
    COUNTRY_OF_ORIGIN "USA"
    SHIPPED      "Yes"
    INCLUSION_TYPE "Dynamic Link"
    USEABLE_STAND_ALONE "No"
    CONTAINS_CRYPTO "No"
    ECCN         "EAR99"
)                   

elseif (WIN32) # windows
# PETSc is already built with Hypre for Windows (though statically linked, which is not license-compliant)
endif ()

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
