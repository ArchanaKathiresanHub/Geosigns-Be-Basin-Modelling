//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ProxyFitTester.h"

// SUMlib includes
#include <CompoundProxyCollection.h>

#include <gtest/gtest.h>

using namespace SUMlib;
using namespace SumLibTestUtils;

namespace
{
static const double eps = 1.e-6;

//Checks if the monomials and coefficients in m1 are also present in m2. m2 may have additional coefficients.
void areCoefficientMapsEqual( const CubicProxy::CoefficientsMap& m1, const CubicProxy::CoefficientsMap& m2)
{
   for (const auto& it1 : m1)
   {
      const auto key = it1.first;
      const auto it2 = m2.find(key);
      EXPECT_TRUE(it2 != m2.end()); //If equal, key should be present in both coefficient maps.
      EXPECT_NEAR(it1.second.first, it2->second.first, eps); //Compare the coefficients
   }
}
}

//Proxy functions are fitted to the DoE points by linear regression. It depends on the settings what terms are fitted.
//The available settings (used in these tests) are defined in the TestInfo struct.
//Generally, terms are fitted by performing linear regression on the DoE points. When modelsearch in enabled,
//the fitting procedure finds and only includes the monomial terms that contribute to improving the fitting quality
//(thus reducing the RMSE). This is an iterative procedure that is performed in the SUMlib::ProxyEstimator::autoEstimate function.
// Automated proxy determination starts by only considering zero or first order terms
// (depending on the number of data points).
// Based on the resulting proxy quality, additional points are added iteratively, until the algorithm has
// converged to a stable selection of terms.
// The constant term is always included, even if its zero.
//The actual procedure is best studied by looking at the code in this function.
//Performing the linear regression fit is done in the SUMlib::ProxyBuilder::fit function, which is called during the autoEstimate
//procedure for each potential polynomial function. The tests below validate the functioning of the proxy fitting procedure, and highlight
//some aspects of its behaviour.
TEST(RSProxyTest, polyFitManyDoEPoints1D)
{
   //Polynomial function represention a model to be fitted.
   CubicProxy::CoefficientsMap stubModel;
   const double c = 15;
   stubModel[{}] = { c,0.0 }; //Constant
   stubModel[{0}] = { c,0.0 }; //Linear
   stubModel[{0, 0}] = { c,0.0 }; //Quadratic
   stubModel[{0, 0, 0}] = { c,0.0 }; //Cubic

   const int nDataPointsPerDim = 10;
   const int nDimensions = 1;
   ProxyFitTester fitTester(nDataPointsPerDim,nDimensions);

   TestInfo info;
   //Order zero with modelsearch allows for terms up to cubic order to be included.
   info.order = 0;
   info.modelSearch = true;
   info.targetR2 = 1.0;
   info.confLevel = 0.5;

   {
      //The fitting procedure should be able to accurately identify the monomial terms for the stub model:
      fitTester.makeTestFit(stubModel,info);
      CubicProxy::CoefficientsMap fittedCoefficients = fitTester.fittedCoefficients();

      areCoefficientMapsEqual(stubModel,fittedCoefficients);

      //Check for R2, such that the tests fails if changes reduce the proxy quality:
      EXPECT_NEAR(fitTester.r2Adj(), 1.0, eps); //Fit should be close to exact
   }

   {
      //With a lower target R2, proxy search is stopped earlier and the optimal proxy is not found.
      info.targetR2 = 0.9;
      fitTester.makeTestFit(stubModel,info);

      // The 0.9 is just the lower limit, and the search stops when the actual r2Adj of a candidate proxy exceeds that value.
      // The actual r2Adj may well be higher than the lower limit, and is in this case.
      EXPECT_TRUE(fitTester.r2Adj() < 0.99);
   }

   {
      //Proxy never goes higher than cubic, even if real response is (only) higher order:
      info.targetR2 = 1.0;
      CubicProxy::CoefficientsMap stubFourthOrder;
      stubFourthOrder[{0, 0, 0, 0}] = { 12.0,0.0 }; //Fourth order
      fitTester.makeTestFit(stubFourthOrder,info);
      CubicProxy::CoefficientsMap fitFourthOrder = fitTester.fittedCoefficients();
      EXPECT_TRUE(fitTester.r2Adj() > 0.9222);
      EXPECT_TRUE(fitFourthOrder.find({0,0,0,0}) == fitFourthOrder.end()); //Fourth order term is not present in fit.

      //Check if the coefficients fitted using sumlib are equal to the coefficients fitted using eigen:
      areCoefficientMapsEqual(fitFourthOrder,fitTester.leastSquaresComparisonFit());
   }

   //With modelsearch false, the fitting just sticks to the specified order:

   {
      //Only constant term
      info.modelSearch = false;
      info.order = 0;
      fitTester.makeTestFit(stubModel,info);
      const CubicProxy::CoefficientsMap fittedCoefficients = fitTester.fittedCoefficients();

      //Constant term is the only term:
      EXPECT_EQ(fittedCoefficients.size(),1);
      EXPECT_TRUE(fittedCoefficients.find({}) != fittedCoefficients.end());

      //Check if the coefficients fitted using sumlib are equal to the coefficients fitted using eigen:
      areCoefficientMapsEqual(fittedCoefficients,fitTester.leastSquaresComparisonFit());
   }

   {
      //Constant + linear term
      info.modelSearch = false;
      info.order = 1;
      fitTester.makeTestFit(stubModel,info);
      const CubicProxy::CoefficientsMap fittedCoefficients = fitTester.fittedCoefficients();

      //Only constant and linear term:
      EXPECT_EQ(fittedCoefficients.size(),2);
      EXPECT_TRUE(fittedCoefficients.find({}) != fittedCoefficients.end());
      EXPECT_TRUE(fittedCoefficients.find({0}) != fittedCoefficients.end());

      //Check if the coefficients fitted using sumlib are equal to the coefficients fitted using eigen:
      areCoefficientMapsEqual(fittedCoefficients,fitTester.leastSquaresComparisonFit());
   }

   {
      //Constant + linear + squared term
      info.modelSearch = false;
      info.order = 2;
      fitTester.makeTestFit(stubModel,info);
      const CubicProxy::CoefficientsMap fittedCoefficients = fitTester.fittedCoefficients();

      //Only constant, linear and quadratic term:
      EXPECT_EQ(fittedCoefficients.size(),3);
      EXPECT_TRUE(fittedCoefficients.find({}) != fittedCoefficients.end());
      EXPECT_TRUE(fittedCoefficients.find({0}) != fittedCoefficients.end());
      EXPECT_TRUE(fittedCoefficients.find({0,0}) != fittedCoefficients.end());

      //Check if the coefficients fitted using sumlib are equal to the coefficients fitted using eigen:
      areCoefficientMapsEqual(fittedCoefficients,fitTester.leastSquaresComparisonFit());
   }
}

