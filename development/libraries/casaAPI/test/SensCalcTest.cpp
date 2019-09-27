#include "../src/cmbAPI.h"
#include "../src/casaAPI.h"

#include <memory>
#include <cstdlib>
#include <sstream>

#include <gtest/gtest.h>

using namespace casa;


class SensCalcTest : public ::testing::Test
{
public:
   SensCalcTest()
   {
      eps = 1.0e-3;
      m_validationMode = false;

      const char * envVal = getenv( "VALIDATE_UNIT_TEST" );
      if ( envVal )
      {
         m_validationMode = true;
      }

      m_sc.reset( casa::ScenarioAnalysis::loadScenario( "Ottoland_casa_state.txt", "txt" ) );

      if ( ErrorHandler::NoError != m_sc->errorCode() )
      {
         std::cerr << "Deserialization failed: " << m_sc->errorMessage();
      }
   }

   double reldif( double a, double b ) { return  abs( (a-b) / ( abs(a+b) > 1.e-8 ? (a+b) : 1.0 ) ); }

   ~SensCalcTest() {;}

   std::unique_ptr<casa::ScenarioAnalysis> m_sc;

   bool   m_validationMode;
   double eps;
};

double tornadoSensVals[9][4] = {
   { 740.957,    756.707,   18.9239,    20.687   },
   { 2656.62,    2710.678, -5.63549,   26.2149  },
   { 3066.6799,  3110.96,   6.6427,     22.5001  },
   { 3492.27,    3548.34,   6.20708,    36.8933  },
   { 4126.85,    4187.42,   37.0541,    47.6717  },
   { 4.64393,    4.64393,   0,          0        },
   { 4.64393,    4.64393,   0,          0        },
   { 33124.23,   33002.2,  -0.323937, -0.648485 },
   { 6041,       6013.68,  -0.379981, -0.76045  }
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
const char * mostInfluentialPrmName[] = { "VarPrmTopCrustHeatProduction"
                                        , "VarPrmTopCrustHeatProduction"
                                        , "VarPrmTopCrustHeatProduction"
                                        , "VarPrmTopCrustHeatProduction"
                                        , "VarPrmTopCrustHeatProduction"
                                        , "VarPrmTopCrustHeatProduction"
                                        , "VarPrmTopCrustHeatProduction"
                                        , "VarPrmTopCrustHeatProduction"
                                        , "VarPrmTopCrustHeatProduction"
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

   std::ostringstream sensValsOut;
   std::ostringstream prmNamesOut;
   std::ostringstream obsNamesOut;

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
      else
      {
         if ( i == 0 ) { obsNamesOut << "\nconst char * observablesName[] = {\n"; }
         obsNamesOut << "\"" << obsName << "\"" << ( i < tornadoData.size() - 1 ? "," : "" ) << "\n";
      }

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
            ASSERT_TRUE( reldif( minPrmAbsSens, tornadoSensVals[i][0] ) < eps );
            ASSERT_TRUE( reldif( maxPrmAbsSens, tornadoSensVals[i][1] ) < eps );
            ASSERT_TRUE( reldif( minPrmRelSens, tornadoSensVals[i][2] ) < eps );
            ASSERT_TRUE( reldif( maxPrmRelSens, tornadoSensVals[i][3] ) < eps );
            ASSERT_TRUE( name.compare( mostInfluentialPrmName[i] ) == 0 );
            ASSERT_EQ( subPrmNum, 0 );
         }
         else
         {
            if ( i == 0 && j == 0 )
            {
               sensValsOut << "\ndouble tornadoSensVals[" << tornadoData.size() << "][4] = { \n";
               prmNamesOut << "\nconst char * mostInfluentialPrmName[] = {\n";
            }
            sensValsOut << "   { " << std::scientific <<
                                   minPrmAbsSens << ", " << maxPrmAbsSens << ", " << minPrmRelSens << ", " << maxPrmRelSens <<
                             " }" << ( i < tornadoData.size() - 1 ? "," : "" ) << "\n";
            prmNamesOut << "\"" << name << "\"" << ( i < tornadoData.size() - 1 ? "," : "" ) << "\n";

            if ( i == tornadoData.size() - 1 && j == 0 )
            {
               sensValsOut << "};\n";
               prmNamesOut << "};\n";
            }
         }
      }
      if ( m_validationMode && i == tornadoData.size() - 1 ) { obsNamesOut << "};\n"; }
   }

   if ( m_validationMode ) { std::cerr << sensValsOut.str() << obsNamesOut.str() <<  prmNamesOut.str(); }
}

double paretoSensValues[] = { 26.6732, 20.1689, 18.6447, 15.3444, 12.6147, 6.55409 };

