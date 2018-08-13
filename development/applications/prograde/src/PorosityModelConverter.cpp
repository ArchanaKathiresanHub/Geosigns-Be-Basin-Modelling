//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "PorosityModelConverter.h"

//utilities
#include "LogHandler.h"

//cmbAPI
#include "LithologyManager.h"

using namespace mbapi;

std::string Prograde::PorosityModelConverter::upgradeDescription
(const std::string & lithoName, const std::string & originalDescription, const LithologyManager::PorosityModel porModel)
{
   std::string upgradedDescription = originalDescription;
   if (porModel == LithologyManager::PorSoilMechanics)
   {
      upgradedDescription += " (upgraded to double exponential model)";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << lithoName <<
         " description has been modified to reflect porosity model upgradation ";
   }

   return upgradedDescription;
}

LithologyManager::PorosityModel Prograde::PorosityModelConverter::upgradePorosityModel
(const std::string & lithoName, const LithologyManager::PorosityModel porModel)
{
   LithologyManager::PorosityModel myPorModel = porModel;
   if (myPorModel == LithologyManager::PorSoilMechanics)
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) <<
         "Deprecated Soil Mechanics porosity model detected for " << lithoName;
      myPorModel = LithologyManager::PorDoubleExponential;
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << lithoName <<
         " upgraded to double exponential porosity model ";
   }
   else
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) <<
         "Valid porosity model detected for " << lithoName;
   }
   return myPorModel;
}

std::vector<double> Prograde::PorosityModelConverter::upgradeModelParameters
(const std::string & lithoName, const mbapi::LithologyManager::PorosityModel porModel, const std::vector<double> & porModelPrms)
{
   std::vector<double> myPorModelPrms;
   if (porModel == LithologyManager::PorSoilMechanics)
   {
      std::vector<double> myCompacPrms(
         computeDblExpCoefficients(porModelPrms[LithologyManager::PorModelParameters::CompactionCoef],
            porModelPrms[LithologyManager::PorModelParameters::PhiSurf] / 100.0));
      myPorModelPrms.push_back( porModelPrms[LithologyManager::PorModelParameters::PhiSurf] );
      myPorModelPrms.push_back( myCompacPrms[0] * 100.0 );
      myPorModelPrms.push_back( 3.0 );
      myPorModelPrms.push_back( myCompacPrms[1] * 100.0 );
      myPorModelPrms.push_back( myCompacPrms[2] );
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) <<
         "Upgraded model parameters calculated and updated for " << lithoName;
   }
   else
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) <<
         "No upgradation of model parameters required for " << lithoName;
      return porModelPrms;
   }
   return myPorModelPrms;
}


