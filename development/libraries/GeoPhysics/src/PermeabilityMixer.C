//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "PermeabilityMixer.h"

// std library
#include <cmath>
#include <iostream>
using namespace std;

// utilities library
#include "FormattingException.h"
#include "ConstantsMathematics.h"
using Utilities::Maths::MilliDarcyToM2;

GeoPhysics::PermeabilityMixer::PermeabilityMixer () :
   m_layeringIndex ( -1.0 ),
   m_mixModel ( DataAccess::Interface::UNDEFINED ),
   m_legacyBehaviour ( true ),
   m_isFault ( false )
{
}

void GeoPhysics::PermeabilityMixer::reset ( const std::vector<double>&                percentages,
                                            const std::vector<double>&                anisotropies,
                                            const bool                                legacyBehaviour,
                                            const double                              layeringIndex,
                                            const DataAccess::Interface::MixModelType mixModel,
                                            const bool                                isFault ) {

   m_weights = percentages;

   // if ( m_weights.size () == 2 and std::abs ( m_weights [ 0 ] + m_weights [ 1 ] - 1.0 ) > tolerance ) {
   //    throw fastCauldronException () << "Incorrectly defined lithology weights w1 + w2 != 1."
   //                                   << " w1 = " << m_weights [ 0 ]
   //                                   << " w2 = " << m_weights [ 1 ];
   // } else if ( m_weights.size () == 3 and std::abs ( m_weights [ 0 ] + m_weights [ 1 ] + m_weights [ 2 ] - 1.0 ) > tolerance ) {
   //    throw fastCauldronException () << "Incorrectly defined lithology weights w1 + w2 + w3 != 1."
   //                                   << " w1 = " << m_weights [ 0 ]
   //                                   << " w2 = " << m_weights [ 1 ]
   //                                   << " w3 = " << m_weights [ 2 ];
   // }


   m_legacyBehaviour = false;//legacyBehaviour;

   // Now scale by 0.01 so that the weights are fraction not percentage.
   std::transform ( m_weights.begin (), m_weights.end (), m_weights.begin (), [](const double x){ return x * 0.01; });

   m_anisotropies = anisotropies;
   m_mixModel = mixModel;
   m_isFault = isFault;

   // Find named constant for -9999.0
   if ( m_mixModel == DataAccess::Interface::LAYERED and not m_legacyBehaviour and layeringIndex != -9999.0 ) {
      m_layeringIndex = layeringIndex;
      m_mixHorizonExp = ( 1.0 + 2.0 * layeringIndex ) / 3.0;
      m_mixVerticalExp = ( 1.0 - 4.0 * layeringIndex ) / 3.0;

      m_inverseMixHorizonExp = 1.0 / m_mixHorizonExp;
      m_inverseMixVerticalExp = 1.0 / m_mixVerticalExp;

      if ( m_weights.size () >= 2 ) {
         m_percentRatio2 = m_weights [ 1 ] / m_weights [ 0 ];
         m_anisoRatioExp2 = std::pow ( m_anisotropies [ 1 ] / m_anisotropies [ 0 ], m_mixHorizonExp );
      } else {
         m_percentRatio2 = 0.0;
         m_anisoRatioExp2 = 0.0;
      }

      if ( m_weights.size () >= 3 ) {
         m_percentRatio3 = m_weights [ 2 ] / m_weights [ 0 ];
         m_anisoRatioExp3 = std::pow ( m_anisotropies [ 2 ] / m_anisotropies [ 0 ], m_mixHorizonExp );
      } else {
         m_percentRatio3 = 0.0;
         m_anisoRatioExp3 = 0.0;
      }

      m_percentPowerPlane  = std::pow ( m_weights [ 0 ], m_inverseMixHorizonExp );
      m_percentPowerNormal = std::pow ( m_weights [ 0 ], m_inverseMixVerticalExp );
   } else {
      m_mixHorizonExp = 0.0;
      m_inverseMixHorizonExp = 0.0;
      m_mixVerticalExp = 0.0;
      m_inverseMixVerticalExp = 0.0;
      m_percentRatio2 = 0.0;
      m_percentPowerPlane = 0.0;
      m_percentPowerNormal = 0.0;
      m_percentRatio3 = 0.0;
      m_anisoRatioExp2 = 0.0;
      m_anisoRatioExp3 = 0.0;
   }

}


