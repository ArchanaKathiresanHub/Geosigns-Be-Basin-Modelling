#include "../src/cmbAPI.h"
#include "../src/casaAPI.h"
#include "../src/FilePath.h"
#include "../src/FolderPath.h"
#include "../src/PrmSourceRockTOC.h"
#include "../src/PrmTopCrustHeatProduction.h"
#include "../src/RunManagerImpl.h"
#include "../src/VarPrmSourceRockTOC.h"
#include "../src/VarPrmTopCrustHeatProduction.h"

#include <memory>
#include <cstdlib>

#include <gtest/gtest.h>


using namespace casa;
using namespace casa::BusinessLogicRulesSet;

static const double eps = 1.e-5;

class RunManagerTest : public ::testing::Test
{
public:
   RunManagerTest( ) : m_minTOC( 5.0 )
             , m_maxTOC( 15.0 )
             , m_minTCHP( 0.1 )
             , m_maxTCHP( 4.9 )
             , m_layerName( "Lower Jurassic" )
             , m_projectFileName( "Ottoland.project3d" )
             , m_caseSetPath( "." )
#ifdef _WIN32
			 , m_scriptExt(".bat")
#else
			 , m_scriptExt(".sh")
#endif
   { 
      m_caseSetPath << "CaseSetRunManagerTest";
   }
   ~RunManagerTest( ) { ; }

   // set of parameters range for DoE
   const double      m_minTOC;
   const double      m_maxTOC;

   const double      m_minTCHP;
   const double      m_maxTCHP;
   
   const char      * m_layerName;

   const char      * m_projectFileName;
   ibs::FolderPath   m_caseSetPath;
   const char      * m_scriptExt;
};


// RunManager test. There is 1 DoE Tornado experiment with 2 parameters
// Test creates DoE and generates set of project files. 
// Then it spawn 1 job to the cluster and check the execution results
TEST_F( RunManagerTest, Tornado2PrmsMutations )
{
   // create new scenario analysis
   casa::ScenarioAnalysis sc;

   ASSERT_EQ( ErrorHandler::NoError, sc.defineBaseCase( m_projectFileName ) );
   
   // vary 2 parameters
   std::vector<double> dblRng( 2, m_minTOC );
   dblRng[1] = m_maxTOC;
   ASSERT_EQ( ErrorHandler::NoError, VarySourceRockTOC( sc, 0, m_layerName, 1, 0, dblRng, vector<string>(),  VarPrmContinuous::Block ) );

   dblRng[0] = m_minTCHP;
   dblRng[1] = m_maxTCHP;
   ASSERT_EQ( ErrorHandler::NoError, VaryTopCrustHeatProduction( sc, 0, dblRng, vector<string>(), VarPrmContinuous::Block ) );

   // set up and generate DoE
   ASSERT_EQ( ErrorHandler::NoError, sc.setDoEAlgorithm( DoEGenerator::Tornado ) );
   casa::DoEGenerator & doe = sc.doeGenerator( );

   doe.generateDoE( sc.varSpace(), sc.doeCaseSet() );
   
   ASSERT_EQ( 5U, sc.doeCaseSet().size() );
   
   ASSERT_EQ( ErrorHandler::NoError, sc.setScenarioLocation( m_caseSetPath.cpath() ) );

   ASSERT_EQ( ErrorHandler::NoError, sc.applyMutations( sc.doeCaseSet() ) );

   RunManagerImpl & rm = dynamic_cast< RunManagerImpl &> ( sc.runManager() );
 
   // set up simulation pipeline, the first is fastcauldron
   CauldronApp * app = RunManager::createApplication( RunManager::fastcauldron );
   app->addOption( "-itcoupled" );
   rm.addApplication( app );

   // then set up fastgenex6
   app = RunManager::createApplication( RunManager::fastgenex6 );
   rm.addApplication( app );

   // and at the end set up track1d
   app = RunManager::createApplication( RunManager::track1d );
   app->addOption( "-coordinates 460001,6750001" );
   app->addOption( "-age 0.0" );
   app->addOption( "-properties Temperature,Vr,OilExpelledCumulative,HcGasExpelledCumulative,TOC" );
   rm.addApplication( app );

   for ( size_t i = 0; i < sc.doeCaseSet().size(); ++i )
   {
      rm.scheduleCase( *(sc.doeCaseSet()[i]), sc.scenarioID() );
   }

   // check generated scripts
   ibs::FolderPath pathToCaseSet = m_caseSetPath;

   pathToCaseSet << "Iteration_1";

   for ( size_t i = 0; i < sc.doeCaseSet().size(); ++i )
   {
      ibs::FilePath casePath( pathToCaseSet.path() );
      casePath << (std::string( "Case_" ) + std::to_string( i + 1 ) );

      // check that all files were generated correctly
      for ( size_t j = 0; j < 3; ++j )
      {
		  ASSERT_TRUE( ( ibs::FilePath(casePath.path() ) << std::string( "Stage_" ) + std::to_string( j ) + m_scriptExt ).exists() );
      }
   }
 
   // cleaning files/folders
   pathToCaseSet.clean();  // clean folder ./CaseSet/Iteration_1
   pathToCaseSet.cutLast();       
   pathToCaseSet.remove(); // delete folder ./CaseSet
   
   ASSERT_FALSE( pathToCaseSet.exists() );
}
