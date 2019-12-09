add_casawizard_sac_gtest( NAME SACScript
                          SOURCES testSACScript.cpp )
configure_file(SACScriptExpected.casa SACScriptExpected.casa COPYONLY)

add_casawizard_sac_gtest( NAME SaveOptimizedScript
                          SOURCES testSaveOptimizedScript.cpp )
configure_file(saveOptimizedScriptExpected.casa saveOptimizedScriptExpected.casa COPYONLY)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/calibration_step1/CaseSet/Iteration_1/casaStateSAC.txt" "")