TEST(RSProxyTest, polyFitManyDoEPoints4D)
{
   //Test if the automatic fit works well with multiple influential parameters, tested here with 4 parameters.
   const int nDataPointsPerDim = 4;
   const int nDimensions = 4;
   ProxyFitTester fitTester(nDataPointsPerDim,nDimensions);

   // The integer vector keys in the coefficient map represent the monomial terms.
   // For example, for influential parameters A,B,C and D, represented by integers 0,1,3 and 4,
   // the integer vector key {3, 3, 3} with value pair {c,0.0) would represent monomial c*D*D*D*D. The integer vector
   // {1, 2, 3} with value pair {c,0.0) would represent monomial c*B*C*D.

   //Define a stub model with some nice crossterms:
   CubicProxy::CoefficientsMap stubModel;
   const double c = 15;
   stubModel[{}] = { c,0.0 };
   stubModel[{0}] = { c,0.0 };
   stubModel[{3}] = { c,0.0 };
   stubModel[{2, 2}] = { c,0.0 };
   stubModel[{0, 2}] = { c,0.0 };
   stubModel[{1, 3}] = { c,0.0 };
   stubModel[{0, 1, 3}] = { c,0.0 };
   stubModel[{3, 3, 3}] = { c,0.0 };
   stubModel[{1, 2, 3}] = { c,0.0 };

   TestInfo info;
   info.order = 0;
   info.modelSearch = true;
   info.targetR2 = 1.0;
   info.confLevel = 0.5;

   fitTester.makeTestFit(stubModel,info);
   CubicProxy::CoefficientsMap fittedCoefficients = fitTester.fittedCoefficients();
   EXPECT_NEAR(fitTester.r2Adj(), 1.0, eps);

   //All terms should be correctly identified by the automated fitting procedure:
   areCoefficientMapsEqual(stubModel,fittedCoefficients);
   //Note: Some additional terms that do not appear in the actual polynomial are sometimes identified by the fitting procedure,
   //with coefficients close to zero.

   //When using random DoE points, the goodness-of-fit is still close to 1, but the original monomial coefficients are not exactly retrieved.
   ProxyFitTester fitTesterRandomDoEPoints(nDataPointsPerDim,nDimensions, new RandomCaseMaker);
   fitTesterRandomDoEPoints.makeTestFit(stubModel,info);
   fittedCoefficients = fitTesterRandomDoEPoints.fittedCoefficients();
   EXPECT_NEAR(fitTesterRandomDoEPoints.r2Adj(), 1.0, eps);

   // This would fail:
   // areCoefficientMapsEqual(stubModel,fitTesterRandomDoEPoints);
}

