#include "../src/cmbAPI.h"
#include "../src/casaAPI.h"
#include <gtest/gtest.h>

#include <string>
#include <fstream>

using namespace casa;

class SerializationTest : public ::testing::Test
{
public:
   SerializationTest()
   { 
      sc.reset( casa::ScenarioAnalysis::loadScenario( "Ottoland_casa_state.txt", "txt" ) );
      if ( ErrorHandler::NoError != sc->errorCode() )
      {
         std::cerr << "Deserialization failed: " << sc->errorMessage();
      }
   }
   ~SerializationTest( ) 
   {
      ;
   }

   std::auto_ptr<casa::ScenarioAnalysis> sc;
};


// Test create scenario by loading scenario state file then it is serialised/deserialised/serialised in binary file
// two freshly created state files are compared
TEST_F( SerializationTest, ReloadStateFromBinTest )
{
   ASSERT_EQ( ErrorHandler::NoError, sc->errorCode() );

   // Do round trip
   // create 1st reloaded scenario
   sc->saveScenario( "casa_state_reloaded_1.bin", "bin" );
   
   // load it again
   casa::ScenarioAnalysis * scn = casa::ScenarioAnalysis::loadScenario( "casa_state_reloaded_1.bin", "bin" );
   // save it as new one
   scn->saveScenario( "casa_state_reloaded_2.bin", "bin" );
   delete scn;

   // compare files
   std::ifstream org( "casa_state_reloaded_1.bin", std::ifstream::in | std::ifstream::binary );
   std::ifstream rel( "casa_state_reloaded_2.bin", std::ifstream::in | std::ifstream::binary );
   while ( !org.eof() && org.good() )
   {
      ASSERT_TRUE( rel.good() );

      int oc = org.get();
      int rc = rel.get();

      ASSERT_EQ( oc, rc );
   }
}


// Test create scenario by loading scenario state file then it is serialised/deserialised/serialised in txt file
// two freshly created state files are compared
TEST_F( SerializationTest, ReloadStateFromTxtTest )
{
   ASSERT_EQ( ErrorHandler::NoError, sc->errorCode() );

   // Do round trip
   // create 1st reloaded scenario
   sc->saveScenario( "casa_state_reloaded_1.txt", "txt" );
   
   // load it again
   casa::ScenarioAnalysis * scn = casa::ScenarioAnalysis::loadScenario( "casa_state_reloaded_1.txt", "txt" );
   // save it as new one
   scn->saveScenario( "casa_state_reloaded_2.txt", "txt" );
   delete scn; // clean and close all

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

// Test is checking deserialization from memory buffer in txt format 
// the sequence of steps are:
//   1. deserealize scenario A from state txt file
//   2. serialize scenario A to text state file "casa_state_reloaded_1.txt"
//   3. read this file to memory buffer
//   4. deserialize scenarion B from memory buffer
//   5. serialize scenario B to "casa_state_reloaded_2.txt"
//   6. Compare files casa_state_reloaded_1.txt and casa_state_reloaded_2.txt
TEST_F( SerializationTest, LoadStateFromMemoryStreamTxtFmt )
{
   // check Text stream
   sc->saveScenario( "casa_state_reloaded_1.txt", "txt" );
   ASSERT_EQ( ErrorHandler::NoError, sc->errorCode() );

   // read file in memory buffer
   std::ostringstream buf;
   std::ifstream input( "casa_state_reloaded_1.txt" );
   buf << input.rdbuf();

   casa::ScenarioAnalysis * scn = casa::ScenarioAnalysis::loadScenario( buf.str().c_str(), buf.str().size() );
   ASSERT_EQ( ErrorHandler::NoError, scn->errorCode() );

   scn->saveScenario( "casa_state_reloaded_2.txt", "txt" );
   ASSERT_EQ( ErrorHandler::NoError, scn->errorCode() );
   delete scn;

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

// Test is checking deserialization from memory buffer in binary format 
// the sequence of steps are:
//   1. deserealize scenario A from state txt file
//   2. serialize scenario A to binary state file "casa_state_reloaded_1.bin"
//   3. read this file to memory buffer
//   4. deserialize scenarion B from memory buffer
//   5. serialize scenario B to "casa_state_reloaded_2.bin"
//   6. Compare files casa_state_reloaded_1.bin and casa_state_reloaded_2.bin
TEST_F( SerializationTest, LoadStateFromMemoryStreamBinFmt )
{
   ASSERT_EQ( ErrorHandler::NoError, sc->errorCode() );

   // check Text stream
   sc->saveScenario( "casa_state_reloaded_1.bin", "bin" );
   ASSERT_EQ( ErrorHandler::NoError, sc->errorCode() );

   // read file in memory buffer
   std::ifstream inpf( "casa_state_reloaded_1.bin", std::ios::binary | std::ios::ate );
   std::streamsize sz = inpf.tellg();
   inpf.seekg( 0, std::ios::beg );
   std::vector<char> buf( sz + 5 );

   ASSERT_EQ( inpf.read( buf.data(), sz ).good(), true );

   casa::ScenarioAnalysis * scn = casa::ScenarioAnalysis::loadScenario( buf.data(), sz );
   ASSERT_EQ( ErrorHandler::NoError, scn->errorCode() );

   scn->saveScenario( "casa_state_reloaded_2.bin", "bin" );
   ASSERT_EQ( ErrorHandler::NoError, scn->errorCode() );
   delete scn;

   // compare files
   std::ifstream org( "casa_state_reloaded_1.bin", std::ifstream::in );
   std::ifstream rel( "casa_state_reloaded_2.bin", std::ifstream::in );
   while ( !org.eof() && org.good() )
   {
      ASSERT_TRUE( rel.good() );

      int oc = org.get();
      int rc = rel.get();

      ASSERT_EQ( oc, rc );
   }
}
