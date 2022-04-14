add_casawizard_ua_gtest( NAME McmcOutputWriter
                         SOURCES TestMcmcOutputWriter.cpp)
configure_file( McmcOutputExpected.csv McmcOutputExpected.csv COPYONLY )
configure_file( McmcOutputExpected2Prop.csv McmcOutputExpected2Prop.csv COPYONLY )
configure_file( McmcOutputExpectedSpecialCharacters.csv McmcOutputExpectedSpecialCharacters.csv COPYONLY )

add_casawizard_ua_gtest( NAME McmcDataCollector
                         SOURCES TestMcmcDataCollector.cpp)
configure_file( runCasesObservables.txt runCasesObservables.txt COPYONLY )
configure_file( proxyEvalObservables.txt proxyEvalObservables.txt COPYONLY )

add_casawizard_ua_gtest( NAME RunCaseSetFileManager
                         SOURCES testRunCaseSetFileManager.cpp )
