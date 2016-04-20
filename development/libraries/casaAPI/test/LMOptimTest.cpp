#include "../src/cmbAPI.h"
#include "../src/casaAPI.h"
#include "../src/VarSpaceImpl.h"
#include "../src/LMOptAlgorithm.h"
#include "../src/ObsGridPropertyWell.h"
#include "../src/ObsGridPropertyXYZ.h"
#include "../src/ObsValueDoubleArray.h"
#include "../src/ObsValueDoubleScalar.h"
#include "../src/RunCaseImpl.h"
#include "../src/RunCaseSetImpl.h"
#include "../src/SUMlibUtils.h"

#include "FilePath.h"
#include "FolderPath.h"

#include <Case.h>

#include <memory>
#include <cstdlib>

#include <gtest/gtest.h>

#include <unsupported/Eigen/LevenbergMarquardt>
#include <unsupported/Eigen/NumericalDiff>


using namespace casa;
using namespace casa::BusinessLogicRulesSet;

static const double eps = 1.e-5;

class LMClassTest : public LMOptAlgorithm
{
public:
   LMClassTest( ScenarioAnalysis & sa ) : LMOptAlgorithm( sa.baseCaseProjectFileName() )
   {
      m_sa = &sa;
      m_eps = 1.e-6;
      m_casesSet.push_back( sa.doeCaseSet().runCase( 0 ) );
      m_xi.push_back( 47.5 );
      m_xi.push_back( 30.0 );
   }

   void test_prepareParameters()
   {
      std::vector<double> initGuess;
      size_t prmDim = prepareParameters( initGuess );

      // check base case values
      ASSERT_EQ( 2U, initGuess.size() );
      ASSERT_EQ( 2U, prmDim );

      ASSERT_NEAR( initGuess[0], 47.5, m_eps );  // Surface porosity for Spekk
      ASSERT_NEAR( initGuess[1], 30.0, m_eps ); // Litho fraction of SM. Sandstone for Lark
      
      ASSERT_EQ( 2U, m_optimPrms.size() );
      ASSERT_EQ( std::string( "SpekkSP"    ), m_optimPrms[0].first->name()[0] );
      ASSERT_EQ( std::string( "LarkLF"     ), m_optimPrms[1].first->name()[0] );
   
      ASSERT_EQ( 2U, m_permPrms.size() );
      ASSERT_EQ( m_permPrms[0], 0U );
      ASSERT_EQ( m_permPrms[1], 1U );
   }

   void test_prepareObservables()
   {
      size_t obsDim = prepareObservables();
      ASSERT_EQ( 29U, obsDim );

      ASSERT_EQ( 3U,  m_optimObs[0]->dimension() );
      ASSERT_EQ( 9U,  m_optimObs[1]->dimension() );
      ASSERT_EQ( 17U, m_optimObs[2]->dimension() );

      ASSERT_EQ( std::string( "Well_030_07_06_P_1"  ), m_optimObs[0]->name()[0] );
      ASSERT_EQ( std::string( "Well_030_07_06_T_1"  ), m_optimObs[1]->name()[0] );
      ASSERT_EQ( std::string( "Well_030_07_06_Vr_1" ), m_optimObs[2]->name()[0] );

      ASSERT_EQ( 1U, m_permObs[0] );
      ASSERT_EQ( 4U, m_permObs[1] );
      ASSERT_EQ( 5U, m_permObs[2] );
  }

   void test_calculateFunctionValue()
   {
      Eigen::VectorXd func( 2 );
      
      calculateFunctionValue( func );

      ASSERT_NEAR( func[0], 1.23919, m_eps * 100.0 );
      ASSERT_NEAR( func[1], 1.0e-10, m_eps * 100.0 );
   }

   ~LMClassTest() {;}
   
   double m_eps;
};

static double Well_030_07_06Pres[3][4] = { { 448197, 6706691, 3849.2, 73.2655 }
                                         , { 448197, 6706691, 3862.4, 73.05   }
                                         , { 448197, 6706691, 3862.5, 73.43   }
                                         };

static double Well_030_07_06Temp[9][4] = { { 448197, 6706691, 2459,   62.8517 }
                                         , { 448197, 6706691, 2551,   53.33   }
                                         , { 448197, 6706691, 3254,   89.7324 }
                                         , { 448197, 6706691, 3498,   112.143 }
                                         , { 448197, 6706691, 3612,   80      }
                                         , { 448197, 6706691, 3665,   105.56  }
                                         , { 448197, 6706691, 3689,   117.232 }
                                         , { 448197, 6706691, 3710,   86.7085 }
                                         , { 448197, 6706691, 4112,   125.016 }
                                         };

