add_casawizard_sac_gtest( NAME WellTrajectoryWriter 
                          SOURCES testWellTrajectoryWriter.cpp )
configure_file( wells/Well1_TemperatureExpected.in  wells/Well1_TemperatureExpected.in COPYONLY )
configure_file( wells/Well1_TwoWayTimeExpected.in   wells/Well1_TwoWayTimeExpected.in  COPYONLY )
configure_file( wells/Well2_TemperatureExpected.in  wells/Well2_TemperatureExpected.in COPYONLY )
