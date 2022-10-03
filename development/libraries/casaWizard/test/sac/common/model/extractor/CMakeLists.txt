add_casawizard_sac_gtest( NAME CaseExtractor                     SOURCES testCaseExtractor.cpp )
add_casawizard_sac_gtest( NAME WellTrajectoryExtractor           SOURCES testWellTrajectoryExtractor.cpp )

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/workingDirectory/calibration_step1/CaseSet/Iteration_1/Case_1)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/workingDirectory/calibration_step1/CaseSet/Iteration_1/Case_2)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/workingDirectory/calibration_step1/CaseSet/Iteration_1/Case_4)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/workingDirectoryNoIterations/calibration_step1/CaseSet)
