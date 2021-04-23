macro( add_casawizard_ua_gtest )
  set( PREFIX "UA" )
  set( OPTIONS )
  set( ONE_VALUE_ARGS NAME )
  set( MULTI_VALUE_ARGS SOURCES AUXINCLUDEDIRS AUXLIBS )
  cmake_parse_arguments( "${PREFIX}" "${OPTIONS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" ${ARGN} )

  set(CFGFLS "${PROJECT_SOURCE_DIR}/geocase/misc")

  add_gtest( NAME ${CASAWIZARD_UA_MODEL_LIBNAME}::${UA_NAME}
             SOURCES ${UA_SOURCES}
             LIBRARIES ${CASAWIZARD_UA_MODEL_LIBNAME} ${CASAWIZARD_COMMON_MODEL_LIBNAME} ${UA_AUXLIBS} Qt5::Core Qt5::Gui Qt5::Widgets QtXlsx cmbapi utilities DataAccess TableIO DataModel Serial_Hdf5 SerialDataAccess
             ENV_VARS CTCDIR=${CFGFLS} EOSPACKDIR=${CFGFLS}/eospack
             INCLUDE_DIRS ${CASAWIZARD_UA_MODEL_INCLUDE_DIRS} ${CASAWIZARD_COMMON_MODEL_INCLUDE_DIRS} ${QTXLSX_INCLUDE_DIRS} ${CASAWIZARD_TESTFUNCTION_INCLUDE_DIRS} ${CASAWIZARD_STUB_INCLUDE_DIRS} ${UA_AUXINCLUDEDIRS} )

endmacro( add_casawizard_ua_gtest )

add_subdirectory( model )
