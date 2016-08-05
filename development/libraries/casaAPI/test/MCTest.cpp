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

#include <gtest/gtest.h>

using namespace casa;
using namespace std;

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

   void printMCResults( const casa::MonteCarloSolver::MCResults & mcSamples )
   {   
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
         cerr << "}" << endl;
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

static double MonteCarlo50SamplesResults[50][6] = {
   { 0,  10.764,  14.8582, 4.66577,  80.0595, 0.570022 },
   { 1,  11.1291, 14.5149, 4.35741,  79.0063, 0.563644 },
   { 2,  12.2233, 13.4896, 4.64509,  75.8604, 0.544595 },
   { 3,  12.2287, 13.4846, 4.37473,  75.845,  0.544502 },
   { 4,  12.2873, 13.4297, 0.975455, 75.6766, 0.543483 },
   { 5,  12.4062, 13.3186, 4.38069,  75.3359, 0.54142  },
   { 6,  12.9832, 12.7799, 0.807402, 73.6831, 0.531411 },
   { 7,  13.1389, 12.6347, 2.86928,  73.2376, 0.528713 },
   { 8,  13.2559, 12.5257, 3.11375,  72.9031, 0.526688 },
   { 9,  13.5007, 12.2977, 2.75138,  72.2033, 0.522451 },
   { 10, 13.5527, 12.2492, 2.82958,  72.0547, 0.521551 },
   { 11, 13.6927, 12.1189, 0.758501, 71.6548, 0.519129 },
   { 12, 13.9038, 11.9224, 0.460476, 71.0519, 0.515479 },
   { 13, 13.9115, 11.9152, 1.281,    71.0299, 0.515345 },
   { 14, 13.9457, 11.8834, 1.46125,  70.9322, 0.514754 },
   { 15, 14.5167, 11.3525, 1.21514,  69.3034, 0.504891 },
   { 16, 14.5933, 11.2814, 3.06722,  69.0852, 0.50357  },
   { 17, 14.6667, 11.2132, 1.66816,  68.876,  0.502303 },
   { 18, 15.0669, 10.8416, 0.838549, 67.7359, 0.4954   },
   { 19, 15.1202, 10.7922, 1.31659,  67.5841, 0.49448  },
   { 20, 16.1399, 9.84673, 1.4414,   64.6834, 0.476916 },
   { 21, 16.183,  9.80681, 1.05155,  64.5609, 0.476174 },
   { 22, 16.3019, 9.69663, 1.33898,  64.2228, 0.474127 },
   { 23, 16.426,  9.58172, 1.6922,   63.8702, 0.471992 },
   { 24, 16.451,  9.55856, 4.47305,  63.7992, 0.471562 },
   { 25, 16.9217, 9.12288, 1.06112,  62.4624, 0.463467 },
   { 26, 17.0696, 8.98599, 1.65238,  62.0424, 0.460924 },
   { 27, 17.2545, 8.81499, 0.424968, 61.5178, 0.457747 },
   { 28, 17.365,  8.71272, 0.501683, 61.204,  0.455847 },
   { 29, 17.9496, 8.17227, 3.82228,  59.5458, 0.445806 },
   { 30, 18.2535, 7.89148, 0.850454, 58.6842, 0.440589 },
   { 31, 18.2943, 7.85373, 0.695143, 58.5684, 0.439888 },
   { 32, 18.3621, 7.79109, 4.14834,  58.3762, 0.438724 },
   { 33, 18.512,  7.65268, 4.21491,  57.9516, 0.436153 },
   { 34, 19.5356, 6.70766, 4.01325,  55.052,  0.418595 },
   { 35, 19.5763, 6.67007, 4.28907,  54.9367, 0.417897 },
   { 36, 19.6339, 6.61694, 0.620838, 54.7737, 0.41691  },
   { 37, 19.7259, 6.53202, 0.462004, 54.5131, 0.415332 },
   { 38, 19.7864, 6.47619, 3.98812,  54.3418, 0.414295 },
   { 39, 19.8867, 6.38373, 1.16266,  54.0581, 0.412577 },
   { 40, 19.8884, 6.38213, 4.41505,  54.0532, 0.412547 },
   { 41, 19.9335, 6.34048, 3.78277,  53.9254, 0.411773 },
   { 42, 20.1296, 6.15961, 0.883042, 53.3705, 0.408413 },
   { 43, 20.2312, 6.06591, 1.09061,  53.083,  0.406672 },
   { 44, 20.8206, 5.5225,  4.0976,   51.4157, 0.396576 },
   { 45, 20.9295, 5.42211, 4.57082,  51.1077, 0.394711 },
   { 46, 20.9516, 5.40167, 3.91781,  51.045,  0.394332 },
   { 47, 21.2822, 5.09698, 4.20715,  50.1101, 0.388671 },
   { 48, 21.2858, 5.09371, 3.92793,  50.1001, 0.38861  },
   { 49, 21.3165, 5.06535, 3.57788,  50.0131, 0.388083 }
};

static double MCMC50SamplesResults[50][6] = {
   { 0,  10.6199, 14.9939, 0.353319, 80.4759, 0.572543 },
   { 1,  10.6241, 14.9899, 2.22,     80.4637, 0.57247  },
   { 2,  10.6246, 14.9894, 0.823811, 80.4621, 0.57246  },
   { 3,  10.625,  14.989,  1.07942,  80.4609, 0.572452 },
   { 4,  10.6293, 14.985,  0.152547, 80.4485, 0.572378 },
   { 5,  10.6296, 14.9847, 0.244148, 80.4477, 0.572373 },
   { 6,  10.6358, 14.9788, 1.79458,  80.4297, 0.572264 },
   { 7,  10.64,   14.9749, 0.925276, 80.4178, 0.572192 },
   { 8,  10.6417, 14.9733, 1.1165,   80.4128, 0.572161 },
   { 9,  10.642,  14.973,  0.426355, 80.4119, 0.572156 },
   { 10, 10.6421, 14.9729, 4.25969,  80.4116, 0.572154 },
   { 11, 10.6422, 14.9728, 3.57137,  80.4113, 0.572152 },
   { 12, 10.6425, 14.9725, 2.14452,  80.4103, 0.572146 },
   { 13, 10.6426, 14.9725, 3.47253,  80.4102, 0.572146 },
   { 14, 10.6441, 14.9711, 0.140859, 80.406,  0.57212  },
   { 15, 10.6455, 14.9697, 0.531559, 80.4016, 0.572094 },
   { 16, 10.6463, 14.969,  3.96393,  80.3995, 0.572081 },
   { 17, 10.6464, 14.9689, 2.5711,   80.3993, 0.572079 },
   { 18, 10.6492, 14.9662, 0.380807, 80.3911, 0.57203  },
   { 19, 10.6498, 14.9657, 1.21838,  80.3895, 0.57202  },
   { 20, 10.6503, 14.9652, 4.19547,  80.3878, 0.57201  },
   { 21, 10.6504, 14.9651, 1.01329,  80.3875, 0.572008 },
   { 22, 10.6522, 14.9634, 1.05815,  80.3823, 0.571977 },
   { 23, 10.6558, 14.96,   0.23379,  80.372,  0.571914 },
   { 24, 10.6562, 14.9597, 2.57613,  80.3709, 0.571908 },
   { 25, 10.6563, 14.9595, 1.13229,  80.3705, 0.571905 },
   { 26, 10.6588, 14.9572, 2.57458,  80.3634, 0.571862 },
   { 27, 10.6592, 14.9568, 2.15885,  80.3622, 0.571855 },
   { 28, 10.6593, 14.9568, 2.28329,  80.362,  0.571854 },
   { 29, 10.6631, 14.9531, 2.25454,  80.3509, 0.571787 },
   { 30, 10.6664, 14.95,   2.18242,  80.3413, 0.571729 },
   { 31, 10.6688, 14.9478, 2.22615,  80.3345, 0.571687 },
   { 32, 10.6698, 14.9468, 2.69632,  80.3315, 0.571669 },
   { 33, 10.6728, 14.944,  0.530648, 80.3228, 0.571616 },
   { 34, 10.68,   14.9372, 3.96848,  80.3021, 0.571491 },
   { 35, 10.685,  14.9326, 1.17187,  80.2878, 0.571404 },
   { 36, 10.6868, 14.9308, 0.40862,  80.2824, 0.571372 },
   { 37, 10.6877, 14.93,   2.6942,   80.28,   0.571357 },
   { 38, 10.6911, 14.9268, 0.56615,  80.27,   0.571297 },
   { 39, 10.6912, 14.9267, 2.42626,  80.2696, 0.571294 },
   { 40, 10.7057, 14.913,  3.32914,  80.2278, 0.571041 },
   { 41, 10.7062, 14.9126, 0.597936, 80.2264, 0.571032 },
   { 42, 10.7141, 14.9052, 0.541473, 80.2037, 0.570895 },
   { 43, 10.7147, 14.9046, 0.293939, 80.2018, 0.570884 },
   { 44, 10.7196, 14.8999, 3.53698,  80.1877, 0.570798 },
   { 45, 10.7215, 14.8981, 0.884713, 80.1821, 0.570765 },
   { 46, 10.7285, 14.8916, 4.25707,  80.162,  0.570642 },
   { 47, 10.7324, 14.8879, 2.23697,  80.1507, 0.570574 },
   { 48, 10.7336, 14.8868, 0.268091, 80.1474, 0.570554 },
   { 49, 10.7351, 14.8854, 3.33775,  80.143,  0.570527 }
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
   //cerr << sc.mcSolver().GOF() << endl;
   checkMCResults( sc.mcSolver().getSimulationResults(), MCMC50SamplesResults );
}

