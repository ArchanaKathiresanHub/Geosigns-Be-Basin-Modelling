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

if (BM_BUILD_HYPRE)

add_external_project_to_repository(
      NAME PETSC
      VERSION ${PETSC_VERSION}_Hypre
      ARCHIVE "${THIRD_PARTY_DIR}/sources/petsc-${PETSC_VERSION}.tar.gz"
      ARCHIVE_MD5 "E21EC10AEA4111A315DF83DD398E141A"
      DEPENDS_NAME "Hypre"
      DEPENDS_VER "2.18.2"
      CONFIGURE_COMMAND
            "python3"
            "./configure"
            "--prefix={ROOT}"
            #"--with-clanguage=cxx"
            "--with-large-file-io=1"
            "--with-precision=double"
            "--with-x=0"
            "--with-petsc-arch=${CMAKE_SYSTEM_NAME}"
            "--with-ssl=0"
            "--with-fc=0"
            "--with-pthread=0"
            "--with-threadcomm=0"
            "--with-pthreadclasses=0"
            "--with-blas-lapack-dir=${BLAS_ROOT}"
            "--with-mkl_pardiso=1"
            "--with-mkl_pardiso-dir=${INTEL_MKL_ROOT}"
            "--with-hypre=1"
            "--with-hypre-dir=${HYPRE_ROOT}"

      BUILD_COMMAND   "make"
      TEST_COMMAND    "make" "check"
      INSTALL_COMMAND "make" "install"
      CONFIGURE_OPTIONS
        #COMPILER "{CurrentCompiler}"  "--with-cc=${INTEL_MPI_ROOT}/bin64/mpigcc" "--with-cxx=${INTEL_MPI_ROOT}/bin64/mpigxx"
		#COMPILER "{CurrentCompiler}"  "--with-cc=gcc" "--with-cxx=g++"
		COMPILER "{CurrentCompiler}"  "--with-cc={CC}" "--with-cxx={CXX}"
        #MPI      "{CurrentMPI}"    "--with-mpi=1"  "--with-mpi-dir=${INTEL_MPI_ROOT}"
		MPI      "{CurrentMPI}"    "--with-mpi=1" "--with-mpi-compilers=0"
        SPEED    "Release"         "--COPTFLAGS=-O3" "--CXXOPTFLAGS=-O3" "--CFLAGS=-g" "--CXXFLAGS=-g" "--with-debugging=no"
        SPEED    "Debug"           "--COPTFLAGS=-O3" "--CXXOPTFLAGS=-O3" "--CFLAGS=-g" "--CXXFLAGS=-g" "--with-debugging=no"
        SPEED    "DebugAll"        "--with-debugging"
        SPEED    "MemCheck"        "--with-debugging"
        SPEED    "CodeCoverage"    "--COPTFLAGS=-O3" "--CXXOPTFLAGS=-O3" "--CFLAGS=-g" "--CXXFLAGS=-g" "--with-debugging=no"
        OS       "{CurrentPlatform}"
        LINK     "Dynamic"    "--with-shared-libraries=1"
        LINK     "Static"     "--with-shared-libraries=0"
      YIELD_LIBRARIES "petsc"
)

# Set the path to the include directory
set(PETSC_INCLUDE_DIRS "${PETSC_ROOT}/include" ${HYPRE_INCLUDE_DIRS})
set(PETSC_LIBRARIES "petsc" ${HYPRE_LIBRARIES} ${BLAS_LIBRARIES} ${BM_DL_LIB})

else ()

