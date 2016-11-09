#include "../src/cmbAPI.h"
#include "../src/casaAPI.h"
#include "../src/ObsSpaceImpl.h"
#include "../src/RunCaseImpl.h"
#include "../src/RunCaseSetImpl.h"
#include "../src/RSProxyImpl.h"
#include "../src/ObsValueDoubleScalar.h"
#include "../src/ObsGridPropertyXYZ.h"
#include "../src/VarPrmSourceRockTOC.h"
#include "../src/VarPrmTopCrustHeatProduction.h"

#include <memory>
#include <cmath>
#include <cstdlib>

#include <gtest/gtest.h>

using namespace casa;
using namespace std;

static const double eps  = 1.e-5;
static const double reps = 0.01;

class MCTest : public ::testing::Test
{
public:
   MCTest()
   { 
      m_validate = false;
      const char * envVal = getenv( "VALIDATE_UNIT_TEST" );
      if ( envVal )
      {
         m_validate = true;
      }
   }
   ~MCTest() { ; }

   bool m_validate;

   double relativeError( double v1, double v2 ) { return fabs( (v1-v2)/(v1+v2) ); }

   void prepareScenarioUpToMC( casa::ScenarioAnalysis & sc, RSProxy::RSKrigingType krig, RSProxy ** proxy )
   {
      double obsVals[5][2] = { { 65.1536, 0.479763 },
                               { 49.8126, 0.386869 },
                               { 80.4947, 0.572657 },
                               { 65.1536, 0.479763 },
                               { 65.1536, 0.479763 }
                             };

      VarSpace      & vrs = sc.varSpace();
      ObsSpaceImpl  & obs = dynamic_cast<ObsSpaceImpl&>( sc.obsSpace() );

      vector<double> dblRng( 3, 5.0 );
      dblRng[1] = 15.0;
      dblRng[2] = 10.0;
      ASSERT_EQ( ErrorHandler::NoError, vrs.addParameter( new VarPrmSourceRockTOC( "Lower Jurassic", dblRng, vector<string>() ) ) );
   
      dblRng[0] = 0.1;
      dblRng[1] = 4.9;
      dblRng[2] = 2.5;
      ASSERT_EQ( ErrorHandler::NoError, vrs.addParameter( new VarPrmTopCrustHeatProduction( dblRng, vector<string>() ) ) );

      Observable * ob = ObsGridPropertyXYZ::createNewInstance( 460001.0, 6750001.0, 2751.0, "Temperature", 0.0 );
      ob->setReferenceValue( new ObsValueDoubleScalar( ob, 108.6 ), new ObsValueDoubleScalar( ob, 2.0 ) ); 
      ASSERT_EQ( ErrorHandler::NoError, obs.addObservable( ob ) );

      ob = ObsGridPropertyXYZ::createNewInstance( 460001.0, 6750001.0, 2730.0, "Vr", 0.0 );
      ob->setReferenceValue( new ObsValueDoubleScalar( ob, 1.1 ), new ObsValueDoubleScalar( ob, 0.1 ) );
      ASSERT_EQ( ErrorHandler::NoError, obs.addObservable( ob ) );

      ASSERT_EQ( ErrorHandler::NoError, sc.setDoEAlgorithm( DoEGenerator::Tornado ) );

      DoEGenerator  & doe = sc.doeGenerator();
      ASSERT_EQ( ErrorHandler::NoError, doe.generateDoE( sc.varSpace(), sc.doeCaseSet() ) );

      vector<const RunCase*> proxyRC;

      // add observables values without running cases
      RunCaseSetImpl & rcs = dynamic_cast<RunCaseSetImpl&>( sc.doeCaseSet() );
      for ( size_t i = 0; i < rcs.size(); ++i ) 
      {
         RunCaseImpl * rc = dynamic_cast<RunCaseImpl*>( rcs[ i ].get() );

         proxyRC.push_back( rc ); // collect run cases for proxy calculation

         for ( size_t j = 0; j < 2; ++j )
         {
            rc->addObsValue( new ObsValueDoubleScalar( obs[j], obsVals[i][j] ) );
         }
         rc->setRunStatus( RunCase::Completed );
      }

      // Create and calculate RS proxy
      vector<string> doeList;
      doeList.push_back( DoEGenerator::DoEName( DoEGenerator::Tornado ) );

      ASSERT_EQ( ErrorHandler::NoError, sc.addRSAlgorithm( "TestFirstOrderTornadoRS", 1, krig, doeList ) );
      *proxy = dynamic_cast<RSProxyImpl*>( sc.rsProxySet().rsProxy( "TestFirstOrderTornadoRS" ) );
   }

