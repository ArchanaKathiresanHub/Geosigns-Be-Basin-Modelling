macro( add_casawizard_common_gtest )
  set( PREFIX "COMMON" )
  set( OPTIONS )
  set( ONE_VALUE_ARGS NAME )
  set( MULTI_VALUE_ARGS SOURCES )
  cmake_parse_arguments( "${PREFIX}" "${OPTIONS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" ${ARGN} )

  set(CFGFLS "${PROJECT_SOURCE_DIR}/geocase/misc")

  add_gtest( NAME ${CASAWIZARD_COMMON_LIBNAME}::${COMMON_NAME}
             SOURCES ${COMMON_SOURCES}
             LIBRARIES ${CASAWIZARD_COMMON_MODEL_LIBNAME} Qt5::Core QtXlsx cmbapi utilities DataAccess TableIO DataModel Serial_Hdf5 SerialDataAccess
             ENV_VARS CTCDIR=${CFGFLS} EOSPACKDIR=${CFGFLS}/eospack
             INCLUDE_DIRS ${CASAWIZARD_COMMON_MODEL_INCLUDE_DIRS} ${QTXLSX_INCLUDE_DIRS} ${CASAWIZARD_TESTFUNCTION_INCLUDE_DIRS} ${CASAWIZARD_STUB_INCLUDE_DIRS} )

endmacro( add_casawizard_common_gtest )

add_subdirectory(model)
