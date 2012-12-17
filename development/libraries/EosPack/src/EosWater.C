// Copyright 2011, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include "EosPvtDefinitions.h"
#include "EosPvtTable.h"
#include "EosUtils.h"
#include "EosWater.h"

#include <math.h>

// Construct the water data structure
//
// For a description of data see EosPvtModel.h
EosWater::EosWater( void )
{
   m_iWaterModel = EOS_WATER_CORRELATIONS;
   m_dConvPressure = 1.0;
   m_dConvTemperature = 1.0;
   m_dConvVolume = 1.0;
   m_dConvViscosity = 1.0;
   m_dConvDensity = 1.0;
   m_dConvHeat = 1.0;
   m_dWaterIft = 0.035;
   m_dWaterDens = 1000.0;
   m_dWaterDensDp = 0.0;
   m_dWaterDensDt = 0.0;
   m_dWaterDensDs = 0.0;
   m_dWaterDensDpDs = 0.0;
   m_dWaterDensDtDs = 0.0;
   m_dWaterVisc = 0.001;
   m_dWaterViscDp = 0.0;
   m_dWaterViscDt = 0.0;
   m_dWaterViscDs = 0.0;
   m_dWaterViscDpDs = 0.0;
   m_dWaterViscDtDs = 0.0;
   m_dWaterHeat = 4185.0;
   m_dWaterHeatDs = 0.0;
}


/*
// Copy constructor
*/
EosWater::EosWater( const EosWater &self )
{
   m_iWaterModel = self.m_iWaterModel;
   m_dConvPressure = self.m_dConvPressure;
   m_dConvTemperature = self.m_dConvTemperature;
   m_dConvVolume = self.m_dConvVolume;
   m_dConvViscosity = self.m_dConvViscosity;
   m_dConvDensity = self.m_dConvDensity;
   m_dConvHeat = self.m_dConvHeat;
   m_dWaterIft = self.m_dWaterIft;
   m_dWaterDens = self.m_dWaterDens;
   m_dWaterDensDp = self.m_dWaterDensDp;
   m_dWaterDensDt = self.m_dWaterDensDt;
   m_dWaterDensDs = self.m_dWaterDensDs;
   m_dWaterDensDpDs = self.m_dWaterDensDpDs;
   m_dWaterDensDtDs = self.m_dWaterDensDtDs;
   m_dWaterVisc = self.m_dWaterVisc;
   m_dWaterViscDp = self.m_dWaterViscDp;
   m_dWaterViscDt = self.m_dWaterViscDt;
   m_dWaterViscDs = self.m_dWaterViscDs;
   m_dWaterViscDpDs = self.m_dWaterViscDpDs;
   m_dWaterViscDtDs = self.m_dWaterViscDtDs;
   m_dWaterHeat = self.m_dWaterHeat;
   m_dWaterHeatDs = self.m_dWaterHeatDs;
}


/* 
// WaterDensity
//
// Function for the density of water 
//
// iM ** constant **
//    Number of objects
// iNc ** constant **
//    Number of components in aqueous phase
// iDrv ** constant **
//    Indicator for derivative level
//       EOS_NODRV
//          No derivatives
//       EOS_DRV_N
//          Derivatives wrt accumulations only
//       EOS_DRV_P
//          Derivatives wrt accumulations and pressure
//       EOS_DRV_T
//          Derivatives wrt accumulations, pressure, and temperature
// iVolume ** constant **
//    Indicator whether volumes or density required
//       EOS_DENSITY
//          Get density
//       EOS_VOLUME
//          Get volume
// pP ** constant **
//    Pressures of objects
// pT ** constant **
//    Temperatures of objects
// pDSda ** constant **
//    Accumulation derivative of salinity
//       first index corresponds to object
//       second index corresponds to component
// pRo 
//    The mass volume or density of each object
// pDRodp 
//    Pressure derivative of the mass volume or density of each object
// pDRodt 
//    Temperature derivative of the mass volume or density of each 
//    object
// pDRods 
//    Salinity derivative of the mass volume or density of each 
//    object
// pDRoda 
//    Accumulation derivative of mass volume or density of each object
//       first index corresponds to object
//       second index corresponds to component
//
// 1) If using the linear model, compute water density as
//
//    DENS = DENS0 * ( 1 + DENSDP * P ) * ( 1 + DENSDT * T )
//
// 2) Otherwise convert to density if required and then compute
//    accumulation derivatives
*/
void EosWater::WaterDensity( int iM, int iNc, int iDrv, int iVolume, double *pP, double *pT, double *pS, double *pDSda,
                             double *pRo, double *pDRodp, double *pDRodt, double *pDRods, double *pDRoda )
{
   double  dA;
   double  dB;
   double  dC;
   double  dD;
   double  dE;
   double  dF;
   double  dG;
   double  dH;
   double  dI;
   double  dJ;
   double  dK;
   double  dL;
   double  dM;
   double *pTa;
   double *pTb;
   int     iNi;
   int     i;
   int     iConvert;

   /* Linear model */
   if ( m_iWaterModel == EOS_WATER_LINEAR )
   {
      dA = m_dWaterDens;
      dB = m_dWaterDensDp;
      dC = m_dWaterDensDt;
      dD = m_dWaterDensDs;
      dE = m_dWaterDensDpDs;
      dF = m_dWaterDensDtDs;

      /* Temperature derivatives */
      if ( iDrv >= EOS_DRV_T )
      {
         for ( i = 0; i < iM; i++ )
         {
            dK = pP[i];
            dL = pT[i];
            dM = pS[i];
            dG = dB + dE * dM;
            dH = dC + dF * dM;
            dI = dA + dM * dD + dG * dK;
            dJ = 1.0 + dH * dL;
            pRo[i] = dI * dJ;
            pDRods[i] = dI * dF * dL + dJ * ( dD + dE * dK );
            pDRodp[i] = dG * dJ;
            pDRodt[i] = dI * dH;
         }
      }

      /* Pressure derivatives */
      else if ( iDrv >= EOS_DRV_P )
      {
         for ( i = 0; i < iM; i++ )
         {
            dK = pP[i];
            dL = pT[i];
            dM = pS[i];
            dG = dB + dE * dM;
            dI = dA + dM * dD + dG * dK;
            dJ = 1.0 + ( dC + dF * dM ) * dL;
            pRo[i] = dI * dJ;
            pDRods[i] = dI * dF * dL + dJ * ( dD + dE * dK );
            pDRodp[i] = dG * dJ;
         }
      }

      /* Derivatives */
      else if ( iDrv )
      {
         for ( i = 0; i < iM; i++ )
         {
            dK = pP[i];
            dL = pT[i];
            dM = pS[i];
            dI = dA + dM * dD + ( dB + dE * dM ) * dK;
            dJ = 1.0 + ( dC + dF * dM ) * dL;
            pRo[i] = dI * dJ;
            pDRods[i] = dI * dF * dL + dJ * ( dD + dE * dK );
         }
      }

      /* No derivatives */
      else
      {
         for ( i = 0; i < iM; i++ )
         {
            dM = pS[i];
            pRo[i] = ( dA + dM * dD + ( dB + dE * dM ) * pP[i] ) * ( 1.0 + ( dC + dF * dM ) * pT[i] );
         }
      }

      /* Flag if to take inverse */
      iConvert = ( iVolume == EOS_DENSITY ? 0 : 1 );
   }

   /* Convert to density */
   else
   {
      iConvert = ( iVolume == EOS_DENSITY ? 1 : 0 );
   }

   /* Perform conversion */
   if ( iConvert )
   {
      if ( iDrv >= EOS_DRV_T )
      {
         for ( i = 0; i < iM; i++ )
         {
            dA = pRo[i];
            dB = 1.0 / dA;
            dC = -dB * dB;
            pRo[i] = dB;
            pDRodp[i] *= dC;
            pDRods[i] *= dC;
            pDRodt[i] *= dC;
         }
      }
      else if ( iDrv >= EOS_DRV_P )
      {
         for ( i = 0; i < iM; i++ )
         {
            dA = pRo[i];
            dB = 1.0 / dA;
            dC = -dB * dB;
            pRo[i] = dB;
            pDRodp[i] *= dC;
            pDRods[i] *= dC;
         }
      }
      else if ( iDrv )
      {
         for ( i = 0; i < iM; i++ )
         {
            dA = pRo[i];
            pRo[i] = 1.0 / dA;
            pDRods[i] *= ( -pRo[i] / dA );
         }
      }
      else
      {
         for ( i = 0; i < iM; i++ )
         {
            pRo[i] = 1.0 / pRo[i];
         }
      }
   }

   /* Finally get accumulation derivatives */
   if ( iDrv )
   {
      pTa = pDSda;
      pTb = pDRoda;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         for ( i = 0; i < iM; i++ )
         {
            *pTb++ = pDRods[i] **pTa++;
         }
      }
   }
}


/* 
// WaterHeat
//
// Function for calculating the heat 
//
// iM ** constant **
//    Number of objects
// iNc ** constant **
//    Number of components in aqueous phase
// iDrv ** constant **
//    Indicator for derivative level
//       EOS_NODRV
//          No derivatives
//       EOS_DRV_N
//          Derivatives wrt accumulations only
//       EOS_DRV_P
//          Derivatives wrt accumulations and pressure
//       EOS_DRV_T
//          Derivatives wrt accumulations, pressure, and temperature
// iTherm ** constant **
//    Indicator whether only one temperature use (0) or multiple (1)
// pP ** constant **
//    Pressures of objects
// pT ** constant **
//    Temperatures of objects
// pS ** constant **
//    Salinity correction
// pDSda ** constant **
//    Accumulation derivative of salinity
//       first index corresponds to object
//       second index corresponds to component
// pHeat 
//    The enthalpy of each object
// pDHeatdp 
//    Pressure derivative of enthalpy of each object
// pDHeatdt 
//    Temperature derivative of enthalpy of each object
// pDHeatds 
//    Salinity derivative of enthalpy of each object
// pDHeatda 
//    Accumulation derivative of enthalpy of each object
//       first index corresponds to object
//       second index corresponds to component
// pWork1
//    Work vector
//
// 1) If using the linear model simply compute the water heat as
//
//    Heat = HEATDT * T
//
// 2) The temperature dependent terms are computed in 
//    WaterHeatTerms.  Note that the loops are different 
//    as to whether or not a single temperature or multiple
//    temperatures apply
// 3) One can verify that the correlations are of the form
//
//    dH = INT ( V - T dV/dT ) dT + correction
//
//    vis-a-vis routine WaterDensity
// 4) The correlations are then applied.  By definition of salinity,
//
//    H(S) = H0 / S
//
// 5) Call SaltHeat for contribution from the salt terms
//
// 6) Even though extensizely modified the following information must
//    appear
//
// Authored by Bernhard Spang, The Mining Company
// URL:        http://chemengineer.miningco.com
// email:      chemengineer.guide@miningco.com
//                                              
// Water property calculations Copyright (C) 1998 by Bernhard Spang.
// All rights reserved. May be used and modified for free.
// Redistribution is also allowed if full credit is given to the
// author and a link to http://chemengineer.miningco.com is included
// Provided "as is" without warranty of any kind.
//                  
// For documentation see
// http://chemengineer.miningco.com/library/weekly/aa081798.htm
// new URL: http://www.cheresources.com/iapwsif97.shtml                 
//                         
// Reference:
// Properties of Water and Steam in SI-Units, 
// 2nd Revised and Updated Printing, Springer 1979, pp. 175 ff.
*/
void EosWater::WaterHeat( int iM, int iNc, int iDrv, int iTherm, double *pP, double *pT, double *pS, double *pDSda,
                          double *pHeat, double *pDHeatdp, double *pDHeatdt, double *pDHeatds, double *pDHeatda,
                          double *pWork1 )
{
   double *pRt0;
   double *pRt1;
   double *pRt2;
   double *pRt3;
   double *pDRt4;
   double *pDRt5;
   double *pDRt6;
   double *pDRt7;
   double *pDRt0;
   double *pDRt1;
   double *pDRt2;
   double *pDRt3;
   double *pRt4;
   double *pRt5;
   double *pRt6;
   double *pRt7;
   double  dA;
   double  dB;
   double  dC;
   double  dD;
   double  dE;
   double  dF;
   double  dH;
   double  dG;
   double  dP;
   double  dQ;
   double  dR;
   double  dS;
   double  dT;
   double  dU;
   double  dV;
   double *pRt8;
   double  dY;
   double  dZ;
   double  dR0;
   double  dR1;
   double  dR2;
   double  dR3;
   double  dR4;
   double  dR5;
   double  dR6;
   double  dR7;
   double  dR8;
   double  dW;
   double  dX;
   double *pDRt8;
   double *pRt9;
   double *pDRt9;
   double  dR9;
   double *pTa;
   double *pTb;
   int     iNi;
   int     i;

   /* Conversion constant */
   dH = m_dConvHeat / m_dConvDensity;

   /* Linear model */
   if ( m_iWaterModel == EOS_WATER_LINEAR )
   {
      dA = m_dWaterHeat;
      dB = m_dWaterHeatDs;

      /* Temperature derivatives */
      if ( iDrv >= EOS_DRV_T )
      {
         for ( i = 0; i < iM; i++ )
         {
            dC = dB * pS[i];
            dD = pT[i];
            dE = dA + dC;
            pHeat[i] = dE * dD;
            pDHeatds[i] = dB * dD;
            pDHeatdp[i] = 0.0;
            pDHeatdt[i] = dE;
         }
      }

      /* Pressure derivatives */
      else if ( iDrv >= EOS_DRV_P )
      {
         for ( i = 0; i < iM; i++ )
         {
            dD = pT[i];
            pHeat[i] = ( dA + dB * pS[i] ) * dD;
            pDHeatds[i] = dB * dD;
            pDHeatdp[i] = 0.0;
         }
      }

      /* Derivatives */
      else if ( iDrv )
      {
         for ( i = 0; i < iM; i++ )
         {
            dD = pT[i];
            pHeat[i] = ( dA + dB * pS[i] ) * dD;
            pDHeatds[i] = dB * dD;
         }
      }

      /* No derivatives */
      else
      {
         for ( i = 0; i < iM; i++ )
         {
            pHeat[i] = ( dA + dB * pS[i] ) * pT[i];
         }
      }
   }

   /* Isothermal case */
   else if ( iTherm == 0 )
   {
      pRt0 = pWork1;
      pDRt0 = pRt0 + 10;

      /* Get density terms */
      WaterHeatTerms( 1, EOS_NODRV, pT, pRt0, pDRt0 );

      /* Store the coefficients */
      dR0 = pRt0[0];
      dR1 = pRt0[1];
      dR2 = pRt0[2];
      dR3 = pRt0[3];
      dR4 = pRt0[4];
      dR5 = pRt0[5];
      dR6 = pRt0[6];
      dR7 = pRt0[7];
      dR8 = pRt0[8];
      dR9 = pRt0[9];

      /* Form terms */
      for ( i = 0; i < iM; i++ )
      {
         dP = pP[i] / m_dConvPressure;
         dA = dR7 + 4.4989682369e-9 * dP;
         dB = sqrt( dA );
         dE = dR6;
         dC = dE + dB;
         dY = 0.294117647059 / dC;
         dV = dY * dR8;
         dX = dY * dR9;
         dD = 2.60596148132e8 * dB - 3.69177876520e8 * dE;
         dW = 6.2977402465e8 * dB + 2.1412316838e9 * dE;
         dA = dV * dW;
         dZ = dX * 7.5572882958e8;
         dU = pow( dC, 0.705882352941 );
         dE = 0.00125914784989 * dU;
         dF = ( dD + dA - dZ ) * dE;
         dQ = 1.0 + 6.4557404117e-9 * dP;
         dR = dQ * dQ;
         dG = dR * dR;
         dS = dR4 / dG;
         dT = 5.16336333365e7 * dQ;

         /* Function */
         pHeat[i] = dH * ( dF - dT * dS + dP * ( dP * ( dP * ( dP * dR3 * 0.25 + dR2 / 3.0 ) + dR1 * 0.5 ) + dR0 ) + dR5 ) / pS[i];

         /* Form terms for derivative */
         pWork1[i] = dE * ( 1.0 + dV + dX / dB / 2.0 ) / dC + dS;
      }

      /* Derivatives */
      if ( iDrv )
      {
         for ( i = 0; i < iM; i++ )
         {
            pDHeatds[i] = -pHeat[i] / pS[i];
         }

         if ( iDrv >= EOS_DRV_P )
         {
            dT = dH / m_dConvPressure;
            for ( i = 0; i < iM; i++ )
            {
               dP = pP[i] / m_dConvPressure;
               pDHeatdp[i] = dT * ( pWork1[i] + dP * ( dP * ( dP * dR3 + dR2 ) + dR1 ) + dR0 ) / pS[i];
            }

            if ( iDrv >= EOS_DRV_T )
            {
               for ( i = 0; i < iM; i++ )
               {
                  pDHeatdt[i] = 0.0;
               }
            }
         }
      }

      /* Correct for salt volume */
      SaltHeat( iM, iDrv, iTherm, pP, pT, pS, pHeat, pDHeatdp, pDHeatdt, pDHeatds );
   }

   /* Thermal case */
   else
   {
      /* Set up memory */
      pRt0 = pWork1;
      pRt1 = pRt0 + iM;
      pRt2 = pRt1 + iM;
      pRt3 = pRt2 + iM;
      pRt4 = pRt3 + iM;
      pRt5 = pRt4 + iM;
      pRt6 = pRt5 + iM;
      pRt7 = pRt6 + iM;
      pRt8 = pRt7 + iM;
      pRt9 = pRt8 + iM;
      pDRt0 = pRt9 + iM;
      pDRt1 = pDRt0 + iM;
      pDRt2 = pDRt1 + iM;
      pDRt3 = pDRt2 + iM;
      pDRt4 = pDRt3 + iM;
      pDRt5 = pDRt4 + iM;
      pDRt6 = pDRt5 + iM;
      pDRt7 = pDRt6 + iM;
      pDRt8 = pDRt7 + iM;
      pDRt9 = pDRt8 + iM;

      /* Get the density terms */
      WaterHeatTerms( iM, iDrv, pT, pRt0, pDRt0 );

      /* Form terms */
      for ( i = 0; i < iM; i++ )
      {
         dP = pP[i] / m_dConvPressure;
         dA = pRt7[i] + 4.4989682369e-9 * dP;
         dB = sqrt( dA );
         dE = pRt6[i];
         dC = dE + dB;
         dY = 0.294117647059 / dC;
         dV = dY * pRt8[i];
         dX = dY * pRt9[i];
         dD = 2.60596148132e8 * dB - 3.69177876520e8 * dE;
         dW = 6.2977402465e8 * dB + 2.1412316838e9 * dE;
         dA = dV * dW;
         dZ = dX * 7.5572882958e8;
         dU = pow( dC, 0.705882352941 );
         dE = 0.00125914784989 * dU;
         dF = ( dD + dA - dZ ) * dE;
         dY = dY * dE;
         dD = dD / dC;
         dA = dA / dC;
         dZ = dZ / dC;
         dQ = 1.0 + 6.4557404117e-9 * dP;
         dR = dQ * dQ;
         dG = dR * dR;
         dS = pRt4[i] / dG;
         dT = 5.16336333365e7 * dQ;

         /* Function */
         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         pHeat[i] = dH * ( dF - dT * dS + dP * ( dP * ( dP * ( dP * pRt3[i] * 0.25 + pRt2[i] / 3.0 ) + pRt1[i] * 0.5 ) + pRt0[i] ) + pRt5[i] ) / pS[i];

         /* Form terms for derivative */
         pRt4[i] = dE * ( 1.0 + dV + dX / dB / 2.0 ) / dC + dS;
         pRt5[i] = dT / dG;
         pRt6[i] = dE * ( 0.705882352941 * dD - 0.294117647059 * dA - 3.6917787652e8 + dV * 2.1412316838e9 + 0.294117647059 * dZ );
         pRt7[i] = dE * ( 0.352941176471 * dD - 0.147058823530 * dA + 1.3029807407e8 + dV * 3.1488701233e8 + 0.147058823530 * dZ ) / dB;
         pRt8[i] = dY * dW;
         pRt9[i] = -dY * 7.5572882958e8;
      }

      /* Derivatives */
      if ( iDrv )
      {
         for ( i = 0; i < iM; i++ )
         {
            pDHeatds[i] = -pHeat[i] / pS[i];
         }

         if ( iDrv >= EOS_DRV_P )
         {
            dT = dH / m_dConvPressure;
            for ( i = 0; i < iM; i++ )
            {
               dP = pP[i] / m_dConvPressure;
               pDHeatdp[i] = dT * ( pRt4[i] + dP * ( dP * ( dP * pRt3[i] + pRt2[i] ) + pRt1[i] ) + pRt0[i] ) / pS[i];
            }

            if ( iDrv >= EOS_DRV_T )
            {
               dT = dH / m_dConvTemperature;
               for ( i = 0; i < iM; i++ )
               {
                  dP = pP[i] / m_dConvPressure;

                  /* TODO: The next line is too long. Please rewrite to make it shorter. */
                  pDHeatdt[i] = dT * ( dP * ( dP * ( dP * ( dP * pDRt3[i] * 0.25 + pDRt2[i] / 3.0 ) + pDRt1[i] * 0.5 ) + pDRt0[i] ) - pDRt4[i] * pRt5[i] + pDRt6[i] * pRt6[i] + pDRt7[i] * pRt7[i] + pDRt8[i] * pRt8[i] + pDRt9[i] * pRt9[i] + pDRt5[i] ) / pS[i];
               }
            }
         }
      }

      /* Correct for salt volume */
      SaltHeat( iM, iDrv, iTherm, pP, pT, pS, pHeat, pDHeatdp, pDHeatdt, pDHeatds );
   }

   /* Finally get accumulation derivatives */
   if ( iDrv )
   {
      pTa = pDSda;
      pTb = pDHeatda;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         for ( i = 0; i < iM; i++ )
         {
            *pTb++ = pDHeatds[i] **pTa++;
         }
      }
   }
}


/* 
// WaterHeatTerms
//
// Function to return coefficients for water heat correlations 
//
// iM ** constant **
//    Number of objects
// iDrv ** constant **
//    Indicator for derivative level
//       EOS_NODRV
//          No derivatives
//       EOS_DRV_N
//          Derivatives wrt accumulations only
//       EOS_DRV_P
//          Derivatives wrt accumulations and pressure
//       EOS_DRV_T
//          Derivatives wrt accumulations, pressure, and temperature
// pT ** constant **
//    Temperatures of object
// pWTerms
//    Terms for the water correlations
// pDTerms
//    Derivatives of terms for the water correlations
//
// 1) Even though extensizely modified the following information must
//    appear
//
// Authored by Bernhard Spang, The Mining Company
// URL:        http://chemengineer.miningco.com
// email:      chemengineer.guide@miningco.com
//                                              
// Water property calculations Copyright (C) 1998 by Bernhard Spang.
// All rights reserved. May be used and modified for free.
// Redistribution is also allowed if full credit is given to the
// author and a link to http://chemengineer.miningco.com is included
// Provided "as is" without warranty of any kind.
//                  
// For documentation see
// http://chemengineer.miningco.com/library/weekly/aa081798.htm
// new URL: http://www.cheresources.com/iapwsif97.shtml                 
//                         
// Reference:
// Properties of Water and Steam in SI-Units, 
// 2nd Revised and Updated Printing, Springer 1979, pp. 175 ff.
*/
void EosWater::WaterHeatTerms( int iM, int iDrv, double *pT, double *pWTerms, double *pDTerms )
{
   double *pDRt0;
   double *pDRt1;
   double *pDRt2;
   double *pDRt3;
   double *pRt4;
   double *pRt5;
   double *pRt6;
   double *pRt7;
   double *pRt0;
   double *pRt1;
   double *pRt2;
   double *pRt3;
   double *pDRt4;
   double *pDRt5;
   double *pDRt6;
   double *pDRt7;
   double  dA;
   double  dB;
   double  dC;
   double  dD;
   double  dE;
   double  dF;
   double  dTr2;
   double *pRt8;
   double *pDRt8;
   double *pRt9;
   double *pDRt9;
   double  dG;
   double  dH;
   double  dI;
   double  dT;
   double  dTr;
   double  dTr6;
   double  dTr12;
   double  dTr18;
   int     i;

   /* Set up memory for case without derivatives */
   if ( iDrv < EOS_DRV_T )
   {
      pRt0 = pWTerms;
      pRt1 = pRt0 + iM;
      pRt2 = pRt1 + iM;
      pRt3 = pRt2 + iM;
      pRt4 = pRt3 + iM;
      pRt5 = pRt4 + iM;
      pRt6 = pRt5 + iM;
      pRt7 = pRt6 + iM;
      pRt8 = pRt7 + iM;
      pRt9 = pRt8 + iM;

      /* Form temperature terms */
      for ( i = 0; i < iM; i++ )
      {
         dT = pT[i] / m_dConvTemperature;
         dTr = dT / 647.3;
         dTr2 = dTr * dTr;
         dTr6 = dTr2 * dTr2 * dTr2;
         dTr12 = dTr6 * dTr6;
         dTr18 = dTr6 * dTr12;

         /* Other terms */
         dC = 0.65371543 - dTr;
         dD = dC * dC;
         dE = dD * dD;
         dI = dE * dE;
         dA = dI * dD;
         dF = dTr18 * dTr;
         dB = 0.00000115 + dF;
         dE = 19.0 * dF / dB;
         dH = 4.025e-13 * ( 1.0 + dE ) / dB;
         dI = dI * dTr;
         dG = 7.6766209995 * dI * dC + 0.76766209995 * dA;
         dI = dTr12 / dTr;
         dC = 1.5108e-5 + dI;
         dI = dI / dC;
         dF = ( 1.0 + 11.0 * dI ) / dC;
         dA = -dTr18 * ( 1.23373612372e-4 + 9.7186463912e-4 * dTr2 );

         /* Polynomial terms */
         pRt0[i] = dG + dH - 7.24116460118e-5 * dTr2 - 6.57723749976e-10 * dF - 0.2401 * dA - 8.29453274231e-5;
         pRt1[i] = -6.2311433178e-18 * dF;
         pRt2[i] = ( 5.186659247328e-23 - 2.1490204744e-26 * dF );
         pRt3[i] = 1.487880767751e-36 / dTr18 / dTr2;
         pRt4[i] = dA;

         /* Nonlinear terms */
         dC = 1.0 - 0.8438375405 * dTr2 - 0.000536216216 / dTr6;
         dD = 3.217297296e-03 / dTr6 - 1.687675081 * dTr2;
         dA = 0.14684556978 * dTr;
         dB = 3.44 * dC * dD - dA;
         pRt6[i] = dC;
         pRt7[i] = 1.72 * dC * dC - dA;
         pRt8[i] = dD;
         pRt9[i] = dB;

         /* Heat term */
         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         pRt5[i] = -38019727.360 + dT * ( 7.39301458787e+05 - dT * ( 6.59585664669e+03 - dT * ( 3.48164180485e+01 - dT * ( 1.18654126027e-01 - dT * ( 2.69996813002e-04 - dT * ( 4.09464695114e-07 - dT * ( 3.98607493164e-10 - dT * ( 2.25848050803e-13 - dT * ( 5.67215615125e-17 ) ) ) ) ) ) ) ) );
      }
   }

   /* Temperature derivatives */
   else
   {
      pRt0 = pWTerms;
      pRt1 = pRt0 + iM;
      pRt2 = pRt1 + iM;
      pRt3 = pRt2 + iM;
      pRt4 = pRt3 + iM;
      pRt5 = pRt4 + iM;
      pRt6 = pRt5 + iM;
      pRt7 = pRt6 + iM;
      pRt8 = pRt7 + iM;
      pRt9 = pRt8 + iM;
      pDRt0 = pDTerms;
      pDRt1 = pDRt0 + iM;
      pDRt2 = pDRt1 + iM;
      pDRt3 = pDRt2 + iM;
      pDRt4 = pDRt3 + iM;
      pDRt5 = pDRt4 + iM;
      pDRt6 = pDRt5 + iM;
      pDRt7 = pDRt6 + iM;
      pDRt8 = pDRt7 + iM;
      pDRt9 = pDRt8 + iM;

      /* Form temperature terms */
      for ( i = 0; i < iM; i++ )
      {
         dT = pT[i] / m_dConvTemperature;
         dTr = dT / 647.3;
         dTr2 = dTr * dTr;
         dTr6 = dTr2 * dTr2 * dTr2;
         dTr12 = dTr6 * dTr6;
         dTr18 = dTr6 * dTr12;

         /* Other terms */
         dC = 0.65371543 - dTr;
         dD = dC * dC;
         dE = dD * dD;
         dI = dE * dE;
         dA = dI * dD;
         dF = dTr18 * dTr;
         dB = 0.00000115 + dF;
         dE = 19.0 * dF / dB;
         dH = 4.025e-13 * ( 1.0 + dE ) / dB;
         dE = 4.025e-13 * dE * ( 18.0 - dE - dE ) / dB / dT;
         dI = dI * dTr;
         dG = 7.6766209995 * dI * dC + 0.76766209995 * dA;
         dD = 0.1067350362977 * dI;
         dI = dTr12 / dTr;
         dC = 1.5108e-5 + dI;
         dI = dI / dC;
         dF = ( 1.0 + 11.0 * dI ) / dC;
         dI = dI * ( 110.0 - 242.0 * dI ) / dC / dT;
         dA = -dTr18 * ( 1.23373612372e-4 + 9.7186463912e-4 * dTr2 );
         dB = ( 18.0 * dA - dTr18 * 1.94372927824e-3 * dTr2 ) / dT;

         /* Polynomial terms */
         pRt0[i] = dG + dH - 7.24116460118e-5 * dTr2 - 6.57723749976e-10 * dF - 0.2401 * dA - 8.29453274231e-5;
         pRt1[i] = -6.2311433178e-18 * dF;
         pRt2[i] = ( 5.186659247328e-23 - 2.1490204744e-26 * dF );
         pRt3[i] = 1.487880767751e-36 / dTr18 / dTr2;
         pRt4[i] = dA;

         /* Derivatives */
         pDRt0[i] = dE - dD - 0.2401 * dB - 6.57723749976e-10 * dI - 1.448232920236e-4 * dTr2 / dT;
         pDRt1[i] = -6.2311433178e-18 * dI;
         pDRt2[i] = -2.1490204744e-26 * dI;
         pDRt3[i] = -20.0 * pRt3[i] / dT;
         pDRt4[i] = dB;

         /* Nonlinear terms */
         dC = 1.0 - 0.8438375405 * dTr2 - 0.000536216216 / dTr6;
         dD = 3.217297296e-03 / dTr6 - 1.687675081 * dTr2;
         dE = 1.9303783776e-02 / dTr6 + 3.375350162 * dTr2;
         dA = 0.14684556978 * dTr;
         dB = 3.44 * dC * dD - dA;
         pRt6[i] = dC;
         pRt7[i] = 1.72 * dC * dC - dA;
         pRt8[i] = dD;
         pRt9[i] = dB;
         pDRt6[i] = dD / dT;
         pDRt7[i] = dB / dT;
         pDRt8[i] = -dE / dT;
         pDRt9[i] = ( 3.44 * ( dD * dD - dC * dE ) - dA ) / dT;

         /* Heat term */
         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         pRt5[i] = -38019727.360 + dT * ( 7.39301458787e+05 - dT * ( 6.59585664669e+03 - dT * ( 3.48164180485e+01 - dT * ( 1.18654126027e-01 - dT * ( 2.69996813002e-04 - dT * ( 4.09464695114e-07 - dT * ( 3.98607493164e-10 - dT * ( 2.25848050803e-13 - dT * ( 5.67215615125e-17 ) ) ) ) ) ) ) ) );

         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         pDRt5[i] = 739301.45878 - dT * ( 1.31917132934e+04 - dT * ( 1.04449254145e+02 - dT * ( 4.74616504107e-01 - dT * ( 1.34998406501e-03 - dT * ( 2.45678817068e-06 - dT * ( 2.79025245215e-09 - dT * ( 1.80678440642e-12 - dT * ( 5.10494053612e-16 ) ) ) ) ) ) ) );
      }
   }
}


/* 
// WaterIft
// 
// Function for the ift of water 
//
// iM ** constant **
//    Number of objects
// iNc ** constant **
//    Number of components in aqueous phase
// iDrv ** constant **
//    Indicator for derivative level
//       EOS_NODRV
//          No derivatives
//       EOS_DRV_N
//          Derivatives wrt accumulations only
//       EOS_DRV_P
//          Derivatives wrt accumulations and pressure
//       EOS_DRV_T
//          Derivatives wrt accumulations, pressure, and temperature
// pIft 
//    The interfacial tension wrt reference phase of each object
// pDIftdp 
//    Pressure derivative interfacial tension wrt reference phase of 
//    each object
// pDIftdt 
//    Temperature derivative interfacial tension wrt reference phase 
//    of each object
// pDIftda 
//    Accumulation derivative interfacial tension wrt reference phase
//    of each object
//       first index corresponds to object
//       second index corresponds to component
//
// 1) Set the interfacial tension between water and the reference
//    phase to a constant value
*/
void EosWater::WaterIft( int iM, int iNc, int iDrv, double *pIft, double *pDIftdp, double *pDIftdt, double *pDIftda )
{
   double *pTa;
   int     iNi;
   int     i;

   /* Multiple grid block case */
   if ( iM > 1 )
   {
      for ( i = 0; i < iM; i++ )
      {
         pIft[i] = m_dWaterIft;
      }

      if ( iDrv )
      {
         pTa = pDIftda;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               *pTa++ = 0.0;
            }
         }

         if ( iDrv >= EOS_DRV_P )
         {
            for ( i = 0; i < iM; i++ )
            {
               pDIftdp[i] = 0.0;
            }

            if ( iDrv >= EOS_DRV_T )
            {
               for ( i = 0; i < iM; i++ )
               {
                  pDIftdt[i] = 0.0;
               }
            }
         }
      }
   }

   /* Single grid block case */
   else
   {
      *pIft = m_dWaterIft;
      if ( iDrv )
      {
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            pDIftda[iNi] = 0.0;
         }

         if ( iDrv >= EOS_DRV_P )
         {
            *pDIftdp = 0.0;
            if ( iDrv >= EOS_DRV_T )
            {
               *pDIftdt = 0.0;
            }
         }
      }
   }
}


/* 
// WaterVaporPressure
//
// Function for the vapor pressure of water
//
// Returns the vapor pressure of water
//
// dT ** constant **
//    Temperature in K
// 1) Even though modified the following information must appear
//
// Authored by Bernhard Spang, The Mining Company
// URL:        http://chemengineer.miningco.com
// email:      chemengineer.guide@miningco.com
//                                              
// Water property calculations Copyright (C) 1998 by Bernhard Spang.
// All rights reserved. May be used and modified for free.
// Redistribution is also allowed if full credit is given to the
// author and a link to http://chemengineer.miningco.com is included
// Provided "as is" without warranty of any kind.
//                  
// For documentation see
// http://chemengineer.miningco.com/library/weekly/aa081798.htm
// new URL: http://www.cheresources.com/iapwsif97.shtml                 
//                         
// Reference:
// Properties of Water and Steam in SI-Units, 
// 2nd Revised and Updated Printing, Springer 1979, pp. 175 ff.
*/
double EosWater::WaterVaporPressure( double dT )
{
   double dR = 647.3 * m_dConvTemperature;
   double dS = 273.16 * m_dConvTemperature;
   double dQ = ( dT > dS ? dT : dS );
   double dU = ( 1.45220717 - 2.0 * pow( dR / ( dQ > dR ? dR : dQ ) - 0.95, 0.4 ) ) / 0.84878953;

   return exp( 14.03079 + dU * ( 4.771298    - dU * ( 2.31523     - dU * ( 4.973715e-1 - dU * ( 8.098531e-2 - 
                          dU * ( 8.365383e-3 + dU * ( 4.688029e-2 - dU * ( 7.770678e-2 + dU * ( 2.587746e-2 - 
                          dU * ( 7.086141e-2 + dU * ( 7.834283e-3 - dU * ( 2.145744e-2 ) ) ) ) ) ) ) ) ) ) ) ) * m_dConvPressure;
}


/* 
// WaterViscosity
//
// Function for the viscosity of water 
//
// iM ** constant **
//    Number of objects
// iNc ** constant **
//    Number of components in aqueous phase
// iDrv ** constant **
//    Indicator for derivative level
//       EOS_NODRV
//          No derivatives
//       EOS_DRV_N
//          Derivatives wrt accumulations only
//       EOS_DRV_P
//          Derivatives wrt accumulations and pressure
//       EOS_DRV_T
//          Derivatives wrt accumulations, pressure, and temperature
// iTherm ** constant **
//    Indicator whether only one temperature use (0) or multiple (1)
// pP ** constant **
//    Pressures of objects
// pT ** constant **
//    Temperatures of objects
// pS ** constant **
//    Salinity of objects
// pDSda ** constant **
//    Accumulation derivative of salinity of objects
// pRo ** constant ** 
//    The mass volume of each object
// pDRodp ** constant ** 
//    Pressure derivative of the mass volume of each object
// pDRodt ** constant ** 
//    Temperature derivative of the mass volume of each object
// pDRods ** constant ** 
//    Salinity derivative of the mass volume of each object
//       first index corresponds to object
//       second index corresponds to component
// pMu 
//    The viscosity of each object
// pDMudp 
//    Pressure derivative of viscosity of each object
// pDMudt 
//    Temperature derivative of viscosity of each object
// pDMuds 
//    Salinity derivative of viscosity of each object
// pDMuda 
//    Accumulation derivative of viscosity of each object
//       first index corresponds to object
//       second index corresponds to component
// pWork1
//    Work vector
//
// 1) If using the linear model, compute water density as
//
//    VISC = VISC0 * ( 1 + VISCDP * P ) * ( 1 + VISCDT * T )
//
// 2) The temperature dependent terms are computed in 
//    WaterViscosityTerms.  Note that the loops are different 
//    as to whether or not a single temperature or multiple
//    temperatures apply
// 3) The density must have already been computed in routine 
//    WaterDensity
// 4) The correlations are then applied. 
// 5) This has been checked against the data of C.S. Matthews and
//    D.G. Russell, "Pressure Buildup and Flow Tests in Wells,"
//    Monograph Series, Society of Petroleum Engineers of AIME,
//    Dallas (1967) 1, Appendix G
// 6) Even though extensizely modified the following information must
//    appear
//
// Authored by Bernhard Spang, The Mining Company
// URL:        http://chemengineer.miningco.com
// email:      chemengineer.guide@miningco.com
//                                              
// Water property calculations Copyright (C) 1998 by Bernhard Spang.
// All rights reserved. May be used and modified for free.
// Redistribution is also allowed if full credit is given to the
// author and a link to http://chemengineer.miningco.com is included
// Provided "as is" without warranty of any kind.
//                  
// For documentation see
// http://chemengineer.miningco.com/library/weekly/aa081798.htm
// new URL: http://www.cheresources.com/iapwsif97.shtml                 
//                         
// Reference:
// Properties of Water and Steam in SI-Units, 
// 2nd Revised and Updated Printing, Springer 1979, pp. 175 ff.
*/
void EosWater::WaterViscosity( int iM, int iNc, int iDrv, int iTherm, double *pP, double *pT, double *pS, double *pDSda,
                               double *pRo, double *pDRodp, double *pDRodt, double *pDRods, double *pMu, double *pDMudp,
                               double *pDMudt, double *pDMuds, double *pDMuda, double *pWork1 )
{
   double  dL;
   double  dM;
   double *pRt0;
   double *pRt1;
   double *pRt2;
   double *pRt3;
   double *pRt4;
   double *pRt5;
   double *pTa;
   double *pTb;
   double  dA;
   double  dB;
   double  dC;
   double  dE;
   double  dF;
   double  dG;
   double  dH;
   double  dI;
   double  dJ;
   double  dR0;
   double  dR1;
   double  dR2;
   double  dR3;
   double  dR4;
   double  dR5;
   double  dD;
   double  dK;
   double *pDRt0;
   double *pDRt1;
   double *pDRt2;
   double *pDRt3;
   double *pDRt4;
   double *pDRt5;
   int     iNi;
   int     i;

   /* Conversion */
   dA = m_dConvVolume / m_dConvDensity;

   /* Linear model */
   if ( m_iWaterModel == EOS_WATER_LINEAR )
   {
      dA = m_dWaterVisc;
      dB = m_dWaterViscDp;
      dC = m_dWaterViscDt;
      dD = m_dWaterViscDs;
      dE = m_dWaterViscDpDs;
      dF = m_dWaterViscDtDs;

      /* Temperature derivatives */
      if ( iDrv >= EOS_DRV_T )
      {
         for ( i = 0; i < iM; i++ )
         {
            dK = pP[i];
            dL = pT[i];
            dM = pS[i];
            dG = dB + dE * dM;
            dH = dC + dF * dM;
            dI = dA + dM * dD + dG * dK;
            dJ = 1.0 + dH * dL;
            pMu[i] = dI * dJ;
            pDMuds[i] = dI * dF * dL + dJ * ( dD + dE * dK );
            pDMudp[i] = dG * dJ;
            pDMudt[i] = dI * dH;
         }
      }

      /* Pressure derivatives */
      else if ( iDrv >= EOS_DRV_P )
      {
         for ( i = 0; i < iM; i++ )
         {
            dK = pP[i];
            dL = pT[i];
            dM = pS[i];
            dG = dB + dE * dM;
            dI = dA + dM * dD + dG * dK;
            dJ = 1.0 + ( dC + dF * dM ) * dL;
            pMu[i] = dI * dJ;
            pDMuds[i] = dI * dF * dL + dJ * ( dD + dE * dK );
            pDMudp[i] = dG * dJ;
         }
      }

      /* Derivatives */
      else if ( iDrv )
      {
         for ( i = 0; i < iM; i++ )
         {
            dK = pP[i];
            dL = pT[i];
            dM = pS[i];
            dI = dA + dM * dD + ( dB + dE * dM ) * dK;
            dJ = 1.0 + ( dC + dF * dM ) * dL;
            pMu[i] = dI * dJ;
            pDMuds[i] = dI * dF * dL + dJ * ( dD + dE * dK );
         }
      }

      /* No derivatives */
      else
      {
         for ( i = 0; i < iM; i++ )
         {
            dM = pS[i];
            pMu[i] = ( dA + dM * dD + ( dB + dE * dM ) * pP[i] ) * ( 1.0 + ( dC + dF * dM ) * pT[i] );
         }
      }
   }

   /* Isothermal case */
   else if ( iTherm == 0 )
   {
      pRt0 = pWork1 + iM;
      pDRt0 = pRt0 + 6;

      /* Set up the coefficients */
      WaterViscosityTerms( 1, EOS_NODRV, pT, pRt0, pDRt0 );

      /* Get the viscosity */
      dR0 = pRt0[0] * m_dConvViscosity;
      dR1 = pRt0[1];
      dR2 = pRt0[2];
      dR3 = pRt0[3];
      dR4 = pRt0[4];
      dR5 = pRt0[5];

      /* Standard viscosity */
      for ( i = 0; i < iM; i++ )
      {
         dB = dA / pRo[i];
         pWork1[i] = dB;
         pMu[i] = ( dR1 + ( dR2 + ( dR3 + ( dR4 + dR5 * dB ) * dB ) * dB ) * dB ) * dB;
      }

      EosUtils::VectorExp( iM, pMu, pMu );
      for ( i = 0; i < iM; i++ )
      {
         pMu[i] *= dR0;
      }

      /* Form the derivative */
      if ( iDrv )
      {
         for ( i = 0; i < iM; i++ )
         {
            dB = pWork1[i];
            pDMuds[i] = -dB * pMu[i] * ( dR1 + ( dR2 + dR2 + ( 3.0 * dR3 + ( 4.0 * dR4 + 5.0 * dR5 * dB ) * dB ) * dB ) * dB ) / pRo[i];
         }

         if ( iDrv >= EOS_DRV_P )
         {
            for ( i = 0; i < iM; i++ )
            {
               pDMudp[i] = pDMuds[i] * pDRodp[i];
            }

            if ( iDrv >= EOS_DRV_T )
            {
               for ( i = 0; i < iM; i++ )
               {
                  pDMudt[i] = 0.0;
               }
            }
         }

         for ( i = 0; i < iM; i++ )
         {
            pDMuds[i] *= pDRods[i];
         }
      }
   }

   /* Thermal case */
   else
   {
      pRt0 = pWork1 + iM;
      pDRt0 = pRt0 + 6 * iM;

      /* Set up the coefficients */
      WaterViscosityTerms( iM, iDrv, pT, pRt0, pDRt0 );

      /* Get the viscosity */
      pRt1 = pRt0 + iM;
      pRt2 = pRt1 + iM;
      pRt3 = pRt2 + iM;
      pRt4 = pRt3 + iM;
      pRt5 = pRt4 + iM;

      /* Standard viscosity */
      for ( i = 0; i < iM; i++ )
      {
         dB = dA / pRo[i];
         pWork1[i] = dB;
         pMu[i] = ( pRt1[i] + ( pRt2[i] + ( pRt3[i] + ( pRt4[i] + pRt5[i] * dB ) * dB ) * dB ) * dB ) * dB;
      }

      EosUtils::VectorExp( iM, pMu, pMu );
      for ( i = 0; i < iM; i++ )
      {
         pMu[i] *= ( pRt0[i] * m_dConvViscosity );
      }

      /* Form the derivative */
      if ( iDrv )
      {
         for ( i = 0; i < iM; i++ )
         {
            dB = pWork1[i];

            /* TODO: The next line is too long. Please rewrite to make it shorter. */
            pDMuds[i] = -dB * pMu[i] * ( pRt1[i] + ( pRt2[i] + pRt2[i] + ( 3.0 * pRt3[i] + ( 4.0 * pRt4[i] + 5.0 * pRt5[i] * dB ) * dB ) * dB ) * dB ) / pRo[i];
         }

         if ( iDrv >= EOS_DRV_P )
         {
            for ( i = 0; i < iM; i++ )
            {
               pDMudp[i] = pDMuds[i] * pDRodp[i];
            }

            if ( iDrv >= EOS_DRV_T )
            {
               pDRt1 = pDRt0 + iM;
               pDRt2 = pDRt1 + iM;
               pDRt3 = pDRt2 + iM;
               pDRt4 = pDRt3 + iM;
               pDRt5 = pDRt4 + iM;
               for ( i = 0; i < iM; i++ )
               {
                  dB = pWork1[i];

                  /* TODO: The next line is too long. Please rewrite to make it shorter. */
                  pDMudt[i] = pDMuds[i] * pDRodt[i] + pMu[i] * ( ( pDRt1[i] + ( pDRt2[i] + ( pDRt3[i] + ( pDRt4[i] + pDRt5[i] * dB ) * dB ) * dB ) * dB ) * dB + pDRt0[i] / pRt0[i] ) / m_dConvTemperature;
               }
            }
         }

         for ( i = 0; i < iM; i++ )
         {
            pDMuds[i] *= pDRods[i];
         }
      }
   }

   /* Get accumulation derivatives */
   if ( iDrv )
   {
      pTa = pDSda;
      pTb = pDMuda;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         for ( i = 0; i < iM; i++ )
         {
            *pTb++ = pDMuds[i] **pTa++;
         }
      }
   }
}


