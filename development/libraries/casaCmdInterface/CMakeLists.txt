create_bm_library( TARGET "casaCmdInterface"
   LIBRARIES utilities cmbapi
   INSTALLTARGET )

add_gtest( NAME "casaCmdInterface::casaCmdInterface"
   SOURCES test/TestCasaCmdInterface.cpp
   LIBRARIES casaCmdInterface DataAccess SerialDataAccess)
