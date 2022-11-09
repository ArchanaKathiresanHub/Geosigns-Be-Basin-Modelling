set( LIB_NAME "casaCmdInterface" )

create_bm_library( TARGET ${LIB_NAME}
   LIBRARIES utilities DataAccess cmbapi
   INSTALLTARGET )

add_gtest( NAME "casaCmdInterface::casaCmdInterface"
   SOURCES test/TestCasaCmdInterface.cpp
   LIBRARIES casaCmdInterface DataAccess SerialDataAccess
   FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)
