add_casawizard_sac_gtest( NAME TCHPManager          SOURCES testTCHPManager.cpp )
add_casawizard_sac_gtest( NAME ThermalScenario      SOURCES testThermalScenario.cpp )

configure_file( TCHPManager.dat TCHPManager.dat )

add_subdirectory( input )
#add_subdirectory( output )
add_subdirectory( script )
add_subdirectory( extractor )