/* 
// WaterViscosityTerms
//
// Function to return coefficients for water viscosity correlations 
//
// iM ** constant **
//    Number of objects
// iDrv ** constant **
//    Indicator for derivative level
//       EOS_NODRV
//          No derivatives
//       EOS_DRV_N
//          Derivatives wrt accumulations only
//       EOS_DRV_P
//          Derivatives wrt accumulations and pressure
//       EOS_DRV_T
//          Derivatives wrt accumulations, pressure, and temperature
// pT ** constant **
//    Temperatures of object
// pWTerms
//    Terms for the water correlations
// pDTerms
//    Derivatives of terms for the water correlations
//
// 1) Even though extensizely modified the following information must
//    appear
//
// Authored by Bernhard Spang, The Mining Company
// URL:        http://chemengineer.miningco.com
// email:      chemengineer.guide@miningco.com
//                                              
// Water property calculations Copyright (C) 1998 by Bernhard Spang.
// All rights reserved. May be used and modified for free.
// Redistribution is also allowed if full credit is given to the
// author and a link to http://chemengineer.miningco.com is included
// Provided "as is" without warranty of any kind.
//                  
// For documentation see
// http://chemengineer.miningco.com/library/weekly/aa081798.htm
// new URL: http://www.cheresources.com/iapwsif97.shtml                 
//                         
// Reference:
// Properties of Water and Steam in SI-Units, 
// 2nd Revised and Updated Printing, Springer 1979, pp. 175 ff.
*/
void EosWater::WaterViscosityTerms( int iM, int iDrv, double *pT, double *pWTerms, double *pDTerms )
{
   double *pDMt0;
   double *pDMt1;
   double *pDMt2;
   double *pDMt3;
   double *pDMt4;
   double *pDMt5;
   double *pMt0;
   double *pMt1;
   double *pMt2;
   double *pMt3;
   double *pMt4;
   double *pMt5;
   double  dC;
   double  dD;
   double  dT;
   double  dTr2;
   int     i;

   /* Case without temperature derivatives */
   if ( iDrv < EOS_DRV_T )
   {
      pMt0 = pWTerms;
      pMt1 = pMt0 + iM;
      pMt2 = pMt1 + iM;
      pMt3 = pMt2 + iM;
      pMt4 = pMt3 + iM;
      pMt5 = pMt4 + iM;

      /* Standard viscosity */
      for ( i = 0; i < iM; i++ )
      {
         dT = pT[i] / m_dConvTemperature;
         pMt0[i] = sqrt( dT ) / ( 4.619744170e05 + ( 2.925026896e08 + ( 1.122246986e11 - 2.535044618e13 / dT ) / dT ) / dT );

         /* Density correction terms */
         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         pMt1[i] = -3.05921002920e-03 + ( 1.13777696760e+01 - ( 1.26736482482e+04 - ( 6.59580486763e+06 - ( 1.85047365652e+09 - 1.56902136656e+11 / dT ) / dT ) / dT ) / dT ) / dT;

         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         pMt2[i] = -3.13856263180e-07 - ( 3.93548484898e-03 - ( 1.97010281562e+01 - ( 1.57519830357e+04 - ( 6.23359317197e+06 - 5.54337618755e+08 / dT ) / dT ) / dT ) / dT ) / dT;

         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         pMt3[i] = -4.37734458244e-09 - ( 4.25830115234e-06 + ( 1.37165066060e-02 - ( 1.39935919264e+01 - ( 7.11514857923e+03 - 6.10899342529e+05 / dT ) / dT ) / dT ) / dT ) / dT;

         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         pMt4[i] = -3.79483685546e-11 + ( 1.19652284399e-07 - ( 1.02546710426e-04 - ( 4.21385747841e-02 - ( 6.55368785039e+00 - 5.36060258568e+02 / dT ) / dT ) / dT ) / dT ) / dT;

         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         pMt5[i] = 4.14422162573e-14 - ( 1.12046571348e-10 - ( 1.01389763575e-07 - ( 4.40098272612e-05 - ( 8.75103585798e-03 - 7.10460325286e-01 / dT ) / dT ) / dT ) / dT ) / dT;
      }
   }

   /* Viscosity derivatives */
   else
   {
      pMt0 = pWTerms;
      pMt1 = pMt0 + iM;
      pMt2 = pMt1 + iM;
      pMt3 = pMt2 + iM;
      pMt4 = pMt3 + iM;
      pMt5 = pMt4 + iM;
      pDMt0 = pDTerms;
      pDMt1 = pDMt0 + iM;
      pDMt2 = pDMt1 + iM;
      pDMt3 = pDMt2 + iM;
      pDMt4 = pDMt3 + iM;
      pDMt5 = pDMt4 + iM;

      /* Standard viscosity */
      for ( i = 0; i < iM; i++ )
      {
         dT = pT[i] / m_dConvTemperature;
         dTr2 = dT * dT;
         dC = 4.619744170e05 + ( 2.925026896e08 + ( 1.122246986e11 - 2.535044618e13 / dT ) / dT ) / dT;
         dD = ( 2.925026896e08 + ( 2.244493972e11 - 7.605133835e13 / dT ) / dT ) / dT;
         pMt0[i] = sqrt( dT ) / dC;
         pDMt0[i] = pMt0[i] * ( 0.5 + dD / dC ) / dT;

         /* Density correction terms */
         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         pMt1[i] = -3.05921002920e-03 + ( 1.13777696760e+01 - ( 1.26736482482e+04 - ( 6.59580486763e+06 - ( 1.85047365652e+09 - 1.56902136656e+11 / dT ) / dT ) / dT ) / dT ) / dT;

         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         pMt2[i] = -3.13856263180e-07 - ( 3.93548484898e-03 - ( 1.97010281562e+01 - ( 1.57519830357e+04 - ( 6.23359317197e+06 - 5.54337618755e+08 / dT ) / dT ) / dT ) / dT ) / dT;

         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         pMt3[i] = -4.37734458244e-09 - ( 4.25830115234e-06 + ( 1.37165066060e-02 - ( 1.39935919264e+01 - ( 7.11514857923e+03 - 6.10899342529e+05 / dT ) / dT ) / dT ) / dT ) / dT;

         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         pMt4[i] = -3.79483685546e-11 + ( 1.19652284399e-07 - ( 1.02546710426e-04 - ( 4.21385747841e-02 - ( 6.55368785039e+00 - 5.36060258568e+02 / dT ) / dT ) / dT ) / dT ) / dT;

         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         pMt5[i] = 4.14422162573e-14 - ( 1.12046571348e-10 - ( 1.01389763575e-07 - ( 4.40098272612e-05 - ( 8.75103585798e-03 - 7.10460325286e-01 / dT ) / dT ) / dT ) / dT ) / dT;

         /* Density correction term derivatives */
         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         pDMt1[i] = -( 1.13777696760e+01 - ( 2.53472964964E+04 - ( 1.97874146029E+07 - ( 7.40189462608E+09 - 7.84510683280E+11 / dT ) / dT ) / dT ) / dT ) / dTr2;

         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         pDMt2[i] = ( 3.93548484898e-03 - ( 3.94020563124E+01 - ( 4.72559491071E+04 - ( 2.49343726879E+07 - 2.77168809378E+09 / dT ) / dT ) / dT ) / dT ) / dTr2;

         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         pDMt3[i] = ( 4.25830115234e-06 + ( 2.74330132120E-02 - ( 4.19807757792E+01 - ( 2.84605943169E+04 - 3.05449671265E+06 / dT ) / dT ) / dT ) / dT ) / dTr2;

         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         pDMt4[i] = -( 1.19652284399e-07 - ( 2.05093420852E-04 - ( 1.26415724352E-01 - ( 2.62147514016E+01 - 2.68030129284E+03 / dT ) / dT ) / dT ) / dT ) / dTr2;

         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         pDMt5[i] = ( 1.12046571348e-10 - ( 2.02779527150E-07 - ( 1.32029481784E-04 - ( 3.50041434319E-02 - 3.55230162643E+00 / dT ) / dT ) / dT ) / dT ) / dTr2;
      }
   }
}


