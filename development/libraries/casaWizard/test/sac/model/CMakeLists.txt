add_casawizard_sac_gtest( NAME LithofractionManager      SOURCES testLithofractionManager.cpp )
add_casawizard_sac_gtest( NAME SACScenario               SOURCES testSACScenario.cpp )
add_casawizard_sac_gtest( NAME WellTrajectoryManager     SOURCES testWellTrajectoryManager.cpp )
add_casawizard_sac_gtest( NAME Case3DTrajectoryConvertor SOURCES testCase3DTrajectoryConvertor.cpp )

configure_file( lithofractionManager.dat lithofractionManager.dat )
configure_file( lithofractionManagerVersion0.dat lithofractionManagerVersion0.dat )

configure_file( wellTrajectoryManager.dat wellTrajectoryManager.dat )

add_subdirectory( input )
add_subdirectory( output )
add_subdirectory( script )
add_subdirectory( extractor )
