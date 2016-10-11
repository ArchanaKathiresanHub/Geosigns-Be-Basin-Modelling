//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "stdafx.h"

#include "capillarySealStrength.h"

#include <algorithm>
#include <math.h>
#include <limits>
#include <assert.h>

using std::numeric_limits;

namespace CBMGenerics {

namespace capillarySealStrength {

void capParameters( const vector<LithoProp>                      & liths
                  , const vector<double>                         & fracs
                  , CBMGenerics::capillarySealStrength::MixModel   mixModel
                  , double                                       & capC1
                  , double                                       & capC2
                  )
{
   assert( liths.size() == fracs.size() );

   capC1 = 0.0; capC2 = 0.0;
   double minDepoPerm = numeric_limits<double>::max();

   for ( size_t l = 0; l < liths.size(); ++l )
   {
      double lithDepoPerm = liths[l].permeability;
      double lithCapC1 = liths[l].capC1;
      double lithCapC2 = liths[l].capC2;

      if ( mixModel == Layered )
      {
         // Use the constants associated with the smallest depositional permeability:
         if ( fracs[l] > 0.0 && lithDepoPerm < minDepoPerm )
         {
            capC1 = lithCapC1;
            capC2 = lithCapC2;
            minDepoPerm = lithDepoPerm;
         }
      }
      else // Homogeneous
      {
         capC1 += fracs[l] * lithCapC1;
         capC2 += fracs[l] * lithCapC2;
      }
   }
}

/// Calculate the capillary entry pressure for an air mercury sediment interface. 
/// Mercury is the standard non-wetting agent commonly used in experiments when 
/// the capillary entry pressure of rocks is determined experimentally.  In this case 
/// the capillary entry pressure is estimated with use of the following formula:
///
///   capSealStrength       =  pow (10, -capC2) * pow( permeability, -capC1) [MPa]
///                  Air_Hg
///

double capSealStrength_Air_Hg( const double & capC1, const double & capC2, const double & permeability )
{
   double perm = std::max( permeability, 1e-9 );
   double capSealStrength = pow( perm, -capC1 );
   capSealStrength *= pow( 10.0, -capC2 );

   // Convert from MPa to Pa:
   capSealStrength *= 1e6;

   return capSealStrength;
}

/// Put it all together:
///
double capSealStrength_Air_Hg( const vector<LithoProp>                      & liths
                             , const vector<double>                         & fracs
                             , CBMGenerics::capillarySealStrength::MixModel   mixModel
                             , const double                                 & permeability
                             )
{
   double capC1, capC2;
   capParameters( liths, fracs, mixModel, capC1, capC2 );
   return capSealStrength_Air_Hg( capC1, capC2, permeability );
}

/// The interfacial tension between HC and water can be estimated using the Firoozabadi 
/// and Ramey relation:
///
///                                                 1.024                -1.25
/// capTension       = 111 (density    - density   )      (T_K / T_K    ) 
///           H2O_HC               H2O          HC                  c_HC
///
/// with:
///
/// capTension       : interfacial tension [mN/m]
/// density_H2O      : the density of water
/// density_HC       : the density of the hydro carbons:
/// T_K              : the temperature of the formation in [K]
/// T_c_HC_K         : the critical temperature of the hydrocarbons in [K].  It is best if T_c_HC is calculated 
///                    with Li's mixing rule (EosPack::pvtFlash::criticalTemperatureAccordingToLiMixingRule which 
///                    takes as input the componentWeights fractions).
/// capTension_H2O_HC: the resultant hydroncarbon water interfacial tension

double capTension_H2O_HC_FR( const double & density_H2O, const double & density_HC, const double & T_K, const double & T_c_HC_K )
{
   // In the rather strange case of the density of HC larger than the density of H2O, 
   // HC's should sink.  So we can make capTension_H2O_HC infinite:
   if (density_H2O <= density_HC) { return numeric_limits<double>::max(); }

   double capTension_H2O_HC = 111.0 * pow ( (density_H2O - density_HC ) / 1000.0, 1.024) *
                                      pow ( T_K / T_c_HC_K, -1.25 );
   return capTension_H2O_HC;
}

double capTension_H2O_HC(const double & density_H2O, const double & density_HC, const double & T_K,  const double& T_c_HC_K )
{
   static const int PolyDegree = 11;

   // Coefficients of degree PolyDegree, = 11, polynomial:
   //
   // P(x)=a_0 x^11 + a_1 x^10 + a_2 x^9 + ... + a_10 x + a_11
   ///
   static const double polyCoeffs [PolyDegree + 1] = { -1.573112526079411e+05,
                                                        1.054161566793549e+06,
                                                       -3.128305226343527e+06,
                                                        5.413466994146141e+06,
                                                       -6.053513195029191e+06,
                                                        4.580069092358146e+06,
                                                       -2.385790817957751e+06,
                                                        8.537003874356834e+05,
                                                       -2.055623747107917e+05,
                                                        3.188267261055797e+04,
                                                       -2.929063381534887e+03,
                                                        1.345836979783050e+02
                                                     };

   // In the rather strange case of the density of HC larger than the density of H2O, 
   // HC's should sink.  So we can make capTension_H2O_HC infinite:
   if ( density_H2O <= density_HC )
   {
      return numeric_limits<double>::max();
   }
   else
   {
      double capTen_H2O_HC;

      const double tempR = T_c_HC_K / T_K;
      const double densDiff  = 0.001 * ( density_H2O - density_HC );

      // First compute:
      //
      //    ift^0.25 / diff * ( T / T_c )^0.3125 = \sum_{i=0}^12 a_i * diff^i
      //
      // This is a degree 11 least-squares fit to the data sampled by Olivier.
      //
      capTen_H2O_HC = polyCoeffs [ 0 ];
      capTen_H2O_HC = capTen_H2O_HC * densDiff + polyCoeffs [  1 ];
      capTen_H2O_HC = capTen_H2O_HC * densDiff + polyCoeffs [  2 ];
      capTen_H2O_HC = capTen_H2O_HC * densDiff + polyCoeffs [  3 ];
      capTen_H2O_HC = capTen_H2O_HC * densDiff + polyCoeffs [  4 ];
      capTen_H2O_HC = capTen_H2O_HC * densDiff + polyCoeffs [  5 ];
      capTen_H2O_HC = capTen_H2O_HC * densDiff + polyCoeffs [  6 ];
      capTen_H2O_HC = capTen_H2O_HC * densDiff + polyCoeffs [  7 ];
      capTen_H2O_HC = capTen_H2O_HC * densDiff + polyCoeffs [  8 ];
      capTen_H2O_HC = capTen_H2O_HC * densDiff + polyCoeffs [  9 ];
      capTen_H2O_HC = capTen_H2O_HC * densDiff + polyCoeffs [ 10 ];
      capTen_H2O_HC = capTen_H2O_HC * densDiff + polyCoeffs [ 11 ];

      //
      // ift^0.25 / diff * ( T / T_c )^0.3125 = P (P from above calculation)
      //
      //  => ift = ( P * diff * ( T_c / T )^0.3125 ) ^4
      //         = ( P * diff )^4 * ( T_c / T )^1.25
      capTen_H2O_HC *= densDiff;

      // It makes sense to rearrange the ( T_c / T )^1.25 term. 
      // 
      //  tempR =  T_c / T
      // 
      //  tempR^1.25 = tempR * tempR^0.25 = tempR * sqrt ( sqrt ( tempR ))
      // 
      // Since, on most processors, there is a square-root function in hardware.
      // Also the sqrt, in software, is usually fewer flops to compute than an pow.
      capTen_H2O_HC = ( capTen_H2O_HC * capTen_H2O_HC ) * ( capTen_H2O_HC * capTen_H2O_HC ) * tempR * sqrt ( sqrt ( tempR ));

//       capTen_H2O_HC *= capTen_H2O_HC *= capTen_H2O_HC;

//    double capTen_H2O_HC = 111.0 *
//    capTen_H2O_HC = 111.0 *
//       pow ( (density_H2O - density_HC) / 1000.0, 1.024) *
//       pow ( T_K / T_c_HC_K, -1.25);

      return capTen_H2O_HC;

   }

}

/// The capillary pressure is given by the formula:
///
///                             2 capTension       cos( Theta      )
///                                         H2O_HC           H20_HC
///    capSealStrength       =  ------------------------------------
///                   H2O_HC                 R
///
/// with:
///
/// capTension_H2O_HC: the interfacial tension between water and HC
/// Theta_H2O_HC     : the wettability expressed as the contact angle of the HC-water interface against 
///                    the rock surface.
/// R                : radius of the pore throat.
///
/// Theta_H2O_HC is usually taken as 180 degrees. Unforfunately, there is a lot of uncertainty 
/// with respect to what value we must take for R.  
///
/// Fortunately, the capillary pressure of mercury with air is known, and this value is given by:
///
///                             2 capTension       cos( Theta      )
///                                         Air_Hg           Air_Hg
///    capSealStrength       =  -----------------------------------
///                   Air_Hg                 R
///
/// with:
///
/// capTension_Air_Hg: the interfacial tension between air and Mercury. 
/// Theta_Air_Hg     : the wettability expressed as the contact angle of the Hg-air interface against 
///                    the rock surface.
/// R                : radius of the pore throat.
///
/// The capTension_Air_Hg is known ( 480 [mN/m] ), as is Theta_Air_Hg (in laboratory conditions 
/// usually 140 degrees, for mercury is a standard nonwetting fluid).  So we can eliminate R from these 
/// formulas and obtain:
///
///
///                                                       capTension        cos( Theta      )
///                                                                 H2O_HC            H2O_HC
///     capSealStrength       =  capSealStrength        * -----------------------------------
///                    H2O_HC                   Air_Hg
///                                                       capTension        cos( Theta      )
///                                                                 Air_Hg            Air_Hg
///
///
///                                                   capTension        cos( 180 )
///                                                             H2O_HC     
///                       =  capSealStrength        * -----------------------------------
///                                         Air_Hg
///                                                   480               cos( 140 )
///
///
///                       =  capSealStrength        * captension       / 367.70133
///                                         Air_Hg              H2O_HC
///

double capSealStrength_H2O_HC( const double & capSealStrength_Air_Hg, const double & capTension_H2O_HC )
{
   if ( capTension_H2O_HC != numeric_limits<double>::max() )
   {
      // capSealStrength_Air_Hg is large so use this succession:
      double capSealStrength_H2O_HC = capSealStrength_Air_Hg;

      capSealStrength_H2O_HC /= 367.70133;
      capSealStrength_H2O_HC *= capTension_H2O_HC;

      return capSealStrength_H2O_HC;
   }
   return numeric_limits<double>::max();
}

/// Put it all together:
///
double capSealStrength_H2O_HC(const vector<LithoProp>& lithProps, const vector<double>& lithFracs, 
   MixModel mixModel, const double& permeability, const double& density_H2O, 
   const double& density_HC, const double& T_K, const double& T_c_HC_K)
{
   //account for the different permeability and lithological parameters 
   double capPres_Air_Hg = capSealStrength_Air_Hg(lithProps, lithFracs, mixModel, permeability);
   double capTens_H2O_HC = capTension_H2O_HC(density_H2O, density_HC, T_K, T_c_HC_K);
   return capSealStrength_H2O_HC(capPres_Air_Hg, capTens_H2O_HC);
}


double capPressure( const unsigned int phaseId, const double& density_H2O, 
                    const double& density_HC, const double& T_K, const double& T_c_HC_K,
                    const double& specificSurfArea, const double& geometricVariance,
                    const double& wettingSaturation, const double& porosity,  const double& solidRockDensity ) {
  
   // if capTension_H2O_HC return infinite interfacial tension - set it to 0.001 
   double capTens_H2O_HC = 0.001;
   if( density_H2O > density_HC )
   { 
      capTens_H2O_HC = capTension_H2O_HC(density_H2O, density_HC, T_K, T_c_HC_K);
   }
   return capPressure( phaseId, capTens_H2O_HC, specificSurfArea, 
                       geometricVariance, wettingSaturation,  porosity,  solidRockDensity );
}

double capPressure( const unsigned int /* phaseId */,     const double & capTens_H2O_HC,
                    const double     & specificSurfArea,  const double & geometricVariance,
                    const double     & wettingSaturation, const double & porosity,  const double & solidRockDensity )
{
   return computeCapillaryPressureData( specificSurfArea, solidRockDensity, geometricVariance,
                                        porosity, wettingSaturation ) * capTens_H2O_HC * 0.001;
}

double computeCapillaryPressureData( const double & specificSurfArea,  const double & solidRockDensity,
                                     const double & geometricVariance, const double & porosity, 
                                     const double & wettingSaturation)
{
   const double porosityThreshold = 1e-12;

   double capPressure;

   double porosityUsed = porosity;

   if (porosityUsed < porosityThreshold)
      porosityUsed = porosityThreshold;

#if 0
   if (phaseId == 1 /* OIL */ )
      capPressure = 0.866;
   else             /* GAS */
      capPressure = 1.0;
#endif
   // cos( theta ) = cos(180) = -1
   capPressure = 1.0;

   // capPressure [Pa], int.tension [N/m], specificSurfArea [m2/kg],
   // rockDensity [kg/m3],  porosity [-]

   if( porosityUsed >= 0.1 ) 
   {
       capPressure *= specificSurfArea * solidRockDensity *
         exp (-wettingSaturation * 2.0 * sqrt (geometricVariance)) * (1.0 - porosityUsed) / porosityUsed;
   } else {
      capPressure *= specificSurfArea * solidRockDensity *
         exp (-wettingSaturation * 2.0 * sqrt (geometricVariance)) * 0.1 * (1.0 - porosityUsed) / ( porosityUsed * porosityUsed );
   }

   return capPressure;
}

double computeBrooksCoreyCorrection( const double Sw, const double lambda )
{
   //Now these values are hard coded: it might change if required   
   const double Adjustment = 1.0e-4;
   const double Sir = 0.1;
   double Sr;

   if ( Sw <= Sir + Adjustment ) {
      Sr = Adjustment / ( 1.0 - Sir );
   }
   else if ( Sw == 1.0 ) {
      Sr = 1;
   }
   else {
      Sr = ( Sw - Sir ) / ( 1.0 - Sir );
   }

   assert( 0 <= Sr && Sr <= 1 );

   return pow( Sr, -lambda );
}
 
} // namespace capillarySealStrength

} // namespace CBMGenerics
