set( CTCWIZARD_TARGETNAME ctcwizard )
##include_directories( ${PROJECT_SOURCE_DIR}/libraries )
include_directories( ${CTCWIZARD_INCLUDE_DIRS} )
add_executable( ${CTCWIZARD_TARGETNAME} main.cpp )
set_target_properties( ${CTCWIZARD_TARGETNAME} PROPERTIES SUFFIX ".exe" )
target_link_libraries( ${CTCWIZARD_TARGETNAME} 
  ctcwiz
  WorkLoadManagers
  cmbapi
  utilities
  DataAccess
  TableIO
  DataModel
  Serial_Hdf5
  SerialDataAccess
  )
install_ctcwizard_application( ${CTCWIZARD_TARGETNAME} )
