add_casawizard_sac_gtest( NAME SACScript
                          SOURCES testSACScript.cpp )
configure_file(SACScriptExpected.casa SACScriptExpected.casa COPYONLY)

add_casawizard_sac_gtest( NAME Generate3DScenarioScript
                          SOURCES testGenerate3DScenarioScript.cpp )
configure_file(generate3DScenarioScriptExpected.casa generate3DScenarioScriptExpected.casa COPYONLY)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/calibration_step1/CaseSet/Iteration_1/casaStateSAC.txt" "")