void GeoPhysics::PermeabilityMixer::mixPermeability ( const FixedSizeArray& simplePermeabilityValues,
                                                      double&               permeabilityNormal,
                                                      double&               permeabilityPlane ) const {

   int numLitho = m_weights.size();

   if ( m_mixModel == DataAccess::Interface::HOMOGENEOUS or m_isFault ) {
      // Homogeneous model is kept for backward compatibility only.
      // Should not be modified

      // Average both vertical and plane permeabilities using weighted geometric mean
      permeabilityNormal = std::pow(simplePermeabilityValues[0], m_weights [0]);
      permeabilityPlane = std::pow(simplePermeabilityValues[0] * m_anisotropies [0], m_weights [0]);

      permeabilityNormal *= std::pow(simplePermeabilityValues[1], m_weights [1]);
      permeabilityPlane *= std::pow(simplePermeabilityValues[1] * m_anisotropies [1], m_weights [1]);

      if (numLitho == 3)
      {
         permeabilityNormal *= std::pow(simplePermeabilityValues[2], m_weights [2]);
         permeabilityPlane *= std::pow(simplePermeabilityValues[2] * m_anisotropies [2], m_weights [2]);
      }

   } else if ( m_mixModel == DataAccess::Interface::LAYERED and m_layeringIndex != 0.25 ) {
      // If layering index = 0.25, particular case because vertical exponent is 0
      // Average both vertical and plane permeabilities using generalized weigthed mean with vertical and horizontal exponents
      //
      // K = ( 1/(sum( a_i ) ) * (sum (a_i K_i^p ))^(1/p)               if p!=0
      //      where K is the permability (or permeability*anisotropy if horizontal permeability)
      //            i from 1 to 3,
      //            p depends on layering index,    p_h = (1+2*LI)/3,
      //                                            p_v = 1-2*p_h
      // Equation can be optimized to use less std::pow functions
      //             a_2       K_2*A_2           a_3       K_3*A_3            1           1
      // K_h = [ 1 + ---- * ( --------- )^p_h +  ---- * ( --------- )^p_h ]^(---) * a_1^(---) * (K_1*A_1)
      //             a_1       K_1*A_1           a_1       K_1*A_1           p_h         p_h
      //
      //             a_2   K_2      K_2             a_3   K_3      K_3               1           1
      // K_v = [ 1 + --- * --- * (( --- )^p_h)^-2 + --- * --- * (( --- )^p_h)^-2 ]^(---) * a_1^(---) * K_1
      //             a_1   K_1      K_1             a_1   K_1      K_1              p_v         p_v

      const double permRatio2 = simplePermeabilityValues[1] / simplePermeabilityValues[0];
      const double permRatioExp2 = std::pow(permRatio2, m_mixHorizonExp);
      double permRatio3;
      double permRatioExp3;

      permeabilityPlane = 1.0;
      permeabilityNormal = 1.0;

      permeabilityPlane += m_percentRatio2 * m_anisoRatioExp2 * permRatioExp2;
      permeabilityNormal += m_percentRatio2 * permRatio2 * 1 / (permRatioExp2*permRatioExp2);

      if (numLitho == 3)
      {
         permRatio3 = simplePermeabilityValues[2] / simplePermeabilityValues[0];
         permRatioExp3 = std::pow(permRatio3, m_mixHorizonExp);
         permeabilityPlane += m_percentRatio3 * m_anisoRatioExp3 * permRatioExp3;
         permeabilityNormal += m_percentRatio3 * permRatio3 * 1 / (permRatioExp3*permRatioExp3);
      }

      permeabilityPlane = std::pow(permeabilityPlane, 1 / m_mixHorizonExp);
      permeabilityPlane *= m_percentPowerPlane * simplePermeabilityValues[0] * m_anisotropies [0];

      permeabilityNormal = std::pow(permeabilityNormal, 1 / m_mixVerticalExp);
      permeabilityNormal *= m_percentPowerNormal * simplePermeabilityValues[0];
   } else if ( m_mixModel == DataAccess::Interface::LAYERED and m_layeringIndex == 0.25 ) {
      // Average vertical permeabilities using geometric mean (p_v = 0)
      // Average horizontal permeabilities using generalized mean (see above)

      permeabilityPlane = 1.0;
      permeabilityNormal = std::pow(simplePermeabilityValues[0], m_weights [0]);

      permeabilityPlane += m_percentRatio2 * m_anisoRatioExp2 * std::pow(simplePermeabilityValues[1] / simplePermeabilityValues[0], m_mixHorizonExp);
      permeabilityNormal *= std::pow(simplePermeabilityValues[1], m_weights [1]);

      if (numLitho == 3)
      {
         permeabilityPlane += m_percentRatio3 * m_anisoRatioExp3 * std::pow(simplePermeabilityValues[2] / simplePermeabilityValues[0], m_mixHorizonExp);
         permeabilityNormal *= std::pow(simplePermeabilityValues[2], m_weights [2]);
      }

      permeabilityPlane = std::pow(permeabilityPlane, 1 / m_mixHorizonExp);
      permeabilityPlane *= m_percentPowerPlane * simplePermeabilityValues[0] * m_anisotropies [0];

   } else { //Wrong mixing model
      permeabilityNormal = DataAccess::Interface::DefaultUndefinedMapValue;
      permeabilityPlane = DataAccess::Interface::DefaultUndefinedMapValue;

      throw formattingexception::GeneralException () << "Undefined mixing model type";
   }
}

