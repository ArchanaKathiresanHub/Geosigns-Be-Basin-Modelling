#include "../src/cmbAPI.h"
#include "../src/casaAPI.h"
#include "../src/VarSpaceImpl.h"
#include "../src/PrmSourceRockTOC.h"
#include "../src/PrmTopCrustHeatProduction.h"
#include "../src/VarPrmSourceRockTOC.h"
#include "../src/VarPrmTopCrustHeatProduction.h"

#include <memory>
//#include <cmath>

#include <gtest/gtest.h>

using namespace casa;

static const double eps = 1.e-5;

// Test Business logic rules set (BLRS) methods in  CASA API
class BLRSTest : public ::testing::Test
{
public:
   BLRSTest()  { ; }
   ~BLRSTest() { ; }
};

// Test how ones can add variable parameter TopCrustHeatProduction to scenario analysis
TEST_F( BLRSTest, VaryTopCrustHeatProductionTest )
{
   // create new scenario analysis
   ScenarioAnalysis sc;

   // load base case to scenario
   ASSERT_EQ( ErrorHandler::NoError, sc.defineBaseCase( "Ottoland.project3d" ) );

   // add the new variable parameter TopCrustHeatProduction to the scenario analysis by using one of the BLRS API function
   ASSERT_EQ( ErrorHandler::NoError, casa::BusinessLogicRulesSet::VaryTopCrustHeatProduction( sc,  0.2, 4.0, VarPrmContinuous::Block ) );

   // get varspace 
   casa::VarSpaceImpl & varPrms = dynamic_cast<casa::VarSpaceImpl&>( sc.varSpace() );

   // check how the parameter was set
   ASSERT_EQ( varPrms.size(), 1 );
   const VarPrmTopCrustHeatProduction * p1c = dynamic_cast<const VarPrmTopCrustHeatProduction*>( varPrms.continuousParameter( 0 ) );

   ASSERT_TRUE( p1c != NULL ); // do we have the required parameter in the list?
   ASSERT_NEAR( p1c->minValueAsDouble(), 0.2, eps ); // does it range have given min value
   ASSERT_NEAR( p1c->maxValueAsDouble(), 4.0, eps ); // does it range have given max value
}

// Test how ones can add variable parameter source rock TOC to scenario analysis
TEST_F( BLRSTest, VarySourceRockTOCTest )
{
   // create new scenario analysis
   ScenarioAnalysis sc;

   // load base case to scenario
   ASSERT_EQ( ErrorHandler::NoError, sc.defineBaseCase( "Ottoland.project3d" ) );

   // set the parameter
   ASSERT_EQ( ErrorHandler::NoError, casa::BusinessLogicRulesSet::VarySourceRockTOC( sc, "Lower Jurassic", 10.0, 30.0, VarPrmContinuous::Block ) );

   // get varspace 
   casa::VarSpaceImpl & varPrms = dynamic_cast<casa::VarSpaceImpl&>( sc.varSpace( ) );

   // check how the parameter was set
   ASSERT_EQ( varPrms.size(), 1 );
   const VarPrmSourceRockTOC * p1c = dynamic_cast<const VarPrmSourceRockTOC*>( varPrms.continuousParameter( 0 ) );
   ASSERT_TRUE( p1c != NULL ); // do we have required the parameter in the list?
   ASSERT_NEAR( p1c->minValueAsDouble(), 10.0, eps );   // does it range have given min value
   ASSERT_NEAR( p1c->maxValueAsDouble( ), 30.0, eps );  // does it range have given max value
}