/* 
// WaterVolume
//
// Function for the volume of water 
//
// iM ** constant **
//    Number of objects
// iDrv ** constant **
//    Indicator for derivative level
//       EOS_NODRV
//          No derivatives
//       EOS_DRV_N
//          Derivatives wrt accumulations only
//       EOS_DRV_P
//          Derivatives wrt accumulations and pressure
//       EOS_DRV_T
//          Derivatives wrt accumulations, pressure, and temperature
// iTherm ** constant **
//    Indicator whether only one temperature use (0) or multiple (1)
// pP ** constant **
//    Pressures of objects
// pT ** constant **
//    Temperatures of objects
// pS ** constant **
//    Salinity correction
// pRo 
//    The mass volume of each object
// pDRodp 
//    Pressure derivative of the mass volume of each object
// pDRodt 
//    Temperature derivative of the mass volume of each object
// pDRods 
//    Salinity derivative of the mass volume of each object
// pWork1
//    Work vector
//
// 1) The temperature dependent terms are computed in 
//    WaterVolumeTerms.  Note that the loops are different 
//    as to whether or not a single temperature or multiple
//    temperatures apply
// 2) Even though extensizely modified the following information must
//    appear
//
// Authored by Bernhard Spang, The Mining Company
// URL:        http://chemengineer.miningco.com
// email:      chemengineer.guide@miningco.com
//                                              
// Water property calculations Copyright (C) 1998 by Bernhard Spang.
// All rights reserved. May be used and modified for free.
// Redistribution is also allowed if full credit is given to the
// author and a link to http://chemengineer.miningco.com is included
// Provided "as is" without warranty of any kind.
//                  
// For documentation see
// http://chemengineer.miningco.com/library/weekly/aa081798.htm
// new URL: http://www.cheresources.com/iapwsif97.shtml                 
//                         
// Reference:
// Properties of Water and Steam in SI-Units, 
// 2nd Revised and Updated Printing, Springer 1979, pp. 175 ff.
*/
void EosWater::WaterVolume( int iM, int iDrv, int iTherm, double *pP, double *pT, double *pS, double *pRo,
                            double *pDRodp, double *pDRodt, double *pDRods, double *pWork1 )
{
   double *pRt0;
   double *pRt1;
   double *pRt2;
   double *pRt3;
   double *pDRt4;
   double *pDRt5;
   double *pDRt6;
   double *pDRt7;
   double *pDRt0;
   double *pDRt1;
   double *pDRt2;
   double *pDRt3;
   double *pRt4;
   double *pRt5;
   double *pRt6;
   double *pRt7;
   double  dA;
   double  dB;
   double  dC;
   double  dD;
   double  dE;
   double  dF;
   double  dH;
   double  dG;
   double  dP;
   double  dQ;
   double  dR;
   double  dS;
   double  dT;
   double  dU;
   double  dV;
   double  dR0;
   double  dR1;
   double  dR2;
   double  dR3;
   double  dR4;
   double  dR5;
   double  dR6;
   double  dR7;
   int     i;

   /* Conversion constant */
   dV = m_dConvVolume / m_dConvDensity;

   /* Isothermal case */
   if ( iTherm == 0 )
   {
      pRt0 = pWork1 + iM;
      pDRt0 = pRt0 + 8;

      /* Set up the coefficients */
      WaterVolumeTerms( 1, EOS_NODRV, pT, pRt0, pDRt0 );

      /* Store the coefficients */
      dR0 = pRt0[0];
      dR1 = pRt0[1];
      dR2 = pRt0[2];
      dR3 = pRt0[3];
      dR4 = pRt0[4];
      dR5 = pRt0[5];
      dR6 = pRt0[6];
      dR7 = pRt0[7];

      /* Form terms */
      for ( i = 0; i < iM; i++ )
      {
         dP = pP[i] / m_dConvPressure;
         dA = dR7 + 4.4989682369e-9 * dP;
         dB = sqrt( dA );
         dC = dR6 + dB;
         dD = dR5 / pow( dC, 0.294117647059 );
         dE = 1.0 + 6.4557404117e-9 * dP;
         dF = dE * dE;
         dG = dR4 / dF / dF;

         /* Form term for pressure derivative */
         pWork1[i] = 6.6161297601e-10 * dB * dD / dA / dC + 2.5822961647e-8 * dG / dE;

         /* Volume */
         pRo[i] = dV * ( dD + dG + dP * ( dP * ( dP * dR3 + dR2 ) + dR1 ) + dR0 ) / pS[i];
      }

      /* Derivatives */
      if ( iDrv )
      {
         for ( i = 0; i < iM; i++ )
         {
            pDRods[i] = -pRo[i] / pS[i];
         }

         if ( iDrv >= EOS_DRV_P )
         {
            dU = dV / m_dConvPressure;
            for ( i = 0; i < iM; i++ )
            {
               dP = pP[i] / m_dConvPressure;
               pDRodp[i] = dU * ( dP * ( dP * 3.0 * dR3 + dR2 + dR2 ) + dR1 - pWork1[i] ) / pS[i];
            }

            if ( iDrv >= EOS_DRV_T )
            {
               for ( i = 0; i < iM; i++ )
               {
                  pDRodt[i] = 0.0;
               }
            }
         }
      }
   }

   /* Thermal case */
   else
   {
      /* Set up memory */
      pRt0 = pWork1 + iM;
      pRt1 = pRt0 + iM;
      pRt2 = pRt1 + iM;
      pRt3 = pRt2 + iM;
      pRt4 = pRt3 + iM;
      pRt5 = pRt4 + iM;
      pRt6 = pRt5 + iM;
      pRt7 = pRt6 + iM;
      pDRt0 = pRt7 + iM;
      pDRt1 = pDRt0 + iM;
      pDRt2 = pDRt1 + iM;
      pDRt3 = pDRt2 + iM;
      pDRt4 = pDRt3 + iM;
      pDRt5 = pDRt4 + iM;
      pDRt6 = pDRt5 + iM;
      pDRt7 = pDRt6 + iM;

      /* Set up the coefficients */
      WaterVolumeTerms( iM, EOS_DRV_T, pT, pRt0, pDRt0 );

      /* Form terms */
      for ( i = 0; i < iM; i++ )
      {
         dP = pP[i] / m_dConvPressure;
         dA = pRt7[i] + 4.4989682369e-9 * dP;
         dB = sqrt( dA );
         dC = pRt6[i] + dB;
         dQ = pDRt6[i] + 0.5 * pDRt7[i] / dB;
         dR = pow( dC, 0.294117647059 );
         dD = pRt5[i] / dR;
         dT = -0.294117647059 * dQ * dD / dC;
         dE = 1.0 + 6.4557404117e-9 * dP;
         dF = dE * dE;
         dS = dF * dF;
         dG = pRt4[i] / dS;
         dH = dG + dD;

         /* Form terms for derivative */
         pRt4[i] = 6.6161297601e-10 * dB * dD / dA / dC + 2.5822961647e-8 * dG / dE;
         pRt5[i] = dS;
         pRt6[i] = dT;

         /* Function */
         pRo[i] = dV * ( dH + dP * ( dP * ( dP * pRt3[i] + pRt2[i] ) + pRt1[i] ) + pRt0[i] ) / pS[i];
         pWork1[i] = dP;
      }

      /* Derivatives */
      if ( iDrv )
      {
         for ( i = 0; i < iM; i++ )
         {
            pDRods[i] = -pRo[i] / pS[i];
         }

         if ( iDrv >= EOS_DRV_P )
         {
            dU = dV / m_dConvPressure;
            for ( i = 0; i < iM; i++ )
            {
               dP = pWork1[i];
               pDRodp[i] = dU * ( dP * ( dP * 3.0 * pRt3[i] + pRt2[i] + pRt2[i] ) + pRt1[i] - pRt4[i] ) / pS[i];
            }

            if ( iDrv >= EOS_DRV_T )
            {
               dU = dV / m_dConvTemperature;
               for ( i = 0; i < iM; i++ )
               {
                  dP = pWork1[i];

                  /* TODO: The next line is too long. Please rewrite to make it shorter. */
                  pDRodt[i] = dU * ( pRt6[i] + pDRt4[i] / pRt5[i] + dP * ( dP * ( dP * pDRt3[i] + pDRt2[i] ) + pDRt1[i] ) + pDRt0[i] ) / pS[i];
               }
            }
         }
      }
   }
}


/* 
// WaterVolumeTerms
// 
// Function to return coefficients for water volume correlations 
//
// iM ** constant **
//    Number of objects
// iDrv ** constant **
//    Indicator for derivative level
//       EOS_NODRV
//          No derivatives
//       EOS_DRV_N
//          Derivatives wrt accumulations only
//       EOS_DRV_P
//          Derivatives wrt accumulations and pressure
//       EOS_DRV_T
//          Derivatives wrt accumulations, pressure, and temperature
// pT ** constant **
//    Temperatures of object
// pWTerms
//    Terms for the water correlations
// pDTerms
//    Derivatives of terms for the water correlations
//
// 1) Even though extensizely modified the following information must
//    appear
//
// Authored by Bernhard Spang, The Mining Company
// URL:        http://chemengineer.miningco.com
// email:      chemengineer.guide@miningco.com
//                                              
// Water property calculations Copyright (C) 1998 by Bernhard Spang.
// All rights reserved. May be used and modified for free.
// Redistribution is also allowed if full credit is given to the
// author and a link to http://chemengineer.miningco.com is included
// Provided "as is" without warranty of any kind.
//                  
// For documentation see
// http://chemengineer.miningco.com/library/weekly/aa081798.htm
// new URL: http://www.cheresources.com/iapwsif97.shtml                 
//                         
// Reference:
// Properties of Water and Steam in SI-Units, 
// 2nd Revised and Updated Printing, Springer 1979, pp. 175 ff.
*/
void EosWater::WaterVolumeTerms( int iM, int iDrv, double *pT, double *pWTerms, double *pDTerms )
{
   double *pDRt0;
   double *pDRt1;
   double *pDRt2;
   double *pDRt3;
   double *pRt4;
   double *pRt5;
   double *pRt6;
   double *pRt7;
   double *pRt0;
   double *pRt1;
   double *pRt2;
   double *pRt3;
   double *pDRt4;
   double *pDRt5;
   double *pDRt6;
   double *pDRt7;
   double  dT;
   double  dT1;
   double  dT2;
   double  dT3;
   double  dT4;
   double  dTr;
   double  dTr2;
   double  dTr6;
   double  dTr12;
   double  dTr18;
   double  dA;
   double  dB;
   double  dC;
   double  dD;
   double  dE;
   double  dF;
   double  dG;
   double  dH;
   double  dI;
   int     i;

   /* Set up memory for case without derivatives */
   if ( iDrv < EOS_DRV_T )
   {
      pRt0 = pWTerms;
      pRt1 = pRt0 + iM;
      pRt2 = pRt1 + iM;
      pRt3 = pRt2 + iM;
      pRt4 = pRt3 + iM;
      pRt5 = pRt4 + iM;
      pRt6 = pRt5 + iM;
      pRt7 = pRt6 + iM;

      /* Form temperature terms */
      for ( i = 0; i < iM; i++ )
      {
         dT = pT[i] / m_dConvTemperature;
         dTr = dT / 647.3;
         dTr2 = dTr * dTr;
         dTr6 = dTr2 * dTr2 * dTr2;
         dTr12 = dTr6 * dTr6;
         dTr18 = dTr6 * dTr12;

         /* Other terms */
         dC = 0.65371543 - dTr;
         dD = dC * dC;
         dE = dD * dD;
         dA = dE * dE * dD;
         dB = 0.00000115 + dTr18 * dTr;
         dD = 0.76766209995 * dA;
         dE = 4.025e-13 / dB;
         dC = 1.5108e-5 + dTr12 / dTr;
         dA = ( 0.14188 + dTr2 ) * ( 4.1002109308e-2 * dTr18 );

         /* Polynomial terms */
         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         pRt0[i] = 7.24116460118e-5 * dTr2 + 4.8260453743e-6 * dTr - 8.29453274231e-5 + dD + dE - 6.57723749976e-10 / dC - 0.0002995284926 * dA;
         pRt1[i] = -6.2311433178e-18 / dC;
         pRt2[i] = 2.5424800232e-22 * ( 0.204 - dTr ) - 2.1490204744e-26 / dC;
         pRt3[i] = 7.0851465131e-38 / dTr18 / dTr2;
         pRt4[i] = 1.2475155875e-3 * dA;
         pRt5[i] = 0.00125914784989;

         /* Nonlinear terms */
         dC = 1.0 - 0.8438375405 * dTr2 - 0.000536216216 / dTr6;
         pRt7[i] = 1.72 * dC * dC - 0.14684556978 * dTr;
         pRt6[i] = dC;
      }
   }

   /* Temperature derivatives */
   else
   {
      pRt0 = pWTerms;
      pRt1 = pRt0 + iM;
      pRt2 = pRt1 + iM;
      pRt3 = pRt2 + iM;
      pRt4 = pRt3 + iM;
      pRt5 = pRt4 + iM;
      pRt6 = pRt5 + iM;
      pRt7 = pRt6 + iM;
      pDRt0 = pDTerms;
      pDRt1 = pDRt0 + iM;
      pDRt2 = pDRt1 + iM;
      pDRt3 = pDRt2 + iM;
      pDRt4 = pDRt3 + iM;
      pDRt5 = pDRt4 + iM;
      pDRt6 = pDRt5 + iM;
      pDRt7 = pDRt6 + iM;

      /* Form temperature terms */
      for ( i = 0; i < iM; i++ )
      {
         dT = pT[i] / m_dConvTemperature;
         dTr = dT / 647.3;
         dTr2 = dTr * dTr;
         dTr6 = dTr2 * dTr2 * dTr2;
         dTr12 = dTr6 * dTr6;
         dTr18 = dTr6 * dTr12;

         /* Other terms */
         dC = 0.65371543 - dTr;
         dD = dC * dC;
         dE = dD * dD;
         dT1 = dE * dE;
         dA = dT1 * dD;
         dT2 = dTr18 * dTr;
         dB = 0.00000115 + dT2;
         dD = 0.76766209995 * dA;
         dG = -7.6766209995 * dT1 * dC / 647.3;
         dE = 4.025e-13 / dB;
         dH = -19.0 * dT2 * dE / dB / dT;
         dT1 = dTr12 / dTr;
         dC = 1.5108e-5 + dT1;
         dF = -11.0 * dT1 / dC / dC / dT;
         dT1 = 0.14188 + dTr2;
         dT2 = 4.1002109308e-2 * dTr18;
         dA = dT1 * dT2;
         dI = dT2 * ( 18.0 * dT1 + dTr2 + dTr2 ) / dT;

         /* Polynomial terms */
         dT1 = 1.2475155875e-3;
         dT2 = -6.2311433178e-18;
         dT3 = 7.24116460118e-5 * dTr2;
         dT4 = 4.8260453743e-6 * dTr;
         pRt0[i] = dT3 + dT4 - 8.29453274231e-5 + dD + dE - 6.57723749976e-10 / dC - 0.0002995284926 * dA;
         pRt1[i] = dT2 / dC;
         pRt2[i] = 2.5424800232e-22 * ( 0.204 - dTr ) - 2.1490204744e-26 / dC;
         pRt3[i] = 7.0851465131e-38 / dTr18 / dTr2;
         pRt4[i] = dT1 * dA;
         pRt5[i] = 0.00125914784989;

         /* Derivatives */
         pDRt0[i] = dG + dH + ( dT3 + dT3 + dT4 ) / dT - 6.57723749976e-10 * dF - 0.0002995284926 * dI;
         pDRt1[i] = dT2 * dF;
         pDRt2[i] = -2.5424800232e-22 * dTr / dT - 2.1490204744e-26 * dF;
         pDRt3[i] = -20.0 * pRt3[i] / dT;
         pDRt4[i] = dT1 * dI;
         pDRt5[i] = 0.0;

         /* Nonlinear terms */
         dC = 1.0 - 0.8438375405 * dTr2 - 0.000536216216 / dTr6;
         dD = ( 3.217297296e-03 / dTr6 - 1.687675081 * dTr2 ) / dT;
         dA = 0.14684556978 * dTr;
         pRt6[i] = dC;
         pRt7[i] = 1.72 * dC * dC - dA;
         pDRt6[i] = dD;
         pDRt7[i] = 3.44 * dC * dD - dA / dT;
      }
   }
}


