set( CASAWIZARD_TARGETNAME casasacwizard )
include_directories( ${CASAWIZARD_COMMON_INCLUDE_DIRS} ${CASAWIZARD_SAC_INCLUDE_DIRS}
                     ${CASAWIZARD_COMMON_MODEL_INCLUDE_DIRS} ${CASAWIZARD_SAC_MODEL_INCLUDE_DIRS})
add_executable( ${CASAWIZARD_TARGETNAME} main.cpp )
set_target_properties( ${CASAWIZARD_TARGETNAME} PROPERTIES SUFFIX ".exe" )
target_link_libraries( ${CASAWIZARD_TARGETNAME}
  casawizard_sac
  casawizard_sac_model
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
