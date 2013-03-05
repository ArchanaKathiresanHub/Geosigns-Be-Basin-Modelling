set(SERIALHDF5_SOURCES
	src/buffer_readwrite.C
	src/h5_file_types.C
	src/h5_struct_types.C
	src/h5_types.C
	src/readwriteobject.C
)

add_library(Serial_Hdf5 ${SERIALHDF5_LIB_MODE}
   ${SERIALHDF5_SOURCES}
)


include_directories(${HDF5_serial_INCLUDE_DIRS})

target_link_libraries(Serial_Hdf5 ${HDF5_serial_LIBRARIES})

	
install(TARGETS Serial_Hdf5
	RUNTIME DESTINATION bin
	LIBRARY	DESTINATION lib
	ARCHIVE DESTINATION lib
)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End: