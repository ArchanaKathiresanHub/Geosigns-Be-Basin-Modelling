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
   DoETest() : m_minTOC( 5.0 )
             , m_midTOC( 10.0 )
             , m_maxTOC( 15.0 )
             , m_minTCHP( 0.1 )
             , m_midTCHP( 2.5 )
             , m_maxTCHP( 4.9 )
   { ; }
   ~DoETest() {;}

   const double m_minTOC;
   const double m_midTOC;
   const double m_maxTOC;

   const double m_minTCHP;
   const double m_midTCHP;
   const double m_maxTCHP;
};
  
TEST_F( DoETest, Tornado2Prms )
{
   std::auto_ptr<casa::ScenarioAnalysis> sc;
   sc.reset( new ScenarioAnalysis() );

   ASSERT_EQ( ErrorHandler::NoError, sc->defineBaseCase( "Ottoland.project3d" ) );
   ASSERT_EQ( ErrorHandler::NoError, sc->setDoEAlgorithm( DoEGenerator::Tornado ) );

   // extract base case parameters values
   mbapi::Model & bsCase = sc->baseCase();
   double tchp = bsCase.tableValueAsDouble( "BasementIoTbl", 0, "TopCrustHeatProd" ); // 2.5
   ASSERT_EQ( ErrorHandler::NoError, bsCase.errorCode() );
   
   double tocLowJur = bsCase.sourceRockManager().tocIni( 1 ); // 10%
   ASSERT_EQ( ErrorHandler::NoError, bsCase.sourceRockManager().errorCode() );

   casa::DoEGenerator & doe = sc->doeGenerator();
   casa::VarSpace     & varPrms = sc->varSpace();
   casa::RunCaseSet   & doeCaseSet = sc->doeCaseSet();

   ASSERT_EQ( ErrorHandler::NoError, varPrms.addParameter( new VarPrmSourceRockTOC( "Lower Jurassic", tocLowJur, m_minTOC, m_maxTOC, VarPrmContinuous::Block ) ) );
   ASSERT_EQ( ErrorHandler::NoError, varPrms.addParameter( new VarPrmTopCrustHeatProduction( tchp, m_minTCHP, m_maxTCHP, VarPrmContinuous::Block ) ) );

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
      case 0: EXPECT_NEAR( val1, m_midTOC, eps ); EXPECT_NEAR( val2, m_midTCHP, eps ); break;
      case 1: EXPECT_NEAR( val1, m_minTOC, eps ); EXPECT_NEAR( val2, m_midTCHP, eps ); break;
      case 2: EXPECT_NEAR( val1, m_maxTOC, eps ); EXPECT_NEAR( val2, m_midTCHP, eps ); break;
      case 3: EXPECT_NEAR( val1, m_midTOC, eps ); EXPECT_NEAR( val2, m_minTCHP, eps ); break;
      case 4: EXPECT_NEAR( val1, m_midTOC, eps ); EXPECT_NEAR( val2, m_maxTCHP, eps ); break;
      }
   }
}

TEST_F( DoETest, BoxBehnken2Prms )
{
   casa::ScenarioAnalysis sc;
   
   ASSERT_EQ( ErrorHandler::NoError, sc.defineBaseCase( "Ottoland.project3d" ) );
   ASSERT_EQ( ErrorHandler::NoError, sc.setDoEAlgorithm( DoEGenerator::BoxBehnken ) );
   
   // extract base case parameters values
   mbapi::Model & bsCase = sc.baseCase();
   double tchp = bsCase.tableValueAsDouble( "BasementIoTbl", 0, "TopCrustHeatProd" ); // 2.5
   ASSERT_EQ( ErrorHandler::NoError, bsCase.errorCode() );

   double tocLowJur = bsCase.sourceRockManager().tocIni( 1 ); // 10%
   ASSERT_EQ( ErrorHandler::NoError, bsCase.sourceRockManager().errorCode() );

   casa::DoEGenerator & doe = sc.doeGenerator();
   casa::VarSpace     & varPrms = sc.varSpace();
   casa::RunCaseSet   & doeCaseSet = sc.doeCaseSet();

   ASSERT_EQ( ErrorHandler::NoError, varPrms.addParameter( new VarPrmSourceRockTOC( "Lower Jurassic", tocLowJur, m_minTOC, m_maxTOC, VarPrmContinuous::Block ) ) );
   ASSERT_EQ( ErrorHandler::NoError, varPrms.addParameter( new VarPrmTopCrustHeatProduction( tchp, m_minTCHP, m_maxTCHP, VarPrmContinuous::Block ) ) );

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
      case 0: EXPECT_NEAR( val1, m_midTOC, eps ); EXPECT_NEAR( val2, m_midTCHP, eps ); break;
      case 1: EXPECT_NEAR( val1, m_maxTOC, eps ); EXPECT_NEAR( val2,  m_maxTCHP, eps ); break;
      case 2: EXPECT_NEAR( val1,  m_minTOC, eps ); EXPECT_NEAR( val2,  m_maxTCHP, eps ); break;
      case 3: EXPECT_NEAR( val1, m_maxTOC, eps ); EXPECT_NEAR( val2,  m_minTCHP, eps ); break;
      case 4: EXPECT_NEAR( val1,  m_minTOC, eps ); EXPECT_NEAR( val2,  m_minTCHP, eps ); break;
      }
   }
}
      