void GeoPhysics::PermeabilityMixer::mixPermeabilityDerivatives ( const FixedSizeArray& permeabilities,
                                                                 const FixedSizeArray& derivativesWRTVes,
                                                                 double&               permeabilityDerivativeNormal,
                                                                 double&               permeabilityDerivativePlane) const {

   const int lithologiesNum = m_weights.size();

   std::vector<double> anisotropyPow(lithologiesNum);

   for (int lithoIdx = 0; lithoIdx < lithologiesNum; ++lithoIdx)
   {
      anisotropyPow[lithoIdx] = std::pow(m_anisotropies[lithoIdx], m_mixHorizonExp);
   }


   if (m_mixModel == DataAccess::Interface::LAYERED && m_layeringIndex != 0.25)
   {
      // If layering index = 0.25, particular case because vertical exponent is 0
      // Mixed permeability derivative with respect to ves in the PLANE direction (horizontal permeability derivative)
      //
      // d k_p                 p_p     p_p   (1-p_p)/p_p                    p_p            p_p-1   d k_i
      // ----- = [sum( w_i *a_i    *k_i    )]             *  sum( w_i * a_i    * p_p * k_i      * ----- )
      // d ves     i                                          i                                   d ves
      //
      // Mixed permeability derivative with respect to ves in the NORMAL direction (vertical permeability derivative)
      //
      // d k_n     1                   p_n   (1-p_n)/p_n                         p_n-1   d k_i
      // ----- = ---- * [sum( w_i * k_i    )]             *  sum( w_i * p_n * k_i      * ----- )
      // d ves    p_n     i                                   i                          d ves
      //
      //                                  1          (1-p_n)/p_n                      1             d k_i
      //       =  [sum( w_i * k_i -----------------)]             *  sum( w_i * ----------------- * ----- )
      //             i             k_i^p_p * k_i^p_p                      i      k_i^p_p * k_i^p_p   d ves
      //
      //
      // w_i : volume fraction
      // a_i : anisotropy

      double A(0.0);
      double B(0.0);
      double C(0.0);
      double D(0.0);
      for (int lithoIdx = 0; lithoIdx < lithologiesNum; ++lithoIdx)
      {
         double KPow = std::pow(permeabilities[lithoIdx], m_mixHorizonExp - 1);
         double sumPlane = m_weights[lithoIdx] * anisotropyPow[lithoIdx] * KPow;
         A += sumPlane * permeabilities[lithoIdx];
         B += sumPlane * derivativesWRTVes[lithoIdx];

         double sumNormal = m_weights[lithoIdx] * 1 / (KPow * KPow * permeabilities[lithoIdx]);
         C += sumNormal;
         D += sumNormal / permeabilities[lithoIdx] * derivativesWRTVes[lithoIdx];
      }

      permeabilityDerivativePlane = std::pow(A, (1 - m_mixHorizonExp) / m_mixHorizonExp) * B;
      permeabilityDerivativeNormal = std::pow(C, (1 - m_mixVerticalExp) / m_mixVerticalExp) * D;
   }
   else if (m_mixModel == DataAccess::Interface::LAYERED && m_layeringIndex == 0.25)
   {

      //
      // Mixed permeability derivative with respect to ves in the PLANE direction (horizontal permeability derivative)
      // Horizontal exponent = 0.5
      //
      // d k_p                 p_p     p_p   (1-p_p)/p_p                    p_p            p_p-1   d k_i
      // ----- = [sum( w_i *a_i    *k_i    )]             *  sum( w_i * a_i    * p_p * k_i      * ----- )
      // d ves     i                                          i                                   d ves
      //
      // Mixed permeability derivative with respect to ves in the NORMAL direction (vertical permeability derivative)
      // Vertical exponent = 0 => geometric mean
      //
      // d k_n                                  d k_i
      // ----- = sum( w_i * k_i ^ ( w_i - 1 ) * ----- * prod( k_j ^ w_j ) )
      // d ves    i                             d ves   j!=i
      //
      //
      // w_i : volume fraction
      // a_i : anisotropy

      double A(0.0);
      double B(0.0);

      permeabilityDerivativeNormal = 0.0;
      permeabilityDerivativePlane = 0.0;

      for (int lithoIdx = 0; lithoIdx < lithologiesNum; ++lithoIdx)
      {
         double KPow = std::pow(permeabilities[lithoIdx], m_mixHorizonExp - 1);
         double sumPlane = m_weights[lithoIdx] * anisotropyPow[lithoIdx] * KPow;
         A += sumPlane * permeabilities[lithoIdx];
         B += sumPlane * derivativesWRTVes[lithoIdx];

         double productsSequenceN(1.0);
         double productsSequenceP(1.0);
         for (int lithoIdx2 = 0; lithoIdx2 < lithologiesNum; ++lithoIdx2)
         {
            if (lithoIdx != lithoIdx2)
            {
               productsSequenceN *= std::pow(permeabilities[lithoIdx2], m_weights[lithoIdx2]);
               productsSequenceP *= std::pow(m_anisotropies[lithoIdx2] * permeabilities[lithoIdx2], m_weights[lithoIdx2]);

            }
         }
         permeabilityDerivativeNormal += m_weights[lithoIdx] * std::pow(permeabilities[lithoIdx], m_weights[lithoIdx] - 1.0) *
            derivativesWRTVes[lithoIdx] * productsSequenceN;
      }

      permeabilityDerivativePlane = std::pow(A, (1 - m_mixHorizonExp) / m_mixHorizonExp) * B;

   }
   else if (m_mixModel == DataAccess::Interface::HOMOGENEOUS)
   {
      //
      // Mixed permeability derivative with respect to ves in the NORMAL direction (vertical permeability derivative)
      //
      // d k_n                                  d k_i
      // ----- = sum( w_i * k_i ^ ( w_i - 1 ) * ----- * prod( k_j ^ w_j ) )
      // d ves    i                             d ves   j!=i
      //
      // Mixed permeability derivative with respect to ves in the PLANE direction (horizontal permeability derivative)
      //
      // d k_p                                                  d k_i
      // ----- = sum( w_i * a_i * ( a_i * k_i ) ^ ( w_i - 1 ) * ----- * prod( ( a_j * k_j ) ^ w_j ) )
      // d ves    i                                             d ves   j!=i
      //
      // w_i : volume fraction
      // a_i : anisotropy

      permeabilityDerivativeNormal = 0.0;
      permeabilityDerivativePlane = 0.0;

      for (int lithoIdx1 = 0; lithoIdx1 < lithologiesNum; ++lithoIdx1)
      {
         double productsSequenceN(1.0);
         double productsSequenceP(1.0);
         for (int lithoIdx2 = 0; lithoIdx2 < lithologiesNum; ++lithoIdx2)
         {
            if (lithoIdx1 != lithoIdx2)
            {
               productsSequenceN *= std::pow(permeabilities[lithoIdx2], m_weights[lithoIdx2]);
               productsSequenceP *= std::pow(m_anisotropies[lithoIdx2] * permeabilities[lithoIdx2], m_weights[lithoIdx2]);
            }
         }
         permeabilityDerivativeNormal += m_weights[lithoIdx1] * std::pow(permeabilities[lithoIdx1], m_weights[lithoIdx1] - 1.0) *
            derivativesWRTVes[lithoIdx1] * productsSequenceN;
         permeabilityDerivativePlane += m_weights[lithoIdx1] * m_anisotropies[lithoIdx1] *
            std::pow(m_anisotropies[lithoIdx1] * permeabilities[lithoIdx1], m_weights[lithoIdx1] - 1.0) *
            derivativesWRTVes[lithoIdx1] * productsSequenceP;
      }
   }
   else
   {
      permeabilityDerivativeNormal = DataAccess::Interface::DefaultUndefinedMapValue;
      permeabilityDerivativePlane  = DataAccess::Interface::DefaultUndefinedMapValue;
      throw formattingexception::GeneralException () << "Undefined mixing model type";
   }

   permeabilityDerivativeNormal *= MilliDarcyToM2;
   permeabilityDerivativePlane *= MilliDarcyToM2;

}


