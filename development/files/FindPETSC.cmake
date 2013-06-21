#
#
#

if(PETSC_FOUND)
	return()
endif()

if(UNIX)
	if(NOT CBM_HOME)
		message(FATAL_ERROR "CBM_HOME is not defined")
	endif()
	set (HPC_HOME ${CBM_HOME}/hpc)
	set (CSCE_FULL "LinuxRHEL64_x86_64_57WS")

	# PETSc definitions
	set(PETSC_VERSION "3.1.8-intel13" CACHE STRING "PETSC Version")

	set(PETSC_ARCH linux-gnu-intel)
	set(PETSC_RELEASE_HOME ${HPC_HOME}/petsc/${PETSC_VERSION}/${CSCE_FULL})
	set(PETSC_DEBUG_HOME ${HPC_HOME}/petsc-debug/${PETSC_VERSION}/${CSCE_FULL})

	set(PETSC_INCLUDE_DIR ${PETSC_RELEASE_HOME}/include)
	set(PETSC_INCLUDE_DIRS)
	list(APPEND PETSC_INCLUDE_DIRS ${PETSC_INCLUDE_DIR})

	set(PETSC_DEBUG_LIBRARY ${PETSC_DEBUG_HOME}/lib/libpetsc.a)
	set(PETSC_RELEASE_LIBRARY ${PETSC_RELEASE_HOME}/lib/libpetsc.a)

	set(PETSC_LIBRARIES)
	if(CMAKE_BUILD_TYPE STREQUAL "Release")
		list(APPEND PETSC_LIBRARIES ${PETSC_RELEASE_LIBRARY})
	else()
		list(APPEND PETSC_LIBRARIES ${PETSC_DEBUG_LIBRARY})
	endif()

	set(PETSC_FOUND TRUE)
endif()

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End: