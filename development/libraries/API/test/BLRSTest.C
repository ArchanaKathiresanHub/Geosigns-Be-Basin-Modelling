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

class BLRSTest : public ::testing::Test
{
public:
   BLRSTest()  { ; }
   ~BLRSTest() { ; }
};
  
TEST_F( BLRSTest, VariateTopCrustHeatProductionTest )
{
   ScenarioAnalysis sc;

   ASSERT_EQ( ErrorHandler::NoError, sc.defineBaseCase( "Ottoland.project3d" ) );
   casa::VarSpaceImpl & varPrms = dynamic_cast<casa::VarSpaceImpl&>( sc.varSpace() );
   casa::RunCaseSet   & doeCaseSet = sc.doeCaseSet();

   // set the parameter
   ASSERT_EQ( ErrorHandler::NoError, casa::BusinessLogicRulesSet::VariateTopCrustHeatProduction( sc,  0.2, 4.0, VarPrmContinuous::Block ) );

   // check how the parameter was set
   ASSERT_EQ( varPrms.size(), 1 );
   const VarPrmTopCrustHeatProduction * p1c = dynamic_cast<const VarPrmTopCrustHeatProduction*>( varPrms.continuousParameter( 0 ) );
   ASSERT_TRUE( p1c != NULL );
   ASSERT_NEAR( p1c->minValueAsDouble(), 0.2, eps );
   ASSERT_NEAR( p1c->maxValueAsDouble(), 4.0, eps );
}

TEST_F( BLRSTest, VariateSourceRockTOCTest )
{
   ScenarioAnalysis sc;

   ASSERT_EQ( ErrorHandler::NoError, sc.defineBaseCase( "Ottoland.project3d" ) );
   casa::VarSpaceImpl & varPrms = dynamic_cast<casa::VarSpaceImpl&>( sc.varSpace() );
   casa::RunCaseSet   & doeCaseSet = sc.doeCaseSet();

   // set the parameter
   ASSERT_EQ( ErrorHandler::NoError, casa::BusinessLogicRulesSet::VariateSourceRockTOC( sc, "Lower Jurassic", 10.0, 30.0, VarPrmContinuous::Block ) );

   // check how the parameter was set
   ASSERT_EQ( varPrms.size(), 1 );
   const VarPrmSourceRockTOC * p1c = dynamic_cast<const VarPrmSourceRockTOC*>( varPrms.continuousParameter( 0 ) );
   ASSERT_TRUE( p1c != NULL );
   ASSERT_NEAR( p1c->minValueAsDouble(), 10.0, eps );
   ASSERT_NEAR( p1c->maxValueAsDouble(), 30.0, eps );
}