TEST_F( DoETest, FullFactorial2Prms )
{
   casa::ScenarioAnalysis sc;

   ASSERT_EQ( ErrorHandler::NoError, sc.defineBaseCase( "Ottoland.project3d" ) );
   ASSERT_EQ( ErrorHandler::NoError, sc.setDoEAlgorithm( DoEGenerator::FullFactorial ) );

   // extract base case parameters values
   mbapi::Model & bsCase = sc.baseCase();
   double tchp = bsCase.tableValueAsDouble( "BasementIoTbl", 0, "TopCrustHeatProd" ); // m_mid_TCHP
   ASSERT_EQ( ErrorHandler::NoError, bsCase.errorCode() );

   double tocLowJur = bsCase.sourceRockManager().tocIni( 1 ); // 10%
   ASSERT_EQ( ErrorHandler::NoError, bsCase.sourceRockManager().errorCode() );

   casa::DoEGenerator & doe = sc.doeGenerator();
   casa::VarSpace     & varPrms = sc.varSpace();
   casa::RunCaseSet   & doeCaseSet = sc.doeCaseSet();

   ASSERT_EQ( ErrorHandler::NoError, varPrms.addParameter( new VarPrmSourceRockTOC( "Lower Jurassic", tocLowJur, m_minTOC, m_maxTOC, VarPrmContinuous::Block ) ) );
   ASSERT_EQ( ErrorHandler::NoError, varPrms.addParameter( new VarPrmTopCrustHeatProduction( tchp, m_minTCHP, m_maxTCHP, VarPrmContinuous::Block ) ) );

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
      case 0: EXPECT_NEAR( val1, m_midTOC, eps ); EXPECT_NEAR( val2, m_midTCHP, eps ); break;
      case 1: EXPECT_NEAR( val1,  m_minTOC, eps ); EXPECT_NEAR( val2, m_minTCHP, eps ); break;
      case 2: EXPECT_NEAR( val1, m_maxTOC, eps ); EXPECT_NEAR( val2, m_minTCHP, eps ); break;
      case 3: EXPECT_NEAR( val1,  m_minTOC, eps ); EXPECT_NEAR( val2, m_maxTCHP, eps ); break;
      case 4: EXPECT_NEAR( val1, m_maxTOC, eps ); EXPECT_NEAR( val2, m_maxTCHP, eps ); break;
      }
   }
}