inline void GeoPhysics::PermeabilityMixer::mixTwoHomogeneous ( const unsigned int           size,
                                                               PermeabilityWorkSpaceArrays& simplePermeabilities,
                                                               ArrayDefs::Real_ptr          permeabilityNormal,
                                                               ArrayDefs::Real_ptr          permeabilityPlane ) const {

   // Homogeneous model is kept for backward compatibility only.

   double permeabilityAnisotropy1 = m_anisotropies [ 0 ];
   double permeabilityAnisotropy2 = m_anisotropies [ 1 ];

   double fractionLithology1 = m_weights [ 0 ];
   double fractionLithology2 = m_weights [ 1 ]; // = 1 - fractionLithology1

   // This can be done when the object is "reset"
   double mixedAnisotropy = std::pow ( permeabilityAnisotropy1 / permeabilityAnisotropy2, fractionLithology1 ) * permeabilityAnisotropy2;
   double mixedPermeability;

   ArrayDefs::Real_ptr permeabilities1 = simplePermeabilities.getData ( 0 );
   ArrayDefs::Real_ptr permeabilities2 = simplePermeabilities.getData ( 1 );

   // The permeability cannot be zero
#pragma omp simd aligned ( permeabilities1, permeabilities2, permeabilityNormal, permeabilityPlane )
   for ( unsigned int i = 0; i < size; ++i ) {
      mixedPermeability = MilliDarcyToM2 * std::pow ( permeabilities1 [ i ] / permeabilities2 [ i ], fractionLithology1 ) * permeabilities2 [ i ];
      permeabilityNormal [ i ] = mixedPermeability;
      permeabilityPlane [ i ] = mixedAnisotropy * mixedPermeability;
   }

}

inline void GeoPhysics::PermeabilityMixer::mixThreeHomogeneous ( const unsigned int           size,
                                                                 PermeabilityWorkSpaceArrays& simplePermeabilities,
                                                                 ArrayDefs::Real_ptr          permeabilityNormal,
                                                                 ArrayDefs::Real_ptr          permeabilityPlane ) const {

   double permeabilityAnisotropy1 = m_anisotropies [ 0 ];
   double permeabilityAnisotropy2 = m_anisotropies [ 1 ];
   double permeabilityAnisotropy3 = m_anisotropies [ 2 ];

   double fractionLithology1 = m_weights [ 0 ];
   double fractionLithology2 = m_weights [ 1 ];
   double fractionLithology3 = m_weights [ 2 ]; // = 1 - f1 - f2

   ArrayDefs::Real_ptr permeabilities1 = simplePermeabilities.getData ( 0 );
   ArrayDefs::Real_ptr permeabilities2 = simplePermeabilities.getData ( 1 );
   ArrayDefs::Real_ptr permeabilities3 = simplePermeabilities.getData ( 2 );

   double mixedAnisotropy = std::pow ( permeabilityAnisotropy1 / permeabilityAnisotropy3, fractionLithology1 ) *
                            std::pow ( permeabilityAnisotropy1 / permeabilityAnisotropy3, fractionLithology2 ) * permeabilityAnisotropy3;
   double mixedPermeability;

   // The permeability cannot be zero
   #pragma omp simd aligned ( permeabilities1, permeabilities2, permeabilities3, permeabilityNormal, permeabilityPlane )
   for ( unsigned int i = 0; i < size; ++i ) {
      double invPerm3 = 1.0 / permeabilities3 [ i ];
      mixedPermeability = MilliDarcyToM2 * ( std::pow ( permeabilities1 [ i ] * invPerm3, fractionLithology1 ) * std::pow ( permeabilities2 [ i ] * invPerm3, fractionLithology2 ) * permeabilities3 [ i ]);
      permeabilityNormal [ i ] = mixedPermeability;
      permeabilityPlane [ i ] = mixedAnisotropy * mixedPermeability;
   }

}

