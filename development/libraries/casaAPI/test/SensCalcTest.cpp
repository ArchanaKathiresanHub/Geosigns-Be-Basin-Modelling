#include "../src/cmbAPI.h"
#include "../src/casaAPI.h"

#include <memory>
//#include <cmath>

#include <gtest/gtest.h>

using namespace casa;


class SensCalcTest : public ::testing::Test
{
public:
   SensCalcTest()
   {       
      eps = 1.0e-3;
      m_validationMode = false;

      m_sc.reset( casa::ScenarioAnalysis::loadScenario( "Ottoland_casa_state.txt", "txt" ) );

      if ( ErrorHandler::NoError != m_sc->errorCode() )
      {
         std::cerr << "Deserialization failed: " << m_sc->errorMessage();
      }
   }

   ~SensCalcTest() {;}

   std::auto_ptr<casa::ScenarioAnalysis> m_sc;

   bool   m_validationMode;
   double eps;
};

double tornadoSensVals[9][4] = { 
   { 71.2261,  86.8823,  -34.2862,  32.6302 },
   { 105.925,  130.491,  -34.2572,  32.6538 },
   { 115.647,  142.738,  -34.2513,  32.6542 },
   { 130.91,   162.055,  -34.2464,  32.6557 },
   { 147.502,  183.167,  -34.2418,  32.6658 },
   { 0.523384, 0.644557, -35.1909,  35.7262 },
   { 1.01984,  1.55975,  -31.6728,  36.8529 },
   { -21.5211, 261.72,    -8.27071, 27.5288 },
   { 22.9862,  150.407,  -10.8376,  12.4701 }
};

const char * observablesName[] = { "Temperature(460001,6.75e+06,1293,0)"
                                 , "Temperature(460001,6.75e+06,2129,0)"
                                 , "Temperature(460001,6.75e+06,2362,0)" 
                                 , "Temperature(460001,6.75e+06,2751,0)"
                                 , "Temperature(460001,6.75e+06,3200,0)" 
                                 , "Vr(460001,6.75e+06,1590,0)" 
                                 , "Vr(460001,6.75e+06,2722,0)" 
                                 , "OilExpelledCumulative(460001,6.75e+06,Lower Jurassic,0)" 
                                 , "HcGasExpelledCumulative(460001,6.75e+06,Lower Jurassic,0)" 
                                 };
const char * mostInfluentialPrmName[] = { "TopCrustHeatProdRate [\\mu W/m^3]"
                                        , "TopCrustHeatProdRate [\\mu W/m^3]"
                                        , "TopCrustHeatProdRate [\\mu W/m^3]"
                                        , "TopCrustHeatProdRate [\\mu W/m^3]"
                                        , "TopCrustHeatProdRate [\\mu W/m^3]"
                                        , "TopCrustHeatProdRate [\\mu W/m^3]"
                                        , "TopCrustHeatProdRate [\\mu W/m^3]"
                                        , "TopCrustHeatProdRate [\\mu W/m^3]"
                                        , "TopCrustHeatProdRate [\\mu W/m^3]"
                                        };

TEST_F( SensCalcTest, SensitivityCalculatorTornadoTest )
{
   ASSERT_EQ( ErrorHandler::NoError, m_sc->errorCode() );

   casa::SensitivityCalculator & sensCalc = m_sc->sensitivityCalculator();
   std::vector<std::string> doeNames;
   doeNames.push_back( "BoxBehnken" );
   doeNames.push_back( "SpaceFilling" );

   const casa::RunCaseSet & cs = m_sc->doeCaseSet();
   size_t csNum = cs.size();

   std::vector<TornadoSensitivityInfo> tornadoData = sensCalc.calculateTornado( m_sc->doeCaseSet(), doeNames );
   ASSERT_EQ( tornadoData.size(), 9 ); // number of observables

   for ( size_t i = 0; i < tornadoData.size(); ++i ) // do test only for 1 observable
   {
      const Observable * obs = tornadoData[i].observable();
      const std::vector<std::string> & obsNames = obs->name();

      int subObsNum = tornadoData[i].observableSubID();
      const std::string & obsName = obsNames[subObsNum];

      // first observable is a temperature at 1293 m
      if ( !m_validationMode )
      {
         ASSERT_TRUE( obsName.compare( observablesName[i] ) == 0 );
      }
      else { std::cerr << "\"" << obsName << "\" "; }

      const std::vector<std::string> & prmNames = tornadoData[i].varParametersNameList();

      for ( size_t j = 0; j < 1; ++j ) // compare only one set of parameters sensitivities in this test
      {
         double minPrmAbsSens = tornadoData[i].minAbsSensitivityValue( j );
         double maxPrmAbsSens = tornadoData[i].maxAbsSensitivityValue( j );
         double minPrmRelSens = tornadoData[i].minRelSensitivityValue( j );
         double maxPrmRelSens = tornadoData[i].maxRelSensitivityValue( j );

         const std::string  & name      = prmNames[j];
         const VarParameter * vprm      = tornadoData[i].varParameter( j );
         int                  subPrmNum = tornadoData[i].varParameterSubID( j );

         // check results
         if ( !m_validationMode )
         {
            ASSERT_NEAR( minPrmAbsSens, tornadoSensVals[i][0], eps );
            ASSERT_NEAR( maxPrmAbsSens, tornadoSensVals[i][1], eps );
            ASSERT_NEAR( minPrmRelSens, tornadoSensVals[i][2], eps );
            ASSERT_NEAR( maxPrmRelSens, tornadoSensVals[i][3], eps );
            ASSERT_TRUE( name.compare( mostInfluentialPrmName[i] ) == 0 );
            ASSERT_EQ( subPrmNum, 0 );
         }
         else
         {
            std::cerr << "{ " << minPrmAbsSens << ", " << maxPrmAbsSens << ", " << minPrmRelSens << ", " << maxPrmRelSens << " } " <<
               "\"" << name << "\", " << subPrmNum << std::endl;
         }
      }
   }
}

