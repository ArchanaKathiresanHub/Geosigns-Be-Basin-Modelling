

#include <gtest/gtest.h>

//std
#include <stdlib.h>
#include <time.h>

#include "PorosityModelConverter.h"

//cmbAPI
#include "LithologyManager.h"

static const double eps = 1.e-5;

using namespace mbapi;

class porosityModelConverterTest : public ::testing::Test, public Prograde::PorosityModelConverter
{
};

TEST(PorosityModelConverter, upgrade)
{
   Prograde::PorosityModelConverter modelConverter;
   std::string smSandstone("Soil Mechanics Sandstone");
   std::string lithoName("test");

   //Testing appending description
   EXPECT_EQ(smSandstone, modelConverter.upgradeDescription(smSandstone, smSandstone, LithologyManager::PorosityModel::PorExponential));
   EXPECT_EQ(smSandstone, modelConverter.upgradeDescription(smSandstone, smSandstone, LithologyManager::PorosityModel::PorDoubleExponential));
   EXPECT_EQ("Soil Mechanics Sandstone (upgraded to double exponential model)", 
      modelConverter.upgradeDescription(smSandstone, smSandstone, LithologyManager::PorosityModel::PorSoilMechanics));

   //Testing updating model
   EXPECT_EQ(LithologyManager::PorosityModel::PorExponential,
      modelConverter.upgradePorosityModel(smSandstone, LithologyManager::PorosityModel::PorExponential));
   EXPECT_EQ(LithologyManager::PorosityModel::PorDoubleExponential,
      modelConverter.upgradePorosityModel(smSandstone, LithologyManager::PorosityModel::PorDoubleExponential));
   EXPECT_EQ(LithologyManager::PorosityModel::PorDoubleExponential,
      modelConverter.upgradePorosityModel(smSandstone, LithologyManager::PorosityModel::PorSoilMechanics));

   //Testing creation of model parameters
   std::vector<double> modelPrms1;
   modelPrms1.push_back(1.0);
   modelPrms1.push_back(2.0);
   std::vector<double> modelPrms2 = modelConverter.upgradeModelParameters(smSandstone, LithologyManager::PorosityModel::PorSoilMechanics,
      modelPrms1);
   EXPECT_EQ(5, modelPrms2.size());
   modelPrms2 = modelConverter.upgradeModelParameters(smSandstone, LithologyManager::PorosityModel::PorDoubleExponential, modelPrms1);
   EXPECT_EQ(2, modelPrms2.size());
   modelPrms2 = modelConverter.upgradeModelParameters(smSandstone, LithologyManager::PorosityModel::PorExponential, modelPrms1);
   EXPECT_EQ(2, modelPrms2.size());

}

// This needs to be checked and make it available later
/*
TEST_F(porosityModelConverterTest, CurveFitting)
{
   srand(time(NULL));

   double c1 = static_cast<double>((rand() % 50) + 1) / 100.0;             //c1 is set to a random value between 0-0.5
   double c2 = static_cast<double>((rand() % 50) + 1) / 100.0;             //c2 is set to a random value between 0-0.5
   double ratio = static_cast<double>((rand() % 100) + 1) / 100.0;         //ratio is set to a random value between 0-1.0 
   double myPhiSurf = static_cast<double>((rand() % 97) + 3) / 100.0;      //surface porosity is set to a random value between 0.03-0.99
   vector<double> myCompacPrms{ c1, c2, ratio };

   EXPECT_NEAR(myPhiSurf, calculatePorosityDblExponential(0.03, myPhiSurf, myCompacPrms, 0.0), eps); //no compaction if there is no VES

   for (size_t i = 0; i < 50; i++)
   {
      double mySigma = static_cast<double> (i);
      vector<double> myJacobian = calculateDblExpPorosityDerivatives(0.03, myPhiSurf, myCompacPrms, mySigma);

      //testing if the double exponential model has been applied properly
      {
         EXPECT_NEAR(0.03, calculatePorosityDblExponential(0.03, 0.03, myCompacPrms, mySigma), eps);
         EXPECT_NEAR(myPhiSurf, calculatePorosityDblExponential(0.03, myPhiSurf, myCompacPrms, 0.0), eps);
         vector<double> myCompacPrms1{ 0.0, 0.0, ratio };
         EXPECT_NEAR(myPhiSurf, calculatePorosityDblExponential(0.03, myPhiSurf, myCompacPrms1, mySigma), eps);
      }

      //testing if the derivative function using the analytical equatiion, is giving approximately the dame result as the numerical derivative
      {
         vector<double> myCompacPrms1{ c1 + eps, c2, ratio };
         vector<double> myCompacPrms2{ c1 - eps, c2, ratio };
         double dPhidC1 = (calculatePorosityDblExponential(0.03, myPhiSurf, myCompacPrms1, mySigma) - calculatePorosityDblExponential(0.03, myPhiSurf, myCompacPrms2, mySigma))
            / (2.0 * eps);
         EXPECT_NEAR(myJacobian[0], dPhidC1, eps);
      }

      {
         vector<double> myCompacPrms1{ c1, c2 + eps, ratio };
         vector<double> myCompacPrms2{ c1, c2 - eps, ratio };
         double dPhidC2 = (calculatePorosityDblExponential(0.03, myPhiSurf, myCompacPrms1, mySigma) - calculatePorosityDblExponential(0.03, myPhiSurf, myCompacPrms2, mySigma))
            / (2.0 * eps);
         EXPECT_NEAR(myJacobian[1], dPhidC2, eps);
      }

      {
         vector<double> myCompacPrms1{ c1, c2, ratio + eps};
         vector<double> myCompacPrms2{ c1, c2, ratio - eps};
         double dPhidR = (calculatePorosityDblExponential(0.03, myPhiSurf, myCompacPrms1, mySigma) - calculatePorosityDblExponential(0.03, myPhiSurf, myCompacPrms2, mySigma))
            / (2.0 * eps);
         EXPECT_NEAR(myJacobian[2], dPhidR, eps);
      }
   }
 
   //testing if the matrix equation solver is working fine

   double matC[3][3];

   for (size_t i = 0; i < 3; i++)
   {
      for (size_t j = 0; j < 3; j++)
      {
         matC[i][j] = static_cast<double>((rand() % 10000) + 1) / 100.0;         //fills the matrix with entries between 0.00 to 100.00
      }
   }

   for (size_t i = 0; i < 3; i++)
   {
      matC[i][i] = 100.0;                                                        //to make sure the matrix is well conditioned and diagonally dominant
   }

   double matA[3][3];
   for (size_t i = 0; i < 3; i++)
   {
      for (size_t j = 0; j < 3; j++)
      {
         matA[i][j] = (matC[i][j] + matC[j][i])/2.0;                             //creates symmetric matrix
      }
   }

   double rhsB[3];
   for (size_t i = 0; i < 3; i++)
   {
      rhsB[i] = static_cast<double>((rand() % 10000) + 1) / 100.0;               //fills the rhs vector with entries between 0.00 to 100.00
   }

   double solX[3];
   solveMatrixEqnCholesky<3>(matA, rhsB, solX);

   for (size_t i = 0; i < 3; i++)
   {
      double sum = 0.0;
      for (size_t j = 0; j < 3; j++)
      {
         sum += matA[i][j] * solX[j];
      }
      EXPECT_NEAR(rhsB[i], sum, eps);
   }

} 
*/