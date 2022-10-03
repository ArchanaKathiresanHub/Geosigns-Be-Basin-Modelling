set( CASAWIZARD_TARGETNAME SACLithology )
include_directories( ${CASAWIZARD_COMMON_INCLUDE_DIRS} ${CASAWIZARD_SACLITHO_INCLUDE_DIRS}
                     ${CASAWIZARD_COMMON_MODEL_INCLUDE_DIRS} ${CASAWIZARD_SACLITHO_MODEL_INCLUDE_DIRS})
add_executable( ${CASAWIZARD_TARGETNAME} Main.cpp )
set_target_properties( ${CASAWIZARD_TARGETNAME} PROPERTIES SUFFIX ".exe" )
target_link_libraries( ${CASAWIZARD_TARGETNAME}
  casawizard_sacLitho
  casawizard_sacLitho_model
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