TEST_F( DoETest, PlackettBurman2Prms )
{
   casa::ScenarioAnalysis sc;

   ASSERT_EQ( ErrorHandler::NoError, sc.defineBaseCase( "Ottoland.project3d" ) );
   ASSERT_EQ( ErrorHandler::NoError, sc.setDoEAlgorithm( DoEGenerator::PlackettBurman ) );

   // extract base case parameters values
   mbapi::Model & bsCase = sc.baseCase();
   double tchp = bsCase.tableValueAsDouble( "BasementIoTbl", 0, "TopCrustHeatProd" ); // 2.5
   ASSERT_EQ( ErrorHandler::NoError, bsCase.errorCode() );

   double tocLowJur = bsCase.sourceRockManager().tocIni( 1 ); // 10%
   ASSERT_EQ( ErrorHandler::NoError, bsCase.sourceRockManager().errorCode() );

   casa::DoEGenerator & doe = sc.doeGenerator();
   casa::VarSpace     & varPrms = sc.varSpace();
   casa::RunCaseSet   & doeCaseSet = sc.doeCaseSet();

   ASSERT_EQ( ErrorHandler::NoError, varPrms.addParameter( new VarPrmSourceRockTOC( "Lower Jurassic", tocLowJur, m_minTOC, m_maxTOC, VarPrmContinuous::Block ) ) );
   ASSERT_EQ( ErrorHandler::NoError, varPrms.addParameter( new VarPrmTopCrustHeatProduction( tchp, m_minTCHP, m_maxTCHP, VarPrmContinuous::Block ) ) );

   doe.generateDoE( varPrms, doeCaseSet );

   ASSERT_EQ( 4, doeCaseSet.size() );
   for ( size_t i = 0; i < doeCaseSet.size(); ++i )
   {
      ASSERT_EQ( 2, doeCaseSet[ i ]->parametersNumber() );

      const casa::PrmSourceRockTOC * prm1 = dynamic_cast<casa::PrmSourceRockTOC*>( doeCaseSet[ i ]->parameter( 0 ) );
      ASSERT_TRUE( prm1 != NULL );

      const casa::PrmTopCrustHeatProduction * prm2 = dynamic_cast<casa::PrmTopCrustHeatProduction*>( doeCaseSet[ i ]->parameter( 1 ) );
      ASSERT_TRUE( prm2 != NULL );
      double val1 = prm1->value();
      double val2 = prm2->value();

      switch ( i )
      {
      case 0: EXPECT_NEAR( val1, m_maxTOC, eps ); EXPECT_NEAR( val2, m_maxTCHP, eps ); break;
      case 1: EXPECT_NEAR( val1, m_minTOC, eps ); EXPECT_NEAR( val2, m_maxTCHP, eps ); break;
      case 2: EXPECT_NEAR( val1, m_maxTOC, eps ); EXPECT_NEAR( val2, m_minTCHP, eps ); break;
      case 3: EXPECT_NEAR( val1, m_minTOC, eps ); EXPECT_NEAR( val2, m_minTCHP, eps ); break;
      }
   }
}

TEST_F( DoETest, PlackettBurmanMirror2Prms )
{ 
   casa::ScenarioAnalysis sc;

   ASSERT_EQ( ErrorHandler::NoError, sc.defineBaseCase( "Ottoland.project3d" ) );
   ASSERT_EQ( ErrorHandler::NoError, sc.setDoEAlgorithm( DoEGenerator::PlackettBurmanMirror ) );

   // extract base case parameters values
   mbapi::Model & bsCase = sc.baseCase();
   double tchp = bsCase.tableValueAsDouble( "BasementIoTbl", 0, "TopCrustHeatProd" ); // 2.5
   ASSERT_EQ( ErrorHandler::NoError, bsCase.errorCode() );

   double tocLowJur = bsCase.sourceRockManager().tocIni( 1 ); // 10%
   ASSERT_EQ( ErrorHandler::NoError, bsCase.sourceRockManager().errorCode() );

   casa::DoEGenerator & doe = sc.doeGenerator();
   casa::VarSpace     & varPrms = sc.varSpace();
   casa::RunCaseSet   & doeCaseSet = sc.doeCaseSet();

   ASSERT_EQ( ErrorHandler::NoError, varPrms.addParameter( new VarPrmSourceRockTOC( "Lower Jurassic", tocLowJur, m_minTOC, m_maxTOC, VarPrmContinuous::Block ) ) );
   ASSERT_EQ( ErrorHandler::NoError, varPrms.addParameter( new VarPrmTopCrustHeatProduction( tchp, m_minTCHP, m_maxTCHP, VarPrmContinuous::Block ) ) );

   doe.generateDoE( varPrms, doeCaseSet );

   ASSERT_EQ( 4, doeCaseSet.size() );
   for ( size_t i = 0; i < doeCaseSet.size(); ++i )
   {
      ASSERT_EQ( 2, doeCaseSet[ i ]->parametersNumber() );

      const casa::PrmSourceRockTOC * prm1 = dynamic_cast<casa::PrmSourceRockTOC*>( doeCaseSet[ i ]->parameter( 0 ) );
      ASSERT_TRUE( prm1 != NULL );

      const casa::PrmTopCrustHeatProduction * prm2 = dynamic_cast<casa::PrmTopCrustHeatProduction*>( doeCaseSet[ i ]->parameter( 1 ) );
      ASSERT_TRUE( prm2 != NULL );
      double val1 = prm1->value();
      double val2 = prm2->value();

      switch ( i )
      {
      case 0: EXPECT_NEAR( val1, m_maxTOC, eps ); EXPECT_NEAR( val2, m_maxTCHP, eps ); break;
      case 1: EXPECT_NEAR( val1, m_minTOC, eps ); EXPECT_NEAR( val2, m_minTCHP, eps ); break;
      case 2: EXPECT_NEAR( val1, m_minTOC, eps ); EXPECT_NEAR( val2, m_maxTCHP, eps ); break;
      case 3: EXPECT_NEAR( val1, m_maxTOC, eps ); EXPECT_NEAR( val2, m_minTCHP, eps ); break;
      }
   } 
}

