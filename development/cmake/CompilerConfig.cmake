#########################################################################
#                                                                       #
# Copyright (C) 2012-2014 Shell International Exploration & Production. #
# All rights reserved.                                                  #
#                                                                       #
# Developed under license for Shell by PDS BV.                          #
#                                                                       #
# Confidential and proprietary source code of Shell.                    #
# Do not distribute without written permission from Shell.              #
#                                                                       #
#########################################################################

include(cmake/AddPackage.cmake)
include(cmake/EnvSetup.cmake)

set(INTEL_CXX_ROOT "INTEL_CXX_ROOT-NOTFOUND" CACHE PATH "Path to Intel's compiler collection")
set(INTEL_MPI_ROOT "INTEL_MPI_ROOT-NOTFOUND" CACHE PATH "Path to Intel MPI library" )
set(INTEL_MPI_FLAVOUR "opt" CACHE STRING "Intel MPI library type. Choose from: opt, opt_mt, dbg, dbg_mt, log, log_mt" ) 

option(BM_USE_INTEL_COMPILER "Whether to use the Intel compiler (UNIX only)" OFF)
option(BM_USE_INTEL_MPI "Whether to use the Intel MPI (UNIX only)" OFF)
   
if (UNIX) 
   #
   # Set the compiler on Unix to Intel if enabled
   #

   if (BM_USE_INTEL_COMPILER)

      # Add environment set-up scripts to generated script
      add_environment_source_script(CSHELL "${INTEL_CXX_ROOT}/bin/compilervars.csh intel64")
      add_environment_source_script(BOURNE "${INTEL_CXX_ROOT}/bin/compilervars.sh intel64")

      add_environment_source_script_to_wrapper( cc "${INTEL_CXX_ROOT}/bin/compilervars.sh intel64")
      add_environment_source_script_to_wrapper( cxx "${INTEL_CXX_ROOT}/bin/compilervars.sh intel64")

      # Add package info
      add_external_package_info(
          CAPABILITY   Compiler
          NAME         "Compiler"
          VENDOR       "Intel"
          VERSION      "13.1.3 20130607"
          LICENSE_TYPE "Commercial"
          LICENSE_FILE "${INTEL_CXX_ROOT}/Documentation/en_US/clicense"
          URL          "http://software.intel.com/en-us/intel-compilers"
          DESCRIPTION  "Intel Compilers"
          REQUIRED_AT  "Build"
          COUNTRY_OF_ORIGIN "USA"
          SHIPPED      "No"
          INCLUSION_TYPE "NA"
          USEABLE_STAND_ALONE "No"
          CONTAINS_CRYPTO "Unknown"
          ECCN         "Unknown"
      )

   endif(BM_USE_INTEL_COMPILER)

   if( NOT BM_PARALLEL)
      # Write and set the wrapper if not building parallel applications
      if (BM_USE_INTEL_COMPILER)
         finish_wrapper( cc "icc" C_Compiler)
         finish_wrapper( cxx "icpc" CXX_Compiler)
      endif()
      # Rely on system default if not using Intel compiler
   
   else(NOT BM_PARALLEL)

      # If we do build parallel applications
      if (BM_USE_INTEL_MPI)
         
         set( MPI_NAME "IntelMPI_${INTEL_MPI_FLAVOUR}" CACHE STRING "Name of the MPI implementation")

         #  Add MPI to the environment set-up script and wrappers
         add_environment_source_script_to_wrapper( cc "${INTEL_MPI_ROOT}/intel64/bin/mpivars.sh")
         add_environment_source_script_to_wrapper( cxx "${INTEL_MPI_ROOT}/intel64/bin/mpivars.sh")

         add_environment_source_script(CSHELL "${INTEL_MPI_ROOT}/intel64/bin/mpivars.csh")
         add_environment_source_script(BOURNE "${INTEL_MPI_ROOT}/intel64/bin/mpivars.sh")

         if (BM_USE_INTEL_COMPILER)
            # Use the MPI compiler frontends to the Intel compiler -- mpiicc and mpiicpc -- as compilers.
            finish_wrapper( cc "mpiicc -link_mpi=${INTEL_MPI_FLAVOUR}" C_Compiler)
            finish_wrapper( cxx "mpiicpc -link_mpi=${INTEL_MPI_FLAVOUR}" CXX_Compiler)
         
            # start generating environment for mpiexec and mpirun utilitise
            add_environment_source_script_to_wrapper( mpiexec "${INTEL_CXX_ROOT}/bin/compilervars.sh intel64")
            add_environment_source_script_to_wrapper( mpirun "${INTEL_CXX_ROOT}/bin/compilervars.sh intel64")

         else(BM_USE_INTEL_COMPILER)

            # Use the MPI compiler frontends to the normal compilers -- mpicc and mpicxx -- as compilers.
            finish_wrapper( cc "mpicc -link_mpi=${INTEL_MPI_FLAVOUR}" C_Compiler)
            finish_wrapper( cxx "mpicxx -link_mpi=${INTEL_MPI_FLAVOUR}" CXX_Compiler)
         endif()

         # Write wrappers for mpiexec and mpirun utilities
         add_environment_source_script_to_wrapper( mpiexec "${INTEL_MPI_ROOT}/intel64/bin/mpivars.sh")
         add_environment_source_script_to_wrapper( mpirun "${INTEL_MPI_ROOT}/intel64/bin/mpivars.sh")

         finish_wrapper( mpiexec "mpiexec" MpiExec )
         finish_wrapper( mpirun "mpirun" MpiRun)

         # Write info about Intel MPI
         add_external_package_info(
             CAPABILITY MPIlib
             NAME         "MPI"
             VENDOR       "Intel"
             VERSION      "4.1.1.036"
             LICENSE_TYPE "Commercial"
             LICENSE_FILE "${INTEL_MPI_ROOT}/mpiEULA.txt"
             URL          "http://software.intel.com/en-us/intel-mpi-library"
             DESCRIPTION  "Intel's MPI implementation"
             REQUIRED_AT  "Runtime"
             COUNTRY_OF_ORIGIN "USA"
             SHIPPED      "Yes"
             INCLUSION_TYPE "Static Link"
             USEABLE_STAND_ALONE "No"
             CONTAINS_CRYPTO "No"
             ECCN         "Unknown"
         )


         # If linking statically then link with the MPI libraries statically
         # See main CMakeLists.txt file for comment on linking statically in
         # general
         if (NOT BUILD_SHARED_LIBS)
            set(CMAKE_EXE_LINKER_FLAGS "-static_mpi ${CMAKE_EXE_LINKER_FLAGS}")
         endif()

         # Set various MPI variables
         set(MPI_C_COMPILER "${CMAKE_C_COMPILER}" CACHE FILEPATH "MPI C Compiler")
         set(MPI_CXX_COMPILER "${CMAKE_CXX_COMPILER}" CACHE FILEPATH "MPI C Compiler")
         set(MPIEXEC "${MpiExec}" CACHE FILEPATH "Path to mpiexec script")
         set(MPIRUN "${MpiRun}" CACHE FILEPATH "Path to mpirun script")
         set(MPI_INCLUDE_DIRS "" CACHE PATH "Semicolon separated list of file paths that should be added to the include file path to compile MPI code" )
         set(MPI_LIBRARIES "" CACHE FILEPATH "Semicolon separated list of file paths to libraries with which MPI programs should be linked" )

         # Detect MPI
         set(MPI_INCLUDE_PATH "${MPI_INCLUDE_PATH}")
         set(MPI_CXX_LIBRARIES "${MPI_LIBRARIES}")
         set(MPI_C_LIBRARIES "${MPI_LIBRARIES}")
         find_package(MPI REQUIRED)

       else(BM_USE_INTEL_MPI)
         # Detect compiler so that we can detect MPI
         enable_language(CXX)
         enable_language(C)
  
         # Detect MPI
         find_package(MPI REQUIRED)

         set(C_Compiler "${MPI_C_COMPILER}")
         set(CXX_Compiler "${MPI_CXX_COMPILER}")

         # Unset variables, so that compiler can be redetected
         unset(CMAKE_C_COMPILER CACHE)
         unset(CMAKE_CXX_COMPILER CACHE)
         
       endif(BM_USE_INTEL_MPI)

   endif(NOT BM_PARALLEL)

   # Set compiler to wrapper
   set(CMAKE_C_COMPILER "${C_Compiler}" CACHE FILEPATH "C Compiler")
   set(CMAKE_CXX_COMPILER "${CXX_Compiler}" CACHE FILEPATH "C++ Compiler")

   # Force detection of the compiler
   enable_language(CXX)
   enable_language(C)

   # Detection of MPI
   if (BM_PARALLEL)
      find_package(MPI REQUIRED)
   endif()
   
