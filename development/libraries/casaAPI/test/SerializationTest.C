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

   sc->saveScenario( "casa_state_reloaded.txt", "txt" );
   delete sc;

   std::ifstream org( "Ottoland_casa_state.txt", std::ifstream::in );
   std::ifstream rel( "casa_state_reloaded.txt", std::ifstream::in );
   while ( !org.eof() && org.good() )
   {
      ASSERT_TRUE( rel.good() );

      int oc = org.get();
      int rc = rel.get();

      ASSERT_EQ( oc, rc );
   }
}
