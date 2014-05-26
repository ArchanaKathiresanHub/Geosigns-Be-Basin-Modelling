#include "../src/cmbAPI.h"
#include "../src/casaAPI.h"
#include "../src/PrmSourceRockTOC.h"
#include "../src/PrmTopCrustHeatProduction.h"
#include "../src/VarPrmSourceRockTOC.h"
#include "../src/VarPrmTopCrustHeatProduction.h"

#include <memory>
//#include <cmath>

#include <gtest/gtest.h>

using namespace casa;

static const double eps = 1.e-5;

class DoETest : public ::testing::Test
{
public:
   DoETest()  { ; }
   ~DoETest() {;}
};
  
TEST_F( DoETest, Tornado2Prms )
{
   std::auto_ptr<casa::ScenarioAnalysis> sc;
   sc.reset( new ScenarioAnalysis() );

   ASSERT_EQ( ErrorHandler::NoError, sc->defineBaseCase( "Project.project3d" ) );
   ASSERT_EQ( ErrorHandler::NoError, sc->setDoEAlgorithm( DoEGenerator::Tornado ) );

   casa::DoEGenerator & doe = sc->doeGenerator();
   casa::VarSpace     & varPrms = sc->varSpace();
   casa::RunCaseSet   & doeCaseSet = sc->doeCaseSet();

   ASSERT_EQ( ErrorHandler::NoError, varPrms.addParameter( new VarPrmSourceRockTOC( "Layer1", 25, 10, 40, VarPrmContinuous::Block ) ) );
   ASSERT_EQ( ErrorHandler::NoError, varPrms.addParameter( new VarPrmTopCrustHeatProduction( 2.05, 0.1, 4.0, VarPrmContinuous::Block ) ) );

   doe.generateDoE( varPrms, doeCaseSet );

   ASSERT_EQ( 5, doeCaseSet.size() );
   for ( size_t i = 0; i < doeCaseSet.size(); ++i )
   {
      ASSERT_EQ( 2, doeCaseSet[i]->parametersNumber() );

      const casa::PrmSourceRockTOC * prm1 = dynamic_cast<casa::PrmSourceRockTOC*>( doeCaseSet[ i ]->parameter( 0 ) );
      ASSERT_TRUE( prm1 != NULL );

      const casa::PrmTopCrustHeatProduction * prm2 = dynamic_cast<casa::PrmTopCrustHeatProduction*>( doeCaseSet[ i ]->parameter( 1 ) );
      ASSERT_TRUE( prm2 != NULL );
      double val1 = prm1->value();
      double val2 = prm2->value();

      switch ( i )
      {
      case 0: EXPECT_NEAR( val1, 25.0, eps ); EXPECT_NEAR( val2, 2.05, eps ); break;
      case 1: EXPECT_NEAR( val1, 10.0, eps ); EXPECT_NEAR( val2, 2.05, eps ); break;
      case 2: EXPECT_NEAR( val1, 40.0, eps ); EXPECT_NEAR( val2, 2.05, eps ); break;
      case 3: EXPECT_NEAR( val1, 25.0, eps ); EXPECT_NEAR( val2, 0.1, eps ); break;
      case 4: EXPECT_NEAR( val1, 25.0, eps ); EXPECT_NEAR( val2, 4.0, eps ); break;
      }
   }
}

TEST_F( DoETest, BoxBehnken2Prms )
{
   casa::ScenarioAnalysis sc;
   
   ASSERT_EQ( ErrorHandler::NoError, sc.defineBaseCase( "Project.project3d" ) );
   ASSERT_EQ( ErrorHandler::NoError, sc.setDoEAlgorithm( DoEGenerator::BoxBehnken ) );
   
   casa::DoEGenerator & doe = sc.doeGenerator( );
   casa::VarSpace     & varPrms = sc.varSpace( );
   casa::RunCaseSet   & doeCaseSet = sc.doeCaseSet();
   
   ASSERT_EQ( ErrorHandler::NoError, varPrms.addParameter( new VarPrmSourceRockTOC( "Layer1", 25, 10, 40, VarPrmContinuous::Block ) ) );
   ASSERT_EQ( ErrorHandler::NoError, varPrms.addParameter( new VarPrmTopCrustHeatProduction( 2.05, 0.1, 4.0, VarPrmContinuous::Block ) ) );
   
   doe.generateDoE( varPrms, doeCaseSet );
   
   ASSERT_EQ( 5, doeCaseSet.size( ) );
   for ( size_t i = 0; i < doeCaseSet.size( ); ++i )
   {
      ASSERT_EQ( 2, doeCaseSet[ i ]->parametersNumber() );
   
      const casa::PrmSourceRockTOC * prm1 = dynamic_cast<casa::PrmSourceRockTOC*>( doeCaseSet[ i ]->parameter( 0 ) );
      ASSERT_TRUE( prm1 != NULL );
   
      const casa::PrmTopCrustHeatProduction * prm2 = dynamic_cast<casa::PrmTopCrustHeatProduction*>( doeCaseSet[ i ]->parameter( 1 ) );
      ASSERT_TRUE( prm2 != NULL );
      double val1 = prm1->value( );
      double val2 = prm2->value( );
   
      switch ( i )
      {
      case 0: EXPECT_NEAR( val1, 25.0, eps ); EXPECT_NEAR( val2, 2.05, eps ); break;
      case 1: EXPECT_NEAR( val1, 40.0, eps ); EXPECT_NEAR( val2,  4.0, eps ); break;
      case 2: EXPECT_NEAR( val1, 10.0, eps ); EXPECT_NEAR( val2,  4.0, eps ); break;
      case 3: EXPECT_NEAR( val1, 40.0, eps ); EXPECT_NEAR( val2,  0.1, eps ); break;
      case 4: EXPECT_NEAR( val1, 10.0, eps ); EXPECT_NEAR( val2,  0.1, eps ); break;
      }
   }
}
      