TEST(RSProxyTest, polyFitFewDoEPoints)
{
   const int nDataPointsPerDim = 4;
   const int nDimensions = 1;
   ProxyFitTester fitTester(nDataPointsPerDim,nDimensions);

   //Polynomial function represention a model to be fitted.
   CubicProxy::CoefficientsMap stubModel;
   const double c = 15;
   stubModel[{}] = { c,0.0 }; //Constant
   stubModel[{0}] = { c,0.0 }; //Linear
   stubModel[{0, 0}] = { c,0.0 }; //Quadratic
   stubModel[{0, 0, 0}] = { c,0.0 }; //Cubic

   //Default TestInfo values, included for clarity:
   TestInfo info;
   info.order = 0;
   info.modelSearch = true;
   info.targetR2 = 1.0;
   info.confLevel = 0.5;

   fitTester.makeTestFit(stubModel,info);

   //The polynomial can at maximum contain the nDataPoints-1 coefficients when modelsearch is used. Therefore a perfect fit cannot be achieved in this case.
   const CubicProxy::CoefficientsMap fittedCoefficientsFewerData = fitTester.fittedCoefficients();

   //Check for R2, such that the tests fails if changes reduce the proxy quality:
   EXPECT_TRUE(fitTester.r2Adj() > 0.947  && fitTester.r2Adj() < 0.9999);

   //Expected result:
   //4 data points, thus only three terms can be fitted:
   EXPECT_TRUE(fittedCoefficientsFewerData.size() == 3);
   //Cubic term has the lowest influence (for equal coefficients c), thus should be discarded:
   EXPECT_TRUE(fittedCoefficientsFewerData.find({ 0,0,0 }) == fittedCoefficientsFewerData.end());

   //Increase the importance of the cubic term. Now it should be included at the expence of (one of) the lower-order terms:
   stubModel[{0, 0, 0}] = { c*100,0.0 }; //Cubic
   fitTester.makeTestFit(stubModel,info);

   const CubicProxy::CoefficientsMap fitCofDominantCubic = fitTester.fittedCoefficients();

   //Check for R2, such that the tests fails if changes reduce the proxy quality:
   EXPECT_TRUE(fitTester.r2Adj() > 0.9999);

   //Expected result:
   //4 data points, thus only three terms are fitted with model search enabled:
   EXPECT_TRUE(fitCofDominantCubic.size() == 3);
   //Cubic term has a higher influence now, and is therefore included,
   //as it follows from model search that including the cubic term at the expense of a lower-order term improved the fit quality:
   EXPECT_TRUE(fitCofDominantCubic.find({ 0,0,0 }) != fitCofDominantCubic.end());

   //In the current automated fitting implementation, the constant term is always there, even if its zero and could be neglected:
   stubModel[{}] = { 0.0,0.0 }; //zero constant term
   stubModel[{0, 0, 0}] = { c,0.0 }; //Cubic
   fitTester.makeTestFit(stubModel,info);

   const CubicProxy::CoefficientsMap fitZeroConstTerm = fitTester.fittedCoefficients();

   //Test that casa and sumlib give the same R2adj value:
   EXPECT_TRUE(fitTester.compareCasaSumlibR2Adj());

   //Check for R2, such that the tests fails if changes reduce the proxy quality. 0.947 is (close to) the current performance:
   EXPECT_TRUE(fitTester.r2Adj() > 0.947);
   
   EXPECT_TRUE(fitZeroConstTerm.find({}) != fitZeroConstTerm.end()); //Confirm zero-order term is there
   EXPECT_NEAR(fitZeroConstTerm.at({}).first, 0.0, eps); //And that it has a value close to zero

   //A higher confidence level may reduce the fit quality as it may trigger a break in the proxy iteration phase.
   //The higher the confidence level, the larger must the improvement of a new candidate proxy be compared to the current best fit to be accepted.
   //The casa API currently does not enable (user) control of this parameter.
   //For most tests in this file, a change in confidence level didn't seem to influence the fit results.
   const double previousR2Adj = fitTester.r2Adj();

   info.confLevel = 0.99;
   fitTester.makeTestFit(stubModel,info);
   EXPECT_TRUE(previousR2Adj > fitTester.r2Adj());

   //With modelSearch disabled, the fitting procedure sticks to the specified order, even if a higher order term is dominant:
   info.modelSearch = false;
   info.order = 2;
   stubModel[{0, 0, 0}] = { c * 100,0.0 }; //dominant cubic
   fitTester.makeTestFit(stubModel,info);

   const CubicProxy::CoefficientsMap fitNoModelSearchOrder1 = fitTester.fittedCoefficients();
   EXPECT_TRUE(fitNoModelSearchOrder1.size() == 3);
   EXPECT_TRUE(fitNoModelSearchOrder1.find({}) != fitNoModelSearchOrder1.end());
   EXPECT_TRUE(fitNoModelSearchOrder1.find({0}) != fitNoModelSearchOrder1.end());
   EXPECT_TRUE(fitNoModelSearchOrder1.find({0,0}) != fitNoModelSearchOrder1.end());
   EXPECT_TRUE(fitTester.r2Adj() > 0.768);
}

