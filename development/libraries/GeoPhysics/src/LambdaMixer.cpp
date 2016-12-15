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

LambdaMixer::LambdaMixer (const std::vector<double> percentages, const std::vector<double> lambdaPc, const std::vector<double> lambdaKr):
   m_fraction1(0.0), m_fraction2(0.0), m_fraction3(0.0),
   m_lambdaPc1(0.0), m_lambdaPc2(0.0), m_lambdaPc3(0.0),
   m_lambdaKr1(0.0), m_lambdaKr2(0.0), m_lambdaKr3(0.0),
   m_numberOfComponents(1)
{
   // Put the compound-lithology values in member data
   m_fraction1  = percentages [0] / 100.0;
   if (m_fraction1 < 0.0) m_fraction1 = 0.0;

   m_lambdaPc1 = lambdaPc [0];
   if (m_lambdaPc1 < 0 or m_lambdaPc1 == DataAccess::Interface::DefaultUndefinedMapValue or m_lambdaPc1 == DataAccess::Interface::DefaultUndefinedScalarValue)
      m_lambdaPc1 = 1.0;
   m_lambdaKr1 = lambdaKr [0];
   if (m_lambdaKr1 < 0 or m_lambdaKr1 == DataAccess::Interface::DefaultUndefinedMapValue or m_lambdaKr1 == DataAccess::Interface::DefaultUndefinedScalarValue)
      m_lambdaKr1 = 1.0;

   if (percentages.size () == 2)
   {
      m_fraction2  = percentages [1] / 100.0;
      if (m_fraction2 < 0.0) m_fraction2 = 0.0;

      m_lambdaPc2 = lambdaPc [1];
      if (m_lambdaPc2 < 0 or m_lambdaPc2 == DataAccess::Interface::DefaultUndefinedMapValue or m_lambdaPc2 == DataAccess::Interface::DefaultUndefinedScalarValue)
         m_lambdaPc2 = 1.0;
      m_lambdaKr2 = lambdaKr [1];
      if (m_lambdaKr2 < 0 or m_lambdaKr2 == DataAccess::Interface::DefaultUndefinedMapValue or m_lambdaKr2 == DataAccess::Interface::DefaultUndefinedScalarValue)
         m_lambdaKr2 = 1.0;

      m_numberOfComponents = 2;
   }
   if (percentages.size () == 3)
   {
      m_fraction2  = percentages [1] / 100.0;
      if (m_fraction2 < 0.0) m_fraction2 = 0.0;

      m_lambdaPc2 = lambdaPc [1];
      if (m_lambdaPc2 < 0 or m_lambdaPc2 == DataAccess::Interface::DefaultUndefinedMapValue or m_lambdaPc2 == DataAccess::Interface::DefaultUndefinedScalarValue)
         m_lambdaPc2 = 1.0;
      m_lambdaKr2 = lambdaKr [1];
      if (m_lambdaKr2 < 0 or m_lambdaKr2 == DataAccess::Interface::DefaultUndefinedMapValue or m_lambdaKr2 == DataAccess::Interface::DefaultUndefinedScalarValue)
         m_lambdaKr2 = 1.0;

      m_fraction3  = percentages [2] / 100.0;
      if (m_fraction3 < 0.0) m_fraction3 = 0.0;
      
      m_lambdaPc3 = lambdaPc [2];
      if (m_lambdaPc3 < 0 or m_lambdaPc3 == DataAccess::Interface::DefaultUndefinedMapValue or m_lambdaPc3 == DataAccess::Interface::DefaultUndefinedScalarValue)
         m_lambdaPc3 = 1.0;
      m_lambdaKr3 = lambdaKr [2];
      if (m_lambdaKr3 < 0 or m_lambdaKr3 == DataAccess::Interface::DefaultUndefinedMapValue or m_lambdaKr3 == DataAccess::Interface::DefaultUndefinedScalarValue)
         m_lambdaKr3 = 1.0;

      m_numberOfComponents = 3;
   }


   // Order the data according to percentages, from largest to smallest
   if ( m_fraction1 < m_fraction2 )
   {
      std::swap ( m_lambdaPc1, m_lambdaPc2 );
      std::swap ( m_lambdaKr1, m_lambdaKr2 );
      std::swap ( m_fraction1, m_fraction2 );
   }

   if ( m_fraction2 < m_fraction3 )
   {
      std::swap ( m_lambdaPc2, m_lambdaPc3 );
      std::swap ( m_lambdaKr2, m_lambdaKr3 );
      std::swap ( m_fraction2, m_fraction3 );
   }

   if ( m_fraction1 < m_fraction2 )
   {
      std::swap ( m_lambdaPc1, m_lambdaPc2 );
      std::swap ( m_lambdaKr1, m_lambdaKr2 );
      std::swap ( m_fraction1, m_fraction2 );
   }

}