   void printMCResults( const char * tableName, const casa::MonteCarloSolver::MCResults & mcSamples )
   {  
      cerr << "\nstatic double " << tableName << "[" << mcSamples.size() << "][" << mcSamples[0].second->parametersNumber() + 2 +
                                                                                    mcSamples[0].second->observablesNumber() << "] =\n{\n";
      for ( size_t i = 0; i < mcSamples.size(); ++i )
      {
          // print RMSE
         cerr << "   { " << i << ", " << mcSamples[i].first << ", ";
         // print parameters
         for ( size_t j = 0; j < mcSamples[i].second->parametersNumber(); ++j )
         {
            SharedParameterPtr prm = mcSamples[i].second->parameter( j );
            const vector<double> & prmVals = prm->asDoubleArray();
            for ( size_t k = 0; k < prmVals.size(); ++k )
            {
               cerr << prmVals[k] << ", ";
            }
         }

         for ( size_t j = 0; j < mcSamples[i].second->observablesNumber(); ++j )
         {
            casa::ObsValue * obv = mcSamples[i].second->obsValue( j );
            cerr << obv->asDoubleArray()[0] << (j == mcSamples[i].second->observablesNumber() - 1 ? " " : ", " );
         }
         cerr << "}" << ( i == mcSamples.size() - 1 ? "" : "," ) << endl;
      }
      cerr << "};\n";
   }

   void checkMCResults( const casa::MonteCarloSolver::MCResults & mcSamples, double table[50][6] )
   {
      //compare observables values:
      for ( size_t i = 0; i < mcSamples.size(); ++i )
      {
         size_t off = 1;
         // Check RMSE
         EXPECT_LT( relativeError( table[i][off++], mcSamples[i].first ), reps );
   
         // check generated parameters
         for ( size_t j = 0; j < mcSamples[i].second->parametersNumber(); ++j )
         {
            SharedParameterPtr prm = mcSamples[i].second->parameter( j );
            const vector<double> & prmVals = prm->asDoubleArray();
            for ( size_t k = 0; k < prmVals.size(); ++k )
            {
               EXPECT_LT( relativeError( table[i][off++], prmVals[k] ), reps );
            }
         }

         for ( size_t j = 0; j < mcSamples[i].second->observablesNumber(); ++j )
         {
            casa::ObsValue * obv = mcSamples[i].second->obsValue( j );
            EXPECT_LT( relativeError( table[i][off++], obv->asDoubleArray()[0] ), reps );
         }
      }
   }
};

