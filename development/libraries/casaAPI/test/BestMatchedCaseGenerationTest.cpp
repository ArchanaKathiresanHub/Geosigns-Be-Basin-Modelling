#include "../src/cmbAPI.h"
#include "../src/casaAPI.h"

#include "FilePath.h"
#include "FolderPath.h"

#include <memory>
#include <cmath>

#include <gtest/gtest.h>

using namespace casa;

class BestMatchedCaseGenerationTest : public ::testing::Test
{
public:
   BestMatchedCaseGenerationTest() { ; }
   ~BestMatchedCaseGenerationTest() { ; }

   double relativeError( double v1, double v2 ) { return fabs( (v1 - v2) / (v1 + v2) ); }

};

TEST_F( BestMatchedCaseGenerationTest, LoadScenarioAndGenerateCalibratedCase )
{
   return;
   try
   {
      // create new scenario analysis
      casa::ScenarioAnalysis * sc = casa::ScenarioAnalysis::loadScenario( "BestMatchCaseGeneration_state.txt", "txt" );
      if ( ErrorHandler::NoError != sc->errorCode() )
      {
         std::cerr << "Deserialization failed: " << sc->errorMessage();
      }
      ASSERT_EQ( ErrorHandler::NoError, sc->errorCode() );

      ibs::FolderPath pathToCaseSet = ibs::FolderPath( "." );
      pathToCaseSet << "CaseSet";

      // to prevent failure on the second test run - clean folder
      if ( !pathToCaseSet.empty() ) pathToCaseSet.clean();

      ASSERT_EQ( ErrorHandler::NoError, sc->setScenarioLocation( pathToCaseSet.path().c_str() ) );
      ASSERT_EQ( ErrorHandler::NoError, sc->saveCalibratedCase( "NVGBestMatchCase.project3d", 1 ) );

      // cleaning files/folders
      pathToCaseSet.clean();  // clean folder ./CaseSet/Iteration_1
      pathToCaseSet.remove(); // delete folder ./CaseSet

      ASSERT_FALSE( pathToCaseSet.exists() );

   }
   catch ( const ErrorHandler::Exception & ex )
   {
      FAIL() <<  "Error code:" << ex.errorCode()  << ", message: " << ex.what();
   }
}
