add_executable(cauldron-crop
	src/crop.C
)

bm_include_libraries(
	TableIO
        DataAccess
        SerialDataAccess
        utilities
)

include_directories(
        ${HDF5_serial_INCLUDE_DIRS}
)

target_link_libraries(cauldron-crop
      projectdependencies
      TableIO
      DataAccess
      SerialDataAccess
      utilities
      ${HDF5_serial_LIBRARIES}
      ${HDF5_HOME}/lib/libhdf5_hl.a
)

install(TARGETS cauldron-crop
        RUNTIME DESTINATION bin
)
