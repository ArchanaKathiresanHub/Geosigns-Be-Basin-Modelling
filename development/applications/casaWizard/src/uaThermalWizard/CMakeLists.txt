set( CASAWIZARD_TARGETNAME casathermalwizard )
include_directories( ${CASAWIZARD_COMMON_INCLUDE_DIRS} ${CASAWIZARD_UA_INCLUDE_DIRS}
                     ${CASAWIZARD_COMMON_MODEL_INCLUDE_DIRS} ${CASAWIZARD_UA_MODEL_INCLUDE_DIRS})
add_executable( ${CASAWIZARD_TARGETNAME} main.cpp )
set_target_properties( ${CASAWIZARD_TARGETNAME} PROPERTIES SUFFIX ".exe" )
target_link_libraries( ${CASAWIZARD_TARGETNAME}
  casawizard_ua
  casawizard_ua_model
  casawizard_common
  casawizard_common_model
  cmbapi
  utilities
  DataAccess
  TableIO
  DataModel
  Serial_Hdf5
  SerialDataAccess )

install_casawizard_application( ${CASAWIZARD_TARGETNAME} )