//the function compute coefficients take the soil mechanics compaction coefficient and surface porosity as inputs and gives the double exponential
//compaction coefficients as output. It uses Levenberg-Marquardt algorithm to perform the error minimization.
std::vector<double> Prograde::PorosityModelConverter::computeDblExpCoefficients( const double myCompacCoef, const double myPhiSurf )
{
   std::vector<double> myCompacPrms{ 0.35, 0.17, 0.5 };                 //compaction parameters vector initialized with guess values for C1, C2 and R
   const double prmsMax[3] = { 0.5, 0.5, 1.0 };              //maximum permissible values for the coefficients

   const double myPhiMin = 0.03;                                                          //minimum porosity
   const double mySigmaMin = 0.1;                                                         //reference VES


   const double myPsiSurf = myPhiSurf / (1.0 - myPhiSurf);                                //surface/depositional void ratio
   const double myPsiMin = myPhiMin / (1.0 - myPhiMin);                                   //minimum void ratio
   const double mySigmaMax = mySigmaMin * exp((myPsiSurf - myPsiMin) / myCompacCoef);     //max VES ( min VES required to reach minimum porosity ) 

   const int numSamples = 101;                                                            //number of sampling points
   const int niter = 1000;                                                                //number of iterations
   const double myFactor = (mySigmaMax - mySigmaMin)/static_cast<double> (numSamples-1);  
   
   double phiSoilMech[numSamples];                                                        //value of porosity, using soil mechanics model, at sampling points
   for (size_t i = 0; i < numSamples; i++)
   {
      double mySigma = mySigmaMin + static_cast<double> (i) * myFactor;
      phiSoilMech[i] = calculatePorositySoilMech(myPsiSurf, myCompacCoef, mySigma, mySigmaMin);
   }

   double phiDblExp[numSamples];                                                          //value of porosity, using double exponential model, at sampling points
   double jacobian[numSamples][3];                                                        //jacobian matrix
   double matA[3][3];                                                                     //coefficient matrix
   double solX[3];                                                                        //solution vector
   double rhsB[3];                                                                        //rhs vector


   
   for (size_t iter = 0; iter < niter; iter++)
   {

      for (size_t i = 0; i < numSamples; i++)
      {
         double mySigma = mySigmaMin + static_cast<double> (i) * myFactor;
         phiDblExp[i] = calculatePorosityDblExponential(myPhiMin, myPhiSurf, myCompacPrms, mySigma);
         std::vector<double> myJacobian = calculateDblExpPorosityDerivatives(myPhiMin, myPhiSurf, myCompacPrms, mySigma);
         jacobian[i][0] = myJacobian[0];                                                                                 //d(PhiDbl)/d(C1), derivative of PhiDbl wrt C1
         jacobian[i][1] = myJacobian[1];                                                                                 //d(PhiDbl)/d(C2), derivative of PhiDbl wrt C2
         jacobian[i][2] = myJacobian[2];                                                                                 //d(PhiDbl)/d(R) derivative of PhiDbl wrt R
      }

      for (size_t i = 0; i < 3; i++)
      {
         for (size_t j = 0; j <= i; j++)
         {
            matA[i][j] = 0.0;
            for (size_t k = 0; k < numSamples; k++)
            {
               matA[i][j] += (jacobian[k][i] * jacobian[k][j]);                                                         //matA = tranpose(J) * J
            }
         }

         rhsB[i] = 0.0;
         for (size_t k = 0; k < numSamples; k++)
         {
            rhsB[i] += (jacobian[k][i] * (phiSoilMech[k] - phiDblExp[k]));                                              //rhsB = transpose(J) * (PhiSoilMech - PhiDblExp)
         }
      }

      solveMatrixEqnCholesky<3>(matA, rhsB, solX);

      double magnitude = sqrt(pow(solX[0],2) + pow(solX[1], 2) + pow(solX[2], 2));
      if (magnitude > 0.01)                                                                                 //limiting step size
      {
         for (size_t i = 0; i < 3; i++)
         {
            solX[i] = 0.01 * solX[i] / magnitude;
         }
      }

      for (size_t i = 0; i < 3; i++)
      {
         myCompacPrms[i] = std::min( std::max( ( myCompacPrms[i] + (solX[i] )), 0.0), prmsMax[i] );         //limiting coefficient values
      }
   }
   return myCompacPrms;
}


double Prograde::PorosityModelConverter::calculatePorositySoilMech(const double myPsiSurf, const double myCompacCoef, const double mySigma, const double mySigmaMin)
{
   double myPsi = myPsiSurf - myCompacCoef*log(mySigma / mySigmaMin);
   double myPhi = myPsi / (1.0 + myPsi);
   return myPhi;
}

double Prograde::PorosityModelConverter::calculatePorosityDblExponential(const double myPhiMin, const double myPhiSurf, std::vector<double> myCompacPrms, const double mySigma)
{
   double myPhi = myPhiMin + (myCompacPrms[2] * (myPhiSurf - myPhiMin)*exp(-myCompacPrms[0] * mySigma))           
      + ((1.0 - myCompacPrms[2]) * (myPhiSurf - myPhiMin)*exp(-myCompacPrms[1] * mySigma));
   return myPhi;
}

std::vector<double> Prograde::PorosityModelConverter::calculateDblExpPorosityDerivatives(const double myPhiMin, const double myPhiSurf, const std::vector<double> & myCompacPrms,
   const double mySigma)
{
   std::vector<double> myDerivative;
   myDerivative.push_back(-myCompacPrms[2] * (myPhiSurf - myPhiMin) * mySigma * exp(-myCompacPrms[0] * mySigma));            //d(PhiDbl)/d(C1), derivative of PhiDbl wrt C1
   myDerivative.push_back(-(1.0 - myCompacPrms[2]) * (myPhiSurf - myPhiMin) * mySigma * exp(-myCompacPrms[1] * mySigma));    //d(PhiDbl)/d(C2), derivative of PhiDbl wrt C2
   myDerivative.push_back((myPhiSurf - myPhiMin) * (exp(-myCompacPrms[0] * mySigma) - exp(-myCompacPrms[1] * mySigma)));     //d(PhiDbl)/d(R) derivative of PhiDbl wrt R
   return myDerivative;
}


