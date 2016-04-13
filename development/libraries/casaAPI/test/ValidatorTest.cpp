#include "../src/cmbAPI.h"
#include "../src/casaAPI.h"
#include "../src/FilePath.h"
#include "../src/FolderPath.h"
#include "../src/PrmSourceRockTOC.h"
#include "../src/PrmTopCrustHeatProduction.h"
#include "../src/VarPrmSourceRockTOC.h"
#include "../src/VarPrmTopCrustHeatProduction.h"

#include <memory>
//#include <cmath>

#include <gtest/gtest.h>

using namespace casa;
using namespace casa::BusinessLogicRulesSet;
using namespace std;

static const double eps = 1.e-5;

class ValidatorTest : public ::testing::Test
{
public:
   ValidatorTest( ) : m_minTOC( 5.0 )
             , m_maxTOC( 15.0 )
             , m_minTCHP( 0.1 )
             , m_maxTCHP( 4.9 )
             , m_layerName( "Lower Jurassic" )
             , m_projectFileName( "Ottoland.project3d" )
             , m_caseSetPath( "." )
   { 
      m_caseSetPath << "CaseSetValidatorTest";
   }
   ~ValidatorTest( ) { ; }

   // set of parameters range for DoE
   const double m_minTOC;
   const double m_maxTOC;

   const double m_minTCHP;
   const double m_maxTCHP;
   
   const char * m_layerName;

   const char * m_projectFileName;

   ibs::FolderPath m_caseSetPath;
};
  
// Mutator test. There is 1 DoE Tornado experiment with 2 parameters
// Test creates DoE and generates set of project files. Then do the validation of
// parameters in generated cases
TEST_F( ValidatorTest, TwoPrmsValidationTornadoDoE )
{
   // create new scenario analysis
   casa::ScenarioAnalysis sc;

   ASSERT_EQ( ErrorHandler::NoError, sc.defineBaseCase( m_projectFileName ) );
   
   // vary 2 parameters
   ASSERT_EQ( ErrorHandler::NoError, VarySourceRockTOC(          sc, 0, m_layerName, 1, 0, m_minTOC,  m_maxTOC,  VarPrmContinuous::Block ) );

   vector<double> dblRng( 1, m_minTCHP );
   dblRng.push_back( m_maxTCHP );
   ASSERT_EQ( ErrorHandler::NoError, VaryTopCrustHeatProduction( sc, 0, dblRng, vector<string>(), VarPrmContinuous::Block ) );

   // set up and generate DoE
   ASSERT_EQ( ErrorHandler::NoError, sc.setDoEAlgorithm( DoEGenerator::Tornado ) );
   casa::DoEGenerator & doe = sc.doeGenerator( );

   doe.generateDoE( sc.varSpace(), sc.doeCaseSet() );
   
   ASSERT_EQ( 5U, sc.doeCaseSet().size() );

   ibs::FolderPath pathToCaseSet = m_caseSetPath;

   ASSERT_EQ( ErrorHandler::NoError, sc.setScenarioLocation( pathToCaseSet.cpath() ) );
   ASSERT_EQ( ErrorHandler::NoError, sc.applyMutations( sc.doeCaseSet() ) );

   ASSERT_EQ( ErrorHandler::NoError, sc.validateCaseSet( sc.doeCaseSet( ) ) );
  
   // cleaning files/folders
   pathToCaseSet.remove();  // clean folder
   
   ASSERT_FALSE( pathToCaseSet.exists() );
}

TEST_F( ValidatorTest, TOCPrmValidation )
{
   // create new scenario analysis
   casa::ScenarioAnalysis sc;

   ASSERT_EQ( ErrorHandler::NoError, sc.defineBaseCase( m_projectFileName ) );

   PrmSourceRockTOC prm( NULL, 0.5 * ( m_minTOC + m_maxTOC ), m_layerName );
     
   // check if all comes OK
   const std::string & validationMessage = prm.validate( sc.baseCase() );
   ASSERT_TRUE( validationMessage.empty() );

   // check for error for negative TOC
   PrmSourceRockTOC prm1( NULL, -1, m_layerName );
   const std::string & validationMessage1 = prm1.validate( sc.baseCase() );
   ASSERT_FALSE( validationMessage1.empty( ) ); // negative value message

   // check for error for more then 100% TOC
   PrmSourceRockTOC prm2( NULL, 101, m_layerName );
   const std::string & validationMessage2 = prm2.validate( sc.baseCase() );
   ASSERT_FALSE( validationMessage2.empty() ); // out of range message

   // check for error if wrong layer name is given
   PrmSourceRockTOC prm3( NULL, 0.5 * ( m_minTOC + m_maxTOC ), "Some wrong layer name" );
   const std::string & validationMessage3 = prm3.validate( sc.baseCase() );
   ASSERT_FALSE( validationMessage3.empty() ); // wrong layer message

   PrmSourceRockTOC prm4( NULL, 77, m_layerName );
   const std::string & validationMessage4 = prm4.validate( sc.baseCase() );
   ASSERT_FALSE( validationMessage4.empty() ); // value in the model is differ than given
}

TEST_F( ValidatorTest, TCHPPrmValidation )
{
   // create new scenario analysis
   casa::ScenarioAnalysis sc;

   ASSERT_EQ( ErrorHandler::NoError, sc.defineBaseCase( m_projectFileName ) );

   PrmTopCrustHeatProduction prm( NULL, 0.5 * ( m_minTCHP + m_maxTCHP ) );

   // check if all comes OK
   const std::string & validationMessage = prm.validate( sc.baseCase() );
   ASSERT_TRUE( validationMessage.empty() );

   // check for error for negative TCHP rate
   PrmTopCrustHeatProduction prm1( NULL, -1 );
   const std::string & validationMessage1 = prm1.validate( sc.baseCase() );
   ASSERT_FALSE( validationMessage1.empty() ); // negative value message

   PrmTopCrustHeatProduction prm2( NULL, 77 );
   const std::string & validationMessage2 = prm2.validate( sc.baseCase() );
   ASSERT_FALSE( validationMessage2.empty() ); // value in the model is differ than given
}

