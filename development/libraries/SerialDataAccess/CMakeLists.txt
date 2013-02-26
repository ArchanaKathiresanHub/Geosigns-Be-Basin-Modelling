add_library( SerialDataAccess 
   src/SerialApplicationGlobalOperations.C
   src/SerialGrid.C
   src/SerialGridMap.C
   src/SerialMapWriter.C
   src/SerialMessageHandler.C
   src/SerialObjectFactory.C
   src/SerialProjectHandle.C
   src/Serialhdf5funcs.C
)

bm_include_libraries(
   Serial_Hdf5
   utilities
   TableIO
   CBMGenerics
   DataAccess
)

include_directories(
   ${HDF5_serial_INCLUDE_DIRS}
)

generate_dox( src/SerialDataAccess.cfg )

target_link_libraries(SerialDataAccess
   Serial_Hdf5
   CBMGenerics
   utilities
   TableIO
   DataAccess
)

if (UNIX)
   target_link_libraries(SerialDataAccess rt)
endif(UNIX)

install(TARGETS SerialDataAccess
   RUNTIME DESTINATION bin
   LIBRARY	DESTINATION lib
   ARCHIVE DESTINATION lib
)
