add_casawizard_sac_gtest( NAME OptimizedLithofractionDataCreator 
                          SOURCES testOptimizedLithofractionDataCreator.cpp )
configure_file( LithofractionData/Case_1/optimalLithofractions.txt LithofractionData/Case_1/optimalLithofractions.txt COPYONLY )
configure_file( wellDataCase3D.csv wellDataCase3D.csv COPYONLY )
