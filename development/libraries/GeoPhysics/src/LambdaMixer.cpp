//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "LambdaMixer.h"
#include "FormattingException.h"
#include "Interface/Interface.h"

using namespace GeoPhysics;

void LambdaMixer::sanitizeLambdas ()
{
   if (m_fraction1 < 0.0) m_fraction1 = 0.0;

   if (m_lambda1 < 0 or m_lambda1 == DataAccess::Interface::DefaultUndefinedMapValue or m_lambda1 == DataAccess::Interface::DefaultUndefinedScalarValue)
      m_lambda1 = 1.0;

   if (m_numberOfComponents == 2)
   {
      if (m_fraction2 < 0.0) m_fraction2 = 0.0;
      if (m_lambda2 < 0 or m_lambda2 == DataAccess::Interface::DefaultUndefinedMapValue or m_lambda2 == DataAccess::Interface::DefaultUndefinedScalarValue)
         m_lambda2 = 1.0;
   }
   else if (m_numberOfComponents == 3)
   {
      if (m_fraction2 < 0.0) m_fraction2 = 0.0;
      if (m_lambda2 < 0 or m_lambda2 == DataAccess::Interface::DefaultUndefinedMapValue or m_lambda2 == DataAccess::Interface::DefaultUndefinedScalarValue)
         m_lambda2 = 1.0;

      if (m_fraction3 < 0.0) m_fraction3 = 0.0;
      if (m_lambda3 < 0 or m_lambda3 == DataAccess::Interface::DefaultUndefinedMapValue or m_lambda3 == DataAccess::Interface::DefaultUndefinedScalarValue)
         m_lambda3 = 1.0;
   }

   // Order the data according to percentages, from largest to smallest
   if ( m_fraction1 < m_fraction2 )
   {
      std::swap ( m_lambda1, m_lambda2 );
      std::swap ( m_fraction1, m_fraction2 );
   }

   if ( m_fraction2 < m_fraction3 )
   {
      std::swap ( m_lambda2, m_lambda3 );
      std::swap ( m_fraction2, m_fraction3 );
   }

   if ( m_fraction1 < m_fraction2 )
   {
      std::swap ( m_lambda1, m_lambda2 );
      std::swap ( m_fraction1, m_fraction2 );
   }
}

double GeoPhysics::LambdaMixer::mixLambdas (const std::vector<double> & percentages, const std::vector<double> & lambda)
{
   int sizeP = percentages.size();
   int sizeL = lambda.size();

   if (sizeP != sizeL)
   {
      throw formattingexception::GeneralException() << "Error in calculation of mixed lambdas: size of percentage array (" << sizeP <<
         ")is not the same as the size of the lambda array (" << sizeL << ")" ;
   }

   m_numberOfComponents = sizeP;

   if (m_numberOfComponents == 1)
      return lambda[0];

   switch (sizeP)
   {
   case 3:
      m_fraction3 = percentages[2] / 100.0;
      m_lambda3 = lambda[2];
   case 2:
      m_fraction2 = percentages[1] / 100.0;
      m_lambda2 = lambda[1];
      m_fraction1 = percentages[0] / 100.0; // case 1 is covered above
      m_lambda1 = lambda[0];
      break;
   default:
      throw formattingexception::GeneralException() << "Error in calculation of mixed lambdas: size of lambda array needs to be between 1 and 3";
   }
   
   sanitizeLambdas();

   // If two components, check if there is a dominant lithology
   if (m_numberOfComponents == 2)
   {
      // If there is a single dominant lithology use those lambdas
      if (m_fraction1 >= (1.0 + m_maximumAllowedDifference) / 2.0)
      {
         return m_lambda1;
      }
      // If lambdas of two lithologies need mixing, use harmonic mean
      else
      {
         double difference = m_fraction1 - m_fraction2;
         double weight1 = 0.5 * (1.0 + difference / m_maximumAllowedDifference);
         double weight2 = 1.0 - weight1;

         double lambdaMixed = 0.0;

         // If one of the lambdaPc's or lambdaKr's is zero, don't mix but set mean to zero.
         if (m_lambda1 == 0.0 or m_lambda2 == 0.0)
            lambdaMixed = 0.0;
         else
            lambdaMixed = 1.0 / (weight1 * 1.0/m_lambda1 + weight2 * 1.0/m_lambda2);

         return lambdaMixed;
      }
   }

   // Separate function for 3 components to reduce complexity
   if (m_numberOfComponents == 3)
   {
      return dealWithThreeComponents();
   }

   throw formattingexception::GeneralException() << "Error in calculation of mixed lambdas";
}


double LambdaMixer::dealWithThreeComponents ()
{
   // If there is a single dominant lithology use those lambdas
   if (m_fraction1 > m_maximumAllowedDifference + m_fraction2)
   {
      return m_lambda1;
   }
   
   // So second lithology is important, but if the
   // third one isn't, just average the first two.
   if (m_fraction1 > m_maximumAllowedDifference + m_fraction3)
   {
      double difference = m_fraction1 - m_fraction2;
      double weight1 = 0.5 * (1.0 + difference / m_maximumAllowedDifference);
      double weight2 = 1.0 - weight1;

      double lambdaMixed = 0.0;

      // If one of the lambdaPc's or lambdaKr's is zero, don't mix but set mean to zero.
      if (m_lambda1 == 0.0 or m_lambda2 == 0.0)
         lambdaMixed = 0.0;
      else
         lambdaMixed = 1.0 / (weight1 * 1.0/m_lambda1 + weight2 * 1.0/m_lambda2);

      return lambdaMixed;
   }

   // Third case is the most complicated one. All three components are important

   // First get the percentage differences
   double difference12 = m_fraction1 - m_fraction2;
   double difference13 = m_fraction1 - m_fraction3;

   // Calculate the corresponding weights. Not normalized in this case
   double weight1 = 0.5 * (1.0 + difference12 / m_maximumAllowedDifference);
   double weight2 = 0.5 * (1.0 - difference12 / m_maximumAllowedDifference);
   double weight3 = 0.5 * (1.0 - difference13 / m_maximumAllowedDifference);

   double lambdaMixed = 0.0;

   // Weighted harmonic average of all three gives the mixed lambdas. Unless there is a zero
   if (m_lambda1 == 0.0 or m_lambda2 == 0.0 or m_lambda3 == 0.0)
      lambdaMixed = 0.0;
   else
   lambdaMixed = (weight1 + weight2 + weight3) / (weight1 / m_lambda1  +  weight2 / m_lambda2  +  weight3 / m_lambda3);

   return lambdaMixed;
}