double paretoSensValues[] = { 46.4735, 19.526, 10.8667, 10.2828, 6.81904, 5.16092, 0.871128 };

const char * paretoIPNames[] = { "TopCrustHeatProdRate [\\mu W/m^3]"
                               , "LwJurassicSRTOC"
                               , "LwJurassicSRType"
                               , "EventStartTime [Ma]"
                               , "CrustThinningFactor [m/m]"
                               , "InitialCrustThickness [m]"
                               , "EventDuration [Ma]"
                              };

TEST_F( SensCalcTest, SensitivityCalculatorParetoTest )
{
   ASSERT_EQ( ErrorHandler::NoError, m_sc->errorCode() );

   const casa::RSProxy * secOrdProx = m_sc->rsProxySet().rsProxy( "SecOrdBB" );
   
   ASSERT_TRUE( secOrdProx != NULL );

   casa::SensitivityCalculator & sensCalc = m_sc->sensitivityCalculator();
   casa::ParetoSensitivityInfo paretoData;

   ASSERT_EQ( ErrorHandler::NoError, sensCalc.calculatePareto( secOrdProx, paretoData ) );

   for ( size_t i = 0; i < paretoData.m_vprmPtr.size(); ++i )
   {
      const casa::VarParameter * prm = paretoData.m_vprmPtr[i];
      int prmSubId = paretoData.m_vprmSubID[i];
      std::string prmName = (prm->name())[prmSubId];
      double prmSens = paretoData.getSensitivity( prm, prmSubId );
      
      if ( !m_validationMode )
      {
         ASSERT_NEAR( prmSens, paretoSensValues[i], eps );
         ASSERT_TRUE( prmName.compare( paretoIPNames[i] ) == 0 );
      }
      else
      {
         std::cerr << prmSens << ", \"" << prmName << "\"" << std::endl;
      }
   }
}


double paretoCyclicSensValues[5][7] = { { 54.2874, 11.6106, 11.0444, 10.2629, 6.49942, 5.45748, 0.837727 },
                                        { 54.7406, 11.2622, 11.1563, 10.2754, 6.25168, 5.47749, 0.836298 },
                                        { 54.974,  11.3719, 10.9227, 10.283,  6.12461, 5.48808, 0.83567  },
                                        { 55.1154, 11.4371, 10.7814, 10.2883, 6.04796, 5.49463, 0.83534  },
                                        { 55.2091, 11.4796, 10.6878, 10.292,  5.99726, 5.49905, 0.835148 },
                                      };

const char * paretoCyclicIPName[5][7] = { 
   { "TopCrustHeatProdRate [\\mu W/m^3]", "LwJurassicSRTOC",           "CrustThinningFactor [m/m]", "EventStartTime [Ma]", "LwJurassicSRType", "InitialCrustThickness [m]", "EventDuration [Ma]" },
   { "TopCrustHeatProdRate [\\mu W/m^3]", "CrustThinningFactor [m/m]", "LwJurassicSRTOC",           "EventStartTime [Ma]", "LwJurassicSRType", "InitialCrustThickness [m]", "EventDuration [Ma]" },
   { "TopCrustHeatProdRate [\\mu W/m^3]", "CrustThinningFactor [m/m]", "LwJurassicSRTOC",           "EventStartTime [Ma]", "LwJurassicSRType", "InitialCrustThickness [m]", "EventDuration [Ma]" },
   { "TopCrustHeatProdRate [\\mu W/m^3]", "CrustThinningFactor [m/m]", "LwJurassicSRTOC",           "EventStartTime [Ma]", "LwJurassicSRType", "InitialCrustThickness [m]", "EventDuration [Ma]" },
   { "TopCrustHeatProdRate [\\mu W/m^3]", "CrustThinningFactor [m/m]", "LwJurassicSRTOC",           "EventStartTime [Ma]", "LwJurassicSRType", "InitialCrustThickness [m]", "EventDuration [Ma]" },
};