/* 
// SaltVolume
//
// Function for calculating the salt partial volume
//
// iM ** constant **
//    Number of objects
// iDrv ** constant **
//    Indicator for derivative level
//       EOS_NODRV
//          No derivatives
//       EOS_DRV_N
//          Derivatives wrt accumulations only
//       EOS_DRV_P
//          Derivatives wrt accumulations and pressure
//       EOS_DRV_T
//          Derivatives wrt accumulations, pressure, and temperature
// iTherm ** constant **
//    Indicator whether only one temperature use (0) or multiple (1)
// pP ** constant **
//    Pressures of objects
// pT ** constant **
//    Temperatures of objects
// pS 
//    Salinity correction expressed as 1 + PPM / 1000000
// pVol 
//    On input the water component contribution to the volume
//    On output the volume
// pDVoldp 
//    On input the water component contribution to the volume
//    pressure derivative.  On output the volume pressure
//    derivative
// pDVoldt 
//    On input the water component contribution to the volume
//    temperature derivative.  On output the volume temperature
//    derivative
// pDVolds 
//    On input the water component contribution to the volume
//    salinity derivative.  On output the volume salinity
//    derivative
//
// 1) This correlation is formed from data involving 
//    pressure, temperature, and salinity of the UNESCO International 
//    Equation of State (IES 80) as described in Fofonoff, JGR, 
//    Vol 90 No. C2, pp 3332-3342, March 20, 1985. The range
//    over which it was it was 0-40 C, 0-600 bars, and 0-50 PPT
//    After this it is an extrapolation.  It was also checked
//    against data from the Kansas Geological Survey, with
//    range of 1000-7000 PSI, 200-270 F, and 0-200000 PPM.  The
//    following line must hence appear: "The source of this material 
//    is the Kansas Geological Survey website at http://www.kgs.ku.edu/. 
//    All Rights Reserved."
// 2) On input the salinity correction and accumulation 
//    derivatives are expressed as
//
//    Salinity = 1 + PPM / 1000000
//
//    the inverse of which is the water mass fraction
// 
// 3) The total volume of the mixture is expressed as
//
//    V = Vw * Xw + Vs * Xs
//
//    or in terms of salinity
//
//    V = Vw / S + Vs * ( 1 - 1 / S )
//
//    The volume terms already contain the water part of
//    the equation when this routine is called
// 4) The partial volume of the salt in MKS units is then
//
//    Vs = 4.61894E-04 - ( 1.92000E-04 + 15.4406 * 
//         exp ( -4.44449E-02 * T ) * exp ( - P *
//             ( 2.81050E-09 + 4.27561E-05 * 
//         exp ( -3.77155E-02 * T ) ) )
//
//    or in terms of actual units used
//
//    Vs = dV0 + ( dV + dVT * exp ( dT * T ) ) * 
//         exp ( P * ( dP + dPT * exp ( dTP * T ) ) )
//
// 5) Derivatives are easy....
*/
void EosWater::SaltVolume( int iM, int iDrv, int iTherm, double *pP, double *pT, double *pS, double *pVol,
                           double *pDVoldp, double *pDVoldt, double *pDVolds )
{
   double dA;
   double dB;
   double dC;
   double dD;
   double dE;
   double dF;
   double dG;
   double dH;
   double dI;
   double dJ;
   double dV0;
   double dV;
   double dVT;
   double dT;
   double dTP;
   double dP;
   double dPT;
   int    i;

   /* Constant terms */
   dV0 = m_dConvVolume / m_dConvDensity;
   dV = -0.000192 * dV0;
   dVT = -15.4406 * dV0;
   dP = -2.8105e-9 / m_dConvPressure;
   dPT = -4.27561e-5 / m_dConvPressure;
   dT = -0.0444449 / m_dConvTemperature;
   dTP = -0.0377155 / m_dConvTemperature;
   dV0 *= 4.61894e-4;

   /* Loop with thermal effects */
   if ( iTherm )
   {
      /* Temperature derivatives */
      if ( iDrv >= EOS_DRV_T )
      {
         for ( i = 0; i < iM; i++ )
         {
            dA = pT[i];
            dB = pP[i];
            dC = dVT * exp( dT * dA );
            dD = dPT * exp( dTP * dA );
            dE = dV + dC;
            dF = dP + dD;
            dG = exp( dB * dF );
            dE *= dG;
            dH = dV0 + dE;
            dI = 1.0 / pS[i];
            dJ = 1.0 - dI;
            pVol[i] += dJ * dH;
            pDVolds[i] += dH * dI * dI;
            pDVoldp[i] += dJ * dE * dF;
            pDVoldt[i] += dJ * ( dG * dT * dC + dE * dD * dTP * dB );
         }
      }

      /* Pressure derivatives */
      else if ( iDrv >= EOS_DRV_P )
      {
         for ( i = 0; i < iM; i++ )
         {
            dA = pT[i];
            dF = dP + dPT * exp( dTP * dA );
            dE = ( dV + dVT * exp( dT * dA ) ) * exp( pP[i] * dF );
            dH = dV0 + dE;
            dI = 1.0 / pS[i];
            dJ = 1.0 - dI;
            pVol[i] += dJ * dH;
            pDVolds[i] += dH * dI * dI;
            pDVoldp[i] += dJ * dE * dF;
         }
      }

      /* Derivatives */
      else if ( iDrv )
      {
         for ( i = 0; i < iM; i++ )
         {
            dA = pT[i];
            dB = dV0 + ( dV + dVT * exp( dT * dA ) ) * exp( pP[i] * ( dP + dPT * exp( dTP * dA ) ) );
            dI = 1.0 / pS[i];
            pVol[i] += ( 1.0 - dI ) * dB;
            pDVolds[i] += dB * dI * dI;
         }
      }

      /* No derivatives */
      else
      {
         for ( i = 0; i < iM; i++ )
         {
            dA = pT[i];
            pVol[i] += ( 1.0 - 1.0 / pS[i] ) * ( dV0 + ( dV + dVT * exp( dT * dA ) ) * exp( pP[i] * ( dP + dPT * exp( dTP * dA ) ) ) );
         }
      }
   }

   /* No thermal effects */
   else
   {
      /* Further reduction in terms */
      dA = *pT;
      dC = dP + dPT * exp( dTP * dA );
      dD = dV + dVT * exp( dT * dA );

      /* Pressure derivatives */
      if ( iDrv >= EOS_DRV_P )
      {
         for ( i = 0; i < iM; i++ )
         {
            dE = dD * exp( pP[i] * dC );
            dH = dV0 + dE;
            dI = 1.0 / pS[i];
            dJ = 1.0 - dI;
            pVol[i] += dJ * dH;
            pDVolds[i] += dH * dI * dI;
            pDVoldp[i] += dJ * dE * dC;
         }
      }

      /* Derivatives */
      else if ( iDrv )
      {
         for ( i = 0; i < iM; i++ )
         {
            dB = dV0 + dD * exp( pP[i] * dC );
            dI = 1.0 / pS[i];
            pVol[i] += ( 1.0 - dI ) * dB;
            pDVolds[i] += dB * dI * dI;
         }
      }

      /* No derivatives */
      else
      {
         for ( i = 0; i < iM; i++ )
         {
            pVol[i] += ( 1.0 - 1.0 / pS[i] ) * ( dV0 + dD * exp( pP[i] * dC ) );
         }
      }
   }
}


