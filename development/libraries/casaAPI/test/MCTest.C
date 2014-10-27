#include "../src/cmbAPI.h"
#include "../src/casaAPI.h"
#include "../src/ObsSpaceImpl.h"
#include "../src/RunCaseImpl.h"
#include "../src/RunCaseSetImpl.h"
#include "../src/RSProxyImpl.h"
#include "../src/ObsValueDoubleScalar.h"
#include "../src/VarPrmSourceRockTOC.h"
#include "../src/VarPrmTopCrustHeatProduction.h"

#include <memory>
#include <cmath>

#include <gtest/gtest.h>

using namespace casa;

static const double eps  = 1.e-5;
static const double reps = 0.01;

class MCTest : public ::testing::Test
{
public:
   MCTest() { ; }
   ~MCTest() { ; }

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

      ASSERT_EQ( ErrorHandler::NoError, vrs.addParameter( new VarPrmSourceRockTOC(         "Lower Jurassic", 10.0, 5.0, 15.0, VarPrmContinuous::Block ) ) );
      ASSERT_EQ( ErrorHandler::NoError, vrs.addParameter( new VarPrmTopCrustHeatProduction(                  2.5,  0.1, 4.9,  VarPrmContinuous::Block ) ) );

      Observable * ob = ObsSpace::newObsPropertyXYZ( 460001.0, 6750001.0, 2751.0, "Temperature", 0.0 );
      ob->setReferenceValue( new ObsValueDoubleScalar( ob, 108.6 ), 2.0 ); 
      ASSERT_EQ( ErrorHandler::NoError, obs.addObservable( ob ) );

      ob = ObsSpace::newObsPropertyXYZ( 460001.0, 6750001.0, 2730.0, "Vr", 0.0 );
      ob->setReferenceValue( new ObsValueDoubleScalar( ob, 1.1 ), 0.1 );
      ASSERT_EQ( ErrorHandler::NoError, obs.addObservable( ob ) );

      ASSERT_EQ( ErrorHandler::NoError, sc.setDoEAlgorithm( DoEGenerator::Tornado ) );

      DoEGenerator  & doe = sc.doeGenerator();
      ASSERT_EQ( ErrorHandler::NoError, doe.generateDoE( sc.varSpace(), sc.doeCaseSet() ) );

      std::vector<const RunCase*> proxyRC;

      // add observables values without running cases
      RunCaseSetImpl & rcs = dynamic_cast<RunCaseSetImpl&>( sc.doeCaseSet() );
      for ( size_t i = 0; i < rcs.size(); ++i ) 
      {
         RunCaseImpl * rc = dynamic_cast<RunCaseImpl*>( rcs[ i ] );

         proxyRC.push_back( rc ); // collect run cases for proxy calculation

         for ( size_t j = 0; j < 2; ++j )
         {
            rc->addObsValue( new ObsValueDoubleScalar( obs[j], obsVals[i][j] ) );
         }
      }

      // Create RS proxy
      ASSERT_EQ( ErrorHandler::NoError, sc.addRSAlgorithm( "TestFirstOrderTornadoRS", 1, krig ) );
      *proxy = dynamic_cast<RSProxyImpl*>( sc.rsProxySet().rsProxy( "TestFirstOrderTornadoRS" ) );
      ASSERT_EQ( ErrorHandler::NoError, (*proxy)->calculateRSProxy( proxyRC ) );
   }

   void printMCResults( const casa::MonteCarloSolver::MCResults & mcSamples )
   {   
      for ( size_t i = 0; i < mcSamples.size(); ++i )
      {
          // print RMSE
         std::cerr << i << "   { " << mcSamples[i].first << ", ";
         // print parameters
         for ( size_t j = 0; j < mcSamples[i].second->parametersNumber(); ++j )
         {
            SharedParameterPtr prm = mcSamples[i].second->parameter( j );
            const std::vector<double> & prmVals = prm->asDoubleArray();
            for ( size_t k = 0; k < prmVals.size(); ++k )
            {
               std::cerr << prmVals[k] << ", ";
            }
         }

         for ( size_t j = 0; j < mcSamples[i].second->observablesNumber(); ++j )
         {
            casa::ObsValue * obv = mcSamples[i].second->obsValue( j );
            std::cerr << obv->doubleValue()[0] << (j == mcSamples[i].second->observablesNumber() - 1 ? " " : ", " );
         }
         std::cerr << "}" << std::endl;
      }
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
            const std::vector<double> & prmVals = prm->asDoubleArray();
            for ( size_t k = 0; k < prmVals.size(); ++k )
            {
               EXPECT_LT( relativeError( table[i][off++], prmVals[k] ), reps );
            }
         }

         for ( size_t j = 0; j < mcSamples[i].second->observablesNumber(); ++j )
         {
            casa::ObsValue * obv = mcSamples[i].second->obsValue( j );
            EXPECT_LT( relativeError( table[i][off++], obv->doubleValue()[0] ), reps );
         }
      }
   }
};

