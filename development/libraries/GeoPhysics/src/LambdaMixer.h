//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _LAMBDA_MIXER_H
#define _LAMBDA_MIXER_H

//std library
#include <string>
#include <vector>

/// This class mixes lambdas of individual lithologies in order to produce a lambda for a compound lithology.
/// The two main elements of this approach are:
/// 1) Only mix lithologies when the difference of their percentages is not bigger than 20%
/// 2) Use weighted harmonic mean to mix the different lambdas
/// Furthermore, if a 'contributing' (i.e. difference < 20% with dominant lithology) lithology has zero for
/// any of the lambdas, the mixed lambda will also be zero.

/// FORMULAS
/// We'll assume vectors are ordered in terms of percentage (done in the constructor).
/// Weights are calculated as follows:
/// Define FA >= FB >= FC the fractions of individual lithologies. Also WA >= WB >= WC the corresponding weights
/// WA = 0.5 (1 + (FA-FB)/0.2), WB = 0.5 (1 - (FA-FB)/0.2), WC = 0.5 (1 - (FA-FC)/0.2)
/// Then lambda^-1 = (WA+WB+WC)^-1 (WA/lambdaA + WB/lambdaB + WC/lambdaC)


namespace GeoPhysics
{
   class LambdaMixer
   {
   public:
      /// Mixing lambdas of simple lithologies
      double mixLambdas (const std::vector<double> & percentages, const std::vector<double> & lambda);

   private:
      /// Enforce acceptable values for lambdas
      void sanitizeLambdas();

      /// The case with three components is treated separately to reduce complexity
      double dealWithThreeComponents();

      /// Percentages, lambdaPc's and lambdaKr's of simple lithologies
      double m_fraction1 = 0.0;
      double m_fraction2 = 0.0;
      double m_fraction3 = 0.0;
      double m_lambda1 = 0.0;
      double m_lambda2 = 0.0;
      double m_lambda3 = 0.0;

      /// Number of components in the mix
      int m_numberOfComponents = 1;
      
      /// Maximum difference for mixing: 20%
      const double m_maximumAllowedDifference = 0.2;
   };
}

#endif // _LAMBDA_MIXER_H