inline void GeoPhysics::PermeabilityMixer::mixTwoLayeringIndexZero ( const unsigned int           size,
                                                                     PermeabilityWorkSpaceArrays& simplePermeabilities,
                                                                     ArrayDefs::Real_ptr          permeabilityNormal,
                                                                     ArrayDefs::Real_ptr          permeabilityPlane ) const {

   double permeabilityAnisotropy1 = m_anisotropies [ 0 ];
   double permeabilityAnisotropy2 = m_anisotropies [ 1 ];

   double fractionLithology1 = m_weights [ 0 ];
   double fractionLithology2 = m_weights [ 1 ]; // = 1 - fractionLithology1

   ArrayDefs::Real_ptr permeabilities1 = simplePermeabilities.getData ( 0 );
   ArrayDefs::Real_ptr permeabilities2 = simplePermeabilities.getData ( 1 );

   double k21;
   double normal;
   double plane;

#pragma omp simd aligned ( permeabilities1, permeabilities2, permeabilityNormal, permeabilityPlane )
   for ( unsigned int i = 0; i < size; ++i ) {
      k21 = std::cbrt ( permeabilities2 [ i ] / permeabilities1 [ i ]);
      normal = 1.0 + m_percentRatio2 * k21;
      normal = normal * normal * normal * permeabilities1 [ i ] * m_percentPowerNormal * MilliDarcyToM2;

      plane = 1.0 + m_percentRatio2 * m_anisoRatioExp2 * k21;
      plane = plane * plane * plane * m_percentPowerPlane * permeabilityAnisotropy1 * permeabilities1 [ i ] * MilliDarcyToM2;
   }

}

inline void GeoPhysics::PermeabilityMixer::mixThreeLayeringIndexZero ( const unsigned int           size,
                                                                       PermeabilityWorkSpaceArrays& simplePermeabilities,
                                                                       ArrayDefs::Real_ptr          permeabilityNormal,
                                                                       ArrayDefs::Real_ptr          permeabilityPlane ) const {

   double permeabilityAnisotropy1 = m_anisotropies [ 0 ];
   double permeabilityAnisotropy2 = m_anisotropies [ 1 ];
   double permeabilityAnisotropy3 = m_anisotropies [ 2 ];

   double fractionLithology1 = m_weights [ 0 ];
   double fractionLithology2 = m_weights [ 1 ];
   double fractionLithology3 = m_weights [ 2 ]; // = 1 - f1 - f2

   ArrayDefs::Real_ptr permeabilities1 = simplePermeabilities.getData ( 0 );
   ArrayDefs::Real_ptr permeabilities2 = simplePermeabilities.getData ( 1 );
   ArrayDefs::Real_ptr permeabilities3 = simplePermeabilities.getData ( 2 );

   double k21;
   double k31;
   double normal;
   double plane;
   double k1Inv;

#pragma omp simd aligned ( permeabilities1, permeabilities2, permeabilities3, permeabilityNormal, permeabilityPlane )
   for ( unsigned int i = 0; i < size; ++i ) {
      k1Inv = permeabilities1 [ i ];
      k21 = std::cbrt ( permeabilities2 [ i ] / k1Inv );
      k31 = std::cbrt ( permeabilities3 [ i ] / k1Inv );
      normal = 1.0 + m_percentRatio2 * k21 + m_percentRatio3 * k31;
      permeabilityNormal [ i ] = normal * normal * normal * permeabilities1 [ i ] * m_percentPowerNormal * MilliDarcyToM2;

      plane = 1.0 + m_percentRatio2 * m_anisoRatioExp2 * k21 + m_percentRatio3 * m_anisoRatioExp3 * k31;
      permeabilityPlane [ i ] = plane * plane * plane * m_percentPowerPlane * permeabilityAnisotropy1 * permeabilities1 [ i ] * MilliDarcyToM2;
   }

}

inline void GeoPhysics::PermeabilityMixer::mixTwoLayeringIndexOne ( const unsigned int           size,
                                                                    PermeabilityWorkSpaceArrays& simplePermeabilities,
                                                                    ArrayDefs::Real_ptr          permeabilityNormal,
                                                                    ArrayDefs::Real_ptr          permeabilityPlane ) const {

   double permeabilityAnisotropy1 = m_anisotropies [ 0 ];
   double permeabilityAnisotropy2 = m_anisotropies [ 1 ];

   double fractionLithology1 = m_weights [ 0 ];
   double fractionLithology2 = m_weights [ 1 ]; // = 1 - fractionLithology1

   ArrayDefs::Real_ptr permeabilities1 = simplePermeabilities.getData ( 0 );
   ArrayDefs::Real_ptr permeabilities2 = simplePermeabilities.getData ( 1 );

   double perm1Inv;
   double permRatio21;
   double permRatio21Exp;

#pragma omp simd aligned ( permeabilities1, permeabilities2, permeabilityNormal, permeabilityPlane )
   for ( unsigned int i = 0; i < size; ++i ) {
      perm1Inv = 1.0 / permeabilities1 [ i ];
      permRatio21 = permeabilities2 [ i ] * perm1Inv;

      permRatio21Exp = permRatio21;

      permeabilityNormal [ i ] = 1.0 / ( 1.0 + m_percentRatio2 * permRatio21 / ( permRatio21Exp * permRatio21Exp ));
      permeabilityNormal [ i ] *= m_percentPowerNormal * permeabilities1 [ i ] * MilliDarcyToM2;

      permeabilityPlane  [ i ] = 1.0 + m_percentRatio2 * m_anisoRatioExp2 * permRatio21Exp;
      permeabilityPlane  [ i ] *= m_percentPowerPlane * permeabilityAnisotropy1 * permeabilities1 [ i ] * MilliDarcyToM2;
   }

}

