if ( Qt5_FOUND )
   if( BM_USE_INTEL_COMPILER )
      add_definitions( -Wno-invalid-offsetof )
   endif()

   set( CASAWIZARD_COMMON_LIBNAME casawizard_common )
   set( CASAWIZARD_COMMON_MODEL_LIBNAME casawizard_common_model )

   set( CASAWIZARD_UA_LIBNAME casawizard_ua )
   set( CASAWIZARD_UA_MODEL_LIBNAME casawizard_ua_model )

   set( CASAWIZARD_SACLITHO_LIBNAME casawizard_sacLitho )
   set( CASAWIZARD_SACLITHO_MODEL_LIBNAME casawizard_sacLitho_model )

   set( CASAWIZARD_SACTHERMAL_LIBNAME casawizard_sacThermal )
   set( CASAWIZARD_SACTHERMAL_MODEL_LIBNAME casawizard_sacThermal_model )

   set( CASAWIZARD_SAC_LIBNAME casawizard_sac )
   set( CASAWIZARD_SAC_MODEL_LIBNAME casawizard_sac_model )

   set( CMAKE_AUTOMOC ON )
   set( CMAKE_AUTORCC ON )

   add_subdirectory( QtXlsxWriter )
   add_subdirectory( src/common )
   add_subdirectory( src/ua )
   add_subdirectory( src/sac/common )
   add_subdirectory( src/sac/lithology )
   add_subdirectory( src/sac/thermal )
   add_subdirectory( test )
endif()
