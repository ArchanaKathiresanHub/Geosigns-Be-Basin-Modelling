#include "../src/cmbAPI.h"
#include "../src/casaAPI.h"

#include <memory>
//#include <cmath>

#include <gtest/gtest.h>

using namespace casa;

static const double eps = 1.e-5;

class SensCalcTest : public ::testing::Test
{
public:
   SensCalcTest()
   {       
      eps = 1.0e-6;
      reps = 1.0e-2;
   }
   ~SensCalcTest() {;}

   static const char * m_projectFileName;
   static const char * m_serialisedStateFileName;

   double eps = 1.0e-6;
   double reps = 1.0e-2;
};
const char * SensCalcTest::m_projectFileName = "Ottoland.project3d";
const char * SensCalcTest::m_serialisedStateFileName = "Ottoland_casa_state.txt";


TEST_F( SensCalcTest, SensitivityCalculatorTornadoTest )
{
   return;
   std::auto_ptr<casa::ScenarioAnalysis> sc;
   sc.reset( casa::ScenarioAnalysis::loadScenario( m_serialisedStateFileName, "txt" ) );

   if ( ErrorHandler::NoError != sc->errorCode() )
   {
      std::cerr << "Deserialization failed: " << sc->errorMessage();
   }
   ASSERT_EQ( ErrorHandler::NoError, sc->errorCode() );

   casa::SensitivityCalculator & sensCalc = sc->sensitivityCalculator();
   std::vector<std::string> doeNames;
   doeNames.push_back( "BoxBehnken" );
   doeNames.push_back( "SpaceFilling" );

   const casa::RunCaseSet & cs = sc->doeCaseSet();
   size_t csNum = cs.size();

   std::vector<TornadoSensitivityInfo> tornadoData = sensCalc.calculateTornado( sc->doeCaseSet(), doeNames );
   ASSERT_EQ( tornadoData.size(), 9 ); // number of observables

   for ( size_t i = 0; i < 1; ++i ) // do test only for 1 observable
   {
      const Observable * obs = tornadoData[i].observable();
      const std::vector<std::string> & obsNames = obs->name();

      int subObsNum = tornadoData[i].observableSubID();
      const std::string & obsName = obsNames[subObsNum];

      // first observable is a temperature at 1293 m
      ASSERT_TRUE( obsName.compare( "Temperature(460001,6.75e+06,1293,0)" ) == 0 );

      const std::vector<std::string> & prmNames = tornadoData[i].varParametersNameList();

      for ( size_t j = 0; j < 1; ++j ) // compare only one set of parameters sensitivities in this test
      {
         double minPrmAbsSens = tornadoData[i].minAbsSensitivityValue( j );
         double maxPrmAbsSens = tornadoData[i].maxAbsSensitivityValue( j );
         double minPrmRelSens = tornadoData[i].minRelSensitivityValue( j );
         double maxPrmRelSens = tornadoData[i].maxRelSensitivityValue( j );
         const std::string & name = prmNames[j];

         const VarParameter * vprm = tornadoData[i].varParameter( j );
         int subPrmNum = tornadoData[i].varParameterSubID( j );

         // check results
         ASSERT_TRUE( std::abs( minPrmAbsSens - 55.730 )  < 1e-3 );
         ASSERT_TRUE( std::abs( maxPrmAbsSens - 81.3728 ) < 1e-3 );
         ASSERT_TRUE( std::abs( minPrmRelSens + 42.0058 ) < 1e-3 );
         ASSERT_TRUE( std::abs( maxPrmRelSens - 41.7406 ) < 1e-3 );
         ASSERT_TRUE( name.compare( "TopCrustHeatProdRate [\\mu W/m^3]" ) == 0 );
         ASSERT_EQ( subPrmNum, 0 );
      }
   }
}
