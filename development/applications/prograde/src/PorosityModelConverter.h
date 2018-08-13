//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef PROGRADE_POROSITY_MODEL_CONVERTER_H
#define PROGRADE_POROSITY_MODEL_CONVERTER_H

//cmbAPI
#include "LithologyManager.h"

namespace Prograde
{
   /// @class PorosityModelConverter converts porosity models
   class PorosityModelConverter {

   public:

      PorosityModelConverter() = default;
      PorosityModelConverter(const PorosityModelConverter &) = delete;
      PorosityModelConverter& operator=(const PorosityModelConverter &) = delete;
      ~PorosityModelConverter() = default;


      /// @brief Upgrades the deprecated porosity model
      /// @details The description is upgraded using upgradeDescription, porosity model is upgraded using upgradePorosityModel and the model parameters are upgraded using upgradeModelParameters
      std::string upgradeDescription(const std::string &, const std::string &, const mbapi::LithologyManager::PorosityModel);

      mbapi::LithologyManager::PorosityModel upgradePorosityModel(const std::string &, const mbapi::LithologyManager::PorosityModel);

      std::vector<double> upgradeModelParameters(const std::string &, const mbapi::LithologyManager::PorosityModel, const std::vector<double> &);

   protected:
      ///@details The function compute coefficients are used to compute the coefficients of double exponential model from the soil mechanics coefficient
      std::vector<double> computeDblExpCoefficients( const double, const double );

      ///@details The function calculates the porosity using Soil Mechanics model
      double calculatePorositySoilMech(const double, const double, const double, const double);

      ///@details The function calculates the porosity using double exponential model
      double calculatePorosityDblExponential(const double, const double, std::vector<double>, const double);

      ///@details The function calculates the derivatives of double exponential porosity with respect to model parameters C1, C2 and R
      std::vector<double> calculateDblExpPorosityDerivatives(const double, const double, const std::vector<double> &, const double);

      ///@details The function solves a matrix equation, with symmetric coefficient matrix, using Cholesky decomposition
      template<std::size_t N> void solveMatrixEqnCholesky(const double matA[N][N], const double rhsB[N], double solX[N])
      {
         double solY[N];

         for (size_t i = 0; i < N; i++)
         {
            solX[i] = 0.0;
            solY[i] = 0.0;

         }

         double matL[N][N];                                                                     //lower triangular matrix formed from cholesky factorization

         for (size_t i = 0; i < N; i++)
         {
            for (size_t j = 0; j < N; j++)
            {
               matL[i][j] = 0.0;
            }
         }

         for (size_t i = 0; i < N; i++)                                                          //performing cholesky decomposition
         {
            for (size_t j = 0; j <= i; j++)
            {
               double sum = 0.0;
               for (size_t k = 0; k < j; k++)
               {
                  sum += (matL[i][k] * matL[j][k]);
               }

               if (i == j)
               {
                  matL[i][j] = sqrt(std::max((matA[i][j] - sum), 0.0));
               }
               else
               {
                  matL[i][j] = (matA[i][j] - sum) / matL[j][j];
               }
            }
         }

         for (size_t i = 0; i < N; i++)
         {
            double sum = 0.0;
            for (size_t j = 0; j < i; j++)
            {
               sum += (matL[i][j] * solY[j]);
            }
            solY[i] = (rhsB[i] - sum) / matL[i][i];
         }

         for (int i = N - 1; i >= 0; i--)
         {
            double sum = 0.0;
            for (int j = N - 1; j > i; j--)
            {
               sum += (matL[j][i] * solX[j]);
            }
            solX[i] = (solY[i] - sum) / matL[i][i];
         }
      }
   };
}

#endif