static double Well_030_07_06VR[17][4] = { { 448197,  6706691,  1800,  0.31 }
                                        , { 448197,  6706691,  2000,  0.33 }
                                        , { 448197,  6706691,  2200,  0.35 }
                                        , { 448197,  6706691,  2400,  0.47 }
                                        , { 448197,  6706691,  2500,  0.49 }
                                        , { 448197,  6706691,  2600,  0.45 }
                                        , { 448197,  6706691,  3000,  0.51 }
                                        , { 448197,  6706691,  3400,  0.71 }
                                        , { 448197,  6706691,  3500,  0.65 }
                                        , { 448197,  6706691,  3575,  0.77 }
                                        , { 448197,  6706691,  3650,  0.75 }
                                        , { 448197,  6706691,  3724,  0.85 }
                                        , { 448197,  6706691,  3775,  0.78 }
                                        , { 448197,  6706691,  3850,  1    }
                                        , { 448197,  6706691,  3900,  0.99 }
                                        , { 448197,  6706691,  4000,  1.03 }
                                        , { 448197,  6706691,  4075,  1.19 }
                                        };

class LMOptimTest : public ::testing::Test 
{
public:
   LMOptimTest( ) : m_projectFileName( "NVG_Project.project3d" )
#ifdef _WIN32
			 , m_scriptExt(".bat")
#else
			 , m_scriptExt(".sh")
#endif
          , m_scLoc( "sometempvalue" )
   { ; }
   
   ~LMOptimTest( ) {
   
      if ( m_scLoc.exists() )
      {
         // cleaning files/folders
         m_scLoc.clean();  // clean folder ./CaseSet/Iteration_1
         m_scLoc.remove(); // delete folder ./CaseSet
      }
   }

   // set of parameters range
   const char      * m_layerName;
   const char      * m_projectFileName;
   const char      * m_scriptExt;
   ibs::FolderPath   m_scLoc;