TEST_F( DoETest, SpaceFilling2Prms )
{
   casa::ScenarioAnalysis sc;

   ASSERT_EQ( ErrorHandler::NoError, sc.defineBaseCase( "Ottoland.project3d" ) );
   ASSERT_EQ( ErrorHandler::NoError, sc.setDoEAlgorithm( DoEGenerator::SpaceFilling ) );

   // extract base case parameters values
   mbapi::Model & bsCase = sc.baseCase();
   double tchp = bsCase.tableValueAsDouble( "BasementIoTbl", 0, "TopCrustHeatProd" ); // 2.5
   ASSERT_EQ( ErrorHandler::NoError, bsCase.errorCode() );

   double tocLowJur = bsCase.sourceRockManager().tocIni( 1 ); // 10%
   ASSERT_EQ( ErrorHandler::NoError, bsCase.sourceRockManager().errorCode() );

   casa::DoEGenerator & doe = sc.doeGenerator();
   casa::VarSpace     & varPrms = sc.varSpace();
   casa::RunCaseSet   & doeCaseSet = sc.doeCaseSet();

   ASSERT_EQ( ErrorHandler::NoError, varPrms.addParameter( new VarPrmSourceRockTOC( "Lower Jurassic", tocLowJur, m_minTOC, m_maxTOC, VarPrmContinuous::Block ) ) );
   ASSERT_EQ( ErrorHandler::NoError, varPrms.addParameter( new VarPrmTopCrustHeatProduction( tchp, m_minTCHP, m_maxTCHP, VarPrmContinuous::Block ) ) );

   doe.generateDoE( varPrms, doeCaseSet, 10 );

   ASSERT_EQ( 10, doeCaseSet.size() );
   for ( size_t i = 0; i < doeCaseSet.size(); ++i )
   {
      ASSERT_EQ( 2, doeCaseSet[ i ]->parametersNumber() );

      const casa::PrmSourceRockTOC * prm1 = dynamic_cast<casa::PrmSourceRockTOC*>( doeCaseSet[ i ]->parameter( 0 ) );
      ASSERT_TRUE( prm1 != NULL );

      const casa::PrmTopCrustHeatProduction * prm2 = dynamic_cast<casa::PrmTopCrustHeatProduction*>( doeCaseSet[ i ]->parameter( 1 ) );
      ASSERT_TRUE( prm2 != NULL );
      double val1 = prm1->value();
      double val2 = prm2->value();

      ASSERT_TRUE( val1 >= m_minTOC  && val1 <= m_maxTOC );
      ASSERT_TRUE( val2 >= m_minTCHP && val2 <= m_maxTCHP );
   }
}


