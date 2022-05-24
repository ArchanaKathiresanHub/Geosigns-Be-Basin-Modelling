add_casawizard_sac_gtest( NAME LithofractionManager      SOURCES testLithofractionManager.cpp )
add_casawizard_sac_gtest( NAME SACScenario               SOURCES testSACScenario.cpp )
add_casawizard_sac_gtest( NAME MapsManager               SOURCES testMapsManager.cpp )

configure_file( Layer_Type1Expected.zyc Layer_Type1Expected.zyc )
configure_file( Layer_Type2Expected.zyc Layer_Type2Expected.zyc )

add_casawizard_sac_gtest( NAME WellTrajectoryManager     SOURCES testWellTrajectoryManager.cpp )
add_casawizard_sac_gtest( NAME Case3DTrajectoryConvertor SOURCES testCase3DTrajectoryConvertor.cpp )

configure_file( lithofractionManager.dat lithofractionManager.dat )
configure_file( lithofractionManagerVersion0.dat lithofractionManagerVersion0.dat )

configure_file( wellTrajectoryManagerVersion0.dat wellTrajectoryManagerVersion0.dat )

add_subdirectory( input )
add_subdirectory( output )
add_subdirectory( script )
add_subdirectory( extractor )