inline void GeoPhysics::PermeabilityMixer::mixThreeLayeringIndexOne ( const unsigned int           size,
                                                                      PermeabilityWorkSpaceArrays& simplePermeabilities,
                                                                      ArrayDefs::Real_ptr          permeabilityNormal,
                                                                      ArrayDefs::Real_ptr          permeabilityPlane ) const {

   double permeabilityAnisotropy1 = m_anisotropies [ 0 ];
   double permeabilityAnisotropy2 = m_anisotropies [ 1 ];
   double permeabilityAnisotropy3 = m_anisotropies [ 2 ];

   double fractionLithology1 = m_weights [ 0 ];
   double fractionLithology2 = m_weights [ 1 ];
   double fractionLithology3 = m_weights [ 2 ]; // = 1 - f1 - f2

   ArrayDefs::Real_ptr permeabilities1 = simplePermeabilities.getData ( 0 );
   ArrayDefs::Real_ptr permeabilities2 = simplePermeabilities.getData ( 1 );
   ArrayDefs::Real_ptr permeabilities3 = simplePermeabilities.getData ( 2 );

   double perm1Inv;
   double permRatio21;
   double permRatio31;
   double permRatio21Exp;
   double permRatio31Exp;

#pragma omp simd aligned ( permeabilities1, permeabilities2, permeabilities3, permeabilityNormal, permeabilityPlane )
   for ( unsigned int i = 0; i < size; ++i ) {
      perm1Inv = 1.0 / permeabilities1 [ i ];
      permRatio21 = permeabilities2 [ i ] * perm1Inv;
      permRatio31 = permeabilities3 [ i ] * perm1Inv;

      permRatio21Exp = permRatio21;
      permRatio31Exp = permRatio31;

      permeabilityNormal [ i ] = 1.0 / ( 1.0 + m_percentRatio2 * permRatio21 / ( permRatio21Exp * permRatio21Exp ) + m_percentRatio3 * permRatio31 / (permRatio31Exp * permRatio31Exp ));
      permeabilityNormal [ i ] *= m_percentPowerNormal * permeabilities1 [ i ] * MilliDarcyToM2;

      permeabilityPlane  [ i ] = 1.0 + m_percentRatio2 * m_anisoRatioExp2 * permRatio21Exp + m_percentRatio3 * m_anisoRatioExp3 * permRatio31Exp;
      permeabilityPlane  [ i ] *= m_percentPowerPlane * permeabilityAnisotropy1 * permeabilities1 [ i ] * MilliDarcyToM2;
   }

}

inline void GeoPhysics::PermeabilityMixer::mixTwoLayeringIndexQuarter ( const unsigned int           size,
                                                                        PermeabilityWorkSpaceArrays& simplePermeabilities,
                                                                        ArrayDefs::Real_ptr          permeabilityNormal,
                                                                        ArrayDefs::Real_ptr          permeabilityPlane ) const {

   double permeabilityAnisotropy1 = m_anisotropies [ 0 ];
   double permeabilityAnisotropy2 = m_anisotropies [ 1 ];

   double fractionLithology1 = m_weights [ 0 ];
   double fractionLithology2 = m_weights [ 1 ]; // = 1 - fractionLithology1

   ArrayDefs::Real_ptr permeabilities1 = simplePermeabilities.getData ( 0 );
   ArrayDefs::Real_ptr permeabilities2 = simplePermeabilities.getData ( 1 );

   double perm;
   double permRatio;

#pragma omp simd aligned ( permeabilities1, permeabilities2, permeabilityNormal, permeabilityPlane )
   for ( unsigned int i = 0; i < size; ++i ) {
      permRatio = permeabilities2 [ i ] / permeabilities1 [ i ];
      permeabilityNormal [ i ] = MilliDarcyToM2 * std::pow ( permRatio, fractionLithology2 ) * permeabilities1 [ i ];
      perm = 1.0 + m_percentRatio2 * m_anisoRatioExp2 * std::sqrt ( permRatio );
      // m_percentPowerPlane := fraction1 * fraction1
      permeabilityPlane [ i ] = MilliDarcyToM2 * perm * perm * permeabilities1 [ i ] * permeabilityAnisotropy1 * m_percentPowerPlane;
   }

}

