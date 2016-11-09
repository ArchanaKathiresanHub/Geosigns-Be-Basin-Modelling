#include "../src/cmbAPI.h"
#include "../src/casaAPI.h"
#include "../src/PrmSourceRockTOC.h"
#include "../src/PrmTopCrustHeatProduction.h"
#include "../src/PrmOneCrustThinningEvent.h"
#include "../src/RunCase.h"
#include "../src/VarPrmSourceRockTOC.h"
#include "../src/VarPrmTopCrustHeatProduction.h"
#include "../src/VarPrmOneCrustThinningEvent.h"

#include <memory>
//#include <cmath>

#include <gtest/gtest.h>

using namespace casa;
using namespace std;

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

             , m_minInitialCrustThickness( 15000.0 )
             , m_midInitialCrustThickness( 27500.0 )
             , m_maxInitialCrustThickness( 40000.0 )

             , m_minT0( 120.0 )
             , m_midT0( 175.0 )
             , m_maxT0( 230.0 )

             , m_mindT( 30.0 )
             , m_middT( 37.5 )
             , m_maxdT( 45.0 ) 
             
             , m_minThickCoeff( 0.5 )
             , m_midThickCoeff( 0.65 )
             , m_maxThickCoeff( 0.8 )
   { ; }
   ~DoETest() {;}

   // Source rock TOC %
   const double m_minTOC;
   const double m_midTOC;
   const double m_maxTOC;

   // Top crust heat production rate
   const double m_minTCHP;
   const double m_midTCHP;
   const double m_maxTCHP;

   // Single crust thinning event
   // Initial crust thickness
   const double m_minInitialCrustThickness;
   const double m_midInitialCrustThickness;
   const double m_maxInitialCrustThickness;
   // T0
   const double m_minT0;
   const double m_midT0;
   const double m_maxT0;
   // dT
   const double m_mindT;
   const double m_middT;
   const double m_maxdT;
   
   // ThickCoeff   
   const double m_minThickCoeff;
   const double m_midThickCoeff;
   const double m_maxThickCoeff;
};