static double MonteCarlo50SamplesResults[50][6] = {
   { 0,  10.7880, 14.8356, 4.665770, 79.9903, 0.569603 }, 
   { 1,  11.1051, 14.5375, 4.357410, 79.0755, 0.564063 }, 
   { 2,  12.1992, 13.5121, 4.645090, 75.9296, 0.545014 }, 
   { 3,  12.2528, 13.4620, 4.374730, 75.7758, 0.544083 }, 
   { 4,  12.3820, 13.3412, 4.380690, 75.4051, 0.541839 }, 
   { 5,  12.4730, 13.2562, 0.975455, 75.1444, 0.54026  },
   { 6,  13.0602, 12.7081, 2.869280, 73.4626, 0.530076 }, 
   { 7,  13.1692, 12.6065, 0.807402, 73.1509, 0.528188 }, 
   { 8,  13.1772, 12.5991, 3.113750, 73.1282, 0.528051 }, 
   { 9,  13.4219, 12.3710, 2.751380, 72.4284, 0.523813 }, 
   { 10, 13.4739, 12.3226, 2.829580, 72.2797, 0.522913 }, 
   { 11, 13.8790, 11.9454, 0.758501, 71.1226, 0.515907 }, 
   { 12, 13.9218, 11.9056, 1.281000, 71.0004, 0.515167 }, 
   { 13, 13.9561, 11.8738, 1.461250, 70.9027, 0.514575 }, 
   { 14, 14.0903, 11.7489, 0.460476, 70.5197, 0.512256 }, 
   { 15, 14.5143, 11.3547, 3.067220, 69.3103, 0.504932 }, 
   { 16, 14.6770, 11.2036, 1.668160, 68.8465, 0.502125 }, 
   { 17, 14.7035, 11.1790, 1.215140, 68.7712, 0.501668 }, 
   { 18, 15.0521, 10.8554, 1.316590, 67.7781, 0.495655 }, 
   { 19, 15.0772, 10.8320, 0.838549, 67.7065, 0.495221 }, 
   { 20, 16.0716, 9.90996, 1.441400, 64.8774, 0.47809  },
   { 21, 16.1933, 9.79720, 1.051550, 64.5314, 0.475995 }, 
   { 22, 16.2337, 9.75986, 1.338980, 64.4168, 0.475301 }, 
   { 23, 16.2853, 9.71205, 4.473050, 64.2701, 0.474413 }, 
   { 24, 16.3577, 9.64494, 1.692200, 64.0642, 0.473166 }, 
   { 25, 17.0013, 9.04922, 1.652380, 62.2364, 0.462099 }, 
   { 26, 17.1086, 8.94991, 1.061120, 61.9317, 0.460254 }, 
   { 27, 17.4415, 8.64202, 0.424968, 60.9871, 0.454533 }, 
   { 28, 17.5521, 8.53975, 0.501683, 60.6733, 0.452633 }, 
   { 29, 17.7836, 8.32576, 3.822280, 60.0167, 0.448658 }, 
   { 30, 18.1960, 7.94458, 4.148340, 58.8471, 0.441576 }, 
   { 31, 18.3458, 7.80617, 4.214910, 58.4225, 0.439004 }, 
   { 32, 18.4407, 7.71851, 0.850454, 58.1535, 0.437376 }, 
   { 33, 18.4816, 7.68076, 0.695143, 58.0377, 0.436674 }, 
   { 34, 19.3693, 6.86114, 4.013250, 55.5229, 0.421447 }, 
   { 35, 19.4815, 6.75755, 4.289070, 55.2051, 0.419522 }, 
   { 36, 19.6847, 6.57004, 0.620838, 54.6298, 0.416039 }, 
   { 37, 19.7768, 6.48512, 0.462004, 54.3692, 0.414461 }, 
   { 38, 19.9375, 6.33684, 1.162660, 53.9143, 0.411706 }, 
   { 39, 19.9832, 6.29465, 4.415050, 53.7848, 0.410922 }, 
   { 40, 20.0284, 6.25299, 3.782770, 53.6570, 0.410148 }, 
   { 41, 20.1805, 6.11272, 0.883042, 53.2266, 0.407542 }, 
   { 42, 20.2077, 6.08762, 3.988120, 53.1496, 0.407076 }, 
   { 43, 20.2821, 6.01901, 1.090610, 52.9391, 0.405801 }, 
   { 44, 20.7800, 5.55989, 4.097600, 51.5304, 0.397271 }, 
   { 45, 20.9700, 5.38472, 4.570820, 50.9930, 0.394017 }, 
   { 46, 21.0465, 5.31419, 3.917810, 50.7766, 0.392706 }, 
   { 47, 21.2216, 5.15283, 3.577880, 50.2815, 0.389708 }, 
   { 48, 21.3228, 5.05959, 4.207150, 49.9954, 0.387976 }, 
   { 49, 21.3263, 5.05631, 3.927930, 49.9854, 0.387915 }, 
};

