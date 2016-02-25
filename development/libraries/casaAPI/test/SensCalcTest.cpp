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

   std::unique_ptr<casa::ScenarioAnalysis> m_sc;

   bool   m_validationMode;
   double eps;
};

double tornadoSensVals[9][4] = { 
   { 55.6078,     81.0514,  -42.0209, 41.7564 },
   { 81.258,      121.177,  -42.0117, 41.7914 },
   { 88.4597,     132.449,  -42.0283, 41.7914 },
   { 99.6968,     150.208,  -42.0177, 41.8049 },
   { 111.823,     169.604,  -41.9781, 41.8438 },
   { 0.431208,    0.599471, -34.0939, 41.1183 },
   { 0.711283,    1.32084,  -26.7844, 43.1506 },
   { 6.81877e-06, 161.83,   -2.03658, 89.2981 },
   { 0.0623489,   221.102,  -8.02639, 64.1602 }
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
   doeNames.push_back( "Tornado" );
   doeNames.push_back( "FullFactorila" );

   std::vector<TornadoSensitivityInfo> tornadoData = sensCalc.calculateTornado( m_sc->doeCaseSet(), doeNames );
   ASSERT_EQ( tornadoData.size(), 9U ); // number of observables

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


double paretoSensValues[] = { 79.6204, 7.57774, 7.11666, 3.78362, 1.46877, 0.43279  };

const char * paretoIPNames[] = { "TopCrustHeatProdRate [\\mu W/m^3]"
                               , "EventStartTime [Ma]"
                               , "InitialCrustThickness [m]"
                               , "CrustThinningFactor [m/m]"
                               , "Lower Jurassic TOC [%]"
                               , "EventDuration [Ma]"
                              };

TEST_F( SensCalcTest, SensitivityCalculatorParetoTest )
{
   ASSERT_EQ( ErrorHandler::NoError, m_sc->errorCode() );

   const casa::RSProxy * secOrdProx = m_sc->rsProxySet().rsProxy( "SecondOrder" );
   
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


double paretoCyclicSensValues[5][6] = { { 79.1661, 7.54919, 7.03656, 4.15736, 1.66057, 0.430239 },
                                        { 79.3384, 7.55971, 7.06475, 4.01406, 1.59182, 0.431209 },
                                        { 79.424,  7.56502, 7.07935, 3.94333, 1.55661, 0.431702 },
                                        { 79.4743, 7.56819, 7.08822, 3.90195, 1.53538, 0.431993 },
                                        { 79.5068, 7.57026, 7.09414, 3.87525, 1.52133, 0.43218  },
                                      };

const char * paretoCyclicIPName[5][6] = { 
 { "TopCrustHeatProdRate [\\mu W/m^3]", "EventStartTime [Ma]", "InitialCrustThickness [m]", "CrustThinningFactor [m/m]", "Lower Jurassic TOC [%]", "EventDuration [Ma]" },
 { "TopCrustHeatProdRate [\\mu W/m^3]", "EventStartTime [Ma]", "InitialCrustThickness [m]", "CrustThinningFactor [m/m]", "Lower Jurassic TOC [%]", "EventDuration [Ma]" },
 { "TopCrustHeatProdRate [\\mu W/m^3]", "EventStartTime [Ma]", "InitialCrustThickness [m]", "CrustThinningFactor [m/m]", "Lower Jurassic TOC [%]", "EventDuration [Ma]" },
 { "TopCrustHeatProdRate [\\mu W/m^3]", "EventStartTime [Ma]", "InitialCrustThickness [m]", "CrustThinningFactor [m/m]", "Lower Jurassic TOC [%]", "EventDuration [Ma]" },
 { "TopCrustHeatProdRate [\\mu W/m^3]", "EventStartTime [Ma]", "InitialCrustThickness [m]", "CrustThinningFactor [m/m]", "Lower Jurassic TOC [%]", "EventDuration [Ma]" },
};


TEST_F( SensCalcTest, SensitivityCalculatorCyclicParetoTest )
{
   const casa::RSProxy * secOrdProx = m_sc->rsProxySet().rsProxy( "SecondOrder" );

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
