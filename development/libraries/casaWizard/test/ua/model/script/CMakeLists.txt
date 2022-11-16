add_casawizard_ua_gtest( NAME DoeScript
                         SOURCES testDoeScript.cpp setupTestCasaScript.h )
configure_file( DoEScriptExpected.casa DoEScriptExpected.casa COPYONLY )

add_casawizard_ua_gtest( NAME AddedCasesScript
                         SOURCES testAddCasesScript.cpp setupTestCasaScript.h )
configure_file( AddCasesScriptExpected.casa AddCasesScriptExpected.casa COPYONLY )

add_casawizard_ua_gtest( NAME CollectObservablesScript
                         SOURCES TestCollectObservablesScript.cpp setupTestCasaScript.h )
configure_file( collectObservablesScriptExpected.casa collectObservablesScriptExpected.casa COPYONLY )

add_casawizard_ua_gtest( NAME OptimalCaseScript
                         SOURCES testOptimalCaseScript.cpp setupTestCasaScript.h )
configure_file( OptimalCaseScriptExpected.casa OptimalCaseScriptExpected.casa COPYONLY )

add_casawizard_ua_gtest( NAME RunOptimalCaseScript
                         SOURCES testRunOptimalCaseScript.cpp setupTestCasaScript.h )
configure_file( RunOptimalCaseScriptExpected.casa RunOptimalCaseScriptExpected.casa COPYONLY )

add_casawizard_ua_gtest( NAME QCScript
                         SOURCES testQCScript.cpp setupTestCasaScript.h )
configure_file( QCScriptExpected.casa QCScriptExpected.casa COPYONLY )

add_casawizard_ua_gtest( NAME McmcScript
                         SOURCES testMcmcScript.cpp setupTestCasaScript.h )
configure_file( McmcScriptExpected.casa McmcScriptExpected.casa COPYONLY )

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/CaseSet/Iteration_1/stateFileTestCasaScriptDoE.txt" "")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/CaseSet/Iteration_1/stateFileTestCasaScriptQC.txt" "")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/CaseSet/Iteration_1/stateFileTestCasaScriptMCMC.txt" "")

