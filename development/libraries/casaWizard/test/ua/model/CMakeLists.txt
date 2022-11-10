add_casawizard_ua_gtest( NAME ArgumentList 
   SOURCES testArgumentList.cpp )
add_casawizard_ua_gtest( NAME DoeOption
   SOURCES testDoeOption.cpp
   AUXINCLUDEDIRS ${SUMPP_INCLUDE_DIR}
   AUXLIBS SUMlib )

add_casawizard_ua_gtest( NAME UaScenario
   SOURCES testUaScenario.cpp )

add_casawizard_ua_gtest( NAME PredictionTargetManager
   SOURCES testPredictionTargetManager.cpp )

add_casawizard_ua_gtest( NAME InfluentialParameterManager
   SOURCES testInfluentialParameterManager.cpp )

add_casawizard_ua_gtest( NAME MonteCarloData
   SOURCES testMonteCarloData.cpp )

add_casawizard_ua_gtest( NAME McmcSettings
   SOURCES TestMcmcSettings.cpp )

add_casawizard_ua_gtest( NAME ManualDesignPoint
   SOURCES TestManualDesignPoint.cpp )

add_casawizard_ua_gtest( NAME ManualDesignPointManager
   SOURCES testManualDesignPointManager.cpp )

add_casawizard_ua_gtest( NAME PredictionTargetCreator
   SOURCES TestPredictionTargetCreator.cpp )

add_casawizard_ua_gtest( NAME PredictionTargetCreatorFromWells
   SOURCES TestPredictionTargetCreatorFromWells.cpp )

add_casawizard_ua_gtest( NAME PredictionTargetCreatorFromASCII
   SOURCES TestPredictionTargetCreatorFromAscii.cpp )

add_casawizard_ua_gtest( NAME PredictionTargetDepth
   SOURCES TestPredictionTargetDepth.cpp )

add_casawizard_ua_gtest( NAME PredictionTargetSurface
   SOURCES TestPredictionTargetSurface.cpp )

add_casawizard_ua_gtest( NAME TargetQC
   SOURCES TestTargetQC.cpp )

add_casawizard_ua_gtest( NAME UAResultsTargetsData
   SOURCES TestUAResultsTargetsData.cpp )

configure_file( predictionTargetManager.dat predictionTargetManager.dat )
configure_file( predictionTargetManagerv2.dat predictionTargetManagerv2.dat )
configure_file( influentialParameterManagerV1.dat influentialParameterManagerV1.dat )
configure_file( influentialParameterManagerV0.dat influentialParameterManagerV0.dat )
configure_file( influentialParameterManagerV0TCHPGrid.dat influentialParameterManagerV0TCHPGrid.dat )
configure_file( influentialParameterManagerV0Compatible.dat influentialParameterManagerV0Compatible.dat )
configure_file( manualDesignPointManager.dat manualDesignPointManager.dat )
configure_file( manualDesignPointManagerV0.dat manualDesignPointManagerV0.dat )
configure_file( monteCarloDataManagerV0.dat monteCarloDataManagerV0.dat )
configure_file( importTest.csv importTest.csv)

add_subdirectory( input )
add_subdirectory( output )
add_subdirectory( script )
