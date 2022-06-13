add_casawizard_ua_gtest( NAME MCdataCreator 
                         SOURCES  testMCdataCreator.cpp )
configure_file( testCasaV20161104MCdata.dat testCasaV20161104MCdata.dat COPYONLY)

add_casawizard_ua_gtest( NAME TargetQCDataCreator
                         SOURCES  TestTargetQCDataCreator.cpp )
configure_file( testTargetQCDataCreatorInput/doeIndices.txt testTargetQCDataCreatorInput/doeIndices.txt COPYONLY)
configure_file( testTargetQCDataCreatorInput/proxyEvalObservables.txt testTargetQCDataCreatorInput/proxyEvalObservables.txt COPYONLY)
configure_file( testTargetQCDataCreatorInput/proxyEvalQuality.txt testTargetQCDataCreatorInput/proxyEvalQuality.txt COPYONLY)
configure_file( testTargetQCDataCreatorInput/runCasesObservables.txt testTargetQCDataCreatorInput/runCasesObservables.txt COPYONLY)
