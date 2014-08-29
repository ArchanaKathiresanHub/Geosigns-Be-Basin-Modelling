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
include(cmake/EnvSetup.cmake)

if (UNIX)

	set( MPI_ROOT "/apps/3rdparty/intel/impi/4.1.1.036" CACHE PATH "Path to MPI library" )
	set( MPI_ROOT64 "${MPI_ROOT}/intel64" )
	set( MPIRUN "${MPI_ROOT64}/bin/mpirun" CACHE FILEPATH "Location of mpirun command" )

	add_environment_source_script(CSHELL "${MPI_ROOT64}/bin/mpivars.csh")
	add_environment_source_script(BOURNE "${MPI_ROOT64}/bin/mpivars.sh")

	add_external_package_info(
		   CAPABILITY MPI
		   NAME         "MPI"
		   VENDOR       "Intel"
		   VERSION      "4.1.1.036"
		   LICENSE_TYPE "Commercial"
		   LICENSE_FILE "${MPI_ROOT}/mpiEULA.txt"
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

	set(MPI_FOUND TRUE)

	# Now figure out the compiler options to add

	# The options that Intel MPI uses, can be found by doing 'mpiicpc -show' on
	# the command line
	set(MPI_INCLUDE_DIRS "${MPI_ROOT64}/include")

	# Link to libraries dl, rt, and pthread always dynamically!
	set( oldLibrarySuffixes ${CMAKE_FIND_LIBRARY_SUFFIXES})
	set( CMAKE_FIND_LIBRARY_SUFFIXES ".so" ".a")
	find_library(MPI_DL_LIBRARY dl)
	find_library(MPI_RT_LIBRARY rt)
	find_library(MPI_PTHREAD_LIBRARY pthread)
	set( CMAKE_FIND_LIBRARY_SUFFIXES ${oldLibrarySuffixes})
  
	# continue linking with the MPI libraries 
	find_library(MPI_LIB1 "mpigc4" PATHS "${MPI_ROOT64}/lib" NO_DEFAULT_PATH)
	find_library(MPI_LIB2 "mpigf"  PATHS "${MPI_ROOT64}/lib" NO_DEFAULT_PATH)
	find_library(MPI_LIB3 "mpi"    PATHS "${MPI_ROOT64}/lib" NO_DEFAULT_PATH)
	find_library(MPI_LIB4 "mpigi"  PATHS "${MPI_ROOT64}/lib" NO_DEFAULT_PATH)
	set(MPI_LIBRARIES "${MPI_LIB1}" "${MPI_LIB2}" "${MPI_LIB3}" "${MPI_LIB4}" ${MPI_DL_LIBRARY} ${MPI_RT_LIBRARY} ${MPI_PTHREAD_LIBRARY})


	# Note: We only need C bindings. The C++ bindings sometimes give trouble
	# because of SEEK_SET, etc... already being defined in stdio.h or iostream
	add_definitions(-DMPICH_SKIP_MPICXX)

else () # windows

	# Note: paths hardcoded for 64bit
	set(MPI_ROOT "${THIRD_PARTY_DIR}/MicrosoftMPI-HPC-Pack-2012-R2" CACHE PATH "Directory where MPI is installed on Windows")
	set( MPI_ROOT64 "${MPI_ROOT}" )
	set( MPIRUN "${MPI_ROOT}/bin/mpiexec.exe" CACHE FILEPATH "Location of mpirun command" )

	add_external_package_info(
		   CAPABILITY MPI
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
	
endif ()