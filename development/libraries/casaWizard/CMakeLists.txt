if ( Qt5_FOUND )
   if( BM_USE_INTEL_COMPILER )
      add_definitions( -Wno-invalid-offsetof )
   endif()

   set( CASAWIZARD_COMMON_LIBNAME casawizard_common )
   set( CASAWIZARD_UA_LIBNAME casawizard_ua )
   set( CASAWIZARD_SAC_LIBNAME casawizard_sac )

   set( CMAKE_AUTOMOC ON )

   add_subdirectory( src/QtXlsxWriter )
   add_subdirectory( src/common )
   add_subdirectory( src/ua )
   add_subdirectory( src/sac )
   add_subdirectory( test )
endif()