elseif(WIN32)
 
   # First detect the Compiler
   enable_language(CXX)
   enable_language(C)

   # If required the MPI implementation
   if (BM_PARALLEL)
       
      set(MPI_ROOT "${THIRD_PARTY_DIR}/MicrosoftMPI-HPC-Pack-2012-R2" CACHE PATH "Directory where MPI is installed on Windows")
      set( MPIEXEC "${MPI_ROOT}/bin/mpiexec.exe" CACHE FILEPATH "Location of mpiexec command" )
      set( MPIRUN "${MPI_ROOT}/bin/mpiexec.exe" CACHE FILEPATH "Location of mpirun command" )

      add_external_package_info(
                CAPABILITY MPIlib
                NAME         "MPI"
                VENDOR       "Microsoft"
                VERSION      "HPC Pack 2012 R2"
                LICENSE_TYPE ""
                LICENSE_FILE "${MPI_ROOT}/License/note_mpi.txt"
                URL          ""
                DESCRIPTION  "Microsoft's MPI implementation"
                REQUIRED_AT  "Runtime"
                COUNTRY_OF_ORIGIN "USA"
                SHIPPED      "Yes"
                INCLUSION_TYPE "Static Link"
                USEABLE_STAND_ALONE "No"
                CONTAINS_CRYPTO "No"
                ECCN         "Unknown"
      )

      set(MPI_FOUND TRUE)
      set(MPI_INCLUDE_DIRS "${MPI_ROOT}/Inc" "${MPI_ROOT}/Inc/amd64")
      set(MPI_LIBRARIES "${MPI_ROOT}/Lib/amd64/msmpi.lib" "${MPI_ROOT}/Lib/amd64/msmpifec.lib" "${MPI_ROOT}/Lib/amd64/msmpifmc.lib")

   endif(BM_PARALLEL)

    #
    # On Windows: Use Microsoft Visual C++ compiler
    #
    add_external_package_info(
       CAPABILITY   Compiler
       NAME         "Visual C/C++"
       VENDOR       "Microsoft"
       VERSION      "${MSVC_VERSION}"
       LICENSE_TYPE "Commercial"
       LICENSE_FILE ""
       URL          "http://www.microsoft.com/visualstudio"
       DESCRIPTION  "Microsoft Visual Studio"
       REQUIRED_AT  "Build"
       COUNTRY_OF_ORIGIN "USA"
       SHIPPED      "No"
       INCLUSION_TYPE "NA"
       USEABLE_STAND_ALONE "No"
       CONTAINS_CRYPTO "Unknown"
       ECCN         "Unknown"
    )
