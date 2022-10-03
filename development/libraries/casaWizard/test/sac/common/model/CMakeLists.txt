add_casawizard_sac_gtest( NAME MapsManager               SOURCES testMapsManager.cpp )

add_casawizard_sac_gtest( NAME WellTrajectoryManager     SOURCES testWellTrajectoryManager.cpp )
add_casawizard_sac_gtest( NAME Case3DTrajectoryConvertor SOURCES testCase3DTrajectoryConvertor.cpp )

configure_file( Layer_Type1Expected.zyc Layer_Type1Expected.zyc )
configure_file( Layer_Type2Expected.zyc Layer_Type2Expected.zyc )

configure_file( wellTrajectoryManagerVersion0.dat wellTrajectoryManagerVersion0.dat )

add_subdirectory( extractor )
add_subdirectory( input )
add_subdirectory( output )
add_subdirectory( script )