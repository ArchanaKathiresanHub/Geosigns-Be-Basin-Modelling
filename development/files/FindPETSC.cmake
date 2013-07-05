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
	set (CSCE_FULL "LinuxRHEL64_x86_64_58WS")

	# PETSc definitions
	set(PETSC_VERSION "3.4.1" CACHE STRING "PETSC Version")

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

	set(HYPRE_VERSION "2.9.0b" CACHE STRING "HYPRE Version")
	set(HYPRE_INCLUDE_DIR ${HPC_HOME}/hypre-${HYPRE_VERSION}/src/hypre/include)
	set(HYPRE_LIBRARY ${HPC_HOME}/hypre-${HYPRE_VERSION}/src/hypre/lib/libHYPRE.a)
	
	list(APPEND PETSC_LIBRARIES ${HYPRE_LIBRARY})
	list(APPEND PETSC_INCLUDE_DIRS ${HYPRE_INCLUDE_DIR})

        set( PETSC_LINK_FLAGS "${OpenMP_CXX_FLAGS} -openmp-link static")

	set(PETSC_FOUND TRUE)

endif()

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