static double MonteCarlo50SamplesResults[50][6] =
{
   { 0,  10.788,  14.8356, 4.66577,  79.9903, 0.569603 },
   { 1,  11.1051, 14.5375, 4.35741,  79.0755, 0.564063 },
   { 2,  12.1992, 13.5121, 4.64509,  75.9296, 0.545014 },
   { 3,  12.2528, 13.462,  4.37473,  75.7758, 0.544083 },
   { 4,  12.382,  13.3412, 4.38069,  75.4051, 0.541839 },
   { 5,  12.473,  13.2562, 0.975455, 75.1444, 0.54026  },
   { 6,  13.0602, 12.7081, 2.86928,  73.4626, 0.530076 },
   { 7,  13.1692, 12.6065, 0.807402, 73.1509, 0.528188 },
   { 8,  13.1772, 12.5991, 3.11375,  73.1282, 0.528051 },
   { 9,  13.4219, 12.371,  2.75138,  72.4284, 0.523813 },
   { 10, 13.4739, 12.3226, 2.82958,  72.2797, 0.522913 },
   { 11, 13.879,  11.9454, 0.758501, 71.1226, 0.515907 },
   { 12, 13.9218, 11.9056, 1.281,    71.0004, 0.515167 },
   { 13, 13.9561, 11.8738, 1.46125,  70.9027, 0.514575 },
   { 14, 14.0903, 11.7489, 0.460476, 70.5197, 0.512256 },
   { 15, 14.5143, 11.3547, 3.06722,  69.3103, 0.504932 },
   { 16, 14.677,  11.2036, 1.66816,  68.8465, 0.502125 },
   { 17, 14.7035, 11.179,  1.21514,  68.7712, 0.501668 },
   { 18, 15.0521, 10.8554, 1.31659,  67.7781, 0.495655 },
   { 19, 15.0772, 10.832,  0.838549, 67.7065, 0.495221 },
   { 20, 16.0716, 9.90996, 1.4414,   64.8774, 0.47809  },
   { 21, 16.1933, 9.7972,  1.05155,  64.5314, 0.475995 },
   { 22, 16.2337, 9.75986, 1.33898,  64.4168, 0.475301 },
   { 23, 16.2853, 9.71205, 4.47305,  64.2701, 0.474413 },
   { 24, 16.3577, 9.64494, 1.6922,   64.0642, 0.473166 },
   { 25, 17.0013, 9.04922, 1.65238,  62.2364, 0.462099 },
   { 26, 17.1086, 8.94991, 1.06112,  61.9317, 0.460254 },
   { 27, 17.4415, 8.64202, 0.424968, 60.9871, 0.454533 },
   { 28, 17.5521, 8.53975, 0.501683, 60.6733, 0.452633 },
   { 29, 17.7836, 8.32576, 3.82228,  60.0167, 0.448658 },
   { 30, 18.196,  7.94458, 4.14834,  58.8471, 0.441576 },
   { 31, 18.3458, 7.80617, 4.21491,  58.4225, 0.439004 },
   { 32, 18.4407, 7.71851, 0.850454, 58.1535, 0.437376 },
   { 33, 18.4816, 7.68076, 0.695143, 58.0377, 0.436674 },
   { 34, 19.3693, 6.86114, 4.01325,  55.5229, 0.421447 },
   { 35, 19.4815, 6.75755, 4.28907,  55.2051, 0.419522 },
   { 36, 19.6847, 6.57004, 0.620838, 54.6298, 0.416039 },
   { 37, 19.7768, 6.48512, 0.462004, 54.3692, 0.414461 },
   { 38, 19.9375, 6.33684, 1.16266,  53.9143, 0.411706 },
   { 39, 19.9832, 6.29465, 4.41505,  53.7848, 0.410922 },
   { 40, 20.0284, 6.25299, 3.78277,  53.657,  0.410148 },
   { 41, 20.1805, 6.11272, 0.883042, 53.2266, 0.407542 },
   { 42, 20.2077, 6.08762, 3.98812,  53.1496, 0.407076 },
   { 43, 20.2821, 6.01901, 1.09061,  52.9391, 0.405801 },
   { 44, 20.78,   5.55989, 4.0976,   51.5304, 0.397271 },
   { 45, 20.97,   5.38472, 4.57082,  50.993,  0.394017 },
   { 46, 21.0465, 5.31419, 3.91781,  50.7766, 0.392706 },
   { 47, 21.2216, 5.15283, 3.57788,  50.2815, 0.389708 },
   { 48, 21.3228, 5.05959, 4.20715,  49.9954, 0.387976 },
   { 49, 21.3263, 5.05631, 3.92793,  49.9854, 0.387915 }
};

