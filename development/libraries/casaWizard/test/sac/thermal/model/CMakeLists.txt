#add_casawizard_sac_gtest( NAME HCPManager      SOURCES testHCPManager.cpp )
#add_casawizard_sac_gtest( NAME SacThermalScenario      SOURCES testThermalScenario.cpp )

#configure_file( hcpManager.dat hcpManager.dat )
#configure_file( hcpManagerVersion0.dat hcpManagerVersion0.dat )

#add_subdirectory( input )
#add_subdirectory( output )
add_subdirectory( script )
#add_subdirectory( extractor )