inline void GeoPhysics::PermeabilityMixer::mixThreeLayeringIndexQuarter ( const unsigned int           size,
                                                                          PermeabilityWorkSpaceArrays& simplePermeabilities,
                                                                          ArrayDefs::Real_ptr          permeabilityNormal,
                                                                          ArrayDefs::Real_ptr          permeabilityPlane ) const {

   double permeabilityAnisotropy1 = m_anisotropies [ 0 ];
   double permeabilityAnisotropy2 = m_anisotropies [ 1 ];
   double permeabilityAnisotropy3 = m_anisotropies [ 2 ];

   double fractionLithology1 = m_weights [ 0 ];
   double fractionLithology2 = m_weights [ 1 ];
   double fractionLithology3 = m_weights [ 2 ]; // = 1 - f1 - f2

   ArrayDefs::Real_ptr permeabilities1 = simplePermeabilities.getData ( 0 );
   ArrayDefs::Real_ptr permeabilities2 = simplePermeabilities.getData ( 1 );
   ArrayDefs::Real_ptr permeabilities3 = simplePermeabilities.getData ( 2 );

   double perm;
   double permRatio21;
   double permRatio31;

#pragma omp simd aligned ( permeabilities1, permeabilities2, permeabilities3, permeabilityNormal, permeabilityPlane )
   for ( unsigned int i = 0; i < size; ++i ) {
      permRatio21 = permeabilities2 [ i ] / permeabilities1 [ i ];
      permRatio31 = permeabilities3 [ i ] / permeabilities1 [ i ];

      permeabilityNormal [ i ] = MilliDarcyToM2 * std::pow ( permRatio21, fractionLithology2 ) * std::pow ( permRatio31, fractionLithology3 ) * permeabilities1 [ i ];

      perm = 1.0 + m_percentRatio2 * m_anisoRatioExp2 * std::sqrt ( permRatio21 ) + m_percentRatio3 * m_anisoRatioExp3 * std::sqrt ( permRatio31 );
      permeabilityPlane [ i ] = MilliDarcyToM2 * perm * perm * permeabilities1 [ i ] * permeabilityAnisotropy1 * m_percentPowerPlane;
   }

}


inline void GeoPhysics::PermeabilityMixer::mixTwoLayeringIndexGeneral ( const unsigned int           size,
                                                                        PermeabilityWorkSpaceArrays& simplePermeabilities,
                                                                        ArrayDefs::Real_ptr          permeabilityNormal,
                                                                        ArrayDefs::Real_ptr          permeabilityPlane ) const {

   // Homogeneous model is kept for backward compatibility only.
   double permeabilityAnisotropy1 = m_anisotropies [ 0 ];
   double permeabilityAnisotropy2 = m_anisotropies [ 1 ];

   double fractionLithology1 = m_weights [ 0 ];
   double fractionLithology2 = m_weights [ 1 ]; // = 1 - fractionLithology1

   ArrayDefs::Real_ptr permeabilities1 = simplePermeabilities.getData ( 0 );
   ArrayDefs::Real_ptr permeabilities2 = simplePermeabilities.getData ( 1 );

   double permRatio;
   double permRatioExp;

#pragma omp simd aligned ( permeabilities1, permeabilities2, permeabilityNormal, permeabilityPlane )
   for ( unsigned int i = 0; i < size; ++i ) {
      permRatio = permeabilities2 [ i ] / permeabilities1 [ i ];
      permRatioExp = std::pow(permRatio, m_mixHorizonExp);

      permeabilityNormal [ i ] = std::pow ( 1.0 + m_percentRatio2 * permRatio / (permRatioExp * permRatioExp ), m_inverseMixVerticalExp );
      permeabilityNormal [ i ] *= m_percentPowerNormal * permeabilities1 [ i ] * MilliDarcyToM2;

      permeabilityPlane  [ i ] = std::pow( 1.0 + m_percentRatio2 * m_anisoRatioExp2 * permRatioExp,  m_inverseMixHorizonExp );
      permeabilityPlane  [ i ] *= m_percentPowerPlane * permeabilityAnisotropy1 * permeabilities1 [ i ] * MilliDarcyToM2;
   }

}

inline void GeoPhysics::PermeabilityMixer::mixThreeLayeringIndexGeneral ( const unsigned int           size,
                                                                          PermeabilityWorkSpaceArrays& simplePermeabilities,
                                                                          ArrayDefs::Real_ptr          permeabilityNormal,
                                                                          ArrayDefs::Real_ptr          permeabilityPlane ) const {

   double permeabilityAnisotropy1 = m_anisotropies [ 0 ];
   double permeabilityAnisotropy2 = m_anisotropies [ 1 ];
   double permeabilityAnisotropy3 = m_anisotropies [ 2 ];

   double fractionLithology1 = m_weights [ 0 ];
   double fractionLithology2 = m_weights [ 1 ];
   double fractionLithology3 = m_weights [ 2 ]; // = 1 - f1 - f2

   ArrayDefs::Real_ptr permeabilities1 = simplePermeabilities.getData ( 0 );
   ArrayDefs::Real_ptr permeabilities2 = simplePermeabilities.getData ( 1 );
   ArrayDefs::Real_ptr permeabilities3 = simplePermeabilities.getData ( 2 );

   double perm1Inv;
   double permRatio21;
   double permRatio31;
   double permRatio21Exp;
   double permRatio31Exp;

#pragma omp simd aligned ( permeabilities1, permeabilities2, permeabilities3, permeabilityNormal, permeabilityPlane )
   for ( unsigned int i = 0; i < size; ++i ) {
      perm1Inv = 1.0 / permeabilities1 [ i ];
      permRatio21 = permeabilities2 [ i ] * perm1Inv;
      permRatio31 = permeabilities3 [ i ] * perm1Inv;

      permRatio21Exp = std::pow ( permRatio21, m_mixHorizonExp );
      permRatio31Exp = std::pow ( permRatio31, m_mixHorizonExp );

      permeabilityNormal [ i ] = std::pow ( 1.0 + m_percentRatio2 * permRatio21 / (permRatio21Exp * permRatio21Exp ) + m_percentRatio3 * permRatio31 / (permRatio31Exp * permRatio31Exp ), m_inverseMixVerticalExp );
      permeabilityNormal [ i ] *= m_percentPowerNormal * permeabilities1 [ i ] * MilliDarcyToM2;

      permeabilityPlane  [ i ] = std::pow( 1.0 + m_percentRatio2 * m_anisoRatioExp2 * permRatio21Exp + m_percentRatio3 * m_anisoRatioExp3 * permRatio31Exp,  m_inverseMixHorizonExp );
      permeabilityPlane  [ i ] *= m_percentPowerPlane * permeabilityAnisotropy1 * permeabilities1 [ i ] * MilliDarcyToM2;

   }

}