TEST(RSProxyTest, polyThreeDoEPoints)
{
   //Without modelsearch, the number of term in the fit can be equal to the to the number of datapoints (unlike when modelsearch is enabled).
   const int nDataPointsPerDim = 3;
   const int nDimensions = 1;
   ProxyFitTester fitTester(nDataPointsPerDim,nDimensions);

   //Polynomial function represention a model to be fitted.
   CubicProxy::CoefficientsMap stubModel;
   const double c = 15;
   stubModel[{0}] = { c,0.0 }; //Quadratic

   //Default values, included for clarity:
   TestInfo info;
   info.order = 2;
   info.modelSearch = false;
   info.targetR2 = 1.0;
   info.confLevel = 0.5;

   fitTester.makeTestFit(stubModel,info);
   const CubicProxy::CoefficientsMap fit = fitTester.fittedCoefficients();
   EXPECT_EQ(fit.size(),3);
   EXPECT_NEAR(fit.at({0}).first,c, eps);

   //When the number of unknowns is equal to the number of DoE points, a perfect fit can generally be found.
   //R2 is set to -1 for such case in ProxyCases::test, line 328
   EXPECT_NEAR(fitTester.r2Adj(),-1.0, eps);
}

TEST(RSProxyTest, polyTwoDoEPoints)
{
   //With only two data points, the fit only contains the constant term.
   const int nDataPointsPerDim = 2;
   const int nDimensions = 1;
   ProxyFitTester fitTester(nDataPointsPerDim,nDimensions);

   //Polynomial function represention a model to be fitted.
   CubicProxy::CoefficientsMap stubModel;
   const double c = 15;
   stubModel[{}] = { c,0.0 };
   stubModel[{0}] = { c,0.0 };
   stubModel[{0,0}] = { c,0.0 };

   //Default values, included for clarity:
   TestInfo info;
   info.order = 2;
   info.modelSearch = false;
   info.targetR2 = 1.0;
   info.confLevel = 0.5;

   fitTester.makeTestFit(stubModel,info);
   const CubicProxy::CoefficientsMap fit = fitTester.fittedCoefficients();

   EXPECT_EQ(fit.size(),1);
   EXPECT_NEAR(fit.at({}).first,2*c, eps);
   EXPECT_NEAR(fitTester.r2Adj(),0, eps); //R2_adj is zero
}

