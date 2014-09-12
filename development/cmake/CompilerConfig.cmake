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

if (DEFINED ENV{CXX} )
   set(intel_compiler "OFF")
elseif(UNIX)
   set(intel_compiler "ON")
else()
   set(intel_compiler "OFF")
endif()

option(BM_USE_INTEL_COMPILER "Whether to use the Intel compiler (UNIX only)" ${intel_compiler})
   
if (BM_USE_INTEL_COMPILER AND UNIX)

   #
   # On Shell Global Linux: Choose the Intel compiler
   #

   # It is convenient to have wrappers that set-up the environment apropriately.
   add_environment_source_script_to_wrapper( cc "${INTEL_CXX_ROOT}/bin/compilervars.sh intel64")
   add_environment_source_script_to_wrapper( cxx "${INTEL_CXX_ROOT}/bin/compilervars.sh intel64")

   # Choose Intel MPI when compiling parallel applications
   if (BM_PARALLEL)
      # Use the MPI compiler frontends -- mpiicc and mpiicpc -- as compilers.
      add_environment_source_script_to_wrapper( cc "${INTEL_MPI_ROOT}/intel64/bin/mpivars.sh")
      finish_wrapper( cc "mpiicc -link_mpi=${INTEL_MPI_FLAVOUR}" C_Compiler)

      add_environment_source_script_to_wrapper( cxx "${INTEL_MPI_ROOT}/intel64/bin/mpivars.sh")
      finish_wrapper( cxx "mpiicpc -link_mpi=${INTEL_MPI_FLAVOUR}" CXX_Compiler)
 
      add_environment_source_script_to_wrapper( mpiexec "${INTEL_CXX_ROOT}/bin/compilervars.sh intel64")
      add_environment_source_script_to_wrapper( mpiexec "${INTEL_MPI_ROOT}/intel64/bin/mpivars.sh")
      finish_wrapper( mpiexec "mpiexec" MpiExec )

      add_environment_source_script_to_wrapper( mpirun "${INTEL_CXX_ROOT}/bin/compilervars.sh intel64")
      add_environment_source_script_to_wrapper( mpirun "${INTEL_MPI_ROOT}/intel64/bin/mpivars.sh")
      finish_wrapper( mpirun "mpirun" MpiRun)

      set( MPI_NAME "IntelMPI_${INTEL_MPI_FLAVOUR}" CACHE STRING "Name of the MPI implementation")

   else()
      # Don't use MPI when the parallel applications won't be built.

      finish_wrapper( cc "icc" C_Compiler)
      finish_wrapper( cxx "icpc" CXX_Compiler)

   endif()

   # Set compiler to wrapper
   set(CMAKE_C_COMPILER "${C_Compiler}" CACHE FILEPATH "C Compiler")
   set(CMAKE_CXX_COMPILER "${CXX_Compiler}" CACHE FILEPATH "C++ Compiler")

   # Force detection of the compiler
   enable_language(CXX)
   enable_language(C)

   # Detect MPI
   if (BM_PARALLEL)
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


      # Add MPI to the environment set-up script
      add_environment_source_script(CSHELL "${INTEL_MPI_ROOT}/intel64/bin/mpivars.csh")
      add_environment_source_script(BOURNE "${INTEL_MPI_ROOT}/intel64/bin/mpivars.sh")

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

      # Note: We only need C bindings. The C++ bindings sometimes give trouble
      # because of SEEK_SET, etc... already being defined in stdio.h or iostream
      add_definitions(-DMPICH_SKIP_MPICXX)

   endif()

   # Add environment set-up scripts to generated script
   add_environment_source_script(CSHELL "${INTEL_CXX_ROOT}/bin/compilervars.csh intel64")
   add_environment_source_script(BOURNE "${INTEL_CXX_ROOT}/bin/compilervars.sh intel64")

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

   
else()

   # First detect the Compiler
   enable_language(CXX)
   enable_language(C)

   # If required the MPI implementation
   if (BM_PARALLEL)
       
      if (MSVC)
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

      else()

         find_package(MPI REQUIRED)

      endif(MSVC)
   endif(BM_PARALLEL)

   if (MSVC)
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

endif()

if (BM_PARALLEL)
  # Finally determine the name and the version of the MPI implementation
  # Use mpiexec here, since the MPI standard describes its existence.
   execute_process( COMMAND "${MPIEXEC}" "-V"
         OUTPUT_VARIABLE MPI_AUTODETECT_STRING
         ERROR_QUIET
   )

   if (MPI_AUTODETECT_STRING MATCHES "Intel\\(R\\) MPI Library")
      string(REGEX REPLACE "^.*Version ([0-9]+(.[0-9]+)*).*$" "\\1" mpiVersion "${MPI_AUTODETECT_STRING}")
      set(mpiName "IntelMPI")
   elseif (MPI_AUTODETECT_STRING MATCHES "OpenRTE")
      string(REGEX REPLACE "^.*([0-9]+(.[0-9]+)*).*$" "\\1" mpiVersion "${MPI_AUTODETECT_STRING}")
      set(mpiName "OpenMPI")
   elseif (MSVC)
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
