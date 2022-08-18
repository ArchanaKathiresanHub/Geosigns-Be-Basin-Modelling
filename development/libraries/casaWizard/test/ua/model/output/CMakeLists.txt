add_casawizard_ua_gtest( NAME McmcOutputWriter
                         SOURCES TestMcmcOutputWriter.cpp)
configure_file( McmcOutputExpected.csv McmcOutputExpected.csv COPYONLY )
configure_file( McmcOutputExpected2Prop.csv McmcOutputExpected2Prop.csv COPYONLY )
configure_file( McmcOutputExpectedSpecialCharacters.csv McmcOutputExpectedSpecialCharacters.csv COPYONLY )

add_casawizard_ua_gtest( NAME McmcDataCollector
                         SOURCES TestMcmcDataCollector.cpp)

add_casawizard_ua_gtest( NAME OptimalCaseExporter
                         SOURCES TestOptimalCaseExporter.cpp)
configure_file( Project.project3d Project.project3d COPYONLY )
configure_file( Project.txt Project.txt COPYONLY )
configure_file( Inputs.HDF Inputs.HDF )

add_casawizard_ua_gtest( NAME RunCaseSetFileManager
                         SOURCES testRunCaseSetFileManager.cpp )