endif()


if (BM_PARALLEL)
   # Note: We only need C bindings. The C++ bindings sometimes give trouble
   # because of SEEK_SET, etc... already being defined in stdio.h or iostream
   # For MPICH and MVAPICH  adding the following definition will disable that.
   add_definitions(-DMPICH_SKIP_MPICXX)

   # Finally determine the name and the version of the MPI implementation
   # Use mpiexec here, since the MPI standard describes its existence.
   execute_process( COMMAND "${MPIEXEC}" "-V"
         OUTPUT_VARIABLE MPI_AUTODETECT_STRING
         ERROR_VARIABLE MPI_AUTODETECT_STRING
   )

   if (MPI_AUTODETECT_STRING MATCHES "Intel\\(R\\) MPI Library")
      string(REGEX REPLACE "^.*Version ([0-9]+(.[0-9]+)*).*$" "\\1" mpiVersion "${MPI_AUTODETECT_STRING}")
      set(mpiName "IntelMPI")
   elseif (MPI_AUTODETECT_STRING MATCHES "OpenRTE")
      string(REGEX REPLACE "^[^0-9]*([0-9]+(\\.[0-9]+)*).*$" "\\1" mpiVersion "${MPI_AUTODETECT_STRING}")
      set(mpiName "OpenMPI")
   elseif (WIN32)
     set(mpiName "MicrosoftMPI")
     set(mpiVersion "2012.R2")
   else()
     set(mpiName "Unknown")
     set(mpiVersion "0")
     message(WARNING "Did not recognize MPI implementation automatically. Please set the MPI_NAME an MPI_VERSION variables manually")
   endif()

   set( MPI_NAME "${mpiName}" CACHE STRING "Name of the MPI implementation")
   set( MPI_VERSION "${mpiVersion}" CACHE STRING "Version of the MPI implementation")

endif()