TEST_F( DoETest, FullFactorial2Prms )
{
   
   
   casa::ScenarioAnalysis sc;

   ASSERT_EQ( ErrorHandler::NoError, sc.defineBaseCase( "Project.project3d" ) );
   ASSERT_EQ( ErrorHandler::NoError, sc.setDoEAlgorithm( DoEGenerator::FullFactorial ) );

   casa::DoEGenerator & doe = sc.doeGenerator( );
   casa::VarSpace     & varPrms = sc.varSpace( );
   casa::RunCaseSet   & doeCaseSet = sc.doeCaseSet();

   ASSERT_EQ( ErrorHandler::NoError, varPrms.addParameter( new VarPrmSourceRockTOC( "Layer1", 25, 10, 40, VarPrmContinuous::Block ) ) );
   ASSERT_EQ( ErrorHandler::NoError, varPrms.addParameter( new VarPrmTopCrustHeatProduction( 2.05, 0.1, 4.0, VarPrmContinuous::Block ) ) );

   doe.generateDoE( varPrms, doeCaseSet );

   ASSERT_EQ( 5, doeCaseSet.size( ) );
   for ( size_t i = 0; i < doeCaseSet.size( ); ++i )
   {
      ASSERT_EQ( 2, doeCaseSet[ i ]->parametersNumber() );

      const casa::PrmSourceRockTOC * prm1 = dynamic_cast<casa::PrmSourceRockTOC*>( doeCaseSet[ i ]->parameter( 0 ) );
      ASSERT_TRUE( prm1 != NULL );

      const casa::PrmTopCrustHeatProduction * prm2 = dynamic_cast<casa::PrmTopCrustHeatProduction*>( doeCaseSet[ i ]->parameter( 1 ) );
      ASSERT_TRUE( prm2 != NULL );
      double val1 = prm1->value( );
      double val2 = prm2->value( );

      switch ( i )
      {
      case 0: EXPECT_NEAR( val1, 25.0, eps ); EXPECT_NEAR( val2, 2.05, eps ); break;
      case 1: EXPECT_NEAR( val1, 10.0, eps ); EXPECT_NEAR( val2, 0.1, eps ); break;
      case 2: EXPECT_NEAR( val1, 40.0, eps ); EXPECT_NEAR( val2, 0.1, eps ); break;
      case 3: EXPECT_NEAR( val1, 10.0, eps ); EXPECT_NEAR( val2, 4.0, eps ); break;
      case 4: EXPECT_NEAR( val1, 40.0, eps ); EXPECT_NEAR( val2, 4.0, eps ); break;
      }
   }
}

TEST_F( DoETest, PlackettBurman2Prms )
{
   
   
   casa::ScenarioAnalysis sc;

   ASSERT_EQ( ErrorHandler::NoError, sc.defineBaseCase( "Project.project3d" ) );
   ASSERT_EQ( ErrorHandler::NoError, sc.setDoEAlgorithm( DoEGenerator::PlackettBurman ) );

   casa::DoEGenerator & doe = sc.doeGenerator( );
   casa::VarSpace     & varPrms = sc.varSpace( );
   casa::RunCaseSet   & doeCaseSet = sc.doeCaseSet();

   ASSERT_EQ( ErrorHandler::NoError, varPrms.addParameter( new VarPrmSourceRockTOC( "Layer1", 25, 10, 40, VarPrmContinuous::Block ) ) );
   ASSERT_EQ( ErrorHandler::NoError, varPrms.addParameter( new VarPrmTopCrustHeatProduction( 2.05, 0.1, 4.0, VarPrmContinuous::Block ) ) );

   doe.generateDoE( varPrms, doeCaseSet );

   ASSERT_EQ( 4, doeCaseSet.size( ) );
   for ( size_t i = 0; i < doeCaseSet.size( ); ++i )
   {
      ASSERT_EQ( 2, doeCaseSet[ i ]->parametersNumber( ) );

      const casa::PrmSourceRockTOC * prm1 = dynamic_cast<casa::PrmSourceRockTOC*>( doeCaseSet[ i ]->parameter( 0 ) );
      ASSERT_TRUE( prm1 != NULL );

      const casa::PrmTopCrustHeatProduction * prm2 = dynamic_cast<casa::PrmTopCrustHeatProduction*>( doeCaseSet[ i ]->parameter( 1 ) );
      ASSERT_TRUE( prm2 != NULL );
      double val1 = prm1->value( );
      double val2 = prm2->value( );

      switch ( i )
      {
      case 0: EXPECT_NEAR( val1, 40.0, eps ); EXPECT_NEAR( val2, 4.0, eps ); break;
      case 1: EXPECT_NEAR( val1, 10.0, eps ); EXPECT_NEAR( val2, 4.0, eps ); break;
      case 2: EXPECT_NEAR( val1, 40.0, eps ); EXPECT_NEAR( val2, 0.1, eps ); break;
      case 3: EXPECT_NEAR( val1, 10.0, eps ); EXPECT_NEAR( val2, 0.1, eps ); break;
      }
   }
}