/* 
// SaltHeat
//
// Function for calculating the salt partial enthalpy
//
// iM ** constant **
//    Number of objects
// iNc ** constant **
//    Number of components in aqueous phase
// iDrv ** constant **
//    Indicator for derivative level
//       EOS_NODRV
//          No derivatives
//       EOS_DRV_N
//          Derivatives wrt accumulations only
//       EOS_DRV_P
//          Derivatives wrt accumulations and pressure
//       EOS_DRV_T
//          Derivatives wrt accumulations, pressure, and temperature
// iTherm ** constant **
//    Indicator whether only one temperature use (0) or multiple (1)
// pP ** constant **
//    Pressures of objects
// pT ** constant **
//    Temperatures of objects
// pS 
//    Salinity correction expressed as 1 + PPM / 1000000
// pHeat
//    On input the water component contribution to the enthalpy
//    On output the enthalpy
// pDHeatdp 
//    On input the water component contribution to the enthalpy
//    pressure derivative.  On output the enthalpy pressure
//    derivative
// pDHeatdt 
//    On input the water component contribution to the enthalpy
//    temperature derivative.  On output the enthalpy temperature
//    derivative
// pDHeatds 
//    On input the water component contribution to the heat
//    salinity derivative.  On output the heat salinity
//    derivative
//
// 1) This correlation is formed from data involving 
//    pressure, temperature, and salinity of the UNESCO International 
//    Equation of State (IES 80) as described in Fofonoff, JGR, 
//    Vol 90 No. C2, pp 3332-3342, March 20, 1985. The range
//    over which it was it was 0-40 C, 0-600 bars, and 0-50 PPT
//    After this it is an extrapolation.  It was also checked
//    against data from the Kansas Geological Survey, with
//    range of 1000-7000 PSI, 200-270 F, and 0-200000 PPM.  The
//    following line must hence appear: "The source of this material 
//    is the Kansas Geological Survey website at http://www.kgs.ku.edu/. 
//    All Rights Reserved."
// 2) On input the salinity correction and accumulation 
//    derivatives are expressed as
//
//    Salinity = 1 + PPM / 1000000
//
//    the inverse of which is the water mass fraction
// 
// 3) The total heat of the mixture is expressed as
//
//    H = Hw * Xw + Hs * Xs
//
//    or in terms of salinity
//
//    H = Hw / S + Hs * ( 1 - 1 / S )
//
//    The heat terms already contain the water part of
//    the equation when this routine is called
// 4) The partial volume of the salt in MKS units is 
//
//    Vs = 4.61894E-04 - ( 1.92000E-04 + 15.4406 * 
//         exp ( -4.44449E-02 * T ) * exp ( - P *
//             ( 2.81050E-09 + 4.27561E-05 * 
//         exp ( -3.77155E-02 * T ) ) )
//
//    or in terms of actual units used
//
//    Vs = dV0 + ( dV1 + dV2 * exp ( dV3 * T ) ) * 
//         exp ( P * ( dV4 + dV5 * exp ( dV6 * T ) ) )
//
// 5) The heat is then
//
//    Hs = Int ( V - T dVdT ) dP
//
// 6) The ideal term is approximated as -834 * T
//
// 7) Derivatives are easy....
*/
void EosWater::SaltHeat( int iM, int iDrv, int iTherm, double *pP, double *pT, double *pS, double *pHeat,
                         double *pDHeatdp, double *pDHeatdt, double *pDHeatds )
{
   double dA;
   double dB;
   double dC;
   double dD;
   double dE;
   double dF;
   double dG;
   double dH;
   double dI;
   double dJ;
   double dK;
   double dV0;
   double dV1;
   double dV2;
   double dV3;
   double dV4;
   double dV5;
   double dV6;
   double dH0;
   int    i;

   /* Constant terms */
   dV0 = m_dConvHeat / m_dConvDensity;
   dV1 = -0.000192 * dV0;
   dV2 = -15.4406 * dV0;
   dV4 = -2.8105e-9;
   dV5 = -4.27561e-5;
   dV3 = -0.0444449;
   dV6 = -0.0377155;
   dV0 *= 4.61894e-4;
   dH0 = 834.0;

   /* Loop with thermal effects */
   if ( iTherm )
   {
      /* Temperature derivatives */
      if ( iDrv >= EOS_DRV_T )
      {
         for ( i = 0; i < iM; i++ )
         {
            dA = pT[i] / m_dConvTemperature;
            dB = pP[i] / m_dConvPressure;
            dD = dV2 * exp( dV3 * dA );
            dE = dV5 * exp( dV6 * dA );
            dF = dV1 + dD;
            dG = dV4 + dE;
            dH = exp( dB * dG );
            dF *= dH;
            dD = dA * dV3 * dD * dH;
            dE *= dV6;
            dJ = dA * dE * dF;
            dH = dB - 1.0 / dG;
            dF -= ( dD + dJ * dB );
            dI = dV0 * dB + ( dF + dJ / dG ) / dG - dH0 * dA;
            dA = 1.0 / pS[i];
            dC = 1.0 - dA;
            pHeat[i] += dC * dI;
            pDHeatds[i] += dA * dA * dI;
            pDHeatdp[i] += dC * ( dV0 + dF ) / m_dConvPressure;

            /* TODO: The next line is too long. Please rewrite to make it shorter. */
            pDHeatdt[i] -= dC * ( ( dD * ( dV3 + ( dE + dE ) * dH ) + dJ * ( dV6 * dH + dE * ( dB * dB - ( dH + dH ) / dG ) ) ) / dG + dH0 ) / m_dConvTemperature;
         }
      }

      /* Pressure derivatives */
      else if ( iDrv >= EOS_DRV_P )
      {
         for ( i = 0; i < iM; i++ )
         {
            dA = pT[i] / m_dConvTemperature;
            dB = pP[i] / m_dConvPressure;
            dD = dV2 * exp( dV3 * dA );
            dE = dV5 * exp( dV6 * dA );
            dF = dV1 + dD;
            dG = dV4 + dE;
            dH = exp( dB * dG );
            dF *= dH;
            dJ = dA * dV6 * dE * dF;
            dF -= ( dA * dV3 * dD * dH + dJ * dB );
            dI = dV0 * dB + ( dF + dJ / dG ) / dG - dH0 * dA;
            dA = 1.0 / pS[i];
            dC = 1.0 - dA;
            pHeat[i] += dC * dI;
            pDHeatds[i] += dA * dA * dI;
            pDHeatdp[i] += dC * ( dV0 + dF ) / m_dConvPressure;
         }
      }

      /* Derivatives */
      else if ( iDrv )
      {
         for ( i = 0; i < iM; i++ )
         {
            dA = pT[i] / m_dConvTemperature;
            dB = pP[i] / m_dConvPressure;
            dD = dV2 * exp( dV3 * dA );
            dE = dV5 * exp( dV6 * dA );
            dF = dV1 + dD;
            dG = dV4 + dE;
            dI = dV0 * dB + exp( dB * dG ) * ( dF - dA * ( dD * dV3 + dV6 * dE * dF * ( dB - 1.0 / dG ) ) ) / dG - dH0 * dA;
            dA = 1.0 / pS[i];
            pHeat[i] += ( 1.0 - dA ) * dI;
            pDHeatds[i] += dA * dA * dI;
         }
      }

      /* No derivatives */
      else
      {
         for ( i = 0; i < iM; i++ )
         {
            dA = pT[i] / m_dConvTemperature;
            dB = pP[i] / m_dConvPressure;
            dD = dV2 * exp( dV3 * dA );
            dE = dV5 * exp( dV6 * dA );
            dF = dV1 + dD;
            dG = dV4 + dE;

            /* TODO: The next line is too long. Please rewrite to make it shorter. */
            pHeat[i] += ( 1.0 - 1.0 / pS[i] ) * ( dV0 * dB + exp( dB * dG ) * ( dF - dA * ( dD * dV3 + dV6 * dE * dF * ( dB - 1.0 / dG ) ) ) / dG - dH0 * dA );
         }
      }
   }

   /* No thermal effects */
   else
   {
      /* Terms that can be saved */
      dA = *pT / m_dConvTemperature;
      dD = dV2 * exp( dV3 * dA );
      dE = dV5 * exp( dV6 * dA );
      dF = dV1 + dD;
      dG = dV4 + dE;
      dJ = dA * dV6 * dE * dF;
      dF -= ( dA * dV3 * dD );
      dK = dF + dJ / dG;
      dH0 *= dA;

      /* Pressure derivatives */
      if ( iDrv >= EOS_DRV_P )
      {
         for ( i = 0; i < iM; i++ )
         {
            dB = pP[i] / m_dConvPressure;
            dH = exp( dB * dG );
            dI = dV0 * dB + dH * ( dK - dB * dJ ) / dG - dH0;
            dA = 1.0 / pS[i];
            dC = 1.0 - dA;
            pHeat[i] += dC * dI;
            pDHeatds[i] += dA * dA * dI;
            pDHeatdp[i] += dC * ( dV0 + dH * ( dF - dB * dJ ) ) / m_dConvPressure;
         }
      }

      /* Derivatives */
      else if ( iDrv )
      {
         for ( i = 0; i < iM; i++ )
         {
            dB = pP[i] / m_dConvPressure;
            dI = dV0 * dB + exp( dB * dG ) * ( dK - dB * dJ ) / dG - dH0;
            dA = 1.0 / pS[i];
            pHeat[i] += ( 1.0 - dA ) * dI;
            pDHeatds[i] += dA * dA * dI;
         }
      }

      /* No derivatives */
      else
      {
         for ( i = 0; i < iM; i++ )
         {
            dB = pP[i] / m_dConvPressure;
            pHeat[i] += ( 1.0 - 1.0 / pS[i] ) * ( dV0 * dB + exp( dB * dG ) * ( dK - dB * dJ ) / dG - dH0 );
         }
      }
   }
}


/* 
// Water
//
// Function for water properties
//
// iM ** constant **
//    Number of objects
// iNc ** constant **
//    Number of components in aqueous phase
// iDrv ** constant **
//    Indicator for derivative level
//       EOS_NODRV
//          No derivatives
//       EOS_DRV_N
//          Derivatives wrt accumulations only
//       EOS_DRV_P
//          Derivatives wrt accumulations and pressure
//       EOS_DRV_T
//          Derivatives wrt accumulations, pressure, and temperature
// iVolume ** constant **
//    Indicator whether volumes or density required
//       EOS_DENSITY
//          Get density
//       EOS_VOLUME
//          Get volume
// iGetV ** constant **
//    Indicator whether viscosity required
// iGetT ** constant **
//    Indicator whether tension required
// iGetH ** constant **
//    Indicator whether enthalpy required
// iTherm ** constant **
//    Indicator whether only one temperature use (0) or multiple (1)
// pP ** constant **
//    Pressures of objects
// pT ** constant **
//    Temperatures of objects
// pS ** constant **
//    Salinity correction
// pDSda ** constant **
//    Accumulation derivative of salinity
//       first index corresponds to object
//       second index corresponds to component
// pRo 
//    The mass volume or density of each object
// pDRodp 
//    Pressure derivative of the mass volume or density of each object
// pDRodt 
//    Temperature derivative of the mass volume or density of each 
//    object
// pDRods 
//    Salinity derivative of the mass volume or density of each 
//    object
// pDRoda 
//    Accumulation derivative of mass volume or density of each object
//       first index corresponds to object
//       second index corresponds to component
// pMu 
//    The viscosity of each object
// pDMudp 
//    Pressure derivative of viscosity of each object
// pDMudt 
//    Temperature derivative of viscosity of each object
// pDMuds 
//    Salinity derivative of viscosity of each object
// pDMuda 
//    Accumulation derivative of viscosity of each object
//       first index corresponds to object
//       second index corresponds to component
// pIft 
//    The interfacial tension wrt reference phase of each object
// pDIftdp 
//    Pressure derivative interfacial tension wrt reference phase of 
//    each object
// pDIftdt 
//    Temperature derivative interfacial tension wrt reference phase 
//    of each object
// pDIftda 
//    Accumulation derivative interfacial tension wrt reference phase
//    of each object
//       first index corresponds to object
//       second index corresponds to component
// pHeat 
//    The enthalpy of each object
// pDHeatdp 
//    Pressure derivative of enthalpy of each object
// pDHeatdt 
//    Temperature derivative of enthalpy of each object
// pDHeatds 
//    Salinity derivative of enthalpy of each object
// pDHeatda 
//    Accumulation derivative of enthalpy of each object
//       first index corresponds to object
//       second index corresponds to component
// pWork1
//    Work vector
//
// 1) Get the pure water density or volume
// 2) Call the viscosity routine based upon water partial volume
// 3) Add in the salt volume
// 4) Get IFT if required
// 5) Get enthalpy if required
// 6) All correlations herein are based upon public domain
//    software.  Even though the software extensizely modified 
//    the following information must appear
//
// Authored by Bernhard Spang, The Mining Company
// URL:        http://chemengineer.miningco.com
// email:      chemengineer.guide@miningco.com
//                                              
// Water property calculations Copyright (C) 1998 by Bernhard Spang.
// All rights reserved. May be used and modified for free.
// Redistribution is also allowed if full credit is given to the
// author and a link to http://chemengineer.miningco.com is included
// Provided "as is" without warranty of any kind.
//                  
// For documentation see
// http://chemengineer.miningco.com/library/weekly/aa081798.htm
// new URL: http://www.cheresources.com/iapwsif97.shtml                 
//                         
// Reference:
// Properties of Water and Steam in SI-Units, 
// 2nd Revised and Updated Printing, Springer 1979, pp. 175 ff.
*/
void EosWater::Water( int iM, int iNc, int iDrv, int iVolume, int iGetV, int iGetT, int iGetH, int iTherm, double *pP,
                      double *pT, double *pS, double *pDSda, double *pRo, double *pDRodp, double *pDRodt,
                      double *pDRods, double *pDRoda, double *pMu, double *pDMudp, double *pDMudt, double *pDMuds,
                      double *pDMuda, double *pIft, double *pDIftdp, double *pDIftdt, double *pDIftda, double *pHeat,
                      double *pDHeatdp, double *pDHeatdt, double *pDHeatds, double *pDHeatda, double *pWork1 )
{
   /* Get the water density */
   if ( iVolume != EOS_SEPARATOR && m_iWaterModel != EOS_WATER_LINEAR )
   {
      WaterVolume( iM, iDrv, iTherm, pP, pT, pS, pRo, pDRodp, pDRodt, pDRods, pWork1 );
   }

   /* Get the water viscosity */
   if ( iGetV )
   {
      /* TODO: The next line is too long. Please rewrite to make it shorter. */
      WaterViscosity( iM, iNc, iDrv, iTherm, pP, pT, pS, pDSda, pRo, pDRodp, pDRodt, pDRods, pMu, pDMudp, pDMudt, pDMuds, pDMuda, pWork1 );
   }

   /* Adjust for the salt terms */
   if ( iVolume != EOS_SEPARATOR && m_iWaterModel != EOS_WATER_LINEAR )
   {
      SaltVolume( iM, iDrv, iTherm, pP, pT, pS, pRo, pDRodp, pDRodt, pDRods );
   }

   /* Compute the rest of the density terms */
   if ( iVolume != EOS_SEPARATOR )
   {
      WaterDensity( iM, iNc, iDrv, iVolume, pP, pT, pS, pDSda, pRo, pDRodp, pDRodt, pDRods, pDRoda );
   }

   /* Get interfacial tension */
   if ( iGetT )
   {
      WaterIft( iM, iNc, iDrv, pIft, pDIftdp, pDIftdt, pDIftda );
   }

   /* Get enthalpy */
   if ( iGetH )
   {
      WaterHeat( iM, iNc, iDrv, iTherm, pP, pT, pS, pDSda, pHeat, pDHeatdp, pDHeatdt, pDHeatds, pDHeatda, pWork1 );
   }
}


