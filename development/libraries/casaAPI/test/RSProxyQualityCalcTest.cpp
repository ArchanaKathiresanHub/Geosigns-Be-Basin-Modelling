#include "../src/RSProxyQualityCalculator.h"

#include "../src/ObsValue.h"
#include "../src/ObsValueDoubleScalar.h"
#include "../src/RunCase.h"
#include "../src/ScenarioAnalysis.h"
#include "../src/VarPrmContinuousTemplate.h"
#include "../src/PrmLithoSTPThermalCond.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

const double epsilon = 1.e-6;

// These test values were obtained from a test run with the Ottoland example.
// The influential parameters represent the  top crust heat production and the
// thermal conductivity of the mantle and the shale.
const std::vector<std::vector<double>> parameters =
{
   {2.5, 2.25, 2.75},
   {1.4, 1, 7},
   {3.2, 1, 7}
};

const std::vector<std::vector<double>> runCaseObservablesFullFactorial =
{
   {6.49927336e+01, 9.60027833e+01, 1.04727120e+02, 1.18386054e+02, 1.33183281e+02},
   {4.93337282e+01, 7.30738837e+01, 8.01249347e+01, 9.11525323e+01, 1.03035800e+02},
   {5.31951236e+01, 7.92944548e+01, 8.70524142e+01, 9.92205197e+01, 1.12364468e+02},
   {3.90111080e+01, 5.01086598e+01, 5.23729467e+01, 5.57315776e+01, 5.92536229e+01},
   {4.18970133e+01, 5.40923803e+01, 5.65763461e+01, 6.02708404e+01, 6.41559290e+01},
   {8.00160788e+01, 1.22610259e+02, 1.35319513e+02, 1.55471950e+02, 1.77534213e+02},
   {8.29364080e+01, 1.27332677e+02, 1.40581571e+02, 1.61615464e+02, 1.84671620e+02},
   {6.67054468e+01, 8.82676496e+01, 9.25856144e+01, 9.91149356e+01, 1.06092569e+02},
   {6.90460810e+01, 9.14921028e+01, 9.59807045e+01, 1.02774451e+02, 1.10042060e+02}
};

class MockObservable : public casa::Observable
{
public:
   MOCK_CONST_METHOD0(name, std::vector<std::string>());
   MOCK_CONST_METHOD0(hasReferenceValue, bool());
   MOCK_CONST_METHOD0(referenceValue, const casa::ObsValue*());
   MOCK_CONST_METHOD1(transform, casa::ObsValue*(const casa::ObsValue*));
   MOCK_CONST_METHOD0(dimensionUntransformed, size_t());
   MOCK_CONST_METHOD0(stdDeviationForRefValue, const casa::ObsValue*());
   MOCK_METHOD2(setReferenceValue, void(casa::ObsValue*, casa::ObsValue*));
   MOCK_CONST_METHOD0(saWeight, double());
   MOCK_CONST_METHOD0(uaWeight, double());
   MOCK_METHOD1(setSAWeight, void(double));
   MOCK_METHOD1(setUAWeight, void(double));
   MOCK_METHOD1(requestObservableInModel, ErrorHandler::ReturnCode(mbapi::Model&));
   MOCK_METHOD1(getFromModel, casa::ObsValue*(mbapi::Model&));
   MOCK_CONST_METHOD1(checkObservableForProject, bool(mbapi::Model&));
   MOCK_CONST_METHOD1(checkObservableOriginForProject, bool(mbapi::Model&));
   MOCK_CONST_METHOD1(save, bool(casa::CasaSerializer&));
   MOCK_CONST_METHOD0(typeName, std::string());
   MOCK_CONST_METHOD0(depth, std::vector<double>());
   MOCK_CONST_METHOD0(xCoords, std::vector<double>());
   MOCK_CONST_METHOD0(yCoords, std::vector<double>());
   MOCK_CONST_METHOD5(saveCommon, void(const casa::Observable*, casa::CasaSerializer&, bool&, const std::string&, const std::string&));

   size_t dimension() const override { return 3; }
   casa::ObsValue * createNewObsValueFromDouble( std::vector<double>::const_iterator & val ) const override
   {
      return new casa::ObsValueDoubleScalar( this, *val++ );
   }
};