TEST_F( SensCalcTest, SensitivityCalculatorCyclicParetoTest )
{
   const casa::RSProxy * secOrdProx = m_sc->rsProxySet().rsProxy( "SecOrdBB" );

   ASSERT_TRUE( secOrdProx != NULL );

   casa::SensitivityCalculator & sensCalc = m_sc->sensitivityCalculator();

   //////////////////////////////////////////
   // Create pareto with weights 1.0
   casa::ParetoSensitivityInfo paretoDataEQW;

   // set all observable SA weights to 1.0
   for ( size_t o = 0; o < m_sc->obsSpace().size(); ++o )
   {
      const Observable * ob = m_sc->obsSpace().observable( o );
      const_cast<Observable *>(ob)->setSAWeight( 1.0 );
   }

   // get pareto data for equal weighting
   ASSERT_EQ( ErrorHandler::NoError, sensCalc.calculatePareto( secOrdProx, paretoDataEQW ) );

   // list of parameters - can be different various weights value
   std::vector<std::string> prmNamesEQW;
   // list of parameters sensitivities
   std::vector<double> sensDataEQW;

   for ( size_t i = 0; i < paretoDataEQW.m_vprmPtr.size(); ++i )
   {
      const VarParameter * prm = paretoDataEQW.m_vprmPtr[i];
      int prmSubId = paretoDataEQW.m_vprmSubID[i];
      prmNamesEQW.push_back( prm->name()[prmSubId] );
      sensDataEQW.push_back( paretoDataEQW.getSensitivity( prm, prmSubId ) );
   }

   //////////////////////////////////////////
   // Create set of 5 pareto charts with weights variation
   for ( size_t p = 0; p < 5; ++p )
   {
      // keep results only till the next loop. It will simulate
      // user interaction and pareto recalculation
      std::vector<double>  sensDataVW;
      std::vector<std::string>  prmNamesVW;
      ParetoSensitivityInfo paretoDataVW;

      // variate in some way SA weights for observables
      // user updates SA weights
      for ( size_t o = 0; o < m_sc->obsSpace().size(); ++o )
      {
         const Observable * ob = m_sc->obsSpace().observable( o );
         const_cast<Observable *>(ob)->setSAWeight( 1.0 - 1.0 / (2.0 + o + p) );
      }
      // get pareto data for the new weighting
      ASSERT_EQ( ErrorHandler::NoError, sensCalc.calculatePareto( secOrdProx, paretoDataVW ) );
      // collect new pareto data
      for ( size_t i = 0; i < paretoDataVW.m_vprmPtr.size(); ++i )
      {
         const VarParameter * prm = paretoDataVW.m_vprmPtr[i];
         int prmSubId = paretoDataVW.m_vprmSubID[i];
         prmNamesVW.push_back( prm->name()[prmSubId] );
         sensDataVW.push_back( paretoDataVW.getSensitivity( prm, prmSubId ) );
      }

      // use new pareto (plot for example), in test case just check numbers for the
      // first parameter in the chart
      if ( !m_validationMode )
      {
         for ( size_t i = 0; i < paretoDataVW.m_vprmPtr.size(); ++i )
         {
            ASSERT_NEAR( sensDataVW[i], paretoCyclicSensValues[p][i], eps );
            ASSERT_EQ( prmNamesVW[i], std::string( paretoCyclicIPName[p][i] ) );
         }
      }
      else
      {
         std::cerr << "{";
         for ( size_t i = 0; i < paretoDataVW.m_vprmPtr.size(); ++i ) { std::cerr << (i == 0 ? " " : ", ") << sensDataVW[i]; }
         std::cerr << " };" << std::endl;

         std::cerr << "{";
         for ( size_t i = 0; i < paretoDataVW.m_vprmPtr.size(); ++i ) { std::cerr << (i == 0 ? " " : ", ") << "\"" << prmNamesVW[i] << "\""; }
         std::cerr << " };" << std::endl;
      }
   }
}
