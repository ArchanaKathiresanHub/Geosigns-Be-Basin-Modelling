if ( Qt5_FOUND )
   if( BM_USE_INTEL_COMPILER )
      add_definitions( -Wno-invalid-offsetof )
   endif()


   set( CTCWIZARD_LIBNAME ctcwiz )

   set( CMAKE_AUTOMOC ON )

   add_subdirectory( src )
   add_subdirectory( test )

endif()
