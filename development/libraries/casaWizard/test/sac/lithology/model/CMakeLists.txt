add_casawizard_sac_gtest( NAME LithofractionManager      SOURCES testLithofractionManager.cpp )
add_casawizard_sac_gtest( NAME SacLithologyScenario      SOURCES testSacLithologyScenario.cpp )

configure_file( lithofractionManager.dat lithofractionManager.dat )
configure_file( lithofractionManagerVersion0.dat lithofractionManagerVersion0.dat )

add_subdirectory( input )
add_subdirectory( output )
add_subdirectory( script )
add_subdirectory( extractor )