void GeoPhysics::PermeabilityMixer::mixPermeabilityArrayOneLitho ( const unsigned int           size,
                                                                   PermeabilityWorkSpaceArrays& simplePermeabilities,
                                                                   ArrayDefs::Real_ptr          permeabilityNormal,
                                                                   ArrayDefs::Real_ptr          permeabilityPlane ) const {

   ArrayDefs::Real_ptr permeabilities1 = simplePermeabilities.getData ( 0 );
   double permeabilityAnisotropy = m_anisotropies [ 0 ];

   for ( unsigned int i = 0; i < size; ++i ) {
      permeabilityNormal [ i ] = permeabilities1 [ i ] * MilliDarcyToM2;
      permeabilityPlane [ i ] = permeabilityAnisotropy * permeabilities1 [ i ] * MilliDarcyToM2;
   }

}

void GeoPhysics::PermeabilityMixer::mixPermeabilityArrayTwoLithos ( const unsigned int           size,
                                                                    PermeabilityWorkSpaceArrays& simplePermeabilities,
                                                                    ArrayDefs::Real_ptr          permeabilityNormal,
                                                                    ArrayDefs::Real_ptr          permeabilityPlane ) const {

   if ( m_mixModel == DataAccess::Interface::HOMOGENEOUS or m_isFault ) {
      mixTwoHomogeneous ( size, simplePermeabilities, permeabilityNormal, permeabilityPlane );
   } else if ( m_mixModel == DataAccess::Interface::LAYERED ) {

      if ( m_layeringIndex == 0.0 ) {
         mixTwoLayeringIndexZero ( size, simplePermeabilities, permeabilityNormal, permeabilityPlane );
      } else if ( m_layeringIndex == 0.25 ) {
         mixTwoLayeringIndexQuarter ( size, simplePermeabilities, permeabilityNormal, permeabilityPlane );
      } else if ( m_layeringIndex == 1.0 ) {
         mixTwoLayeringIndexOne ( size, simplePermeabilities, permeabilityNormal, permeabilityPlane );
      } else {
         mixTwoLayeringIndexGeneral ( size, simplePermeabilities, permeabilityNormal, permeabilityPlane );
      }

   } else {
      throw formattingexception::GeneralException () << "Undefined mixing model type";
   }

}

void GeoPhysics::PermeabilityMixer::mixPermeabilityArrayThreeLithos ( const unsigned int           size,
                                                                      PermeabilityWorkSpaceArrays& simplePermeabilities,
                                                                      ArrayDefs::Real_ptr          permeabilityNormal,
                                                                      ArrayDefs::Real_ptr          permeabilityPlane ) const {

   if ( m_mixModel == DataAccess::Interface::HOMOGENEOUS or m_isFault ) {
      mixThreeHomogeneous ( size, simplePermeabilities, permeabilityNormal, permeabilityPlane );
   } else if ( m_mixModel == DataAccess::Interface::LAYERED ) {

      if ( m_layeringIndex == 0.0 ) {
         mixThreeLayeringIndexZero ( size, simplePermeabilities, permeabilityNormal, permeabilityPlane );
      } else if ( m_layeringIndex == 0.25 ) {
         mixThreeLayeringIndexQuarter ( size, simplePermeabilities, permeabilityNormal, permeabilityPlane );
      } else if ( m_layeringIndex == 1.0 ) {
         mixThreeLayeringIndexOne ( size, simplePermeabilities, permeabilityNormal, permeabilityPlane );
      } else {
         mixThreeLayeringIndexGeneral ( size, simplePermeabilities, permeabilityNormal, permeabilityPlane );
      }

   } else {
      throw formattingexception::GeneralException () << "Undefined mixing model type";
   }

}

void GeoPhysics::PermeabilityMixer::mixPermeabilityArray ( const unsigned int           size,
                                                           PermeabilityWorkSpaceArrays& simplePermeabilities,
                                                           ArrayDefs::Real_ptr          permeabilityNormal,
                                                           ArrayDefs::Real_ptr          permeabilityPlane ) const {

   if ( m_weights.size () == 1 ) {
      mixPermeabilityArrayOneLitho ( size, simplePermeabilities, permeabilityNormal, permeabilityPlane );
   } else if ( m_weights.size () == 2 ) {
      mixPermeabilityArrayTwoLithos ( size, simplePermeabilities, permeabilityNormal, permeabilityPlane );
   } else if ( m_weights.size () == 3 ) {
      mixPermeabilityArrayThreeLithos ( size, simplePermeabilities, permeabilityNormal, permeabilityPlane );
   } else {
      throw formattingexception::GeneralException () << "No mixing defined for a single lithology";
   }

}