static double MCMC50SamplesResults[50][6] =
{
   { 0,  10.6138, 14.9996, 3.621,    80.4935, 0.57265  },
   { 1,  10.6181, 14.9955, 4.54124,  80.4808, 0.572573 },
   { 2,  10.6185, 14.9951, 0.621135, 80.4798, 0.572567 },
   { 3,  10.6197, 14.994,  0.511065, 80.4764, 0.572546 },
   { 4,  10.6218, 14.9921, 3.30716,  80.4704, 0.57251  },
   { 5,  10.622,  14.9918, 2.02334,  80.4696, 0.572505 },
   { 6,  10.6231, 14.9908, 2.12795,  80.4665, 0.572487 },
   { 7,  10.6236, 14.9903, 3.03292,  80.465,  0.572478 },
   { 8,  10.6247, 14.9893, 0.662088, 80.4617, 0.572458 },
   { 9,  10.6287, 14.9856, 0.925191, 80.4503, 0.572389 },
   { 10, 10.6319, 14.9825, 0.512678, 80.441,  0.572332 },
   { 11, 10.6335, 14.9811, 3.33186,  80.4365, 0.572305 },
   { 12, 10.6344, 14.9802, 4.60552,  80.4339, 0.572289 },
   { 13, 10.635,  14.9796, 1.12382,  80.4321, 0.572278 },
   { 14, 10.6358, 14.9789, 2.46228,  80.4298, 0.572264 },
   { 15, 10.6367, 14.978,  1.76094,  80.4272, 0.572249 },
   { 16, 10.6378, 14.977,  1.59355,  80.424,  0.572229 },
   { 17, 10.6396, 14.9753, 3.49808,  80.4188, 0.572198 },
   { 18, 10.6435, 14.9716, 1.7125,   80.4076, 0.57213  },
   { 19, 10.6439, 14.9712, 1.89484,  80.4063, 0.572122 },
   { 20, 10.6476, 14.9678, 0.467267, 80.3957, 0.572058 },
   { 21, 10.6502, 14.9653, 1.86743,  80.3881, 0.572012 },
   { 22, 10.6533, 14.9624, 3.81712,  80.3794, 0.571959 },
   { 23, 10.654,  14.9617, 1.95494,  80.3772, 0.571946 },
   { 24, 10.6554, 14.9604, 1.76527,  80.3733, 0.571922 },
   { 25, 10.656,  14.9599, 1.50376,  80.3716, 0.571912 },
   { 26, 10.6654, 14.951,  0.586551, 80.3443, 0.571747 },
   { 27, 10.6661, 14.9503, 0.255906, 80.3422, 0.571733 },
   { 28, 10.668,  14.9486, 0.667534, 80.3368, 0.571701 },
   { 29, 10.6715, 14.9452, 0.69706,  80.3266, 0.571639 },
   { 30, 10.6749, 14.942,  2.04532,  80.3168, 0.57158  },
   { 31, 10.6756, 14.9413, 2.07706,  80.3147, 0.571567 },
   { 32, 10.6786, 14.9385, 4.8162,   80.306,  0.571515 },
   { 33, 10.6911, 14.9268, 1.98568,  80.2701, 0.571297 },
   { 34, 10.7021, 14.9164, 3.18953,  80.2383, 0.571104 },
   { 35, 10.7115, 14.9076, 0.226664, 80.211,  0.57094  },
   { 36, 10.7264, 14.8936, 3.84456,  80.1682, 0.57068  },
   { 37, 10.7268, 14.8931, 0.521945, 80.1668, 0.570672 },
   { 38, 10.7272, 14.8928, 0.754967, 80.1658, 0.570666 },
   { 39, 10.729,  14.8911, 0.114227, 80.1604, 0.570633 },
   { 40, 10.7319, 14.8884, 0.598605, 80.1522, 0.570583 },
   { 41, 10.735,  14.8854, 3.42631,  80.1432, 0.570529 },
   { 42, 10.7406, 14.8802, 4.62221,  80.127,  0.57043  },
   { 43, 10.7552, 14.8664, 3.65213,  80.0849, 0.570176 },
   { 44, 10.7743, 14.8485, 0.375491, 80.0297, 0.569842 },
   { 45, 10.8009, 14.8235, 2.39751,  79.9531, 0.569378 },
   { 46, 10.8052, 14.8194, 0.527,    79.9405, 0.569301 },
   { 47, 10.83,   14.7961, 4.44749,  79.8689, 0.568868 },
   { 48, 10.852,  14.7754, 0.351537, 79.8055, 0.568484 },
   { 49, 10.9136, 14.7175, 0.939383, 79.6278, 0.567408 }
};

TEST_F (MCTest, SolverOptionsSetGetTest )
{
   ScenarioAnalysis sc;
   // create corresponded MC algorithm
   ASSERT_EQ( ErrorHandler::NoError, sc.setMCAlgorithm( casa::MonteCarloSolver::MonteCarlo, casa::MonteCarloSolver::NoKriging,
                                                        casa::MonteCarloSolver::NoPrior,    casa::MonteCarloSolver::Normal ) );

   ASSERT_EQ( sc.mcSolver().algorithm(),          casa::MonteCarloSolver::MonteCarlo );
   ASSERT_EQ( sc.mcSolver().kriging(),            casa::MonteCarloSolver::NoKriging  );
   ASSERT_EQ( sc.mcSolver().measurementDistrib(), casa::MonteCarloSolver::Normal     );
   ASSERT_EQ( sc.mcSolver().priorDistribution(),  casa::MonteCarloSolver::NoPrior    );
 
   ASSERT_EQ( ErrorHandler::NoError, sc.setMCAlgorithm( casa::MonteCarloSolver::MCMC, casa::MonteCarloSolver::SmartKriging,
                                                        casa::MonteCarloSolver::MarginalPrior,    casa::MonteCarloSolver::Robust ) );

   ASSERT_EQ( sc.mcSolver().algorithm(),          casa::MonteCarloSolver::MCMC          );
   ASSERT_EQ( sc.mcSolver().kriging(),            casa::MonteCarloSolver::SmartKriging  );
   ASSERT_EQ( sc.mcSolver().measurementDistrib(), casa::MonteCarloSolver::Robust        );
   ASSERT_EQ( sc.mcSolver().priorDistribution(),  casa::MonteCarloSolver::MarginalPrior );
 
   ASSERT_EQ( ErrorHandler::NoError, sc.setMCAlgorithm( casa::MonteCarloSolver::MCLocSolver, casa::MonteCarloSolver::GlobalKriging,
                                                         casa::MonteCarloSolver::MultivariatePrior, casa::MonteCarloSolver::Mixed ) );

   ASSERT_EQ( sc.mcSolver().algorithm(),          casa::MonteCarloSolver::MCLocSolver       );
   ASSERT_EQ( sc.mcSolver().kriging(),            casa::MonteCarloSolver::GlobalKriging     );
   ASSERT_EQ( sc.mcSolver().measurementDistrib(), casa::MonteCarloSolver::Mixed             );
   ASSERT_EQ( sc.mcSolver().priorDistribution(),  casa::MonteCarloSolver::MultivariatePrior );
}


