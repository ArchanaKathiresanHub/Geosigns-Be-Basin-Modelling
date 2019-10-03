macro( add_casawizard_sac_gtest )
  set( PREFIX "SAC" )
  set( OPTIONS )
  set( ONE_VALUE_ARGS NAME )
  set( MULTI_VALUE_ARGS SOURCES )
  cmake_parse_arguments( "${PREFIX}" "${OPTIONS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" ${ARGN} )

  set(CFGFLS "${PROJECT_SOURCE_DIR}/geocase/misc")

  add_gtest( NAME ${CASAWIZARD_SAC_LIBNAME}::${SAC_NAME}
             SOURCES ${SAC_SOURCES}
             LIBRARIES ${CASAWIZARD_SAC_LIBNAME} ${CASAWIZARD_COMMON_LIBNAME} Qt5::Core Qt5::Gui Qt5::Widgets QtXlsx cmbapi utilities DataAccess TableIO DataModel Serial_Hdf5 SerialDataAccess
             ENV_VARS CTCDIR=${CFGFLS} EOSPACKDIR=${CFGFLS}/eospack
             INCLUDE_DIRS ${CASAWIZARD_SAC_INCLUDE_DIRS} ${CASAWIZARD_COMMON_INCLUDE_DIRS} ${QTXLSX_INCLUDE_DIRS} ${CASAWIZARD_TESTFUNCTION_INCLUDE_DIRS} ${CASAWIZARD_STUB_INCLUDE_DIRS})

endmacro( add_casawizard_sac_gtest )

add_subdirectory( model )