class RSProxyCalcTest : public ::testing::Test
{
public:
   void setupScenario(casa::ScenarioAnalysis& scenario,
                      const std::vector<std::vector<double>>& parameters,
                      const std::vector<std::vector<double>>& runCaseObservables,
                      const std::vector<casa::DoEGenerator::DoEAlgorithm>& generators)
   {
      casa::VarSpace & varSpace = scenario.varSpace();
      casa::ObsSpace & obsSpace = scenario.obsSpace();
      for ( const std::vector<double>& params : parameters)
      {
         const casa::PrmLithoSTPThermalCond base(0, "", params[0]); // fix simple constructor here
         ASSERT_EQ( ErrorHandler::NoError, varSpace.addParameter( new casa::VarPrmContinuousTemplate<casa::PrmLithoSTPThermalCond>( base, "", params[1], params[2])));
      }

      ASSERT_EQ( ErrorHandler::NoError, obsSpace.addObservable( new MockObservable));
      ASSERT_EQ( ErrorHandler::NoError, obsSpace.addObservable( new MockObservable));
      ASSERT_EQ( ErrorHandler::NoError, obsSpace.addObservable( new MockObservable));
      ASSERT_EQ( ErrorHandler::NoError, obsSpace.addObservable( new MockObservable));
      ASSERT_EQ( ErrorHandler::NoError, obsSpace.addObservable( new MockObservable));

      for (const casa::DoEGenerator::DoEAlgorithm& generator : generators )
      {
         ASSERT_EQ( ErrorHandler::NoError, scenario.setDoEAlgorithm( generator ) );
         casa::DoEGenerator  & doe = scenario.doeGenerator();
         ASSERT_EQ( ErrorHandler::NoError, doe.generateDoE( scenario.varSpace(), scenario.doeCaseSet() ) );
      }

      std::vector<const casa::RunCase*> proxyRC;

      // add observables values without running cases
      casa::RunCaseSet& rcs = scenario.doeCaseSet();

      ASSERT_EQ( rcs.size(), runCaseObservables.size());
      for ( size_t i = 0; i < rcs.size(); ++i )
      {
         casa::RunCase* rc = rcs[ i ].get();

         proxyRC.push_back( rc ); // collect run cases for proxy calculation

         for ( size_t j = 0; j < 5; ++j )
         {
            casa::ObsValue * obv = casa::ObsValueDoubleScalar::createNewInstance( obsSpace.observable(j), runCaseObservables[i][j] );
            rc->addObsValue( obv );
         }
         rc->setRunStatus( casa::RunCase::Completed );
      }
   }
};

TEST_F(RSProxyCalcTest, CalculateR2AndR2Adj)
{
   casa::ScenarioAnalysis scenario;
   const casa::DoEGenerator::DoEAlgorithm generator = casa::DoEGenerator::FullFactorial;

   setupScenario(scenario, parameters, runCaseObservablesFullFactorial, {generator});

   // Create and calculate RS proxy
   std::vector<std::string> doeList;
   doeList.push_back( casa::DoEGenerator::DoEName( generator ) );

   ASSERT_EQ( ErrorHandler::NoError, scenario.addRSAlgorithm( "TestRS", 1, casa::RSProxy::NoKriging, doeList ) );

   casa::RSProxyQualityCalculator rsProxyCalculator{scenario};

   const std::vector<std::vector<double>> R2AndR2AdjActual = rsProxyCalculator.calculateR2AndR2adj("TestRS", doeList);

   const std::vector<double> R2Expected = {9.93381700e-01, 9.87494959e-01, 9.84379009e-01, 9.79618779e-01, 9.75046517e-01};
   const std::vector<double> R2adjExpected = {0.9894107194143148, 0.9799919341921606, 0.9750064142396971, 0.9673900464595927, 0.9600744266881495};

   ASSERT_EQ(R2AndR2AdjActual.size(), 2);
   ASSERT_EQ(R2AndR2AdjActual[0].size(), R2Expected.size());
   ASSERT_EQ(R2AndR2AdjActual[1].size(), R2adjExpected.size());

   for (size_t i = 0; i < R2Expected.size(); ++i)
   {
      EXPECT_NEAR(R2Expected[i],    R2AndR2AdjActual[0][i], epsilon);
      EXPECT_NEAR(R2adjExpected[i], R2AndR2AdjActual[1][i], epsilon);
   }
}

TEST_F(RSProxyCalcTest, calculateR2AndR2adjFromObservables)
{
   // runCasesObservables
   std::vector<std::vector<double>> runCasesObservables {{1,2,3,4},{2,3,4,5},{3,4,5,6}};
   std::vector<std::vector<double>> proxyEvaluationObservables {{1.5,2.5,3.5,4.5},{2.5,3.5,4.5,5.5},{3.5,4.5,5.5,6.5}};
   std::vector<int> nCoefficients {1,2,3,1};

   std::vector<std::vector<double>> r2AndR2Adj = casa::RSProxyQualityCalculator::calculateR2AndR2adjFromObservables(runCasesObservables, proxyEvaluationObservables, nCoefficients);

   const std::vector<double>& r2s = r2AndR2Adj[0];
   const std::vector<double>& r2Adjs = r2AndR2Adj[1];

   for (const double& r2 : r2s)
   {
      EXPECT_DOUBLE_EQ(r2,0.625);
   }

   EXPECT_DOUBLE_EQ(r2Adjs[0],0.625);
   EXPECT_DOUBLE_EQ(r2Adjs[1],0.25);
   EXPECT_TRUE(isinf(r2Adjs[2]));
   EXPECT_DOUBLE_EQ(r2Adjs[3],0.625);
}