TEST_F( MCTest, MonteCarloTest )
{
   ScenarioAnalysis sc;
   RSProxy * proxy;

   prepareScenarioUpToMC( sc, RSProxy::NoKriging, &proxy );

   // create corresponded MC algorithm
   ASSERT_EQ( ErrorHandler::NoError, sc.setMCAlgorithm( casa::MonteCarloSolver::MonteCarlo, casa::MonteCarloSolver::NoKriging,
                                                        casa::MonteCarloSolver::NoPrior,    casa::MonteCarloSolver::Normal ) );

   ASSERT_EQ( ErrorHandler::NoError, sc.mcSolver().runSimulation( *proxy, sc.varSpace(), sc.varSpace(), sc.obsSpace(), 50, 10, 1.0 ) );
  
   EXPECT_NEAR( sc.mcSolver().stdDevFactor(), 1.0,  eps );  // VRE
   
   // export MC samples
   const casa::MonteCarloSolver::MCResults & mcSamples = sc.mcSolver().getSimulationResults();

   // must be 50 samples
   ASSERT_EQ( mcSamples.size(), 50U );

   if ( m_validate )
   {
      printMCResults( "MonteCarlo50SamplesResults", sc.mcSolver().getSimulationResults() );
   }
   else
   {
      checkMCResults( mcSamples, MonteCarlo50SamplesResults );
   }
}

TEST_F( MCTest, MCMCTestKrigingMismatch )
{
   ScenarioAnalysis sc;
   RSProxy * proxy;
   prepareScenarioUpToMC( sc, RSProxy::NoKriging, &proxy );

   // create corresponded MC algorithm
   ASSERT_EQ( ErrorHandler::NoError, sc.setMCAlgorithm( casa::MonteCarloSolver::MonteCarlo, casa::MonteCarloSolver::GlobalKriging,
                                                        casa::MonteCarloSolver::NoPrior,    casa::MonteCarloSolver::Normal ) );

   // Expect error here about mismatching kriging
   ASSERT_EQ( ErrorHandler::MonteCarloSolverError , sc.mcSolver().runSimulation( *proxy, sc.varSpace(), sc.varSpace(), sc.obsSpace(), 50, 10, 1.0 ) );
}

TEST_F( MCTest, MCMCTestNoKrigingGOF )
{
   ScenarioAnalysis sc;
   RSProxy * proxy;
   prepareScenarioUpToMC( sc, RSProxy::NoKriging, &proxy );

   // create corresponded MC algorithm
   ASSERT_EQ( ErrorHandler::NoError, sc.setMCAlgorithm( casa::MonteCarloSolver::MCMC,    casa::MonteCarloSolver::NoKriging,
                                                        casa::MonteCarloSolver::NoPrior, casa::MonteCarloSolver::Normal ) );

   // Expect error here about mismatching kriging
   ASSERT_EQ( ErrorHandler::NoError, sc.mcSolver().runSimulation( *proxy, sc.varSpace(), sc.varSpace(), sc.obsSpace(), 50, 10, 1.0 ) );

   if ( m_validate )
   {
      printMCResults( "MCMC50SamplesResults", sc.mcSolver().getSimulationResults() );
      cerr << "double GOR = " << sc.mcSolver().GOF() << ";\n" << endl;
   }
   else
   { 
      checkMCResults( sc.mcSolver().getSimulationResults(), MCMC50SamplesResults );
   }
}

