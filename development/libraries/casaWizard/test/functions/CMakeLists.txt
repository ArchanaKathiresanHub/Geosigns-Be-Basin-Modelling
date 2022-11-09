set(CFGFLS "${PROJECT_SOURCE_DIR}/geocase/misc")

add_gtest( NAME casaWizard_functions::ExpectFileEq
           SOURCES testExpectFileEq.cpp expectFileEq.h
           ENV_VARS CTCDIR=${CFGFLS} EOSPACKDIR=${CFGFLS}/eospack )
configure_file( testExpectFile1.txt testExpectFile1.txt COPYONLY )
configure_file( testExpectFile2.txt testExpectFile2.txt COPYONLY )

set( CASAWIZARD_TESTFUNCTION_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "" )