TEST_F(RSProxyCalcTest, CalculateQ2)
{
   casa::ScenarioAnalysis scenario;

   const casa::DoEGenerator::DoEAlgorithm generator = casa::DoEGenerator::FullFactorial;

   setupScenario(scenario, parameters, runCaseObservablesFullFactorial, {generator});

   // Create and calculate RS proxy
   std::vector<std::string> doeList;
   doeList.push_back( casa::DoEGenerator::DoEName( generator ) );

   ASSERT_EQ( ErrorHandler::NoError, scenario.addRSAlgorithm( "TestRS", 1, casa::RSProxy::NoKriging, doeList ) );

   casa::RSProxyQualityCalculator rsProxyCalculator{scenario};

   const std::vector<double> Q2Expected = {9.84141978e-01, 9.57286716e-01, 9.44561001e-01, 9.25900944e-01, 9.08519254e-01 };
   std::vector<double> Q2Actual = rsProxyCalculator.calculateQ2("TestRS", doeList);

   ASSERT_EQ(Q2Actual.size(), Q2Expected.size());
   for (size_t i = 0; i < Q2Expected.size(); ++i)
   {
      EXPECT_NEAR(Q2Expected[i], Q2Actual[i], epsilon);
   }
}

TEST_F(RSProxyCalcTest, CalculateQ2ForTwoGenerators)
{
   casa::ScenarioAnalysis scenario;
   const std::vector<std::vector<double>> runCaseObservables =
   {
      {6.49927336e+01, 9.60027833e+01, 1.04727120e+02, 1.18386054e+02, 1.33183281e+02},
      {6.34074320e+01, 9.35150439e+01, 1.01984979e+02, 1.15237962e+02, 1.29585166e+02},
      {6.65772837e+01, 9.84904340e+01, 1.07469094e+02, 1.21533846e+02, 1.36782031e+02},
      {6.80356651e+01, 1.03240897e+02, 1.13738707e+02, 1.30308766e+02, 1.48342540e+02},
      {5.49944027e+01, 7.21428081e+01, 7.56036185e+01, 8.08028212e+01, 8.63251521e+01},
      {5.63018864e+01, 7.39430621e+01, 7.74996301e+01, 8.28475344e+01, 8.85325732e+01},
      {5.36859990e+01, 7.03409663e+01, 7.37057084e+01, 7.87559842e+01, 8.41154719e+01},
      {6.97033368e+01, 1.05936807e+02, 1.16742643e+02, 1.33808590e+02, 1.52397571e+02},
      {6.63662954e+01, 1.00542780e+02, 1.10732035e+02, 1.26806043e+02, 1.44284994e+02}
   };

   const std::vector<std::vector<double>> parameters =
   {
      {2.5, 2.25, 2.75},
      {1.4, 1, 7}
   };

   const std::vector<casa::DoEGenerator::DoEAlgorithm> algorithms = {casa::DoEGenerator::Tornado, casa::DoEGenerator::PlackettBurman};

   setupScenario(scenario, parameters, runCaseObservables, algorithms);

   // Create and calculate RS proxy
   std::vector<std::string> doeList;
   doeList.push_back( casa::DoEGenerator::DoEName( algorithms[0] ) );
   doeList.push_back( casa::DoEGenerator::DoEName( algorithms[1] ) );

   ASSERT_EQ( ErrorHandler::NoError, scenario.addRSAlgorithm( "TestRS", 1, casa::RSProxy::NoKriging, doeList ) );

   //const casa::RSProxy * proxy = scenario.rsProxySet().rsProxy( "TestRS" );
   casa::RSProxyQualityCalculator rsProxyCalculator{scenario};

   const std::vector<double> Q2Expected = {9.47296604e-01, 9.46036384e-01, 9.43394513e-01, 9.40000387e-01, 9.37249107e-01 };
   std::vector<double> Q2Actual = rsProxyCalculator.calculateQ2("TestRS", doeList);

   ASSERT_EQ(Q2Actual.size(), Q2Expected.size());
   for (size_t i = 0; i < Q2Expected.size(); ++i)
   {
      EXPECT_NEAR(Q2Expected[i],    Q2Actual[i],            epsilon);
   }
}