   void prepareScenario( ScenarioAnalysis & sa )
   {
      // define the base case
      sa.defineBaseCase( m_projectFileName );

      m_scLoc = ibs::FolderPath( "." );
      // define scenarion cases location
      m_scLoc << "CaseSetLMTest";
      sa.setScenarioLocation( m_scLoc.cpath() );

      // add fastcauldron -itcoupled as application
      CauldronApp * app = RunManager::createApplication( RunManager::fastcauldron, 1, 0, 
#ifdef _WIN32
                                                         CauldronApp::cmd
#else
                                                         CauldronApp::bash
#endif
                                                       );
      app->addOption( "-itcoupled" );

      // add IP as surface pororsity for Spekk layer and LithologiesFraction for Lark layer
      ASSERT_EQ( ErrorHandler::NoError, sa.runManager().addApplication( app ) );

      ASSERT_EQ( ErrorHandler::NoError, VarySurfacePorosity( sa
                                                           , "SpekkSP"
                                                           , std::vector<std::pair<std::string,size_t> >(1, std::pair<std::string,size_t>("Spekk", 0U)) 
                                                           , std::vector<std::string>()
                                                           , std::vector<std::pair<std::string,std::string> >()
                                                           , "Tight_SHALE"
                                                           , 30.0
                                                           , 60.0
                                                           , VarPrmContinuous::Block
                                                           ) );

      ASSERT_EQ( ErrorHandler::NoError, VaryLithoFraction( sa
                                                         , "LarkLF"
                                                         , "Lark"
                                                         , std::vector<int>( 1, 1 )
                                                         , std::vector<double>( 1, 20 )
                                                         , std::vector<double>( 1, 50 )
                                                         , VarPrmContinuous::Block ) );

      // must be ignored by LM due to frozen range
      ASSERT_EQ( ErrorHandler:: NoError, VarySurfacePorosity( sa
                                                            , "JurassicSP"
                                                            , std::vector<std::pair<std::string,size_t> >(1, std::pair<std::string,size_t>("Jurassic", 0U))
                                                            , std::vector<std::string>()
                                                            , std::vector<std::pair<std::string,std::string> >()
                                                            , "Std. Sandstone"
                                                            , 65.0
                                                            , 65.0
                                                            , VarPrmContinuous::Block ) );

      std::vector<casa::ObsValue *> ovs;

      casa::Observable * obsVal = casa::ObsGridPropertyXYZ::createNewInstance( 448197, 6706691, 3849.2, "OverPressure", 0, "OvP1" );
      ASSERT_EQ( ErrorHandler::NoError, sa.obsSpace().addObservable( obsVal ) );
      ovs.push_back( casa::ObsValueDoubleScalar::createNewInstance( obsVal, 75.1 ) );


      // Add well data for Pressure, Temperature and VRe for one well
      std::vector<double> x;
      std::vector<double> y;
      std::vector<double> z;
      std::vector<double> v;

      // Add Pressure well data
      for ( size_t i = 0; i < sizeof( Well_030_07_06Pres )/sizeof(double)/4; ++i )
      {
         x.push_back( Well_030_07_06Pres[i][0] );
         y.push_back( Well_030_07_06Pres[i][1] );
         z.push_back( Well_030_07_06Pres[i][2] );
         v.push_back( Well_030_07_06Pres[i][3] );
      }
      obsVal = casa::ObsGridPropertyWell::createNewInstance( x, y, z, "Pressure", 0, "Well_030_07_06_P" );
      obsVal->setReferenceValue( new casa::ObsValueDoubleArray( obsVal, v ), 2.0 );
      obsVal->setSAWeight( 1.0 );
      obsVal->setUAWeight( 1.0 );
 
      ASSERT_EQ( ErrorHandler::NoError, sa.obsSpace().addObservable( obsVal ) );
      for ( size_t i = 0; i < v.size(); ++i ) v[i] *= 1.023 + 0.1 * static_cast<double>( i ) / v.size() * ( 1 - 2 * i%2 );
      ovs.push_back( casa::ObsValueDoubleArray::createNewInstance( obsVal, v ) ); 

      obsVal = casa::ObsGridPropertyXYZ::createNewInstance( 448197, 6706691, 3862.4, "OverPressure", 0, "OvP2" );
      ASSERT_EQ( ErrorHandler::NoError, sa.obsSpace().addObservable( obsVal ) );
      ovs.push_back( casa::ObsValueDoubleScalar::createNewInstance( obsVal, 75.5 ) );

      obsVal = casa::ObsGridPropertyXYZ::createNewInstance( 448197, 6706691, 3862.5, "OverPressure", 0, "OvP3" );
      ASSERT_EQ( ErrorHandler::NoError, sa.obsSpace().addObservable( obsVal ) );
      ovs.push_back( casa::ObsValueDoubleScalar::createNewInstance( obsVal, 75.9 ) );

      x.clear(); y.clear(); z.clear(); v.clear();

      // Add Temp well data
      for ( size_t i = 0; i < sizeof( Well_030_07_06Temp )/sizeof(double)/4; ++i )
      {
         x.push_back( Well_030_07_06Temp[i][0] );
         y.push_back( Well_030_07_06Temp[i][1] );
         z.push_back( Well_030_07_06Temp[i][2] );
         v.push_back( Well_030_07_06Temp[i][3] );
      }
      obsVal = casa::ObsGridPropertyWell::createNewInstance( x, y, z, "Temperature", 0, "Well_030_07_06_T" );
      obsVal->setReferenceValue( new casa::ObsValueDoubleArray( obsVal, v ), 5.0 );
      obsVal->setSAWeight( 1.0 );
      obsVal->setUAWeight( 1.0 );
 
      ASSERT_EQ( ErrorHandler::NoError, sa.obsSpace().addObservable( obsVal ) );
 
      for ( size_t i = 0; i < v.size(); ++i ) v[i] *= 1.0185 + 0.1 * static_cast<double>( i ) / v.size() * ( 1 - 2 * i%2 );
      ovs.push_back( casa::ObsValueDoubleArray::createNewInstance( obsVal, v ) ); 

      x.clear(); y.clear(); z.clear(); v.clear();

      // Add Vr well data
      for ( size_t i = 0; i < sizeof( Well_030_07_06VR )/sizeof(double)/4; ++i )
      {
         x.push_back( Well_030_07_06VR[i][0] );
         y.push_back( Well_030_07_06VR[i][1] );
         z.push_back( Well_030_07_06VR[i][2] );
         v.push_back( Well_030_07_06VR[i][3] );
      }
      obsVal = casa::ObsGridPropertyWell::createNewInstance( x, y, z, "Vr", 0, "Well_030_07_06_Vr" );
      obsVal->setReferenceValue( new casa::ObsValueDoubleArray( obsVal, v ), 0.1 );
      obsVal->setSAWeight( 1.0 );
      obsVal->setUAWeight( 1.0 );
 
      ASSERT_EQ( ErrorHandler::NoError, sa.obsSpace().addObservable( obsVal ) );
      
      for ( size_t i = 0; i < v.size(); ++i ) v[i] *= 1.0315 + 0.1 * static_cast<double>( i ) / v.size() * ( 1 - 2 * i%2 );
      ovs.push_back( casa::ObsValueDoubleArray::createNewInstance( obsVal, v ) ); 
    
      // Create 1D project around well
      ASSERT_EQ( ErrorHandler::NoError, sa.extractOneDProjects( "OneDProjects" ) );
      
      ASSERT_EQ( 1U, sa.doeCaseSet().size() );

      ASSERT_EQ( ErrorHandler::NoError, sa.applyMutations( sa.doeCaseSet() ) );
      ASSERT_EQ( ErrorHandler::NoError, sa.validateCaseSet( sa.doeCaseSet() ) );

      // Now create run case for LM optimization

      // switch base case first
      sa.defineBaseCase( sa.doeCaseSet().runCase( 0 )->projectPath() );

      ibs::FolderPath fp( sa.doeCaseSet().runCase( 0 )->projectPath() );
      fp.cutLast();
      // redefine scenario case location
      sa.setScenarioLocation( fp.fullPath().cpath() );

      // create base case parameters value
      std::vector<double>       cntPrms;
      std::vector<unsigned int> catPrms;

      for ( size_t i = 0; i < sa.varSpace().size(); ++i )
      {
         switch( sa.varSpace().parameter( i )->variationType() )
         {
            case VarParameter::Continuous:
               {
                  const VarPrmContinuous * vprm = dynamic_cast<const VarPrmContinuous*>( sa.varSpace().parameter( i ) );
                  const std::vector<double> & pva = vprm->baseValue()->asDoubleArray();  
                  cntPrms.insert( cntPrms.end(), pva.begin(), pva.end() );
               }
               break;

            case VarParameter::Categorical:
               catPrms.push_back( dynamic_cast<const VarPrmCategorical*>( sa.varSpace().parameter( i ) )->baseValue()->asInteger() );
               break;

            default: break;
         }
      }
      // create new case with the new parameters values
      SUMlib::Case slCase( cntPrms, std::vector<int>(), catPrms );
      std::unique_ptr<RunCaseImpl> rc( new RunCaseImpl() );

      // convert array of parameters values to case parametrers
      sumext::convertCase( slCase, sa.varSpace(), *(rc.get()) );
   
      // add this case as a new experiment
      std::string expName = "LMStep_" + std::to_string( sa.scenarioIteration() );
      RunCaseSetImpl & rcs = dynamic_cast<RunCaseSetImpl&>( sa.doeCaseSet() );
  
      std::vector<RunCase*> cvec( 1, rc.get() );
      rcs.addNewCases( cvec, expName );
      rcs.filterByExperimentName( expName );
   
      // generate new project
      ASSERT_EQ( ErrorHandler::NoError, sa.applyMutations( rcs ) );
      ASSERT_EQ( ErrorHandler::NoError, sa.dataDigger().requestObservables(sa.obsSpace(), sa.doeCaseSet() ) );

      // Populate observables with value
      for ( auto v : ovs )
      {
        rc->addObsValue( v );
      }
      rc.release();
   }
};