TEST_F( DoETest, LatinHypercube2Prms )
{
   casa::ScenarioAnalysis sc;

   ASSERT_EQ( ErrorHandler::NoError, sc.defineBaseCase( "Ottoland.project3d" ) );
   ASSERT_EQ( ErrorHandler::NoError, sc.setDoEAlgorithm( DoEGenerator::LatinHypercube ) );

   // extract base case parameters values
   mbapi::Model & bsCase = sc.baseCase();
   double tchp = bsCase.tableValueAsDouble( "BasementIoTbl", 0, "TopCrustHeatProd" ); // 2.5
   ASSERT_EQ( ErrorHandler::NoError, bsCase.errorCode() );

   double tocLowJur = bsCase.sourceRockManager().tocIni( 1 ); // 10%
   ASSERT_EQ( ErrorHandler::NoError, bsCase.sourceRockManager().errorCode() );

   casa::DoEGenerator & doe = sc.doeGenerator();
   casa::VarSpace     & varPrms = sc.varSpace();
   casa::RunCaseSet   & doeCaseSet = sc.doeCaseSet();

   ASSERT_EQ( ErrorHandler::NoError, varPrms.addParameter( new VarPrmSourceRockTOC( "Lower Jurassic", tocLowJur, m_minTOC, m_maxTOC, VarPrmContinuous::Block ) ) );
   ASSERT_EQ( ErrorHandler::NoError, varPrms.addParameter( new VarPrmTopCrustHeatProduction( tchp, m_minTCHP, m_maxTCHP, VarPrmContinuous::Block ) ) );

   doe.generateDoE( varPrms, doeCaseSet, 10 );

   ASSERT_EQ( 10, doeCaseSet.size() );
   for ( size_t i = 0; i < doeCaseSet.size(); ++i )
   {
      ASSERT_EQ( 2, doeCaseSet[ i ]->parametersNumber() );

      const casa::PrmSourceRockTOC * prm1 = dynamic_cast<casa::PrmSourceRockTOC*>( doeCaseSet[ i ]->parameter( 0 ) );
      ASSERT_TRUE( prm1 != NULL );

      const casa::PrmTopCrustHeatProduction * prm2 = dynamic_cast<casa::PrmTopCrustHeatProduction*>( doeCaseSet[ i ]->parameter( 1 ) );
      ASSERT_TRUE( prm2 != NULL );
      double val1 = prm1->value();
      double val2 = prm2->value();

      ASSERT_TRUE( val1 >= m_minTOC  && val1 <= m_maxTOC );
      ASSERT_TRUE( val2 >= m_minTCHP && val2 <= m_maxTCHP );
   }
}

TEST_F( DoETest, OutOfBoundAlgorithm )
{
   casa::ScenarioAnalysis sc;

   ASSERT_EQ( ErrorHandler::NoError, sc.defineBaseCase( "Ottoland.project3d" ) );
   
   // set unknown algorithm
   ASSERT_NE( ErrorHandler::NoError, sc.setDoEAlgorithm( static_cast<DoEGenerator::DoEAlgorithm>( 1970 ) ) );
}

TEST_F( DoETest, SetAlgorithmAgain )
{
   casa::ScenarioAnalysis sc;

   ASSERT_EQ( ErrorHandler::NoError, sc.defineBaseCase( "Ottoland.project3d" ) );
   ASSERT_EQ( ErrorHandler::NoError, sc.setDoEAlgorithm( DoEGenerator::Tornado ) );

   // set algorithm one more time - must be and error
   ASSERT_NE( ErrorHandler::NoError, sc.setDoEAlgorithm( DoEGenerator::BoxBehnken ) );
}

TEST_F( DoETest, CheckAlgoNames )
{
   ASSERT_EQ( DoEGenerator::DoEName( DoEGenerator::BoxBehnken ),           "BoxBehnken" );
   ASSERT_EQ( DoEGenerator::DoEName( DoEGenerator::Tornado ),              "Tornado" );
   ASSERT_EQ( DoEGenerator::DoEName( DoEGenerator::PlackettBurman ),       "PlackettBurman" );
   ASSERT_EQ( DoEGenerator::DoEName( DoEGenerator::PlackettBurmanMirror ), "PlackettBurmanMirror" );
   ASSERT_EQ( DoEGenerator::DoEName( DoEGenerator::FullFactorial ),        "FullFactorial" );
   ASSERT_EQ( DoEGenerator::DoEName( DoEGenerator::LatinHypercube ),       "LatinHypercube" );
   ASSERT_EQ( DoEGenerator::DoEName( DoEGenerator::SpaceFilling ),         "SpaceFilling" );
   
   ASSERT_EQ( DoEGenerator::DoEName( static_cast<DoEGenerator::DoEAlgorithm>( 1970 ) ), "Unkown" );

}
