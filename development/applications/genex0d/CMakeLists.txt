set( GENEX0D_APP_NAME "genex0d" )
add_executable( ${GENEX0D_APP_NAME} src/main.cpp )
set_target_properties( ${GENEX0D_APP_NAME} PROPERTIES SUFFIX ".exe" )
set_target_properties( ${GENEX0D_APP_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${GENEX0D_APP_NAME}" )
create_application_run_script( ${GENEX0D_APP_NAME} )

application_env_var( TARGET ${GENEX0D_APP_NAME} VARS
     "GENEXDIR"    "$MISCDIR/genex40"
     "GENEX5DIR"   "$MISCDIR/genex50"
     "GENEX6DIR"   "$MISCDIR/genex60"
     "OTGCDIR"     "$MISCDIR/OTGC"
     "CTCDIR"      "$MISCDIR"
     "EOSPACKDIR"  "$MISCDIR/eospack"
)

include_directories( utilities
    ${CMAKE_CURRENT_BINARY_DIR}
    ${GENEX0D_LIB_INCLUDE_DIRS}
)

include_directories( SYSTEM
    ${HDF5_INCLUDE_DIRS}
)

target_link_libraries( ${GENEX0D_APP_NAME}
    genex0dLib
    DataAccess
    SerialDataAccess
    FileSystem
    Serial_Hdf5
    ${HDF5_LIBRARIES}
)

if (WIN32)
  # Windows.h defines 'min' and 'max' macros if NOMINMAX is not defined. These
  # macros collide with std::min and std::max
  add_definitions(-DNOMINMAX)
endif()

install( TARGETS ${GENEX0D_APP_NAME}
         RUNTIME DESTINATION bin )

