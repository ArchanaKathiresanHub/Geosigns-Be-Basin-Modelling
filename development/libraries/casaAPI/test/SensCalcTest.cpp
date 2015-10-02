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
              "\"" << name << "\"" << std::endl;
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

   const casa::RSProxySet & proxySet = m_sc->rsProxySet();

   const casa::RSProxy * secOrdProx = proxySet.rsProxy( "SecOrdBB" );
   
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