void GeoPhysics::LambdaMixer::mixLambdas (double & lambdaPcMixed, double & lambdaKrMixed)
{
   // If only one component, just use those lambdas
   if (m_numberOfComponents == 1)
   {
      lambdaPcMixed = m_lambdaPc1;
      lambdaKrMixed = m_lambdaKr1;

      return;
   }

   // If two components, check if there is a dominant lithology
   if (m_numberOfComponents == 2)
   {
      // If there is a single dominant lithology use those lambdas
      if (m_fraction1 >= 0.6)
      {
         lambdaPcMixed = m_lambdaPc1;
         lambdaKrMixed = m_lambdaKr1;

         return;
      }
      // If lambdas of two lithologies need mixing, use harmonic mean
      else
      {
         double difference = m_fraction1 - m_fraction2;
         double weight1 = 0.5 * (1.0 + difference / m_maximumAllowedDifference);
         double weight2 = 1.0 - weight1;

         // If one of the lambdaPc's or lambdaKr's is zero, don't mix but set mean to zero.
         if (m_lambdaPc1 == 0.0 or m_lambdaPc2 == 0.0)
            lambdaPcMixed = 0.0;
         else
            lambdaPcMixed = 1.0 / (weight1 * 1.0/m_lambdaPc1 + weight2 * 1.0/m_lambdaPc2);

         if (m_lambdaKr1 == 0.0 or m_lambdaKr2 == 0.0)
            lambdaKrMixed = 0.0;
         else
            lambdaKrMixed = 1.0 / (weight1 * 1.0/m_lambdaKr1 + weight2 * 1.0/m_lambdaKr2);

         return;
      }
   }

   // Separate function for 3 components to reduce complexity
   if (m_numberOfComponents == 3)
   {
      dealWithThreeComponents (lambdaPcMixed, lambdaKrMixed);
      
      return;
   }

   throw formattingexception::GeneralException() << "Error in calculation of mixed lambdas";
}


void LambdaMixer::dealWithThreeComponents (double & lambdaPcMixed, double & lambdaKrMixed)
{
   // If there is a single dominant lithology use those lambdas
   if (m_fraction1 > 0.2 + m_fraction2)
   {
      lambdaPcMixed = m_lambdaPc1;
      lambdaKrMixed = m_lambdaKr1;

      return;
   }
   
   // So second lithology is important, but if the
   // third one isn't, just average the first two.
   if (m_fraction1 > 0.2 + m_fraction3)
   {
      double difference = m_fraction1 - m_fraction2;
      double weight1 = 0.5 * (1.0 + difference / m_maximumAllowedDifference);
      double weight2 = 1.0 - weight1;

      // If one of the lambdaPc's or lambdaKr's is zero, don't mix but set mean to zero.
      if (m_lambdaPc1 == 0.0 or m_lambdaPc2 == 0.0)
         lambdaPcMixed = 0.0;
      else
         lambdaPcMixed = 1.0 / (weight1 * 1.0/m_lambdaPc1 + weight2 * 1.0/m_lambdaPc2);
      
      if (m_lambdaKr1 == 0.0 or m_lambdaKr2 == 0.0)
         lambdaKrMixed = 0.0;
      else
         lambdaKrMixed = 1.0 / (weight1 * 1.0/m_lambdaKr1 + weight2 * 1.0/m_lambdaKr2);

      return;
   }

   // Third case is the most complicated one. All three components are important

   // First get the percentage differences
   double difference12 = m_fraction1 - m_fraction2;
   double difference13 = m_fraction1 - m_fraction3;

   // Calculate the corresponding weights. Not normalized in this case
   double weight1 = 0.5 * (1.0 + difference12 / m_maximumAllowedDifference);
   double weight2 = 0.5 * (1.0 - difference12 / m_maximumAllowedDifference);
   double weight3 = 0.5 * (1.0 - difference13 / m_maximumAllowedDifference);

   // Weighted harmonic average of all three gives the mixed lambdas. Unless there is a zero
   if (m_lambdaPc1 == 0.0 or m_lambdaPc2 == 0.0 or m_lambdaPc3 == 0.0)
      lambdaPcMixed = 0.0;
   else
   lambdaPcMixed = (weight1 + weight2 + weight3) / (weight1 / m_lambdaPc1  +  weight2 / m_lambdaPc2  +  weight3 / m_lambdaPc3);

   if (m_lambdaKr1 == 0.0 or m_lambdaKr2 == 0.0 or m_lambdaKr3 == 0.0)
      lambdaKrMixed = 0.0;
   else
   lambdaKrMixed = (weight1 + weight2 + weight3) / (weight1 / m_lambdaKr1  +  weight2 / m_lambdaKr2  +  weight3 / m_lambdaKr3);
}