TEST_F( DoETest, Tornado2Prms )
{
   std::unique_ptr<casa::ScenarioAnalysis> sc;
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

   std::vector<double> dblRng( 3, m_minTOC );
   dblRng[1] = m_maxTOC;
   dblRng[2] = tocLowJur;

   ASSERT_EQ( ErrorHandler::NoError, varPrms.addParameter( new VarPrmSourceRockTOC( "Lower Jurassic", dblRng, vector<string>() ) ) );

   dblRng[0] = m_minTCHP;
   dblRng[1] = m_maxTCHP;
   dblRng[2] = tchp;

   ASSERT_EQ( ErrorHandler::NoError, varPrms.addParameter( new VarPrmTopCrustHeatProduction( dblRng, vector<string>(), VarPrmContinuous::Block ) ) );

   doe.generateDoE( varPrms, doeCaseSet );

   ASSERT_EQ( 5U, doeCaseSet.size() );
   for ( size_t i = 0; i < doeCaseSet.size(); ++i )
   {
      ASSERT_EQ( 2U, doeCaseSet[i]->parametersNumber() );

      const casa::PrmSourceRockTOC * prm1 = dynamic_cast<casa::PrmSourceRockTOC*>( doeCaseSet[ i ]->parameter( 0 ).get() );
      ASSERT_TRUE( prm1 != NULL );

      const casa::PrmTopCrustHeatProduction * prm2 = dynamic_cast<casa::PrmTopCrustHeatProduction*>( doeCaseSet[ i ]->parameter( 1 ).get() );
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

   std::vector<double> dblRng( 3, m_minTOC );
   dblRng[1] = m_maxTOC;
   dblRng[2] = tocLowJur;

   ASSERT_EQ( ErrorHandler::NoError, varPrms.addParameter( new VarPrmSourceRockTOC( "Lower Jurassic", dblRng, vector<string>() ) ) );

   dblRng[0] = m_minTCHP;
   dblRng[1] = m_maxTCHP;
   dblRng[2] = tchp;

   ASSERT_EQ( ErrorHandler::NoError, varPrms.addParameter( new VarPrmTopCrustHeatProduction( dblRng, vector<string>(), VarPrmContinuous::Block ) ) );

   doe.generateDoE( varPrms, doeCaseSet );
   
   ASSERT_EQ( 5U, doeCaseSet.size( ) );
   for ( size_t i = 0; i < doeCaseSet.size( ); ++i )
   {
      ASSERT_EQ( 2U, doeCaseSet[ i ]->parametersNumber() );
   
      const casa::PrmSourceRockTOC * prm1 = dynamic_cast<casa::PrmSourceRockTOC*>( doeCaseSet[ i ]->parameter( 0 ).get() );
      ASSERT_TRUE( prm1 != NULL );
   
      const casa::PrmTopCrustHeatProduction * prm2 = dynamic_cast<casa::PrmTopCrustHeatProduction*>( doeCaseSet[ i ]->parameter( 1 ).get() );
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

   std::vector<double> dblRng( 3, m_minTOC );
   dblRng[1] = m_maxTOC;
   dblRng[2] = tocLowJur;

   ASSERT_EQ( ErrorHandler::NoError, varPrms.addParameter( new VarPrmSourceRockTOC( "Lower Jurassic", dblRng, vector<string>() ) ) );

   dblRng[0] = m_minTCHP;
   dblRng[1] = m_maxTCHP;
   dblRng[2] = tchp;

   ASSERT_EQ( ErrorHandler::NoError, varPrms.addParameter( new VarPrmTopCrustHeatProduction( dblRng, vector<string>(), VarPrmContinuous::Block ) ) );

   doe.generateDoE( varPrms, doeCaseSet );

   ASSERT_EQ( 5U, doeCaseSet.size( ) );
   for ( size_t i = 0; i < doeCaseSet.size( ); ++i )
   {
      ASSERT_EQ( 2U, doeCaseSet[ i ]->parametersNumber() );

      const casa::PrmSourceRockTOC * prm1 = dynamic_cast<casa::PrmSourceRockTOC*>( doeCaseSet[ i ]->parameter( 0 ).get() );
      ASSERT_TRUE( prm1 != NULL );

      const casa::PrmTopCrustHeatProduction * prm2 = dynamic_cast<casa::PrmTopCrustHeatProduction*>( doeCaseSet[ i ]->parameter( 1 ).get() );
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

   std::vector<double> dblRng( 3, m_minTOC );
   dblRng[1] = m_maxTOC;
   dblRng[2] = tocLowJur;

   ASSERT_EQ( ErrorHandler::NoError, varPrms.addParameter( new VarPrmSourceRockTOC( "Lower Jurassic", dblRng, vector<string>() ) ) );

   dblRng[0] = m_minTCHP;
   dblRng[1] = m_maxTCHP;
   dblRng[2] = tchp;

   ASSERT_EQ( ErrorHandler::NoError, varPrms.addParameter( new VarPrmTopCrustHeatProduction( dblRng, vector<string>(), VarPrmContinuous::Block ) ) );

   doe.generateDoE( varPrms, doeCaseSet );

   ASSERT_EQ( 4U, doeCaseSet.size() );
   for ( size_t i = 0; i < doeCaseSet.size(); ++i )
   {
      ASSERT_EQ( 2U, doeCaseSet[ i ]->parametersNumber() );

      const casa::PrmSourceRockTOC * prm1 = dynamic_cast<casa::PrmSourceRockTOC*>( doeCaseSet[ i ]->parameter( 0 ).get() );
      ASSERT_TRUE( prm1 != NULL );

      const casa::PrmTopCrustHeatProduction * prm2 = dynamic_cast<casa::PrmTopCrustHeatProduction*>( doeCaseSet[ i ]->parameter( 1 ).get() );
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

   std::vector<double> dblRng( 3, m_minTOC );
   dblRng[1] = m_maxTOC;
   dblRng[2] = tocLowJur;

   ASSERT_EQ( ErrorHandler::NoError, varPrms.addParameter( new VarPrmSourceRockTOC( "Lower Jurassic", dblRng, vector<string>() ) ) );

   dblRng[0] = m_minTCHP;
   dblRng[1] = m_maxTCHP;
   dblRng[2] = tchp;

   ASSERT_EQ( ErrorHandler::NoError, varPrms.addParameter( new VarPrmTopCrustHeatProduction( dblRng, vector<string>(), VarPrmContinuous::Block ) ) );

   doe.generateDoE( varPrms, doeCaseSet );

   ASSERT_EQ( 4U, doeCaseSet.size() );
   for ( size_t i = 0; i < doeCaseSet.size(); ++i )
   {
      ASSERT_EQ( 2U, doeCaseSet[ i ]->parametersNumber() );

      const casa::PrmSourceRockTOC * prm1 = dynamic_cast<casa::PrmSourceRockTOC*>( doeCaseSet[ i ]->parameter( 0 ).get() );
      ASSERT_TRUE( prm1 != NULL );

      const casa::PrmTopCrustHeatProduction * prm2 = dynamic_cast<casa::PrmTopCrustHeatProduction*>( doeCaseSet[ i ]->parameter( 1 ).get() );
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

   std::vector<double> dblRng( 3, m_minTOC );
   dblRng[1] = m_maxTOC;
   dblRng[2] = tocLowJur;

   ASSERT_EQ( ErrorHandler::NoError, varPrms.addParameter( new VarPrmSourceRockTOC( "Lower Jurassic", dblRng, vector<string>() ) ) );

   dblRng[0] = m_minTCHP;
   dblRng[1] = m_maxTCHP;
   dblRng[2] = tchp;

   ASSERT_EQ( ErrorHandler::NoError, varPrms.addParameter( new VarPrmTopCrustHeatProduction( dblRng, vector<string>(), VarPrmContinuous::Block ) ) );

   doe.generateDoE( varPrms, doeCaseSet, 10 );

   ASSERT_EQ( 10U, doeCaseSet.size() );
   for ( size_t i = 0; i < doeCaseSet.size(); ++i )
   {
      ASSERT_EQ( 2U, doeCaseSet[ i ]->parametersNumber() );

      const casa::PrmSourceRockTOC * prm1 = dynamic_cast<casa::PrmSourceRockTOC*>( doeCaseSet[ i ]->parameter( 0 ).get() );
      ASSERT_TRUE( prm1 != NULL );

      const casa::PrmTopCrustHeatProduction * prm2 = dynamic_cast<casa::PrmTopCrustHeatProduction*>( doeCaseSet[ i ]->parameter( 1 ).get() );
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

   std::vector<double> dblRng( 3, m_minTOC );
   dblRng[1] = m_maxTOC;
   dblRng[2] = tocLowJur;

   ASSERT_EQ( ErrorHandler::NoError, varPrms.addParameter( new VarPrmSourceRockTOC( "Lower Jurassic", dblRng, vector<string>() ) ) );

   dblRng[0] = m_minTCHP;
   dblRng[1] = m_maxTCHP;
   dblRng[2] = tchp;

   ASSERT_EQ( ErrorHandler::NoError, varPrms.addParameter( new VarPrmTopCrustHeatProduction( dblRng, vector<string>(), VarPrmContinuous::Block ) ) );

   doe.generateDoE( varPrms, doeCaseSet, 10 );

   ASSERT_EQ( 10U, doeCaseSet.size() );
   for ( size_t i = 0; i < doeCaseSet.size(); ++i )
   {
      ASSERT_EQ( 2U, doeCaseSet[ i ]->parametersNumber() );

      const casa::PrmSourceRockTOC * prm1 = dynamic_cast<casa::PrmSourceRockTOC*>( doeCaseSet[ i ]->parameter( 0 ).get() );
      ASSERT_TRUE( prm1 != NULL );

      const casa::PrmTopCrustHeatProduction * prm2 = dynamic_cast<casa::PrmTopCrustHeatProduction*>( doeCaseSet[ i ]->parameter( 1 ).get() );
      ASSERT_TRUE( prm2 != NULL );
      double val1 = prm1->value();
      double val2 = prm2->value();

      ASSERT_TRUE( val1 >= m_minTOC  && val1 <= m_maxTOC );
      ASSERT_TRUE( val2 >= m_minTCHP && val2 <= m_maxTCHP );
   }
}

TEST_F( DoETest, AllVarPrmsTornadoTest )
{
   casa::ScenarioAnalysis sc;

   ASSERT_EQ( ErrorHandler::NoError, sc.defineBaseCase( "Ottoland.project3d" ) );
   ASSERT_EQ( ErrorHandler::NoError, sc.setDoEAlgorithm( DoEGenerator::Tornado ) );

   casa::DoEGenerator & doe = sc.doeGenerator();
   casa::VarSpace     & varPrms = sc.varSpace();
   casa::RunCaseSet   & doeCaseSet = sc.doeCaseSet();

   // add 3 parameters
   std::vector<double> dblRng( 3, m_minTOC );
   dblRng[1] = m_maxTOC;
   dblRng[2] = m_midTOC;

   ASSERT_EQ( ErrorHandler::NoError, varPrms.addParameter( new VarPrmSourceRockTOC( "Lower Jurassic", dblRng, vector<string>() ) ) );

   dblRng[0] = m_minTCHP;
   dblRng[1] = m_maxTCHP;
   dblRng[2] = m_midTCHP;

   ASSERT_EQ( ErrorHandler::NoError, varPrms.addParameter( new VarPrmTopCrustHeatProduction( dblRng, vector<string>(), VarPrmContinuous::Block ) ) );

   ASSERT_EQ( ErrorHandler::NoError, varPrms.addParameter( new VarPrmOneCrustThinningEvent( m_midInitialCrustThickness, m_minInitialCrustThickness, m_maxInitialCrustThickness,
                                                                                            m_midT0,                    m_minT0,                    m_maxT0,
                                                                                            m_middT,                    m_mindT,                    m_maxdT,
                                                                                            m_midThickCoeff,            m_minThickCoeff,            m_maxThickCoeff,
                                                                                            VarPrmContinuous::Block ) ) );
   // must be 13 cases 1 + 2*(2+4)
   doe.generateDoE( varPrms, doeCaseSet );

   ASSERT_EQ( 13U, doeCaseSet.size( ) );
   
   for ( size_t i = 0; i < doeCaseSet.size( ); ++i )
   {
      // 3 parameter per 1 case, 3d parameter - multi-dim (4 doubles for 1 parameter)
      ASSERT_EQ( 3U, doeCaseSet[ i ]->parametersNumber() );

      const casa::PrmSourceRockTOC * prm1 = dynamic_cast<casa::PrmSourceRockTOC*>( doeCaseSet[ i ]->parameter( 0 ).get() );
      ASSERT_TRUE( prm1 != NULL );

      const casa::PrmTopCrustHeatProduction * prm2 = dynamic_cast<casa::PrmTopCrustHeatProduction*>( doeCaseSet[ i ]->parameter( 1 ).get() );
      ASSERT_TRUE( prm2 != NULL );

      const casa::PrmOneCrustThinningEvent * prm3 = dynamic_cast<casa::PrmOneCrustThinningEvent*>( doeCaseSet[ i ]->parameter( 2 ).get() );
      double val1 = prm1->value();
      double val2 = prm2->value();
      const std::vector<double> & val3 = prm3->asDoubleArray();
      
      switch ( i )
      {
      case 0: 
         EXPECT_NEAR( val1,    m_midTOC, eps ); 
         EXPECT_NEAR( val2,    m_midTCHP, eps ); 
         EXPECT_NEAR( val3[0], m_midInitialCrustThickness, eps ); 
         EXPECT_NEAR( val3[1], m_midT0, eps ); 
         EXPECT_NEAR( val3[2], m_middT, eps ); 
         EXPECT_NEAR( val3[3], m_midThickCoeff, eps ); 
         break;
      
      case 1: 
         EXPECT_NEAR( val1,    m_minTOC, eps ); 
         EXPECT_NEAR( val2,    m_midTCHP, eps ); 
         EXPECT_NEAR( val3[0], m_midInitialCrustThickness, eps ); 
         EXPECT_NEAR( val3[1], m_midT0, eps ); 
         EXPECT_NEAR( val3[2], m_middT, eps ); 
         EXPECT_NEAR( val3[3], m_midThickCoeff, eps ); 
         break;

      case 2: 
         EXPECT_NEAR( val1,    m_maxTOC, eps ); 
         EXPECT_NEAR( val2,    m_midTCHP, eps ); 
         EXPECT_NEAR( val3[0], m_midInitialCrustThickness, eps ); 
         EXPECT_NEAR( val3[1], m_midT0, eps ); 
         EXPECT_NEAR( val3[2], m_middT, eps ); 
         EXPECT_NEAR( val3[3], m_midThickCoeff, eps ); 
         break;

      case 3: 
         EXPECT_NEAR( val1,    m_midTOC, eps ); 
         EXPECT_NEAR( val2,    m_minTCHP, eps ); 
         EXPECT_NEAR( val3[0], m_midInitialCrustThickness, eps ); 
         EXPECT_NEAR( val3[1], m_midT0, eps ); 
         EXPECT_NEAR( val3[2], m_middT, eps ); 
         EXPECT_NEAR( val3[3], m_midThickCoeff, eps ); 
         break;

      case 4: 
         EXPECT_NEAR( val1,    m_midTOC, eps ); 
         EXPECT_NEAR( val2,    m_maxTCHP, eps ); 
         EXPECT_NEAR( val3[0], m_midInitialCrustThickness, eps ); 
         EXPECT_NEAR( val3[1], m_midT0, eps ); 
         EXPECT_NEAR( val3[2], m_middT, eps ); 
         EXPECT_NEAR( val3[3], m_midThickCoeff, eps ); 
         break;

      case 5: 
         EXPECT_NEAR( val1,    m_midTOC, eps ); 
         EXPECT_NEAR( val2,    m_midTCHP, eps ); 
         EXPECT_NEAR( val3[0], m_minInitialCrustThickness, eps ); 
         EXPECT_NEAR( val3[1], m_midT0, eps ); 
         EXPECT_NEAR( val3[2], m_middT, eps ); 
         EXPECT_NEAR( val3[3], m_midThickCoeff, eps ); 
         break;

      case 6: 
         EXPECT_NEAR( val1,    m_midTOC, eps ); 
         EXPECT_NEAR( val2,    m_midTCHP, eps ); 
         EXPECT_NEAR( val3[0], m_maxInitialCrustThickness, eps ); 
         EXPECT_NEAR( val3[1], m_midT0, eps ); 
         EXPECT_NEAR( val3[2], m_middT, eps ); 
         EXPECT_NEAR( val3[3], m_midThickCoeff, eps ); 
         break;

      case 7: 
         EXPECT_NEAR( val1,    m_midTOC, eps ); 
         EXPECT_NEAR( val2,    m_midTCHP, eps ); 
         EXPECT_NEAR( val3[0], m_midInitialCrustThickness, eps ); 
         EXPECT_NEAR( val3[1], m_minT0, eps ); 
         EXPECT_NEAR( val3[2], m_middT, eps ); 
         EXPECT_NEAR( val3[3], m_midThickCoeff, eps ); 
         break;

      case 8:
         EXPECT_NEAR( val1,    m_midTOC, eps ); 
         EXPECT_NEAR( val2,    m_midTCHP, eps ); 
         EXPECT_NEAR( val3[0], m_midInitialCrustThickness, eps ); 
         EXPECT_NEAR( val3[1], m_maxT0, eps ); 
         EXPECT_NEAR( val3[2], m_middT, eps ); 
         EXPECT_NEAR( val3[3], m_midThickCoeff, eps ); 
         break;
 
      case 9: 
         EXPECT_NEAR( val1,    m_midTOC, eps ); 
         EXPECT_NEAR( val2,    m_midTCHP, eps ); 
         EXPECT_NEAR( val3[0], m_midInitialCrustThickness, eps ); 
         EXPECT_NEAR( val3[1], m_midT0, eps ); 
         EXPECT_NEAR( val3[2], m_mindT, eps ); 
         EXPECT_NEAR( val3[3], m_midThickCoeff, eps ); 
         break;

       case 10: 
          EXPECT_NEAR( val1,    m_midTOC, eps ); 
          EXPECT_NEAR( val2,    m_midTCHP, eps ); 
          EXPECT_NEAR( val3[0], m_midInitialCrustThickness, eps ); 
          EXPECT_NEAR( val3[1], m_midT0, eps ); 
          EXPECT_NEAR( val3[2], m_maxdT, eps ); 
          EXPECT_NEAR( val3[3], m_midThickCoeff, eps ); 
          break;

      case 11: 
         EXPECT_NEAR( val1,    m_midTOC, eps ); 
         EXPECT_NEAR( val2,    m_midTCHP, eps ); 
         EXPECT_NEAR( val3[0], m_midInitialCrustThickness, eps ); 
         EXPECT_NEAR( val3[1], m_midT0, eps ); 
         EXPECT_NEAR( val3[2], m_middT, eps ); 
         EXPECT_NEAR( val3[3], m_minThickCoeff, eps ); 
         break;

      case 12: 
         EXPECT_NEAR( val1,    m_midTOC, eps ); 
         EXPECT_NEAR( val2,    m_midTCHP, eps ); 
         EXPECT_NEAR( val3[0], m_midInitialCrustThickness, eps ); 
         EXPECT_NEAR( val3[1], m_midT0, eps ); 
         EXPECT_NEAR( val3[2], m_middT, eps ); 
         EXPECT_NEAR( val3[3], m_maxThickCoeff, eps ); 
         break;
      }
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

   // set algorithm one more time - should not be any error, RunCaseSet now can keep
   // several set of experiments
   ASSERT_EQ( ErrorHandler::NoError, sc.setDoEAlgorithm( DoEGenerator::BoxBehnken ) );
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