static double MCMC50SamplesResults[50][6] = {
   { 0,  10.6138, 14.9996, 3.621000, 80.4935, 0.572650 },
   { 1,  10.6181, 14.9955, 4.541240, 80.4808, 0.572573 }, 
   { 2,  10.6185, 14.9951, 0.621135, 80.4798, 0.572567 }, 
   { 3,  10.6197, 14.9940, 0.511065, 80.4764, 0.572546 }, 
   { 4,  10.6218, 14.9921, 3.307160, 80.4704, 0.572510 }, 
   { 5,  10.6220, 14.9918, 2.023340, 80.4696, 0.572505 }, 
   { 6,  10.6231, 14.9908, 2.127950, 80.4665, 0.572487 }, 
   { 7,  10.6236, 14.9903, 3.032920, 80.4650, 0.572478 }, 
   { 8,  10.6247, 14.9893, 0.662088, 80.4617, 0.572458 }, 
   { 9,  10.6287, 14.9856, 0.925191, 80.4503, 0.572389 }, 
   { 10, 10.6319, 14.9825, 0.512678, 80.4410, 0.572332 }, 
   { 11, 10.6335, 14.9811, 3.331860, 80.4365, 0.572305 }, 
   { 12, 10.6344, 14.9802, 4.605520, 80.4339, 0.572289 }, 
   { 13, 10.6350, 14.9796, 1.123820, 80.4321, 0.572278 }, 
   { 14, 10.6358, 14.9789, 2.462280, 80.4298, 0.572264 }, 
   { 15, 10.6367, 14.9780, 1.760940, 80.4272, 0.572249 }, 
   { 16, 10.6378, 14.9770, 1.593550, 80.4240, 0.572229 }, 
   { 17, 10.6396, 14.9753, 3.498080, 80.4188, 0.572198 }, 
   { 18, 10.6435, 14.9716, 1.712500, 80.4076, 0.572130 }, 
   { 19, 10.6439, 14.9712, 1.894840, 80.4063, 0.572122 }, 
   { 20, 10.6476, 14.9678, 0.467267, 80.3957, 0.572058 }, 
   { 21, 10.6502, 14.9653, 1.867430, 80.3881, 0.572012 }, 
   { 22, 10.6533, 14.9624, 3.817120, 80.3794, 0.571959 }, 
   { 23, 10.6540, 14.9617, 1.954940, 80.3772, 0.571946 }, 
   { 24, 10.6554, 14.9604, 1.765270, 80.3733, 0.571922 }, 
   { 25, 10.6560, 14.9599, 1.503760, 80.3716, 0.571912 }, 
   { 26, 10.6654, 14.9510, 0.586551, 80.3443, 0.571747 }, 
   { 27, 10.6661, 14.9503, 0.255906, 80.3422, 0.571733 }, 
   { 28, 10.6680, 14.9486, 0.667534, 80.3368, 0.571701 }, 
   { 29, 10.6715, 14.9452, 0.697060, 80.3266, 0.571639 }, 
   { 30, 10.6749, 14.9420, 2.045320, 80.3168, 0.571580 }, 
   { 31, 10.6756, 14.9413, 2.077060, 80.3147, 0.571567 }, 
   { 32, 10.6786, 14.9385, 4.816200, 80.3060, 0.571515 }, 
   { 33, 10.6911, 14.9268, 1.985680, 80.2701, 0.571297 }, 
   { 34, 10.7021, 14.9164, 3.189530, 80.2383, 0.571104 }, 
   { 35, 10.7115, 14.9076, 0.226664, 80.2110, 0.570940 }, 
   { 36, 10.7264, 14.8936, 3.844560, 80.1682, 0.570680 }, 
   { 37, 10.7268, 14.8931, 0.521945, 80.1668, 0.570672 }, 
   { 38, 10.7272, 14.8928, 0.754967, 80.1658, 0.570666 }, 
   { 39, 10.7290, 14.8911, 0.114227, 80.1604, 0.570633 }, 
   { 40, 10.7319, 14.8884, 0.598605, 80.1522, 0.570583 }, 
   { 41, 10.7350, 14.8854, 3.426310, 80.1432, 0.570529 }, 
   { 42, 10.7406, 14.8802, 4.622210, 80.1270, 0.570430 }, 
   { 43, 10.7552, 14.8664, 3.652130, 80.0849, 0.570176 }, 
   { 44, 10.7743, 14.8485, 0.375491, 80.0297, 0.569842 }, 
   { 45, 10.8009, 14.8235, 2.397510, 79.9531, 0.569378 }, 
   { 46, 10.8052, 14.8194, 0.527000, 79.9405, 0.569301 }, 
   { 47, 10.8300, 14.7961, 4.447490, 79.8689, 0.568868 }, 
   { 48, 10.8520, 14.7754, 0.351537, 79.8055, 0.568484 }, 
   { 49, 10.9136, 14.7175, 0.939383, 79.6278, 0.567408 }, 
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
   ASSERT_EQ( mcSamples.size(), 50 );

   //printMCResults( sc.mcSolver().getSimulationResults() );
   checkMCResults( mcSamples, MonteCarlo50SamplesResults );
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

   //printMCResults( sc.mcSolver().getSimulationResults() );
   //std::cerr << sc.mcSolver().GOF() << std::endl;
   checkMCResults( sc.mcSolver().getSimulationResults(), MCMC50SamplesResults );
}

