#include "../src/cmbAPI.h"
#include "../src/casaAPI.h"
#include <gtest/gtest.h>

#include <string>
#include <fstream>

using namespace casa;

class SerializationTest : public ::testing::Test
{
public:
   SerializationTest() { ; }
   ~SerializationTest( ) { ; }
};


// RunManager test. There is 1 DoE Tornado experiment with 2 parameters
// Test creates DoE and generates set of project files. 
// Then it spawn 1 job to the cluster and check the execution results
TEST_F( SerializationTest, ReloadStateFromTxtTest )
{
   // create new scenario analysis
   casa::ScenarioAnalysis * sc = casa::ScenarioAnalysis::loadScenario( "Ottoland_casa_state.txt", "txt" );
   if ( ErrorHandler::NoError != sc->errorCode() )
   {
      std::cerr << "Deserialization failed: " << sc->errorMessage();
   }
   ASSERT_EQ( ErrorHandler::NoError, sc->errorCode() );

   // Do round trip
   // create 1st reloaded scenario
   sc->saveScenario( "casa_state_reloaded_1.txt", "txt" );
   delete sc;
   // load it again
   sc = casa::ScenarioAnalysis::loadScenario( "casa_state_reloaded_1.txt", "txt" );
   // save it as new one
   sc->saveScenario( "casa_state_reloaded_2.txt", "txt" );
   delete sc; // clean and close all

   // compare files
   std::ifstream org( "casa_state_reloaded_1.txt", std::ifstream::in );
   std::ifstream rel( "casa_state_reloaded_2.txt", std::ifstream::in );
   while ( !org.eof() && org.good() )
   {
      ASSERT_TRUE( rel.good() );

      int oc = org.get();
      int rc = rel.get();

      ASSERT_EQ( oc, rc );
   }
}
