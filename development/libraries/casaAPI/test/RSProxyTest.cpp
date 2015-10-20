#include "../src/cmbAPI.h"
#include "../src/casaAPI.h"
#include "../src/ObsSpaceImpl.h"
#include "../src/RunCaseImpl.h"
#include "../src/RunCaseSetImpl.h"
#include "../src/RSProxyImpl.h"
#include "../src/ObsGridPropertyXYZ.h"
#include "../src/ObsValueDoubleScalar.h"
#include "../src/VarPrmSourceRockTOC.h"
#include "../src/VarPrmTopCrustHeatProduction.h"

#include <memory>
//#include <cmath>

#include <gtest/gtest.h>

using namespace casa;

static const double eps = 1.e-6;

class RSProxyTest : public ::testing::Test
{
public:
   RSProxyTest() { ; }
   ~RSProxyTest() { ; }

};
  
TEST_F( RSProxyTest, Prm2Obs1Proxy1Test )
{
   double obsVals[5][2] = { { 65.1536, 0.479763 },
                            { 49.8126, 0.386869 },
                            { 80.4947, 0.572657 },
                            { 65.1536, 0.479763 },
                            { 65.1536, 0.479763 }
                          };


   ScenarioAnalysis sc;

   VarSpace      & vrs = sc.varSpace();
   ObsSpaceImpl  & obs = dynamic_cast<ObsSpaceImpl&>( sc.obsSpace() );

   ASSERT_EQ( ErrorHandler::NoError, vrs.addParameter( new VarPrmSourceRockTOC( "Lower Jurassic", 10.0, 5.0, 15.0, VarPrmContinuous::Block ) ) );
   ASSERT_EQ( ErrorHandler::NoError, vrs.addParameter( new VarPrmTopCrustHeatProduction( 2.5, 0.1, 4.9, VarPrmContinuous::Block ) ) );

   ASSERT_EQ( ErrorHandler::NoError, obs.addObservable( ObsGridPropertyXYZ::createNewInstance( 460001.0, 6750001.0, 2751.0, "Temperature", 0.01 ) ) );
   ASSERT_EQ( ErrorHandler::NoError, obs.addObservable( ObsGridPropertyXYZ::createNewInstance( 460001.0, 6750001.0, 2730.0, "Vr", 0.002 ) ) );

   ASSERT_EQ( ErrorHandler::NoError, sc.setDoEAlgorithm( DoEGenerator::Tornado ) );

   DoEGenerator  & doe = sc.doeGenerator();
   ASSERT_EQ( ErrorHandler::NoError, doe.generateDoE( sc.varSpace(), sc.doeCaseSet() ) );

   std::vector<const RunCase*> proxyRC;

   RunCaseSetImpl & rcs = dynamic_cast<RunCaseSetImpl&>( sc.doeCaseSet() );
   for ( size_t i = 0; i < rcs.size(); ++i ) 
   {
      RunCaseImpl * rc = dynamic_cast<RunCaseImpl*>( rcs[ i ] );

      proxyRC.push_back( rc ); // collect run cases for proxy calculation

      for ( size_t j = 0; j < 2; ++j )
      {
         rc->addObsValue( new ObsValueDoubleScalar( obs[j], obsVals[i][j] ) );
      }
      rc->setRunStatus( RunCase::Completed );
   }

   ASSERT_EQ( ErrorHandler::NoError, sc.addRSAlgorithm( "TestFirstOrderTornadoRS", 1, RSProxy::NoKriging, std::vector<std::string>() ) );
   RSProxyImpl * proxy = dynamic_cast<RSProxyImpl*>( sc.rsProxySet().rsProxy( "TestFirstOrderTornadoRS" ) );

   ASSERT_EQ( ErrorHandler::NoError, proxy->calculateRSProxy( proxyRC ) );
   const RSProxy::CoefficientsMapList & cml = proxy->getCoefficientsMapList();
   
   ASSERT_EQ( cml.size(), 2 ); // must be 2 observables

   // Check proxy coefficients for
   // first observable obs(1) = 65.1536 + 15.3411 * prm_1 +0 * prm_2 
   size_t cpow = 0;
   for ( RSProxy::CoefficientsMap::const_iterator it = cml[0].begin(); it != cml[0].end(); ++it )
   {
      const std::vector<unsigned int> & prm  = it->first;
      double                            coef = it->second.first; 
      switch ( cpow  )
      {
         case 0:
            ASSERT_EQ( prm.size(), 0 );
            EXPECT_NEAR( 65.15362, coef, eps );
            break;

         case 1:
            ASSERT_EQ( prm.size(), 1 );
            ASSERT_EQ( prm[0], 0 );
            EXPECT_NEAR( 15.34105, coef, eps );
            break;

         case 2:
            ASSERT_EQ( prm.size(), 1 );
            ASSERT_EQ( prm[0], 1 );
            EXPECT_NEAR( 0.0, coef, eps );
            break;
      }
      ++cpow;
   }
   // for the second observable obs(2) = 0.479763 + 0.0928937 * prm_1 +0 * prm_2;
   cpow = 0;
   for ( RSProxy::CoefficientsMap::const_iterator it = cml[1].begin(); it != cml[1].end(); ++it )
   {
      const std::vector<unsigned int> & prm  = it->first;
      double                            coef = it->second.first; 
      switch ( cpow  )
      {
         case 0:
            ASSERT_EQ( prm.size(), 0 );
            EXPECT_NEAR( 0.479763, coef, eps );
            break;

         case 1:
            ASSERT_EQ( prm.size(), 1 );
            ASSERT_EQ( prm[0], 0 );
            EXPECT_NEAR( 0.0928937, coef, eps );
            break;

         case 2:
            ASSERT_EQ( prm.size(), 1 );
            ASSERT_EQ( prm[0], 1 );
            EXPECT_NEAR( 0.0, coef, eps );
            break;
      }
      ++cpow;
   }
   // check response surface evaluation
   // prepare one new case
   std::auto_ptr<RunCaseImpl> nrc( new RunCaseImpl() );

   std::vector<double> prmVals(2);

   // set case parameters, just some arbitrary values inside corresponded ranges
   prmVals[0] = (10.16 - 10.0)/(15.0-10.0); // scale to range [baseVal:maxVal] for [-1:0:1] range
   prmVals[1] = 1.970;

   std::vector<double>::const_iterator vit = prmVals.begin();
   for ( size_t i = 0; i < prmVals.size(); ++i )
   {
      SharedParameterPtr prm = vrs.continuousParameter( i )->newParameterFromDoubles( vit ); // TOC
      nrc->addParameter( prm );
   }
   
   ASSERT_EQ( ErrorHandler::NoError, proxy->evaluateRSProxy( *(nrc.get() ) ) );

   EXPECT_NEAR( nrc->obsValue( 0 )->asDoubleArray()[0], 65.6445336, eps ); // T [0C]
   EXPECT_NEAR( nrc->obsValue( 1 )->asDoubleArray()[0], 0.4827356,  eps );  // VRE
}
