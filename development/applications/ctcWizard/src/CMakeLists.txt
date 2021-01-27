set( CTCWIZARD_TARGETNAME ctcwizard )
include_directories( ${CTCWIZARD_INCLUDE_DIRS} )
add_executable( ${CTCWIZARD_TARGETNAME} main.cpp )
set_target_properties( ${CTCWIZARD_TARGETNAME} PROPERTIES SUFFIX ".exe" )
target_link_libraries( ${CTCWIZARD_TARGETNAME} 
  ctcwiz )
install_ctcwizard_application( ${CTCWIZARD_TARGETNAME} )