// LMOptAlgorithm test.
// Test creates 1D Multi SAC scenario for 1 well, generates project file and runs LM optimization 
TEST_F( LMOptimTest, FullWorkflow )
{
   ScenarioAnalysis sa;

   // set up scenario for 1D calibrartion
   prepareScenario( sa );

   // Create tester
   LMClassTest lmTester( sa );

   // test parameter preparation for LM
   lmTester.test_prepareParameters();

   lmTester.test_prepareObservables();

   lmTester.test_calculateFunctionValue();
}
 
TEST_F( LMOptimTest, TestLMFitSimpleFunc )
{
   struct my_functor : Eigen::DenseFunctor<double>
   {
      my_functor() : Eigen::DenseFunctor<double>( 2, 2 ) {}
 
      int operator() ( const Eigen::VectorXd & x, Eigen::VectorXd & fvec ) const
      {
         // Implement y = 10*(x0+3)^2 + (x1-5)^2
         fvec(0) = 10.0 * (x(0) - 3.0);
         fvec(1) =  x(1) - 5.0;
         return 0;
      }
   };

   Eigen::VectorXd x( 2 );
   x(0) = 1;
   x(1) = 1;

   my_functor functor;

   Eigen::NumericalDiff<my_functor> numDiff( functor );
   Eigen::LevenbergMarquardt< Eigen::NumericalDiff<my_functor> > lm( numDiff );

   lm.setMaxfev( 2000 );
   lm.setXtol( 1.0e-10 );

   ASSERT_EQ( 4, lm.minimize( x ) );
   ASSERT_EQ( 2, lm.iterations() );

   ASSERT_NEAR( 3.0, x(0), 1e-5 );
   ASSERT_NEAR( 5.0, x(1), 1e-5 );
}
