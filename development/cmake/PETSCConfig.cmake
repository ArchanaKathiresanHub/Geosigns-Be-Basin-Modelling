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

if (UNIX)

add_external_project_to_repository(
      NAME PETSC
      VERSION 3.4.1
      ARCHIVE "${THIRD_PARTY_DIR}/sources/petsc-3.4.1.tar.gz"
      ARCHIVE_MD5 "45f45bd9e2af5b52c9b61ef211c18de2"
      CONFIGURE_COMMAND 
         "./configure" 
            "--prefix={ROOT}" 
            "--with-clanguage=cxx" 
            "--with-fortran=0"
            "--with-large-file-io=1"
            "--with-precision=double"
            "--with-x=0"
            "--with-blas-lapack-dir=${BLAS_ROOT}"
            "--with-petsc-arch=${CMAKE_SYSTEM_NAME}"

      BUILD_COMMAND   "make"
      INSTALL_COMMAND "make" "install"
      CONFIGURE_OPTIONS 
        COMPILER "{CurrentCompiler}"  "--with-cc={CC}" "--with-cxx={CXX}"
        MPI      "{CurrentMPI}"  "--with-mpi=1"
        SPEED    "Release"   "--COPTFLAGS=-O3" "--CXXOPTFLAGS=-O3" "--CFLAGS=-g -DPETSC_KERNEL_USE_UNROLL_2" "--CXXFLAGS=-g -DPETSC_KERNEL_USE_UNROLL_2" "--with-debugging=no"
        SPEED    "Debug"      "--COPTFLAGS=-O3" "--CXXOPTFLAGS=-O3" "--CFLAGS=-g -DPETSC_KERNEL_USE_UNROLL_2" "--CXXFLAGS=-g -DPETSC_KERNEL_USE_UNROLL_2" "--with-debugging=no"
        SPEED    "DebugAll"   "--with-debugging"
        SPEED    "MemCheck"   "--with-debugging"
        OS       "{CurrentPlatform}"     
        LINK     "Dynamic"    "--with-shared-libraries=1"
        LINK     "Static"     "--with-shared-libraries=0"
      YIELD_LIBRARIES "petsc"
)

# Set the path to the include directory
set(PETSC_INCLUDE_DIRS "${PETSC_ROOT}/include")
set(PETSC_LIBRARIES "petsc" ${BLAS_LIBRARIES})


add_external_package_info( 
    CAPABILITY  PETScLib
    NAME         "PETSc"
    VENDOR       "Argonne National Laboratory"
    VERSION      "3.4.1"
    LICENSE_TYPE "Simplified BSD"
    LICENSE_FILE "${THIRD_PARTY_DIR}/licenses/Petsc-3.4.1.txt"
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

elseif (WIN32) # windows

   set(PETSC_VERSION "3.4.4" CACHE STRING "PETSC Version")
   set(PETSC_HOME "${THIRD_PARTY_DIR}/PETSc_Windows-3.4.4" CACHE PATH "PETSc home path") 
   set(PETSC_DEBUG "${PETSC_HOME}/PETSc/c-debug_icl_mkl" CACHE PATH "Debug path")
   set(PETSC_RELEASE "${PETSC_HOME}/PETSc/c-opt_icl_mkl" CACHE PATH "Release path")
   set(PETSC_INCLUDE_DIRS "${PETSC_HOME}/PETSc/include")
   set(PETSC_LIBRARIES 
          "${BLAS_LIBRARIES}"
          "${PETSC_HOME}/PETSc/externalpackages/lib/HYPRE.lib"
          "${PETSC_HOME}/PETSc/externalpackages/lib/metis.lib"
          "${PETSC_HOME}/PETSc/externalpackages/lib/parmetis.lib"
   )

   if(CMAKE_BUILD_TYPE STREQUAL "Release")
      set(PETSC_INCLUDE_DIRS "${PETSC_INCLUDE_DIRS}" "${PETSC_RELEASE}/include")
      set(PETSC_LIBRARIES "${PETSC_LIBRARIES}" "${PETSC_RELEASE}/lib/libpetsc.lib")
   else() # Debug
      set(PETSC_INCLUDE_DIRS "${PETSC_INCLUDE_DIRS}" "${PETSC_DEBUG}/include")
      set(PETSC_LIBRARIES "${PETSC_LIBRARIES}" "${PETSC_DEBUG}/lib/libpetsc.lib")
   endif()

   set(PETSC_FOUND TRUE)
   set(PETSC_ROOT "${PETSC_HOME}" CACHE PATH "Installation directory of PETSc")
   #find_library(PETSC_LIBRARIES "petsc" PATHS "${PETSC_LIBRARIES}" NO_DEFAULT_PATH )

   # Because PETSc is is built with the static version of the Visual C Runtime
   # library and other 3rd party libs (like HDF5) are compiled with the DLL
   # version, all kinds of difficult linking problems arise. The following forces
   # linking, although everyone on the internet says its risky
   set(PETSC_LINK_FLAGS "/NODEFAULTLIB:LIBCMT /FORCE:MULTIPLE ${OpenMP_LINK_FLAGS}")

   add_external_package_info( 
          CAPABILITY  PETScLib
          NAME         "PETSc for Windows"
          VENDOR       "Microsoft Innovation Center Rapperswil"
          VERSION      "3.4.4"
          LICENSE_TYPE "Simplified BSD"
          LICENSE_FILE ""
          URL          "http://www.msic.ch/Software"
          DESCRIPTION  "Portable, Extensible Toolkit for Scientific Computation"
          REQUIRED_AT  "Runtime"
          COUNTRY_OF_ORIGIN "USA"
          SHIPPED      "Yes"
          INCLUSION_TYPE "Static Link"
          USEABLE_STAND_ALONE "No"
          CONTAINS_CRYPTO "No"
          ECCN         "EAR99"
   )                   
	

endif ()

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
