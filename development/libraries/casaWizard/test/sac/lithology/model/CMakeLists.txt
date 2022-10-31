add_casawizard_sac_gtest( NAME LithoMapsManager       SOURCES testLithoMapsManager.cpp )
add_casawizard_sac_gtest( NAME LithofractionManager   SOURCES testLithofractionManager.cpp )
add_casawizard_sac_gtest( NAME LithologyScenario      SOURCES testLithologyScenario.cpp )

configure_file( Layer_Type1Expected.zyc Layer_Type1Expected.zyc )
configure_file( Layer_Type2Expected.zyc Layer_Type2Expected.zyc )

configure_file( lithofractionManager.dat lithofractionManager.dat )
configure_file( lithofractionManagerVersion0.dat lithofractionManagerVersion0.dat )

add_subdirectory( input )
add_subdirectory( output )
add_subdirectory( script )
add_subdirectory( extractor )