add_external_project_to_repository(
      NAME PETSC
      VERSION ${PETSC_VERSION}
      ARCHIVE "${THIRD_PARTY_DIR}/sources/petsc-${PETSC_VERSION}.tar.gz"
      ARCHIVE_MD5 "E21EC10AEA4111A315DF83DD398E141A"
      CONFIGURE_COMMAND
            "python3"
            "./configure"
            "--prefix={ROOT}"
            #"--with-clanguage=cxx"
            "--with-large-file-io=1"
            "--with-precision=double"
            "--with-x=0"
            "--with-petsc-arch=${CMAKE_SYSTEM_NAME}"
            "--with-ssl=0"
            "--with-fc=0"
            "--with-pthread=0"
            "--with-threadcomm=0"
            "--with-pthreadclasses=0"
            "--with-blas-lapack-dir=${BLAS_ROOT}"
            "--with-mkl_pardiso=1"
            "--with-mkl_pardiso-dir=${INTEL_MKL_ROOT}"

      BUILD_COMMAND   "make"
      TEST_COMMAND    "make" "check"
      INSTALL_COMMAND "make" "install"
      CONFIGURE_OPTIONS
        #COMPILER "{CurrentCompiler}"  "--with-cc=${INTEL_MPI_ROOT}/bin64/mpigcc" "--with-cxx=${INTEL_MPI_ROOT}/bin64/mpigxx"
		#COMPILER "{CurrentCompiler}"  "--with-cc=gcc" "--with-cxx=g++"
		COMPILER "{CurrentCompiler}"  "--with-cc={CC}" "--with-cxx={CXX}"
        #MPI      "{CurrentMPI}"    "--with-mpi=1"  "--with-mpi-dir=${INTEL_MPI_ROOT}"
		MPI      "{CurrentMPI}"    "--with-mpi=1" "--with-mpi-compilers=0"
        SPEED    "Release"         "--COPTFLAGS=-O3" "--CXXOPTFLAGS=-O3" "--CFLAGS=-g" "--CXXFLAGS=-g" "--with-debugging=no"
        SPEED    "Debug"           "--COPTFLAGS=-O3" "--CXXOPTFLAGS=-O3" "--CFLAGS=-g" "--CXXFLAGS=-g" "--with-debugging=no"
        SPEED    "DebugAll"        "--with-debugging"
        SPEED    "MemCheck"        "--with-debugging"
        SPEED    "CodeCoverage"    "--COPTFLAGS=-O3" "--CXXOPTFLAGS=-O3" "--CFLAGS=-g" "--CXXFLAGS=-g" "--with-debugging=no"
        OS       "{CurrentPlatform}"
        LINK     "Dynamic"    "--with-shared-libraries=1"
        LINK     "Static"     "--with-shared-libraries=0"
      YIELD_LIBRARIES "petsc"
)

# Set the path to the include directory
set(PETSC_INCLUDE_DIRS "${PETSC_ROOT}/include")
set(PETSC_LIBRARIES "petsc" ${BLAS_LIBRARIES} ${BM_DL_LIB})

endif (BM_BUILD_HYPRE)


elseif (WIN32) # windows
   set(PETSC_DEBUG "${PETSC_HOME}/PETSc/debug" CACHE PATH "Debug path")
   set(PETSC_RELEASE "${PETSC_HOME}/PETSc/release" CACHE PATH "Release path")
   set(PETSC_INCLUDE_DIRS "${PETSC_HOME}/PETSc/include")
   set(PETSC_LIBRARIES
          "${BLAS_LIBRARIES}"
          "${MPI_LIBRARIES}"
   )

   if(CMAKE_BUILD_TYPE STREQUAL "Release")
      set(PETSC_INCLUDE_DIRS "${PETSC_INCLUDE_DIRS}" "${PETSC_RELEASE}/include")
      set(PETSC_LIBRARIES "${PETSC_LIBRARIES}" "${PETSC_RELEASE}/lib/libpetsc.lib")
	  set(PETSC_LIBRARIES "${PETSC_LIBRARIES}" "${HYPRE_LIBRARIES}")
   else() # Debug
      set(PETSC_INCLUDE_DIRS "${PETSC_INCLUDE_DIRS}" "${PETSC_DEBUG}/include")
      set(PETSC_LIBRARIES "${PETSC_LIBRARIES}" "${PETSC_DEBUG}/lib/libpetsc.lib")
	  set(PETSC_LIBRARIES "${PETSC_LIBRARIES}" "${HYPRE_LIBRARIES}")
   endif()

   set(PETSC_FOUND TRUE)
   set(PETSC_ROOT "${PETSC_HOME}" CACHE PATH "Installation directory of PETSc")
   #find_library(PETSC_LIBRARIES "petsc" PATHS "${PETSC_LIBRARIES}" NO_DEFAULT_PATH )

endif ()

add_external_package_info(
    CAPABILITY  PETScLib
    NAME         "PETSc"
    VENDOR       "Argonne National Laboratory"
    VERSION      "${PETSC_VERSION}"
    LICENSE_TYPE "2-clause BSD license"
    LICENSE_FILE "${THIRD_PARTY_DIR}/licenses/Petsc-3.13.1.txt"
    URL          "http://www.mcs.anl.gov/petsc/"
    DESCRIPTION  "Portable, Extensible Toolkit for Scientific Computation"
    REQUIRED_AT  "Runtime"
    COUNTRY_OF_ORIGIN "USA"
    SHIPPED      "Yes"
    INCLUSION_TYPE "Static Link"
    USEABLE_STAND_ALONE "No"
    CONTAINS_CRYPTO "No"
    ECCN         "EAR99"
)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