//Kriging is performed on the errors between a fitted proxy and the DoE points.
//The Kriging implementation is equal to simple Kriging as explained on wikipedia: https://en.wikipedia.org/wiki/Kriging.
//A discance-based covariance function cov(i,j) = 1 - distance(1,j)/corLength is used, as defined in the KrigingData::calcCovariance function.
//Global Kriging seems to work correctly, but local Kriging is incorrect and must be debugged/disabled (see testGlobalKriging2D).
TEST(RSProxyTest, testKriging1D)
{
   const int nDataPointsPerDim = 10;
   const int nDims = 1;
   ProxyFitTester fitTester(nDataPointsPerDim,nDims);

   CubicProxy::CoefficientsMap coefficientMap;
   const double c = 15;
   coefficientMap[{0, 0, 0, 0}] = { c,0.0 };

   TestInfo info;
   info.order = 0;
   info.modelSearch = false;
   fitTester.makeTestFit(coefficientMap,info);
   const CubicProxy::CoefficientsMap fittedCoefficients = fitTester.fittedCoefficients();

   const Case doePoint(std::vector<double>{0.9});
   const CompoundProxyCollection& proxyCollection = fitTester.proxyCollection();
   double valGlobalKriging = proxyCollection.getProxyValue(0,doePoint, KrigingType::GlobalKriging);

   //In 1D, Global Kriging results in the same value as simple linear interpolation between the surrounding data points:
   //Value expected from linear interpolation:
   const double x0 = 0.7777777777;
   const double x1 = 1.0;
   const double y0 = polynomialValue({x0},coefficientMap);
   const double y1 = polynomialValue({x1},coefficientMap);
   const double x = doePoint.continuousPar(0);
   const double dLinInterp = (y0*(x1-x)+y1*(x-x0))/(x1-x0);
   EXPECT_NEAR(valGlobalKriging, dLinInterp, eps);

   const double dReal = polynomialValue(doePoint.continuousPart(),coefficientMap);
   double relError = std::abs((dReal-valGlobalKriging)/dReal);
   EXPECT_TRUE(relError < 0.0893);

   //Local Kriging gives a different value, but seems reasonable
   //As the 2D test indicates that the local Kriging implementation is buggy,
   //the better match for this test might be a coincidence and the test could fail upon fixing/improving the local Kriging implementation.
   const double valLocalKriging = proxyCollection.getProxyValue(0,doePoint, KrigingType::LocalKriging);
   relError = std::abs((dReal-valLocalKriging)/dReal);
   EXPECT_TRUE(relError < 0.0393);
}

TEST(RSProxyTest, testKriging2D)
{
   const int nDataPointsPerDim = 11;
   const int nDims = 2;
   ProxyFitTester fitTester(nDataPointsPerDim,nDims);

   CubicProxy::CoefficientsMap coefficientMap;
   const double c = 15;
   coefficientMap[{0,0,0,0}] = { c,0.0 };
   coefficientMap[{1,1,1,1}] = { c,0.0 };

   TestInfo info;
   info.order = 0;
   info.modelSearch = false;
   fitTester.makeTestFit(coefficientMap,info);
   const CubicProxy::CoefficientsMap& fittedCoefficients = fitTester.fittedCoefficients();

   const double mean = fitTester.meanOfTargets(coefficientMap);
   EXPECT_NEAR(fittedCoefficients.at({}).first, mean, eps);

   const CompoundProxyCollection& proxyCollection = fitTester.proxyCollection();

   //Test Kriging estimate at two different points:
   {
      //At 0.9,0.9:
      const Case doePoint(std::vector<double>{0.9,0.9});
      const double valGlobalKriging = proxyCollection.getProxyValue(0,doePoint, KrigingType::GlobalKriging);
      const double valLocalKriging = proxyCollection.getProxyValue(0,doePoint, KrigingType::LocalKriging);

      const double dReal = polynomialValue(doePoint.continuousPart(),coefficientMap);
      const double relErrorGlobal = std::abs((dReal-valGlobalKriging)/dReal);
      EXPECT_TRUE(relErrorGlobal < 0.0562);

      const double relErrorLocal = std::abs((dReal-valLocalKriging)/dReal);

      // valLocalKriging is just close to the constant value coming from the polynomial fit, resulting in a 70 % relative error.
      // The local Kriging procedure doesn't contribute significantly to the estimate. A possible cause of this incorrect behavior
      // is that the negative weights are set to zero in KrigingWeights::sumPositiveWeights. This leads to a sum of weights >> 1,
      // while the sum should be close to 1. The Kriging value is devided by the sum of weights if > 1.
      // However, further investigation is needed.
      EXPECT_TRUE(relErrorLocal < 0.7001);
   }

   {
      //At 0.1,0.1:

      Case doePoint(std::vector<double>{0.1,0.1});
      const double valGlobalKriging = proxyCollection.getProxyValue(0,doePoint, KrigingType::GlobalKriging);
      const double valLocalKriging = proxyCollection.getProxyValue(0,doePoint, KrigingType::LocalKriging);

      const double dReal = polynomialValue(doePoint.continuousPart(),coefficientMap);

      //Use absolute error, as dReal is close to zero:
      const double absErrorGlobal = std::abs(dReal-valGlobalKriging);
      const double absErrorLocal = std::abs(dReal-valLocalKriging);

      EXPECT_TRUE(absErrorGlobal < 0.0233);

      //valLocalKriging is just close to the constant value coming from the polynomial fit,
      //and the Kriging procedure doesn't contribute significantly to the estimate.
      EXPECT_TRUE(absErrorLocal < 5.264);
   }
}
