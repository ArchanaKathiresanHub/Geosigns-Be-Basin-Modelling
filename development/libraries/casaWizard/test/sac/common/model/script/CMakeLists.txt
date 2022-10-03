add_casawizard_sac_gtest( NAME SACScript                         SOURCES testSACScript.cpp )
configure_file(SACScriptExpected.casa SACScriptExpected.casa COPYONLY)

add_casawizard_sac_gtest( NAME Generate3DScenarioScript          SOURCES testGenerate3DScenarioScript.cpp )
configure_file(generate3DScenarioScriptExpected.casa generate3DScenarioScriptExpected.casa COPYONLY)
configure_file(generate3DScenarioScript_IDW_GaussianExpected.casa generate3DScenarioScript_IDW_GaussianExpected.casa COPYONLY)
configure_file(generate3DScenarioScript_NN_MovingAverageExpected.casa generate3DScenarioScript_NN_MovingAverageExpected.casa COPYONLY)

add_casawizard_sac_gtest( NAME Track1Script              SOURCES testTrack1dScript.cpp )
add_casawizard_sac_gtest( NAME Track1dAllWellScript              SOURCES testTrack1dAllWellScript.cpp )
add_casawizard_sac_gtest( NAME CauldronScript                    SOURCES testCauldronScript.cpp )
configure_file(cauldronScriptExpected.casa cauldronScriptExpected.casa COPYONLY)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/calibration_step1/CaseSet/Iteration_1/casaStateSAC.txt" "")