const char * paretoIPNames[] = {
      "VarPrmTopCrustHeatProduction",
      "EventStartTime [Ma]",
      "Lower Jurassic TOC [%]",
      "CrustThinningFactor [m/m]",
      "EventDuration [Ma]",
      "InitialCrustThickness [m]"
};


TEST_F( SensCalcTest, SensitivityCalculatorParetoTest )
{
   std::ostringstream prmVals;
   std::ostringstream prmNames;

   ASSERT_EQ( ErrorHandler::NoError, m_sc->errorCode() );

   casa::RSProxy * secOrdProx = m_sc->rsProxySet().rsProxy( "SecondOrder" );

   ASSERT_TRUE( secOrdProx != NULL );

   casa::SensitivityCalculator & sensCalc = m_sc->sensitivityCalculator();
   casa::ParetoSensitivityInfo paretoData;

   ASSERT_EQ( ErrorHandler::NoError, sensCalc.calculatePareto( secOrdProx, paretoData ) );

   if ( m_validationMode )
   {
      prmVals  << "\ndouble paretoSensValues[] = { ";
      prmNames << "\nconst char * paretoIPNames[] = {\n";
   }

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
         prmVals  << prmSens << (i == (paretoData.m_vprmPtr.size()-1) ? "" : ", ");
         prmNames << "   \"" << prmName << "\"" << (i == (paretoData.m_vprmPtr.size()-1) ? "" : ", ") << "\n";
      }
   }
   if ( m_validationMode )
   {
      prmVals  << " };\n";
      prmNames << " };\n";
      std::cerr << prmVals.str() << prmNames.str();
   }
}
double paretoCyclicSensValues[5][6] = {
     {  26.6732, 20.1689, 18.6447, 15.3444, 12.6147, 6.55409 },
     {  26.6732, 20.1689, 18.6447, 15.3444, 12.6147, 6.55409 },
     {  26.6732, 20.1689, 18.6447, 15.3444, 12.6147, 6.55409 },
     {  26.6732, 20.1689, 18.6447, 15.3444, 12.6147, 6.55409 },
     {  26.6732, 20.1689, 18.6447, 15.3444, 12.6147, 6.55409 }
};

const char * paretoCyclicIPName[5][6] = {
      { "VarPrmTopCrustHeatProduction", "EventStartTime [Ma]", "Lower Jurassic TOC [%]", "CrustThinningFactor [m/m]", "EventDuration [Ma]", "InitialCrustThickness [m]" },
      { "VarPrmTopCrustHeatProduction", "EventStartTime [Ma]", "Lower Jurassic TOC [%]", "CrustThinningFactor [m/m]", "EventDuration [Ma]", "InitialCrustThickness [m]" },
      { "VarPrmTopCrustHeatProduction", "EventStartTime [Ma]", "Lower Jurassic TOC [%]", "CrustThinningFactor [m/m]", "EventDuration [Ma]", "InitialCrustThickness [m]" },
      { "VarPrmTopCrustHeatProduction", "EventStartTime [Ma]", "Lower Jurassic TOC [%]", "CrustThinningFactor [m/m]", "EventDuration [Ma]", "InitialCrustThickness [m]" },
      { "VarPrmTopCrustHeatProduction", "EventStartTime [Ma]", "Lower Jurassic TOC [%]", "CrustThinningFactor [m/m]", "EventDuration [Ma]", "InitialCrustThickness [m]" }

};

TEST_F( SensCalcTest, SensitivityCalculatorCyclicParetoTest )
{
   casa::RSProxy * secOrdProx = m_sc->rsProxySet().rsProxy( "SecondOrder" );

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

   std::ostringstream cyclSensVal;
   std::ostringstream cyclSensNam;

   cyclSensVal << "double paretoCyclicSensValues[5][6] = {\n";
   cyclSensNam << "const char * paretoCyclicIPName[5][6] = {\n";

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
         cyclSensVal << "  { ";
         for ( size_t i = 0; i < paretoDataVW.m_vprmPtr.size(); ++i ) { cyclSensVal << (i == 0 ? " " : ", ") << sensDataVW[i]; }
         cyclSensVal << " }" << (p == 4 ? "" : ",") << "\n";

         cyclSensNam << "   {";
         for ( size_t i = 0; i < paretoDataVW.m_vprmPtr.size(); ++i ) { cyclSensNam << (i == 0 ? " " : ", ") << "\"" << prmNamesVW[i] << "\""; }
         cyclSensNam << " }" << (p == 4 ? "" : ",") << "\n";
      }
   }
   if ( m_validationMode )
   {
      cyclSensVal << "};\n";
      cyclSensNam << "};\n";

      std::cerr << cyclSensVal.str() << cyclSensNam.str();
   }
}
