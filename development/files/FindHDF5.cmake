if(UNIX)
	set (HPC_HOME ${CBM_HOME}/hpc)
	set (CSCE_FULL "LinuxRHEL64_x86_64_57WS")

	set (HDF5_HOME ${HPC_HOME}/hdf5-intel/${HDF5_VERSION}/${CSCE_FULL} CACHE PATH "Home for serial HDF5")
	set (HDF5_P_HOME  ${HPC_HOME}/hdf5-intel-parallel/${HDF5_VERSION}/${CSCE_FULL} CACHE PATH "Home dir for parallel HDF5")

	# serial version
	set (HDF5_serial_INCLUDE_DIR ${HDF5_HOME}/include) 
	set (HDF5_serial_INCLUDE_DIRS ${HDF5_serial_INCLUDE_DIR}) 
	set (HDF5_hdf5_serial_LIBRARY ${HDF5_HOME}/lib/libhdf5.a)
	list (APPEND HDF5_serial_LIBRARIES ${HDF5_hdf5_serial_LIBRARY})
	list (APPEND HDF5_serial_LIBRARIES z)

	# parallel version
	set (HDF5_parallel_INCLUDE_DIR ${HDF5_P_HOME}/include)
	set (HDF5_parallel_INCLUDE_DIRS ${HDF5_parallel_INCLUDE_DIR})
	set (HDF5_hdf5_parallel_LIBRARY ${HDF5_P_HOME}/lib/libhdf5.a)
	list (APPEND HDF5_parallel_LIBRARIES ${HDF5_hdf5_parallel_LIBRARY})
	list (APPEND HDF5_parallel_LIBRARIES z)

	if(MPI_FOUND)
		set(HDF5_parallel_DEFINITIONS "-DH5_HAVE_PARALLEL")
	endif()

	set (HDF5_FOUND TRUE)
	set (HDF5_IS_PARALLEL TRUE)
endif()

if(WIN32)
	math(EXPR _64 "${CMAKE_SIZEOF_VOID_P} * 8")
	set (HDF5_HOME ${PROJECT_SOURCE_DIR}/../3rdparty/hdf5.win${_64}/hdf5)
	# serial version
	set (HDF5_serial_INCLUDE_DIR ${HDF5_HOME}/include)
	set (HDF5_serial_INCLUDE_DIRS ${HDF5_serial_INCLUDE_DIR})
	set (HDF5_hdf5_serial_LIBRARY
		debug ${HDF5_HOME}/lib/hdf5d.lib
		optimized ${HDF5_HOME}/lib/hdf5.lib )
	list (APPEND HDF5_serial_LIBRARIES ${HDF5_hdf5_serial_LIBRARY})
endif()

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
