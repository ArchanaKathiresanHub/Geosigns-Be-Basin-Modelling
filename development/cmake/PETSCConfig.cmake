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

	set(PETSC_VERSION "3.4.1-intel13" CACHE STRING "PETSC Version")
	set(PETSC_RELEASE_HOME ${HPC_HOME}/petsc/${PETSC_VERSION}/LinuxRHEL64_x86_64_58WS)
	set(PETSC_DEBUG_HOME ${HPC_HOME}/petsc-debug/${PETSC_VERSION}/LinuxRHEL64_x86_64_58WS)
	set(PETSC_HOME)
	if(CMAKE_BUILD_TYPE STREQUAL "Release")
	   set(PETSC_HOME "${PETSC_RELEASE_HOME}")
	else()
	   set(PETSC_HOME "${PETSC_DEBUG_HOME}")
	endif()

	set(PETSC_FOUND TRUE)
	set(PETSC_ROOT "${PETSC_HOME}" CACHE PATH "Installation directory of PETSc")
	set(PETSC_INCLUDE_DIRS "${PETSC_ROOT}/include")
	find_library(PETSC_LIBRARIES "petsc" PATHS "${PETSC_ROOT}/lib" NO_DEFAULT_PATH )
	set(PETSC_LINK_FLAGS "${OpenMP_CXX_FLAGS} ${OpenMP_LINK_FLAGS}" )

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

else () # windows

	set(PETSC_VERSION "3.4.4" CACHE STRING "PETSC Version")
	set(PETSC_HOME "${THIRD_PARTY_DIR}/PETSc_Windows-3.4.4" CACHE PATH "PETSc home path") 
	set(PETSC_DEBUG "${PETSC_HOME}/PETSc/c-debug_icl_mkl" CACHE PATH "Debug path")
	set(PETSC_RELEASE "${PETSC_HOME}/PETSc/c-opt_icl_mkl" CACHE PATH "Release path")
	set(PETSC_INCLUDE_DIRS "${PETSC_HOME}/PETSc/include")
	set(PETSC_LIBRARIES 
		"${PETSC_HOME}/lib/intel64/libiomp5md.lib"
		"${PETSC_HOME}/lib/intel64/libmmt.lib"
		"${PETSC_HOME}/lib/intel64/libdecimal.lib"
		"${PETSC_HOME}/lib/intel64/libirc.lib"
		"${PETSC_HOME}/lib/intel64/svml_dispmt.lib"
		"${PETSC_HOME}/lib/intel64/mkl_core.lib"
		"${PETSC_HOME}/lib/intel64/mkl_intel_lp64.lib"
		"${PETSC_HOME}/lib/intel64/mkl_intel_lp64.lib"
		"${PETSC_HOME}/lib/intel64/mkl_intel_thread.lib"
		"${PETSC_HOME}/lib/intel64/mkl_scalapack_lp64.lib"
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
	set(PETSC_LINK_FLAGS "${OpenMP_CXX_FLAGS} ${OpenMP_LINK_FLAGS}")

	add_external_package_info( 
		CAPABILITY  PETScLib
		NAME         "PETSc for Windows"
		VENDOR       "Microsoft Innovation Center Rapperswil"
		VERSION      "3.4.4"
		LICENSE_TYPE ""
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
	

endif (UNIX)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
