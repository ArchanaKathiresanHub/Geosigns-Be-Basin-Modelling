add_casawizard_sac_gtest( NAME WellTrajectoryDataCreator 
                          SOURCES testWellTrajectoryDataCreator.cpp )
configure_file( TrajectoryData/Case_1/wellTrajectory-bestMatchedCase-Temperature.csv TrajectoryData/Case_1/wellTrajectory-bestMatchedCase-Temperature.csv COPYONLY )

add_casawizard_sac_gtest( NAME Case3DTrajectoryReader
                          SOURCES testCase3DTrajectoryReader.cpp )
configure_file( wellDataCase3D.csv wellDataCase3D.csv COPYONLY )
