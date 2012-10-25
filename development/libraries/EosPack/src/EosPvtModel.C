/*********************************************************************
*                                                                    * 
* Confidentiality:                                                   *
*              This is a CONFIDENTIAL document.                      *
*              Copyright is vested in  Shell Internationale Research *
*              Mij. B.V., The Hague.                                 *
*              Neither the  whole  or any  part of this document may *
*              be  reproduced,  stored in  any retrieval  system  or *
*              transmitted in any form or by any means  (electronic, *
*              mechanical,  reprographic,  recording  or  otherwise) *
*              without the  prior  consent of the  copyright  owner. *
*                                                                    *
*  The following information MUST appear in the file for methods     *
*  used for the aqueous phase:                                       *
*                                                                    *
*  Authored by Bernhard Spang, The Mining Company                    *
*  URL:        http://chemengineer.miningco.com                      *
*  email:      chemengineer.guide@miningco.com                       *
*                                                                    *
*  Water property calculations Copyright (C) 1998 by Bernhard Spang. *
*  All rights reserved. May be used and modified for free.           *
*  Redistribution is also allowed if full credit is given to the     *
*  author and a link to http://chemengineer.miningco.com is included *
*  Provided "as is" without warranty of any kind.                    *
*                                                                    *
*  For documentation see                                             *
*  http://chemengineer.miningco.com/library/weekly/aa081798.htm      *
*  new URL: http://www.cheresources.com/iapwsif97.shtml              *   
*                                                                    *
*  Reference:                                                        *
*  Properties of Water and Steam in SI-Units,                        *
*  2nd Revised and Updated Printing, Springer 1979, pp. 175 ff.      *
*                                                                    *
* Package:        MoReS Common System Package                        *
*                                                                    *
* Dependencies:   No platform dependencies                           *
*                                                                    *
* Usage notes:    See Technical Documentation                        *
*                                                                    *
*  Include file for classes is EosPvtModel.h                         *
*  Source module is EosPvtModel.cpp                                  *
*                                                                    *
*********************************************************************/

#include "stdafx.h"
#include "EosPvtModel.h"

/* 
// Do NOT include any application files here - standard include ONLY
// to keep EOS application stand-alone.
*/
//#include "Unit.h"

#include <assert.h>
#include <math.h>
#include <float.h>
#include <stdio.h>

/*
// New Cubic Method
*/
#define EOS_NEW_CUBIC

/*
// Use intel math library
*/
// #define EOS_INTEL
#ifdef EOS_INTEL
#include "C:/Program Files/Intel/MKL61/include/mkl.h"
#endif

/* 
// Use generic memory management 
*/
#ifdef EOS_STANDALONE
#include <stdlib.h>
#define CNEW( type, count ) (type *)malloc( ( count ) * sizeof( type ) )
#define CDELETE( pObject )  free( pObject )

/* 
// Use MoReS memory management 
*/
#else
#include "Mem.h"
#define CNEW( type, count ) (type *)MALLOC( ( count ) * sizeof( type ) )
#define CDELETE( pObject )  FREE( pObject )
#endif


/*
// Class EosUtil
*/

/* 
// Function for vector exponentiation
//
*/
void EosUtil::VectorExp( int iM, double *pInput, double *pOutput )
{
#ifdef EOS_INTEL
   vdExp( iM, pInput, pOutput );
#else
   for ( int i = 0; i < iM; i++ )
   {
      pOutput[i] = exp( pInput[i] );
   }

#endif
}


/* 
//
// Function for vector log
*/
void EosUtil::VectorLog( int iM, double *pInput, double *pOutput )
{
#ifdef EOS_INTEL
   vdLn( iM, pInput, pOutput );
#else
   for ( int i = 0; i < iM; i++ )
   {
      pOutput[i] = log( pInput[i] );
   }

#endif
}


/* 
//
// Function for vector power
*/
void EosUtil::VectorPow( int iM, double *pInput1, double dInput2, double *pOutput )
{
#ifdef EOS_INTEL
   vdPowx( iM, pInput1, dInput2, pOutput );
#else
   for ( int i = 0; i < iM; i++ )
   {
      pOutput[i] = pow( pInput1[i], dInput2 );
   }

#endif
}


/* 
// VectorPowX
//
// Function for vector power
//
// iM ** constant ** Number of objects
// pInput1 ** constant ** Variable to take power of
// pInput2 ** constant ** Exponent
// pOutput   Result
//
*/
void EosUtil::VectorPowX( int iM, double *pInput1, double *pInput2, double *pOutput )
{
#ifdef EOS_INTEL
   vdPow( iM, pInput1, pInput2, pOutput );
#else
   for ( int i = 0; i < iM; i++ )
   {
      pOutput[i] = pow( pInput1[i], pInput2[i] );
   }

#endif
}

/*
// Class EosWater
//
// EosWater
//
// Construct the water data structure
//
// For a description of data see EosPvtModel.h
*/
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

   /* TODO: The next line is too long. Please rewrite to make it shorter. */
   return ( exp( 14.03079 + dU * ( 4.771298 - dU * ( 2.31523 - dU * ( 4.973715e-1 - dU * ( 8.098531e-2 - dU * ( 8.365383e-3 + dU * ( 4.688029e-2 - dU * ( 7.770678e-2 + dU * ( 2.587746e-2 - dU * ( 7.086141e-2 + dU * ( 7.834283e-3 - dU * 2.145744e-2 ) ) ) ) ) ) ) ) ) ) ) * m_dConvPressure );
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

      EosUtil::VectorExp( iM, pMu, pMu );
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

      EosUtil::VectorExp( iM, pMu, pMu );
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


/*
// Class EosPvtTable
*/


/* 
// EosPvtTable
//
// Construction of cached pvttable. 
//
// See EosPvtModel.h EosPvtTable__Constructor for argument description
//
*/
EosPvtTable::EosPvtTable( void )
{
   Initialize( 0, NULL, NULL, NULL, NULL, NULL );
}


/* 
// EosPvtTable
//
// Construction of cached pvttable. 
//
// See EosPvtModel.h EosPvtTable__Constructor for argument description
//
*/
EosPvtTable::EosPvtTable( int iVersion, int *piTables, double *pdTables, double *pCritical, double *pvtData,
                          double *pTSaltMW )
{
   Initialize( iVersion, piTables, pdTables, pCritical, pvtData, pTSaltMW );
}


/* 
// EosPvtTable
//
// Make a copy of the cache
//
// Returns a pointer to the new cache
//
// pCache
//    Pointer to original cache
// dT
//    Temperature
// 
// Note: only the pointers are copied for arrays;
// and hence the original data is used from its
// original location, unless the data is reallocated
// in the copy
//
// 1) Create a new instance
// 2) Make a copy of the data
// 3) Allocate new memory
// 4) Calculate new A and B terms
*/
EosPvtTable::EosPvtTable( EosPvtTable *pCache, double dT )
{
   m_pOwnMemory = NULL;
   Copy( pCache );

   /* Create the temperature cache */
   if ( ( ( m_iMethod != EOS_PVT_TABLE ) || ( m_iTables != 0 ) ) && ( m_iMethod != EOS_PVT_IDEAL ) )
   {
      double  dA;
      double  dB;
      double  dAOverB;
      double  dP;
      double  dHelp1;
      double  dHelp2;
      double  dHelp3;
      double  dHelp4;
      double  dHelp5;
      double  dHelp6;
      double *pX;
      double *pHelp;
      int     iNi;
      int     iAbcOffset;
      int     iMultipleAbc;

      /* Get memory */
      pX = CNEW ( double, 2 *m_iEosComp );
      pHelp = pX + m_iEosComp;

      /* Reset values that are different */
      m_iMethod = EOS_PVT_TABLE;
      m_iTypeOfC = EOS_CONSTANT_C;
      m_iTables = 0;
      *m_pTemperature = dT;

      /* Set the temperatures */
      pCache->WritePvtInfo( 1, 0, 1, EOS_OPTION_OFF, EOS_OPTION_OFF, 0, &iAbcOffset, &iMultipleAbc, m_pTemperature, pX );

      /* Set some dummy terms */
      dP = 1.0 / ( (double)m_iEosComp );
      for ( iNi = 0; iNi < m_iEosComp; iNi++ )
      {
         pX[iNi] = dP;
      }

      dP = 1.0;

      /* Calculate a and b terms */
      /* TODO: The next line is too long. Please rewrite to make it shorter. */
      pCache->GetABTerms( 1, EOS_FUGACITY, EOS_NOHEAT, EOS_DRV_N, iMultipleAbc, &iAbcOffset, &dP, m_pTemperature, pX, &dA, &dB, &dAOverB, m_pCdata, m_pBdata, m_pAdata, NULL, NULL, NULL, pHelp, NULL, NULL, &dHelp1, &dHelp2, &dHelp3, &dHelp4, &dHelp5, &dHelp6 );

      /* Scale A terms */
      dA *= ( *m_pTemperature **m_pTemperature );
      for ( iNi = 0; iNi < m_iEosComp * m_iEosComp; iNi++ )
      {
         m_pAdata[iNi] *= dA;
      }

      /* Scale B terms */
      dB *= *m_pTemperature;
      for ( iNi = 0; iNi < m_iEosComp; iNi++ )
      {
         m_pBdata[iNi] *= dB;
      }

      /* Volume translation */
      if ( m_iTranslate )
      {
         pCache->VolumeTranslationC( 1, EOS_NODRV, EOS_NOHEAT, iMultipleAbc, &iAbcOffset, m_pTemperature, m_pCdata, NULL, NULL );
      }

      /* Remove memory */
      CDELETE( pX );
   }
}


/* 
// EosPvtTable
//
// Make a copy of the cache
//
// Returns a pointer to the new cache
//
// pCache
//    Pointer to original cache
*/
EosPvtTable::EosPvtTable( EosPvtTable &pCache ) :
   EosWater()
{
   /* Make the raw copy */
   Copy( &pCache );
}


/*
// Constructor for corresponding state viscosity model
// To be used for the entry point ViscosityCS
//
// iNcArg - Number of components (input)
// dCvpArg - Conversion factor for pressure from SI (input)
// dCvtArg - Conversion factor for temperature from SI (input)
// dCvvArg - Conversion factor for viscosity from SI (input)
// dTune1  - Tuning parameter 1
// dTune2  - Tuning parameter 2
// dTune3  - Tuning parameter 3
// dTune4  - Tuning parameter 4
// CSVscMethod - method number of viscosity model
// pPCArg  - Critical pressure
// pTCArg  - Critical temperature
// pMWArg  - Molecular weights
*/
EosPvtTable::EosPvtTable( int iNcArg, double dCvpArg, double dCvtArg, double dCvvArg, 
                          double dTune1, double dTune2, double dTune3, double dTune4,
                          int CSVscMethod, double *pPCArg, double *pTCArg, double *pMWArg )
{
   int iNi;

   /* Set conversion factors */
   m_dConvPressure = dCvpArg;
   m_dConvTemperature = dCvtArg;
   m_dConvViscosity = dCvvArg;

   /* Set the tuning factors */
   m_dVTerm1 = dTune1 * 1.304e-4;
   m_dVTerm2 = dTune2 * 2.303;
   m_dVTerm3 = dTune3 * 1.0;
   m_dVTerm4 = dTune4 * 1.0;
   m_CSVscMethod = CSVscMethod; 

   /* Number of components */
   m_iEosComp = iNcArg;

   /* Set up memory for cached terms */
   m_pOwnMemory = CNEW ( double, 2 *iNcArg *iNcArg +3 *iNcArg );
   m_pMolecularWeight = m_pOwnMemory;
   m_pCriticalPressure = m_pMolecularWeight + iNcArg;
   m_pCriticalTemperature = m_pCriticalPressure + iNcArg;
   m_pPCMod = m_pCriticalTemperature + iNcArg;
   m_pTCMod = m_pPCMod + iNcArg * iNcArg;

   /* Store molecular weights and compute other terms */
   for ( iNi = 0; iNi < iNcArg; iNi++ )
   {
      m_pMolecularWeight[iNi] = pMWArg[iNi];
      m_pCriticalPressure[iNi] = pPCArg[iNi];
      m_pCriticalTemperature[iNi] = pTCArg[iNi];
   }

   /* Initialize the viscosity terms */
   InitializeViscosityTerms();
}


/* 
// ~EosPvtTable
//
// Destruction of pvt cache 
//
// Returns a null pointer
// 
// See EosPvtModel.h EosPvtTable__Destructor for a description of 
// arguments
//
// 1) Delete temporary memory
// 2) Delete instance
*/
EosPvtTable::~EosPvtTable( void )
{
   /* Free work area */
   if ( m_pOwnMemory )
   {
      CDELETE( m_pOwnMemory );
      m_pOwnMemory = NULL;
   }
}


/* 
// Copy
//
// Make a copy of the cache
//
// Returns a pointer to the new cache
//
// pCache
//    Pointer to original cache
// 
// Note: only the pointers are copied for arrays;
// and hence the original data is used from its
// original location
//
// 1) Create a new instance
// 2) Make a copy of the data
*/
void EosPvtTable::Copy( EosPvtTable *pCache )
{
   int i;

   /* Make a copy of the data */
   m_iMethod = pCache->m_iMethod;
   m_iEosComp = pCache->m_iEosComp;
   m_iSaltComp = pCache->m_iSaltComp;
   m_iAllocLength = pCache->m_iAllocLength;
   m_iHeat = pCache->m_iHeat;
   m_dKb0 = pCache->m_dKb0;
   m_dKb1 = pCache->m_dKb1;
   m_dKb2 = pCache->m_dKb2;
   m_dKb3 = pCache->m_dKb3;
   m_dKb4 = pCache->m_dKb4;
   m_dKb5 = pCache->m_dKb5;
   m_dKb6 = pCache->m_dKb6;
   m_dKb7 = pCache->m_dKb7;
   m_dKb8 = pCache->m_dKb8;
   m_dKb9 = pCache->m_dKb9;
   m_dConvPressure = pCache->m_dConvPressure;
   m_dConvTemperature = pCache->m_dConvTemperature;
   m_dConvVolume = pCache->m_dConvVolume;
   m_dConvViscosity = pCache->m_dConvViscosity;
   m_dConvDensity = pCache->m_dConvDensity;
   m_dConvHeat = pCache->m_dConvHeat;
   m_dConvTension = pCache->m_dConvTension;
   m_dGravity = pCache->m_dGravity;
   m_dGascon = pCache->m_dGascon;
   m_dSalinity = pCache->m_dSalinity;
   m_dWaterIft = pCache->m_dWaterIft;
   m_iWaterModel = pCache->m_iWaterModel;
   m_dWaterDens = pCache->m_dWaterDens;
   m_dWaterDensDp = pCache->m_dWaterDensDp;
   m_dWaterDensDt = pCache->m_dWaterDensDt;
   m_dWaterDensDs = pCache->m_dWaterDensDs;
   m_dWaterDensDpDs = pCache->m_dWaterDensDpDs;
   m_dWaterDensDtDs = pCache->m_dWaterDensDtDs;
   m_dWaterVisc = pCache->m_dWaterVisc;
   m_dWaterViscDp = pCache->m_dWaterViscDp;
   m_dWaterViscDt = pCache->m_dWaterViscDt;
   m_dWaterViscDs = pCache->m_dWaterViscDs;
   m_dWaterViscDpDs = pCache->m_dWaterViscDpDs;
   m_dWaterViscDtDs = pCache->m_dWaterViscDtDs;
   m_dWaterHeat = pCache->m_dWaterHeat;
   m_dWaterHeatDs = pCache->m_dWaterHeatDs;
   m_dVTune = pCache->m_dVTune;
   m_dVTune0 = pCache->m_dVTune0;
   m_dVTune1 = pCache->m_dVTune1;
   m_dVTune2 = pCache->m_dVTune2;
   m_dVTune3 = pCache->m_dVTune3;
   m_dVTune4 = pCache->m_dVTune4;
   m_dVTerm1 = pCache->m_dVTerm1;
   m_dVTerm2 = pCache->m_dVTerm2;
   m_dVTerm3 = pCache->m_dVTerm3;
   m_dVTerm4 = pCache->m_dVTerm4;
   m_CSVscMethod = pCache->m_CSVscMethod;
   m_iTypeOfSingle = pCache->m_iTypeOfSingle;
   m_dCritZMinusB = pCache->m_dCritZMinusB;
   m_dCritAOverB = pCache->m_dCritAOverB;
   m_iTranslate = pCache->m_iTranslate;
   m_iTypeOfC = pCache->m_iTypeOfC;
   m_iTables = pCache->m_iTables;
   m_pRawAbcData = NULL;

   /* Set pointers */
   if (m_pOwnMemory)
      CDELETE (m_pOwnMemory);
   m_pOwnMemory = CNEW ( double, m_iAllocLength );
   m_pTemperature = m_pOwnMemory + ( pCache->m_pTemperature - pCache->m_pOwnMemory );
   m_pMolecularWeight = m_pOwnMemory + ( pCache->m_pMolecularWeight - pCache->m_pOwnMemory );
   m_pCriticalPressure = m_pOwnMemory + ( pCache->m_pCriticalPressure - pCache->m_pOwnMemory );
   m_pCriticalTemperature = m_pOwnMemory + ( pCache->m_pCriticalTemperature - pCache->m_pOwnMemory );
   m_pCriticalVolume = m_pOwnMemory + ( pCache->m_pCriticalVolume - pCache->m_pOwnMemory );
   m_pParachor = m_pOwnMemory + ( pCache->m_pParachor - pCache->m_pOwnMemory );
   m_pHeat0 = m_pOwnMemory + ( pCache->m_pHeat0 - pCache->m_pOwnMemory );
   m_pHeat1 = m_pOwnMemory + ( pCache->m_pHeat1 - pCache->m_pOwnMemory );
   m_pHeat2 = m_pOwnMemory + ( pCache->m_pHeat2 - pCache->m_pOwnMemory );
   m_pHeat3 = m_pOwnMemory + ( pCache->m_pHeat3 - pCache->m_pOwnMemory );
   m_pHeat4 = m_pOwnMemory + ( pCache->m_pHeat4 - pCache->m_pOwnMemory );
   m_pHeat5 = m_pOwnMemory + ( pCache->m_pHeat5 - pCache->m_pOwnMemory );
   m_pHeat6 = m_pOwnMemory + ( pCache->m_pHeat6 - pCache->m_pOwnMemory );
   m_pSaltMW = m_pOwnMemory + ( pCache->m_pSaltMW - pCache->m_pOwnMemory );
   m_pBdata = m_pOwnMemory + ( pCache->m_pBdata - pCache->m_pOwnMemory );
   m_pCdata = m_pOwnMemory + ( pCache->m_pCdata - pCache->m_pOwnMemory );
   m_pAdata = m_pOwnMemory + ( pCache->m_pAdata - pCache->m_pOwnMemory );
   m_pADefault = m_pOwnMemory + ( pCache->m_pADefault - pCache->m_pOwnMemory );
   m_pDAdsdata = m_pOwnMemory + ( pCache->m_pDAdsdata - pCache->m_pOwnMemory );
   m_pDAdtdata = m_pOwnMemory + ( pCache->m_pDAdtdata - pCache->m_pOwnMemory );
   m_pDBdtdata = m_pOwnMemory + ( pCache->m_pDBdtdata - pCache->m_pOwnMemory );
   m_pDCdtdata = m_pOwnMemory + ( pCache->m_pDCdtdata - pCache->m_pOwnMemory );
   m_pPCMod = m_pOwnMemory + ( pCache->m_pPCMod - pCache->m_pOwnMemory );
   m_pTCMod = m_pOwnMemory + ( pCache->m_pTCMod - pCache->m_pOwnMemory );

   /* Set reals */
   for ( i = 0; i < m_iAllocLength; i++ )
   {
      m_pOwnMemory[i] = pCache->m_pOwnMemory[i];
   }
}


/* 
// Initialize
//
// Initialize construction of cached pvttable. 
//
// See EosPvtModel.h EosPvtTable__Constructor for argument description
//
// 1) Generate an instance of the pvt cache
// 2) Set volumn translation off
// 3) Set defaults if no data supplied
// 5) Initialize the data
*/
void EosPvtTable::Initialize( int iVersion, int *piTables, double *pdTables, double *pCritical, double *pvtData,
                              double *pTSaltMW )
{
   int     iTables;
   int     iComp;
   int     iHeat;
   int     iLength;
   int     iMemory;
   int     iEos;
   double *pointR[EOS_METHOD_LAST_DARRAY];
   int    *pointI[EOS_METHOD_LAST_IARRAY];

   m_pOwnMemory = NULL;

   /* Data supplied so no default */
   if ( piTables )
   {
      /* Set a value for multiple table indicator */
      m_iTranslate = EOS_OPTION_UNKNOWN;

      /* Set integer pointers for pvt data */
      pointI[INTEGERDATA] = piTables;
      iEos = ( piTables[EOS_METHOD] == EOS_PVT_TABLE || piTables[EOS_METHOD] == EOS_PVT_IDEAL ) ? 0 : 1;
      iTables = piTables[EOS_METHOD_NUMBERABC];
      iMemory = piTables[EOS_METHOD_OWNMEMORY];
      iComp = piTables[EOS_METHOD_COMPONENTS];
      iHeat = piTables[EOS_METHOD_HEATS];
      if ( piTables[EOS_METHOD] == EOS_PVT_IDEAL )
      {
         iLength = 1;
      }
      else
      {
         iLength = ( iTables + 1 ) * ( 1 + iComp * ( 3 + iComp ) );
         if ( iVersion > 2 && piTables[EOS_METHOD_TYPE_OF_C] == EOS_LINEAR_C )
         {
            iLength += ( iTables + 1 ) * iComp;
         }

         if ( iVersion > 4 && piTables[EOS_METHOD_TYPE_OF_BIJ] == EOS_LINEAR_C )
         {
            iLength += ( iTables + 1 ) * iComp * iComp;
         }
      }

      /* Set real pointers for pvt data */
      pointR[REALDATA] = pdTables;
      pointR[EOS_METHOD_CRITICAL] = pCritical;
      pointR[EOS_METHOD_RAWABC] = pvtData;
      if ( iMemory )
      {
         pointR[EOS_METHOD_ABC] = (double *)0;
      }
      else
      {
         pointR[EOS_METHOD_ABC] = pvtData + iLength;
      }

      if ( iHeat )
      {
         pointR[EOS_METHOD_HEATS] = pCritical + ( 5 + iEos ) * iComp;
      }
      else
      {
         pointR[EOS_METHOD_HEATS] = (double *)0;
      }

      pointR[EOS_METHOD_SALTMW] = pTSaltMW;

      /* Read all pvt data */
      ReadAllData( iVersion, pointI, pointR );
   }
}


/* 
// ReadAllData
// 
// Set all the terms 
//
// iVersion ** constant **
//    Version number, set to 0, 1, 2, 3, or 4
// pointI 
//    Pointer to integer arrays used in the initialization
// pointR
//    Pointer to real arrays used in the initialization
//
// 1) Read integer and double constants
// 2) Allocate memory if required
// 3) Copy critical data if required
// 4) Set array pointers
// 5) Cache pvt tables
// 6) Determine if volume translation used
*/
void EosPvtTable::ReadAllData( int iVersion, int **pointI, double **pointR )
{
   double *pTerms;
   double *pWaterData;
   double *pAcentric;
   double *pTa;
   int     iLength;
   int    *pITerms;
   int     iNi;

   /* Set data pointers */
   pTerms = pointR[REALDATA];
   pITerms = pointI[INTEGERDATA];
   pWaterData = pointR[EOS_METHOD_SALTMW];

   /* Set the location of the raw ABC data */
   m_pRawAbcData = pointR[EOS_METHOD_RAWABC];

   /* Read the constant terms; returns whether to get memory */
   if ( ReadConstants( iVersion, pITerms, pTerms, pWaterData ) )
   {
      /* Allocate the memory */
      AllocateMemory();

      /* Load molecular weights and critical properties */
      iLength = 5 * m_iEosComp;
      pTa = pointR[EOS_METHOD_CRITICAL];
      for ( iNi = 0; iNi < iLength; iNi++ )
      {
         m_pMolecularWeight[iNi] = *pTa++;
      }

      /* Load ideal heats */
      if ( m_iHeat )
      {
         iLength = 7 * m_iEosComp;
         pTa = pointR[EOS_METHOD_HEATS];
         for ( iNi = 0; iNi < iLength; iNi++ )
         {
            m_pHeat0[iNi] = *pTa++;
         }
      }

      /* Load salt molecular weights */
      if ( m_iSaltComp > 0 )
      {
         pTa = pointR[EOS_METHOD_SALTMW];
         for ( iNi = 0; iNi < m_iSaltComp; iNi++ )
         {
            m_pSaltMW[iNi] = *pTa++;
         }
      }
   }

   /* Load location of properties */
   else
   {
      m_pOwnMemory = NULL;
      m_iAllocLength = 0;
      m_pMolecularWeight = pointR[EOS_METHOD_CRITICAL];
      m_pHeat0 = pointR[EOS_METHOD_HEATS];
      m_pSaltMW = pointR[EOS_METHOD_SALTMW];
      m_pTemperature = pointR[EOS_METHOD_ABC];
   }

   /* Set data pointers */
   SetArrayPointers();

   /* Initialize the viscosity terms if necessary */
   if ( m_dVTune < 0.0 )
   {
      InitializeViscosityTerms();
   }

   /* Set location of acentric factor */
   if ( m_iMethod == EOS_PVT_IDEAL || m_iMethod == EOS_PVT_TABLE || m_iMethod == EOS_PVT_RK )
   {
      pAcentric = (double *)0;
   }
   else
   {
      pAcentric = pointR[EOS_METHOD_CRITICAL] + 5 * m_iEosComp;
   }

   /* CachePvtTables */
   CachePvtTables( pAcentric );

   /* Set volume translation indicator */
   if ( m_iTranslate != EOS_OPTION_OFF )
   {
      SetVolumeTranslation();
   }
}


/* 
// AllocateMemory
// 
// Allocate the memory
//
// 1) Allocate memory for arrays
*/
void EosPvtTable::AllocateMemory( void )
{
   int iLength;

   /* Type of equation of state */
   if ( m_iMethod == EOS_PVT_IDEAL )
   {
      iLength = 1;
   }
   else if ( m_iMethod == EOS_PVT_RK )
   {
      iLength = 1 + m_iEosComp * ( 2 + m_iEosComp );
   }
   else if ( m_iMethod != EOS_PVT_TABLE )
   {
      iLength = 1 + m_iEosComp * ( 2 + 4 * m_iEosComp );
   }
   else if ( m_iTables )
   {
      iLength = m_iTables * ( 1 + m_iEosComp * ( 4 + 2 * m_iEosComp ) );
   }
   else
   {
      iLength = 1 + m_iEosComp * ( 2 + m_iEosComp );
   }

   if ( m_iTypeOfC == EOS_LINEAR_C )
   {
      iLength += m_iEosComp;
   }

   if ( m_iTypeOfBij == EOS_LINEAR_C )
   {
      if ( m_iMethod == EOS_PVT_RK )
      {
         iLength += m_iEosComp * m_iEosComp;
      }
      else
      {
         iLength += 2 * m_iEosComp * m_iEosComp;
      }
   }

   iLength += 5 * m_iEosComp;
   if ( m_iHeat )
   {
      iLength += 7 * m_iEosComp;
   }

   if ( m_dVTune < 0.0 )
   {
      iLength += 2 * m_iEosComp * m_iEosComp;
   }

   if ( m_iSaltComp > 0 )
   {
      iLength += m_iSaltComp;
   }

   /* Allocate memory */
   if (m_pOwnMemory)
      CDELETE (m_pOwnMemory);
   m_pOwnMemory = CNEW ( double, iLength );
   m_iAllocLength = iLength;

   /* Set temperature pointer */
   m_pTemperature = m_pOwnMemory;

   /* Set molecular weight pointer */
   if ( m_iMethod == EOS_PVT_IDEAL )
   {
      m_pMolecularWeight = m_pTemperature + 1;
   }
   else if ( m_iMethod == EOS_PVT_RK )
   {
      m_pMolecularWeight = m_pTemperature + 1 + m_iEosComp * ( 2 + m_iEosComp );
   }
   else if ( m_iMethod != EOS_PVT_TABLE )
   {
      m_pMolecularWeight = m_pTemperature + 1 + m_iEosComp * ( 2 + 4 * m_iEosComp );
   }
   else if ( m_iTables )
   {
      m_pMolecularWeight = m_pTemperature + m_iTables * ( 1 + m_iEosComp * ( 4 + 2 * m_iEosComp ) );
   }
   else
   {
      m_pMolecularWeight = m_pTemperature + 1 + m_iEosComp * ( 2 + m_iEosComp );
   }

   /* Extra volume translation term */
   if ( m_iTypeOfC == EOS_LINEAR_C )
   {
      m_pMolecularWeight += m_iEosComp;
   }

   /* Temperature dependent bij terms */
   if ( m_iTypeOfBij == EOS_LINEAR_C )
   {
      if ( m_iMethod == EOS_PVT_RK )
      {
         m_pMolecularWeight += m_iEosComp * m_iEosComp;
      }
      else
      {
         m_pMolecularWeight += 2 * m_iEosComp * m_iEosComp;
      }
   }

   /* Heats */
   iLength = 5 * m_iEosComp;
   if ( m_iHeat )
   {
      m_pHeat0 = m_pMolecularWeight + iLength;
      iLength += 7 * m_iEosComp;
   }
   else
   {
      m_pHeat0 = (double *)0;
   }

   /* Terms for corresponding state viscosity model */
   if ( m_dVTune < 0.0 )
   {
      m_pPCMod = m_pMolecularWeight + iLength;
      m_pTCMod = m_pPCMod + m_iEosComp * m_iEosComp;
      iLength += 2 * m_iEosComp * m_iEosComp;
   }
   else
   {
      m_pPCMod = (double *)0;
      m_pTCMod = (double *)0;
   }

   /* Salt molecular weight */
   if ( m_iSaltComp > 0 )
   {
      m_pSaltMW = m_pMolecularWeight + iLength;
   }
   else
   {
      m_pSaltMW = (double *)0;
   }
}


/* 
// ReadConstants
// 
// Set all the constant terms 
//
// iVersion ** constant **
//    Version number, set to 0, 1, or 2
// pITerms 
//    Pointer to integer terms
// pTerms
//    Pointer to real terms
// pWaterData
//    Pointer to water data
//
// 1) Read integer and double constants
// 2) Set the cubic equation of state terms
// 3) Set water data
*/
int EosPvtTable::ReadConstants( int iVersion, int *pITerms, double *pTerms, double *pWaterData )
{
   double *pTc;
   double *pTb;
   int     i;
   int     iReadWater;

   /* Set method */
   m_iMethod = pITerms[EOS_METHOD];

   /* Heat and thermal diffusion data to be supplied */
   m_iHeat = pITerms[EOS_METHOD_HEATDATA];

   /* Hydrocarbons and salts */
   m_iEosComp = pITerms[EOS_METHOD_COMPONENTS];
   m_iSaltComp = pITerms[EOS_METHOD_SALTS];

   /* Equation of state parameters */
   SetCubicTerms( pTerms[EOS_METHOD_KB1], pTerms[EOS_METHOD_KB2] );

   /* Conversion factors */
   m_dConvPressure = pTerms[EOS_METHOD_PRESCONV];
   m_dConvTemperature = pTerms[EOS_METHOD_TEMPCONV];
   m_dConvVolume = pTerms[EOS_METHOD_VOLCONV];
   m_dConvViscosity = pTerms[EOS_METHOD_VISCCONV];
   m_dConvDensity = pTerms[EOS_METHOD_DENSCONV];
   m_dConvHeat = pTerms[EOS_METHOD_HEATCONV];
   m_dConvTension = pTerms[EOS_METHOD_TENSCONV];

   /* Set gas and gravity constant */
   m_dGascon = 8314.471 * m_dConvPressure * m_dConvVolume / m_dConvTemperature / m_dConvDensity;
   m_dGravity = 9.807 * m_dConvHeat / m_dConvDensity / pow( m_dConvVolume, 0.333333333333333 );

   /* Phase identification terms */
   m_iTypeOfSingle = pITerms[EOS_METHOD_PHASEID];
   m_dCritZMinusB = pTerms[EOS_METHOD_CRITZMINUSB];
   m_dCritAOverB = pTerms[EOS_METHOD_CRITAOVERB];

   /* Set the corresponding states tuning parameter */
   if ( iVersion < 4 )
   {
      m_dVTerm1 = 1.304e-4;
      m_dVTerm2 = 2.303;
      m_dVTerm3 = 1.0;
      m_dVTerm4 = 1.0;
      m_CSVscMethod = 0;
   }
   else
   {
      m_dVTerm1 = pTerms[EOS_METHOD_VTERM1] * 1.304e-4;
      m_dVTerm2 = pTerms[EOS_METHOD_VTERM2] * 2.303;
      m_dVTerm3 = pTerms[EOS_METHOD_VTERM3] * 1.0; 
      m_dVTerm4 = pTerms[EOS_METHOD_VTERM4] * 1.0; 
      m_CSVscMethod = static_cast< int >( pTerms[EOS_METHOD_CSVSCMETHOD] );
      m_dVTerm1 = m_dVTerm1 ? m_dVTerm1 : 1.304e-4;
      m_dVTerm2 = m_dVTerm2 ? m_dVTerm2 : 2.303;
      m_dVTerm3 = m_dVTerm3 ? m_dVTerm3 : 1.0;
      m_dVTerm4 = m_dVTerm4 ? m_dVTerm4 : 1.0;
      m_CSVscMethod = m_CSVscMethod ? m_CSVscMethod : 0;
   }

   /* Set the Lohrenz viscosity tuning parameter */
   if ( iVersion < 3 )
   {
      m_dVTune0 = 0.1023000;
      m_dVTune1 = 0.0233640;
      m_dVTune2 = 0.0585330;
      m_dVTune3 = -0.0407580;
      m_dVTune4 = 0.0093324;
   }
   else
   {
      m_dVTune0 = pTerms[EOS_METHOD_VTUNE0];
      m_dVTune1 = pTerms[EOS_METHOD_VTUNE1];
      m_dVTune2 = pTerms[EOS_METHOD_VTUNE2];
      m_dVTune3 = pTerms[EOS_METHOD_VTUNE3];
      m_dVTune4 = pTerms[EOS_METHOD_VTUNE4];
   }

   /* Set the Jossi viscosity tuning parameter */
   if ( iVersion < 2 )
   {
      m_dVTune = 0.0;
   }
   else
   {
      m_dVTune = pTerms[EOS_METHOD_VTUNE];
   }

   /* Set the abc terms */
   m_iTables = pITerms[EOS_METHOD_NUMBERABC];

   /* Volume translation */
   if ( m_iMethod == EOS_PVT_IDEAL )
   {
      m_iTranslate = EOS_OPTION_OFF;
   }
   else
   {
      m_iTranslate = EOS_OPTION_UNKNOWN;
   }

   /* See whether to read water properties */
   if ( iVersion < 1 )
   {
      iReadWater = 0;
   }
   else
   {
      iReadWater = ( pITerms[EOS_METHOD_WATERMODEL] == EOS_WATER_LINEAR );
   }

   m_iTables = pITerms[EOS_METHOD_NUMBERABC];

   /* Set type of c parameter */
   if ( iVersion < 3 || m_iMethod == EOS_PVT_TABLE || m_iMethod == EOS_PVT_IDEAL )
   {
      m_iTypeOfC = EOS_CONSTANT_C;
   }
   else
   {
      m_iTypeOfC = pITerms[EOS_METHOD_TYPE_OF_C];
   }

   /* Set type of bij parameter */
   if ( iVersion < 5 || m_iMethod == EOS_PVT_TABLE || m_iMethod == EOS_PVT_IDEAL )
   {
      m_iTypeOfBij = EOS_CONSTANT_C;
   }
   else
   {
      m_iTypeOfBij = pITerms[EOS_METHOD_TYPE_OF_BIJ];
   }

   /* Set salinity */
   m_dSalinity = pTerms[EOS_METHOD_SALINITY];
   m_dWaterIft = pTerms[EOS_METHOD_WATERIFT];

   /* Set base water terms */
   m_dWaterDensDs = 0.0;
   m_dWaterDensDpDs = 0.0;
   m_dWaterDensDtDs = 0.0;
   m_dWaterViscDs = 0.0;
   m_dWaterViscDpDs = 0.0;
   m_dWaterViscDtDs = 0.0;
   m_dWaterHeatDs = 0.0;

   /* Set the other water properties */
   if ( iReadWater )
   {
      m_iWaterModel = EOS_WATER_LINEAR;
      m_dWaterDens = pWaterData[0 + m_iSaltComp];
      m_dWaterDensDp = pWaterData[1 + m_iSaltComp];
      m_dWaterDensDt = pWaterData[2 + m_iSaltComp];
      m_dWaterVisc = pWaterData[3 + m_iSaltComp];
      m_dWaterViscDp = pWaterData[4 + m_iSaltComp];
      m_dWaterViscDt = pWaterData[5 + m_iSaltComp];
      m_dWaterHeat = pWaterData[6 + m_iSaltComp];
      if ( m_iSaltComp )
      {
         m_dWaterDensDs = pWaterData[7 + m_iSaltComp];
         m_dWaterDensDpDs = pWaterData[8 + m_iSaltComp];
         m_dWaterDensDtDs = pWaterData[9 + m_iSaltComp];
         m_dWaterViscDs = pWaterData[10 + m_iSaltComp];
         m_dWaterViscDpDs = pWaterData[11 + m_iSaltComp];
         m_dWaterViscDtDs = pWaterData[12 + m_iSaltComp];
         m_dWaterHeatDs = pWaterData[13 + m_iSaltComp];
      }
   }

   /* Set the water properties to defaults */
   else
   {
      m_iWaterModel = EOS_WATER_CORRELATIONS;
      m_dWaterDens = 1000.0;
      m_dWaterDensDp = 0.0;
      m_dWaterDensDt = 0.0;
      m_dWaterVisc = 0.001;
      m_dWaterViscDp = 0.0;
      m_dWaterViscDt = 0.0;
      m_dWaterHeat = 4185.0;
   }

   /* Check the volume translation temperature terms */
   pTb = m_pRawAbcData + m_iEosComp * ( 3 + m_iEosComp ) + 1;
   pTc = pTb;
   if ( m_iTypeOfC == EOS_LINEAR_C )
   {
      pTb += m_iEosComp;
      m_iTypeOfC = EOS_ZERO_C;
      for ( i = 0; i < m_iEosComp; i++ )
      {
         m_iTypeOfC = pTc[i] ? EOS_LINEAR_C : m_iTypeOfC;
      }
   }

   /* Check the binary interaction temperature terms */
   if ( m_iTypeOfBij == EOS_LINEAR_C )
   {
      m_iTypeOfBij = EOS_ZERO_C;
      for ( i = 0; i < m_iEosComp * m_iEosComp; i++ )
      {
         m_iTypeOfBij = pTb[i] ? EOS_LINEAR_C : m_iTypeOfBij;
      }
   }

   /* Return memory flag */
   return ( pITerms[EOS_METHOD_OWNMEMORY] );
}


/* 
// SetCubicTerms
//
// Set terms for a generalized cubic 
//
// dUserKb0
//    Kb0 term as supplied by the user
// dUserKb1
//    Kb1 term as supplied by the user
//
// 1) A generalized cubic equation of state is of the form
//
//    Z * ( Z * ( Z - ( 1 - dKb7 * B ) ) + A - B * ( dKb4 +
//          dKb6 * B ) ) - B * ( A + dKb5 * B * ( 1 + B ) ) 
//
//    where
//
//    dKb4 = dKb0 + dKb1
//    dKb5 = dKb0 * dKb1
//    dKb6 = dKb4 - dKb5
//    dKb7 = dKb4 - 1.0
//
// 2) The above equation yields for Redlich Kwong (dKb0 = 1, 
//    dKb1 = 0 )
//  
//    Z^3 - Z^2 + ( A - B - B^2 ) Z - B * A = 0
//
// 3) The above equation yields for Peng-Robinson (dKb0 = 1+2^0.5, 
//    dKb1 = 1-2^0.5 )
//  
//    Z^3 - ( 1 - B ) * Z^2 + ( A - 2 * B - 3 * B^2 ) Z - 
//            B * ( A - B - B ^ 2 ) = 0
//
// 4) The critical A and B terms are computed from the
//    point where all three roots are equal.  This is
//    computed using Newton's method.  dKb2 is then the
//    critical A/B, and B is the critical B.  For Redlich
//    Kwong, these numbers work out to be about 5 and 0.086, 
//    respectively.  
//
// 5) dKb8 and dKb9 are for the fugacity, which is most commonly
//
//    ( Z - 1 ) - log ( Z - B ) - ( dKb8 * A / B ) *
//        log ( ( Z + dKb0 * B ) / ( Z + dKb1 * B ) )
//
//    dKb9 is for the van der Waals equation, which is probably
//    only included since he is Dutch
*/
void EosPvtTable::SetCubicTerms( double dUserKb0, double dUserKb1 )
{
   double dA;
   double dB;
   double dA0;
   double dA1;
   double dA2;
   double dC0;
   double dC1;
   double dC2;
   int    iter;

   /* Set permanent terms */
   m_dKb0 = dUserKb0;
   m_dKb1 = dUserKb1;

   /* Equation of state terms */
   m_dKb4 = m_dKb0 + m_dKb1;
   m_dKb5 = m_dKb0 * m_dKb1;
   m_dKb6 = m_dKb4 - m_dKb5;
   m_dKb7 = m_dKb4 - 1.0;

   /* Linear Z terms */
   dC0 = 1.0 / 3.0;
   dC1 = m_dKb4 - 2.0 * m_dKb7 / 3.0;
   dC2 = m_dKb6 + m_dKb7 * m_dKb7 / 3.0;

   /* Constant terms */
   dA = m_dKb5 + m_dKb7 * m_dKb7 * m_dKb7 / 27.0;
   dA2 = m_dKb5 - m_dKb7 * m_dKb7 / 9.0;
   dA1 = m_dKb7 / 9.0;
   dA0 = -1.0 / 27.0;

   /* Set up cubic equation */
   dA = dA + dC2;
   dA2 = ( dA2 + dC1 ) / dA;
   dA1 = ( dA1 + dC0 ) / dA;
   dA0 = dA0 / dA;

   /* Newton's method */
   dA = 0.0;
   dB = 0.1;
   for ( iter = 0; dA != dB && iter < 30; iter++ )
   {
      dA = dB;
      dB -= ( dB * ( dB * ( dB + dA2 ) + dA1 ) + dA0 ) / ( dB * ( 3.0 * dB + 2.0 * dA2 ) + dA1 );
   }

   /* Determine a */
   dA = dC0 + dB * ( dC1 + dB * dC2 );

   /* Determine flasher terms */
   m_dKb2 = dA / dB;
   m_dKb3 = dB;

   /* Fugacity terms */
   if ( m_dKb0 == m_dKb1 )
   {
      m_dKb8 = 0.0;
      m_dKb9 = 1.0;
   }
   else
   {
      m_dKb8 = 1.0 / ( m_dKb0 - m_dKb1 );
      m_dKb9 = 0.0;
   }
}


/* 
// SetArrayPointers
//
// Set array pointers
//
// 1) Set pointers for critical properties and heat terms
// 2) Set pointers to the cached pvt tables
*/
void EosPvtTable::SetArrayPointers( void )
{
   int iOff;

   /* Set critical term offsets */
   m_pCriticalPressure = m_pMolecularWeight + m_iEosComp;
   m_pCriticalTemperature = m_pCriticalPressure + m_iEosComp;
   m_pCriticalVolume = m_pCriticalTemperature + m_iEosComp;
   m_pParachor = m_pCriticalVolume + m_iEosComp;

   /* Heat capacities */
   if ( m_iHeat )
   {
      m_pHeat1 = m_pHeat0 + m_iEosComp;
      m_pHeat2 = m_pHeat1 + m_iEosComp;
      m_pHeat3 = m_pHeat2 + m_iEosComp;
      m_pHeat4 = m_pHeat3 + m_iEosComp;
      m_pHeat5 = m_pHeat4 + m_iEosComp;
      m_pHeat6 = m_pHeat5 + m_iEosComp;
   }
   else
   {
      m_pHeat1 = m_pHeat0;
      m_pHeat2 = m_pHeat1;
      m_pHeat3 = m_pHeat2;
      m_pHeat4 = m_pHeat3;
      m_pHeat5 = m_pHeat4;
      m_pHeat6 = m_pHeat5;
   }

   /* Set pointers for ideal gas */
   if ( m_iMethod == EOS_PVT_IDEAL )
   {
      m_pBdata = m_pTemperature;
      m_pCdata = m_pBdata;
      m_pAdata = m_pCdata;
      m_pADefault = m_pAdata;
      m_pDBdtdata = m_pBdata;
      m_pDCdtdata = m_pCdata;
      m_pDAdsdata = m_pAdata;
      m_pDAdtdata = m_pAdata;
      m_pDAdrdata = m_pAdata;
      m_pDAdldata = m_pAdata;
   }

   /* Set abc table pointers */
   else
   {
      m_pBdata = m_pTemperature + 1;
      m_pCdata = m_pBdata + m_iEosComp;
      m_pAdata = m_pCdata + m_iEosComp;

      /* Other terms */
      if ( m_iMethod == EOS_PVT_RK )
      {
         m_pDBdtdata = m_pBdata;
         m_pDAdsdata = m_pAdata;
         m_pDAdtdata = m_pAdata;
         m_pADefault = m_pAdata;
         if ( m_iTypeOfBij == EOS_LINEAR_C )
         {
            m_pDAdrdata = m_pAdata + m_iEosComp * m_iEosComp;
         }
         else
         {
            m_pDAdrdata = m_pAdata;
         }

         m_pDAdldata = m_pDAdrdata;
         if ( m_iTypeOfC == EOS_LINEAR_C )
         {
            m_pDCdtdata = m_pDAdldata + m_iEosComp * m_iEosComp;
         }
         else
         {
            m_pDCdtdata = m_pCdata;
         }
      }
      else if ( m_iMethod != EOS_PVT_TABLE )
      {
         iOff = m_iEosComp * m_iEosComp;
         m_pDAdsdata = m_pAdata + iOff;
         m_pDAdtdata = m_pDAdsdata + iOff;
         if ( m_iTypeOfBij == EOS_LINEAR_C )
         {
            m_pDAdrdata = m_pDAdtdata + iOff;
            m_pDAdldata = m_pDAdrdata + iOff;
         }
         else
         {
            m_pDAdrdata = m_pDAdtdata;
            m_pDAdldata = m_pDAdtdata;
         }

         m_pADefault = m_pDAdldata + iOff;
         m_pDBdtdata = m_pBdata;
         if ( m_iTypeOfC == EOS_LINEAR_C )
         {
            m_pDCdtdata = m_pADefault + iOff;
         }
         else
         {
            m_pDCdtdata = m_pCdata;
         }
      }
      else if ( m_iTables == 0 )
      {
         m_pDBdtdata = m_pBdata;
         m_pDCdtdata = m_pCdata;
         m_pDAdsdata = m_pAdata;
         m_pDAdtdata = m_pAdata;
         m_pDAdrdata = m_pAdata;
         m_pDAdldata = m_pAdata;
         m_pADefault = m_pAdata;
      }
      else
      {
         m_pDBdtdata = m_pAdata + m_iEosComp * m_iEosComp;
         m_pDCdtdata = m_pDBdtdata + m_iEosComp;
         m_pDAdsdata = m_pDCdtdata + m_iEosComp;
         m_pDAdtdata = m_pDAdsdata;
         m_pDAdrdata = m_pDAdsdata;
         m_pDAdldata = m_pDAdsdata;
         m_pADefault = m_pAdata;
      }
   }
}


/* 
// Set volume translation
//
// Set whether volume translation used
//
// 1) Determine if volume translation used
*/
void EosPvtTable::SetVolumeTranslation( void )
{
   double *pTa;
   double *pTb;
   int     iNi;
   int     iN;
   int     iOff;

   /* Determine whether there is volume translation */
   m_iTranslate = EOS_OPTION_OFF;
   if ( m_iMethod != EOS_PVT_IDEAL )
   {
      /* Loop over all temperatures to check volume translation */
      if ( m_iTypeOfC == EOS_LINEAR_C )
      {
         pTa = m_pCdata;
         pTb = m_pDCdtdata;
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            if ( pTa[iNi] != 0.0 || pTb[iNi] != 0.0 )
            {
               m_iTranslate = EOS_OPTION_ON;
            }
         }
      }
      else if ( m_iTables == 0 )
      {
         pTa = m_pCdata;
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            if ( pTa[iNi] != 0.0 )
            {
               m_iTranslate = EOS_OPTION_ON;
            }
         }
      }
      else
      {
         iOff = 1 + m_iEosComp * ( 4 + 2 * m_iEosComp );
         pTa = m_pCdata;
         pTb = m_pDCdtdata;
         for ( iN = 0; iN < m_iTables; iN++ )
         {
            for ( iNi = 0; iNi < m_iEosComp; iNi++ )
            {
               if ( pTa[iNi] != 0.0 || pTb[iNi] != 0.0 )
               {
                  m_iTranslate = EOS_OPTION_ON;
               }
            }

            pTa += iOff;
            pTb += iOff;
         }
      }
   }
}


/* 
// CachePvtTables
//
// Set calculated data for the pvt tables
//
// pAcentric
//    Pointer to acentric factor terms
//
// 1) Cache the pvt tables
*/
void EosPvtTable::CachePvtTables( double *pAcentric )
{
   /* Cache Pvt Data */
   if ( m_iMethod == EOS_PVT_IDEAL )
   {
      *m_pTemperature = *m_pRawAbcData;
   }
   else if ( m_iMethod == EOS_PVT_TABLE )
   {
      CacheAbcData();
   }
   else if ( m_iMethod == EOS_PVT_RK )
   {
      CacheRKData();
   }
   else
   {
      CacheEosData( pAcentric );
   }
}


/* 
// CacheAbcData
//
// Cache Pvt table data 
//
// 1) Set offsets between tables in storage
// 2) Two different loops for either one temperature
//    or multiple temperatures
// 3) In both loops, need to multiple Ai by RT
// 4) Aij = (AiAj)^0.5 * ( 1 - bij )
// 5) Use linear interpolation in tables between 
//    temperatures, e.g., Bi = Bi0 + DBiDt * T.
*/
void EosPvtTable::CacheAbcData( void )
{
   double  dA;
   double  dB;
   double  dC;
   double  dD;
   double  dE;
   double *pTa;
   double *pTb;
   double *pTc;
   double *pRaw;
   double *pCached;
   int     iJ;
   int     iL;
   int     iNi;
   int     iNj;
   int     iOffsetRaw;
   int     iOffsetCached;
   int     iLoop;

   /* Set offset data */
   iOffsetCached = m_iTables ? ( 1 + m_iEosComp * ( 4 + 2 * m_iEosComp ) ) : 0;
   iOffsetRaw = 1 + m_iEosComp * ( 3 + m_iEosComp );
   iLoop = m_iTables ? m_iTables : 1;
   pCached = m_pTemperature;
   pRaw = m_pRawAbcData;

   /* For a single table */
   for ( iL = 0; iL < iLoop; iL++ )
   {
      *pCached = *pRaw;

      /* Store b terms */
      pTa = pCached + 1;
      pTb = pRaw + 1 + m_iEosComp;
      for ( iNi = 0; iNi < m_iEosComp; iNi++ )
      {
         *pTa++ = *pTb++ / m_dGascon;
      }

      /* Store c terms */
      for ( iNi = 0; iNi < m_iEosComp; iNi++ )
      {
         *pTa++ = *pTb++;
      }

      /* Store a terms */
      dD = *pRaw / m_dGascon;
      for ( iNi = 0; iNi < m_iEosComp; iNi++ )
      {
         dE = dD * sqrt( pRaw[1 + iNi] );
         for ( iNj = 0; iNj < m_iEosComp; iNj++ )
         {
            *pTa++ = dE * sqrt( pRaw[1 + iNj] ) * ( 1.0 -*pTb++ );
         }
      }

      /* Perform linear update */
      if ( iL )
      {
         iJ = m_iEosComp * ( 2 + m_iEosComp );
         pTa = pCached - iOffsetCached;
         pTb = pCached;
         dA = *pTa;
         dB = *pTb - dA;
         *pTa++ = *pTb++;
         pTc = pTa + iJ;
         for ( iNi = 0; iNi < iJ; iNi++ )
         {
            dC = *pTa;
            *pTc = ( *pTb++ -dC ) / dB;
            *pTa++ = dC -*pTc++ *dA;
         }
      }

      /* Update pointers */
      pCached += iOffsetCached;
      pRaw += iOffsetRaw;
   }

   /* Final table */
   if ( m_iTables )
   {
      pCached -= iOffsetCached;
      dA = *pCached;
      dB = *pRaw - dA;
      *pCached = *pRaw;

      /* Store b terms */
      pTa = pCached + 1;
      pTc = pTa + m_iEosComp * ( 2 + m_iEosComp );
      pTb = pRaw + 1 + m_iEosComp;
      for ( iNi = 0; iNi < m_iEosComp; iNi++ )
      {
         dC = *pTa;
         *pTc = ( *pTb++ / m_dGascon - dC ) / dB;
         *pTa++ = dC -*pTc++ *dA;
      }

      /* Store c terms */
      for ( iNi = 0; iNi < m_iEosComp; iNi++ )
      {
         dC = *pTa;
         *pTc = ( *pTb++ -dC ) / dB;
         *pTa++ = dC -*pTc++ *dA;
      }

      /* Store a terms */
      dD = *pRaw / m_dGascon;
      for ( iNi = 0; iNi < m_iEosComp; iNi++ )
      {
         dE = dD * sqrt( pRaw[1 + iNi] );
         for ( iNj = 0; iNj < m_iEosComp; iNj++ )
         {
            dC = *pTa;
            *pTc = ( dE * sqrt( pRaw[1 + iNj] ) * ( 1.0 -*pTb++ ) - dC ) / dB;
            *pTa++ = dC -*pTc++ *dA;
         }
      }
   }
}


/* 
// CacheRKData
//
// Cache Redlich Kwong equation-of-state data 
//
// 1) Set default temperatures
// 2) Set terms for Ai parameter for equations of state.
//
//    Ai = OmegaAi * Tci * Tci * Tci ^ 0.5 / Pci
//
// 3) Then form Aij
//
//    Aij = sqrt ( Ai * Aj ) * ( 1 - bij )
//
// 4) B and C are calculated from
//
//    Bi = OmegaBi * Tci / Pci
//    
//    Ci = Shift * Bi
*/
void EosPvtTable::CacheRKData( void )
{
   double  dA;
   double  dB;
   double *pTa;
   double *pTb;
   double *pTc;
   double *pTd;
   int     iNi;
   int     iNj;

   /* Set temperature data */
   *m_pTemperature = *m_pRawAbcData;

   /* Set A terms; store in B location */
   pTa = m_pRawAbcData + 1;
   for ( iNi = 0; iNi < m_iEosComp; iNi++ )
   {
      dA = m_pCriticalTemperature[iNi];
      dB = sqrt( dA );
      m_pBdata[iNi] = dA * sqrt( dB * pTa[iNi] / m_pCriticalPressure[iNi] );
   }

   /* Set the Aij terms */
   if ( m_iTypeOfBij == EOS_LINEAR_C )
   {
      pTa = m_pAdata;
      pTb = m_pRawAbcData + 3 * m_iEosComp + 1;
      pTc = pTb + m_iEosComp * m_iEosComp;
      pTc += ( m_iTypeOfC == EOS_CONSTANT_C ? 0 : m_iEosComp );
      pTd = m_pDAdldata;
      for ( iNi = 0; iNi < m_iEosComp; iNi++ )
      {
         dA = m_pBdata[iNi];
         for ( iNj = 0; iNj < m_iEosComp; iNj++ )
         {
            dB = dA * m_pBdata[iNj];
            *pTa++ = ( 1.0 -*pTb++ ) * dB;
            *pTd++ = -dB **pTc++;
         }
      }
   }
   else
   {
      pTa = m_pAdata;
      pTb = m_pRawAbcData + 3 * m_iEosComp + 1;
      for ( iNi = 0; iNi < m_iEosComp; iNi++ )
      {
         dA = m_pBdata[iNi];
         for ( iNj = 0; iNj < m_iEosComp; iNj++ )
         {
            *pTa++ = ( 1.0 -*pTb++ ) * dA * m_pBdata[iNj];
         }
      }
   }

   /* Set DCdt terms */
   if ( m_iTypeOfC == EOS_CONSTANT_C )
   {
      pTb -= m_iEosComp;
   }

   for ( iNi = 0; iNi < m_iEosComp; iNi++ )
   {
      m_pDCdtdata[iNi] = pTb[iNi];
   }

   /* Set B and C terms */
   pTb = m_pRawAbcData + m_iEosComp + 1;
   pTa = pTb + m_iEosComp;
   for ( iNi = 0; iNi < m_iEosComp; iNi++ )
   {
      dA = pTb[iNi] * m_pCriticalTemperature[iNi] / m_pCriticalPressure[iNi];
      dB = dA * m_dGascon;
      m_pBdata[iNi] = dA;
      m_pDCdtdata[iNi] *= dB;
      m_pCdata[iNi] = pTa[iNi] * dB;
   }
}


/* 
// CacheEosData
//
// Cache equation-of-state data for Soave Redlich Kwong and
// Peng Robinson equations of state
//
// pAcentric
//    Acentric factor
//
// 1) Set default temperatures
// 2) Set terms for Ai parameter for equations of state.
//
//    Ai = ( 1 + alpha * ( 1 - Tr ) ) ^ 2 * OmegaAi * Tci * Tci / Pci
//
//    Different equations of state have different alpha terms.
//
//    SRK: alpha = 0.48 + Acentric * ( 1.574 - Acentric * 0.176 )
//
//    PR: alpha = 0.37464 + Acentric * ( 1.54226 - 
//                Acentric * 0.26992 )
//
//    Corrected PR: Like PR except when Acentric > 0.49, then
//       alpha = 0.379642 + Acentric * ( 1.48503 - 
//               Acentric * ( 0.164423 - Acentric * 0.016666 ) )
//
//    Begin by storing the square root of the terms not involving
//    alpha, and storing alpha
// 3) Then form Aij
//
//    Aij = sqrt ( Ai * Aj ) * ( 1 - bij )
//
//    Now, since the term involving alpha is squared, we do not
//    need to form the square root.  After some algebra and recalling
//    that we stored stuff in the B and C locations, we can find that 
//
//    Aij = sqrt ( Bi * Bj ) * ( 1 - bij ) *  
//
//             [ ( 1 + Ci + Cj + Ci * Cj ) - sqrt ( T ) * 
//
//               ( ( 1 + Cj ) * Ci / sqrt ( Tci ) +
//
//                 ( 1 + Ci ) * Cj / sqrt ( Tcj ) ) + T * 
//
//                   Ci * Cj / sqrt ( Tci ) / sqrt ( Tcj ) ) ]
//
//    We first form the terms for the various powers of T; then we
//    multiply them by the sqrt ( Bi * Bj ) * ( 1 - bij ) term
// 4) If there are temperature terms involved, then bij is
//    really of the form 
//
//    bij = bij0 + bijt * T
//
//    This needs to be accounted for in the forming of the terms
// 5) We also save a copy of Aij at the default temperature
// 6) B and C are calculated from
//
//    Bi = OmegaBi * Tci / Pci
//    
//    Ci = Shift * Bi
*/
void EosPvtTable::CacheEosData( double *pAcentric )
{
   double  dA;
   double  dB;
   double  dC;
   double  dD;
   double  dE;
   double  dF;
   double  dG;
   double  dH;
   double *pTa;
   double *pTb;
   double *pTc;
   double *pTd;
   double *pTe;
   double *pTf;
   double *pTg;
   double *pTh;
   int     iNi;
   int     iNj;

   /* Set temperature data */
   *m_pTemperature = *m_pRawAbcData;

   /* Set correction terms; store in B and C locations */
   pTa = m_pRawAbcData + 1;
   for ( iNi = 0; iNi < m_iEosComp; iNi++ )
   {
      dB = pAcentric[iNi];

      /* Soave Redlich Kwong */
      if ( m_iMethod == EOS_PVT_SRK )
      {
         dA = 0.48 + dB * ( 1.574 - dB * 0.176 );
      }

      /* Corrected Peng Robinson */
      else if ( m_iMethod == EOS_PVT_PRCORR && dB > 0.49 )
      {
         dA = 0.379642 + dB * ( 1.48503 - dB * ( 0.164423 - dB * 0.016666 ) );
      }

      /* Regular Peng Robinson */
      else
      {
         dA = 0.37464 + dB * ( 1.54226 - dB * 0.26992 );
      }

      /* Finally set the correction term */
      dB = m_pCriticalTemperature[iNi];
      m_pBdata[iNi] = dA;
      m_pCdata[iNi] = dA / sqrt( dB );
   }

   /* Set the Aij terms coming from correction */
   pTa = m_pAdata;
   pTb = m_pDAdsdata;
   pTc = m_pDAdtdata;
   for ( iNi = 0; iNi < m_iEosComp; iNi++ )
   {
      dA = 1.0 + m_pBdata[iNi];
      dB = m_pCdata[iNi];
      for ( iNj = 0; iNj < m_iEosComp; iNj++ )
      {
         dC = 1.0 + m_pBdata[iNj];
         dD = m_pCdata[iNj];
         *pTa++ = dA * dC;
         *pTb++ = -dA * dD - dB * dC;
         *pTc++ = dB * dD;
      }
   }

   /* Set A terms; store in B location */
   pTa = m_pRawAbcData + 1;
   for ( iNi = 0; iNi < m_iEosComp; iNi++ )
   {
      m_pBdata[iNi] = m_pCriticalTemperature[iNi] * sqrt( pTa[iNi] / m_pCriticalPressure[iNi] );
   }

   /* Set the final Aij terms */
   if ( m_iTypeOfBij == EOS_LINEAR_C )
   {
      pTa = m_pAdata;
      pTb = m_pDAdsdata;
      pTc = m_pDAdtdata;
      pTd = m_pDAdrdata;
      pTe = m_pDAdldata;
      pTf = m_pADefault;
      pTg = m_pRawAbcData + 3 * m_iEosComp + 1;
      pTh = pTg + m_iEosComp * m_iEosComp;
      pTh += ( m_iTypeOfC == EOS_CONSTANT_C ? 0 : m_iEosComp );
      dC = *m_pTemperature;
      dD = sqrt( dC );
      for ( iNi = 0; iNi < m_iEosComp; iNi++ )
      {
         dA = m_pBdata[iNi];
         for ( iNj = 0; iNj < m_iEosComp; iNj++ )
         {
            dE = dA * m_pBdata[iNj];
            dB = ( 1.0 -*pTg++ ) * dE;
            dE *= ( -*pTh++ );
            dF = *pTa;
            dG = *pTb;
            dH = *pTc;
            *pTa = dB * dF;
            *pTb = dB * dG;
            *pTc = dB * dH + dE * dF;
            *pTd = dE * dG;
            *pTe = dE * dH;
            *pTf++ = *pTa++ / dC +*pTb++ / dD +*pTc++ +*pTd++ *dD +*pTe++ *dC;
         }
      }

      pTe = pTg;
   }
   else
   {
      pTa = m_pAdata;
      pTb = m_pDAdsdata;
      pTc = m_pDAdtdata;
      pTd = m_pADefault;
      pTe = m_pRawAbcData + 3 * m_iEosComp + 1;
      dC = *m_pTemperature;
      dD = sqrt( dC );
      for ( iNi = 0; iNi < m_iEosComp; iNi++ )
      {
         dA = m_pBdata[iNi];
         for ( iNj = 0; iNj < m_iEosComp; iNj++ )
         {
            dB = ( 1.0 -*pTe++ ) * dA * m_pBdata[iNj];
            *pTa *= dB;
            *pTb *= dB;
            *pTc *= dB;
            *pTd++ = *pTa++ / dC +*pTb++ / dD +*pTc++;
         }
      }
   }

   /* Set DCdt terms */
   if ( m_iTypeOfC == EOS_CONSTANT_C )
   {
      pTe -= m_iEosComp;
   }

   /* Set B and C terms */
   pTb = m_pRawAbcData + m_iEosComp + 1;
   pTc = pTb + m_iEosComp;
   for ( iNi = 0; iNi < m_iEosComp; iNi++ )
   {
      dA = pTb[iNi] * m_pCriticalTemperature[iNi] / m_pCriticalPressure[iNi];
      dB = dA * m_dGascon;
      m_pBdata[iNi] = dA;
      m_pDCdtdata[iNi] = pTe[iNi] * dB;
      m_pCdata[iNi] = pTc[iNi] * dB;
   }
}


/* 
// GetNumberHydrocarbons
//
// Return the number of hydrocarbons 
*/
int EosPvtTable::GetNumberHydrocarbons( void )
{
   return ( m_iEosComp );
}


/* 
// GetNumberSalts
//
// Return the number of salts, not including water
*/
int EosPvtTable::GetNumberSalts( void )
{
   return ( m_iSaltComp );
}


/* 
//
// GetGravity
//
// Return the gravity constant in internal energy units for use
// in the flasher
//
// pGravity
//    Gravity constant
// pGascon
//    Gas constant
//
// 1) The flasher knows nothing about the units of the PVT
//    tables, so have to return these values for the compositional
//    grading routines
*/
void EosPvtTable::GetGravity( double *pGravity, double *pGascon )
{
   *pGravity = m_dGravity;
   *pGascon = m_dConvHeat * m_dGascon / m_dConvPressure / m_dConvVolume;
}


/* 
// InitBubblePoint
//
// Returns an initial guess for the bubble point calculations 
// which is about 4000 PSIA
*/
double EosPvtTable::InitBubblePoint( void )
{
   return ( 2.5e7 * m_dConvPressure );
}


/* 
// SetPointers
// 
// Set the pointers for temporary memory
//
// iN ** constant **
//    Maximum number of flashes to perform in any slice
// pTCubic
//    Pointer to use as start of work array when calling cubic 
//    equation of state routine 
// pTMu
//    Pointer to use as start of work array when calling viscosity
//    routine 
// pTWork
//    Pointer to use as start of work array when calling other
//    routines 
// pAT
//    Pointer to use as start of work array when calling cubic 
//    equation of state routine 
// pSumT
//    Pointer to use as start of work array when calling viscosity
//    routine 
// pFinal
//    On input represents the end of the work array.  Updated
//    in this call to get a possible new end of the work array
//
// This array is needed to ensure that there are no memory
// crashes between the flasher and the pvt tables, as they
// use the same workspace which is allocated in the flasher.  Thus
//
// 1) For each array, calculate the end 
// 2) Return pFinal 
*/
void EosPvtTable::SetPointers( int iN, double *pTCubic, double *pTMu, double *pTWork, double **pAT, double **pSumT,
                               double **pFinal )
{
   int i;
   int i1;
   int iNcm = m_iEosComp * iN;

   /* Set the locations for cubic */
   *pAT = pTCubic;
   // i = 13 * iN + 4 * iNcm;
   i = 14 * iN + 4 * iNcm;

   /* Set the locations for viscosity */
   *pSumT = pTMu;
   i1 = ( (int)( pTMu - pTCubic ) ) + 7 * iN + 3 * iNcm;

   /* For GSS Jossi model */
   if ( m_dVTune > 0.0 )
   {
      i1 += m_iEosComp * ( 4 + iN * ( 4 + m_iEosComp ) ) + 3 * iN;
   }

   i = i1 > i ? i1 : i;

   /* For corresponding states viscosity model */
   if ( m_dVTune < 0.0 )
   {
      i1 = ( (int)( pTMu - pTCubic ) ) + ViscosityWorkArraySize( iN );
      i = i1 > i ? i1 : i;
   }

   /* Other locations */
   i1 = ( (int)( pTWork - pTCubic ) ) + iN + iN + iNcm + iNcm;
   i = i1 > i ? i1 : i;
   *pFinal = pTCubic + i;
}


/* 
// SetAbcPointers
//
// Set the abc table pointers 
//
// iN ** constant **
//    Maximum number of flashes to perform in any slice
// pAbcOffsetT
//    Pointer used for indicating the beginning of the
//    temperature entry in the PVT tables
// pFinal
//    On input represents the end of the work array.  Updated
//    in this call to get a possible new end of the work array
//
// This array is needed to ensure that there are no memory
// crashes between the flasher and the pvt tables, as they
// use the same workspace which is allocated in the flasher.  Thus
//
// 1) If more than one temperature entry we will need to
//    store the entry in the table for each object being
//    flasher.  Thus allocate memory for this
// 2) Update pFinal
*/
void EosPvtTable::SetAbcPointers( int iN, int **pAbcOffsetT, double **pFinal )
{
   if ( m_iTables )
   {
      *pAbcOffsetT = (int *) *pFinal;
      *pFinal = ( (double *) *pAbcOffsetT ) + iN;
   }
   else
   {
      *pAbcOffsetT = (int *)0;
   }
}


/*
// PrintInputData
//
// Debug printing of input data to pvt tables
*/
void EosPvtTable::PrintInputData( void )
{
   int i;
   int j;

   /* Perform the printing */
   printf( "\nEosPvtTable.iMethod: " );
   printf( "%i", m_iMethod );
   printf( "\nEosPvtTable.iEosComp: " );
   printf( "%i", m_iEosComp );
   printf( "\nEosPvtTable.iSaltComp: " );
   printf( "%i", m_iSaltComp );
   printf( "\nEosPvtTable.pOwnMemory: " );
   printf( "%p", (void *)( m_pOwnMemory ) );
   printf( "\nCritical Data:" );
   for ( i = 0; i < m_iEosComp; i++ )
   {
      printf( "\n" );
      printf( "%i", i );
      printf( " " );
      printf( "%e", m_pMolecularWeight[i] );
      printf( " " );
      printf( "%e", m_pCriticalPressure[i] );
      printf( " " );
      printf( "%e", m_pCriticalTemperature[i] );
      printf( " " );
      printf( "%e", m_pCriticalVolume[i] );
      printf( " " );
      printf( "%e", m_pParachor[i] );
   }

   printf( "\nEosPvtTable.iHeat: " );
   printf( "%i", m_iHeat );
   printf( "\nEosPvtTable.dKb0: " );
   printf( "%e", m_dKb0 );
   printf( "\nEosPvtTable.dKb1: " );
   printf( "%e", m_dKb1 );
   printf( "\nEosPvtTable.dKb2: " );
   printf( "%e", m_dKb2 );
   printf( "\nEosPvtTable.dKb3: " );
   printf( "%e", m_dKb3 );
   printf( "\nEosPvtTable.dKb4: " );
   printf( "%e", m_dKb4 );
   printf( "\nEosPvtTable.dKb5: " );
   printf( "%e", m_dKb5 );
   printf( "\nEosPvtTable.dKb6: " );
   printf( "%e", m_dKb6 );
   printf( "\nEosPvtTable.dKb7: " );
   printf( "%e", m_dKb7 );
   printf( "\nEosPvtTable.dKb8: " );
   printf( "%e", m_dKb8 );
   printf( "\nEosPvtTable.dKb9: " );
   printf( "%e", m_dKb9 );
   printf( "\nEosPvtTable.dConvPressure: " );
   printf( "%e", m_dConvPressure );
   printf( "\nEosPvtTable.dConvTemperature: " );
   printf( "%e", m_dConvTemperature );
   printf( "\nEosPvtTable.dConvVolume: " );
   printf( "%e", m_dConvVolume );
   printf( "\nEosPvtTable.dConvViscosity: " );
   printf( "%e", m_dConvViscosity );
   printf( "\nEosPvtTable.dConvDensity: " );
   printf( "%e", m_dConvDensity );
   printf( "\nEosPvtTable.dConvHeat: " );
   printf( "%e", m_dConvHeat );
   printf( "\nEosPvtTable.dConvTension: " );
   printf( "%e", m_dConvTension );
   printf( "\nEosPvtTable.dGascon: " );
   printf( "%e", m_dGascon );
   printf( "\nEosPvtTable.dSalinity: " );
   printf( "%e", m_dSalinity );
   printf( "\nEosPvtTable.dWaterIft: " );
   printf( "%e", m_dWaterIft );
   printf( "\nEosPvtTable.*pSaltMW: " );
   printf( "%p", (void *)( m_pSaltMW ) );
   printf( "\nEosPvtTable.iTypeOfSingle: " );
   printf( "%i", m_iTypeOfSingle );
   printf( "\nEosPvtTable.dCritZMinusB: " );
   printf( "%e", m_dCritZMinusB );
   printf( "\nEosPvtTable.dCritAOverB: " );
   printf( "%e", m_dCritAOverB );
   printf( "\nEosPvtTable.iTranslate: " );
   printf( "%i", m_iTranslate );
   printf( "\nEosPvtTable.iTables: " );
   printf( "%i", m_iTables );
   printf( "\nEosPvtTable.pTemperature[0]: " );
   printf( "%e", m_pTemperature[0] );
   printf( "\nABC Data:" );
   for ( i = 0; i < m_iEosComp; i++ )
   {
      printf( "\n" );
      printf( "%i", i );
      printf( " " );
      printf( "%e", m_pBdata[i] );
      printf( " " );
      printf( "%e", m_pCdata[i] );
      for ( j = 0; j < m_iEosComp; j++ )
      {
         printf( " " );
         printf( "%e", m_pAdata[i * m_iEosComp + j] );
      }
   }

   printf( "\n" );
}


/* 
// WritePvtInfo
//
// Set the correct pvt tables 
//
// iM ** constant **
//    Number of objects in slice
// isothermal ** constant **
//    Indicator as to whether this is an isothermal system
//    or there are temperature dependencies
// isSalt ** constant **
//    Indicator as to whether this is for aqueous phase
//    computations (1) or hydrocarbon phases
// iWaterComp ** constant **
//    Indicator for the index of the water component within
//    the aqueous phase components
// pAbcOffSetT
//    Pointer for the offset within the multiple ABC
//    tables for each object.  This will be the index
//    for the temperature interpolation
// pMultipleAbcT
//    Indicator as to whether there is a single ABC table
//    involved in the computations or multiple.  The
//    computations within the model differ for the 
//    different models
// pT 
//    Temperatures which are set in isothermal mode
// pMw
//    Return a copy of the molecular weights to the flasher
//
// 1) Set the molecular weights.  This depends upon which
//    phase is being calculated and in addition as to whether
//    salt components or a salinity is used for water
// 2) If isothermal set temperature from the temperature
//    entry in the table
// 3) If only one table entry or for water phase, only
//    one table.  Do not set temperature
// 4) If multiple table entries do a table lookup and
//    set the offset in the pvt tables
// 5) If multiple tables see if more than a single table used
*/
void EosPvtTable::WritePvtInfo( int iM, int i1, int i2, int isothermal, int isSalt, int iWaterComp, int *pAbcOffSetT,
                                int *pMultipleAbcT, double *pT, double *pMw )
{
   int    i;
   int    iLow;
   int    iHigh;
   int    iN;
   int    iNi;
   int    iOffsetT;
   double dA;

   /* Set the molecular weights */
   if ( isSalt )
   {
      if ( m_iSaltComp == 0 )
      {
         pMw[0] = 18.01534 + 0.00001801534 * m_dSalinity;
      }
      else
      {
         i = 0;
         for ( iNi = 0; iNi < m_iSaltComp + 1; iNi++ )
         {
            if ( iNi == iWaterComp )
            {
               pMw[iNi] = 18.01534 + 0.00001801534 * m_dSalinity;
            }
            else if ( m_pSaltMW[i] > 0.0 )
            {
               pMw[iNi] = m_pSaltMW[i++];
            }
            else
            {
               pMw[iNi] = 18.01534 - 0.00001801534 * m_pSaltMW[i++];
            }
         }
      }
   }
   else
   {
      for ( iNi = 0; iNi < m_iEosComp; iNi++ )
      {
         pMw[iNi] = m_pMolecularWeight[iNi];
      }
   }

   /* Set temperatures */
   if ( isothermal )
   {
      if ( m_iTables )
      {
         iOffsetT = 1 + m_iEosComp * ( 4 + 2 * m_iEosComp );
         iOffsetT *= ( m_iTables - 1 );
         *pAbcOffSetT = iOffsetT;
      }
      else
      {
         iOffsetT = 0;
      }

      dA = m_pTemperature[iOffsetT];
      for ( i = i1; i < i2; i++ )
      {
         pT[i] = dA;
      }

      *pMultipleAbcT = EOS_OPTION_OFF;
   }

   /* Water phase */
   else if ( isSalt )
   {
      if ( m_iTables )
      {
         *pAbcOffSetT = 0;
      }

      *pMultipleAbcT = EOS_OPTION_OFF;
   }

   /* Not ABC tables */
   else if ( m_iMethod != EOS_PVT_TABLE )
   {
      *pMultipleAbcT = EOS_OPTION_ON;
   }

   /* Default is no multiple abc tables */
   else if ( m_iTables <= 1 )
   {
      if ( m_iTables )
      {
         *pAbcOffSetT = 0;
      }

      *pMultipleAbcT = EOS_OPTION_OFF;
   }

   /* Table lookup */
   else
   {
      iOffsetT = 1 + m_iEosComp * ( 4 + 2 * m_iEosComp );
      for ( i = i1; i < i2; i++ )
      {
         dA = pT[i];
         iLow = 0;
         iHigh = m_iTables - 1;
         while ( iLow != iHigh )
         {
            iN = ( iLow + iHigh ) / 2;
            if ( dA > m_pTemperature[iN * iOffsetT] )
            {
               iLow = iN + 1;
            }
            else
            {
               iHigh = iN;
            }
         }

         pAbcOffSetT[i] = iLow * iOffsetT;
      }

      /* Check to see if same table */
      iN = pAbcOffSetT[0];
      *pMultipleAbcT = EOS_OPTION_OFF;
      for ( i = 1; i < iM; i++ )
      {
         *pMultipleAbcT = ( iN == pAbcOffSetT[i] ) ? *pMultipleAbcT : EOS_OPTION_ON;
      }
   }
}


/* 
// WriteTemperature
//
// Write the default temperature for the table
//
// Returns the default temperature
//
// 1) The default temperature is that of the last pvt
//    table
*/
double EosPvtTable::WriteTemperature( void )
{
   int iOffsetT;

   /* Determine the offset in the array */
   if ( m_iTables )
   {
      iOffsetT = 1 + m_iEosComp * ( 4 + 2 * m_iEosComp );
      iOffsetT *= ( m_iTables - 1 );
   }
   else
   {
      iOffsetT = 0;
   }

   /* Return the proper value */
   return ( m_pTemperature[iOffsetT] );
}


/* 
// SolveCubic
//
// Cubic equation of state routine 
//
// iM ** constant **
//    Number of objects
// iGetF ** constant **
//    Indicator whether to get fugacity
//       EOS_FUGACITY
//          Calculate the fugacity
//       EOS_NOFUGACITY
//          Do not calculate the fugacity
// iGetH ** constant **
//    Indicator whether to get enthalpy
//       EOS_HEAT
//          Calculate the enthalpy
//       EOS_NOFUGACITY
//          Do not calculate the enthalpy
// iGetP ** constant **
//    Indicator whether to get chemical potential
//       EOS_POTENTIAL
//          Calculate the chemical potential
//       EOS_NOFUGACITY
//          Do not calculate the chemical potential
//    Note that in order to calculate the chemical potential
//    the iGetF flag must also be set
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
// iPhase ** constant **
//    Indicator for getting phase identification flag
//       EOS_PHASEID
//          Get the phase id
//       EOS_NOPHASEID
//          Do not get the phase id
// iMultipleAbcT ** constant **
//    Indicator whether there is only a single entry in temperature
//    tables involved or whether there are multiple
// pAbcOffsetT ** constant **
//    Pointer for each object giving its location in the temperature
//    tables
// pP ** constant **
//    Pressure for each object
// pT ** constant **
//    Temperature for each object
// pX ** constant **
//    Composition for each object normalized to one
//       first index corresponds to object
//       second index corresponds to component
// pZ 
//    Z factor for each object
// pDZdp
//    Pressure derivative of z factor of each object
// pDZdt
//    Temperature derivative of z factor of each object
// pDZdz
//    Overall mole derivative of z factor of each object
//       first index corresponds to object
//       second index corresponds to component
// pPot
//    Chemical potential or fugacity
//       first index corresponds to object
//       second index corresponds to component
// pDPotdp
//    Pressure derivative of chemical potential or fugacity
//       first index corresponds to object
//       second index corresponds to component
// pDPotdt
//    Temperature derivative of chemical potential or fugacity
//       first index corresponds to object
//       second index corresponds to component
// pDPotdz
//    Molar derivative of chemical potential or fugacity
//       first index corresponds to object
//       second index corresponds to component
//       third index corresponds to derivative
// pHeat
//    Enthalpy for each object
// pDHeatdp
//    Pressure derivative of enthalpy of each object
// pDHeatdt
//    Temperature derivative of enthalpy of each object
// pDHeatdz
//    Overall mole derivative of enthalpy of each object
//       first index corresponds to object
//       second index corresponds to component
// pAT
//    Location of temporary work terms
// pPhase
//    Phase indicator, if requested
//       EOS_SINGLE_PHASE_GAS 
//          Mixture is a gas
//       EOS_SINGLE_PHASE_OIL 
//          Mixture is an oil
//
// 1) Set temporary terms, mostly storage locations for
//    various A and B terms
// 2) Get the A and B terms, as well as derivatives
// 3) Calculate the non-ideal contribution to the heat if required
// 4) Calculate the ideal contribution to the heat if required
// 5) Calculate the chemical potentials or fugacities if requested
// 6) Add in the enthalpy term to chemical potential if requested
*/
void EosPvtTable::SolveCubic( int iM, int iGetF, int iGetH, int iGetP, int iDrv, int iPhase, int iMultipleAbcT,
                              int *pAbcOffsetT, double *pP, double *pT, double *pX, double *pZ, double *pDZdp,
                              double *pDZdt, double *pDZdz, double *pPot, double *pDPotdp, double *pDPotdt,
                              double *pDPotdz, double *pHeat, double *pDHeatdp, double *pDHeatdt, double *pDHeatdz,
                              double *pAT, int *pPhase )
{
   double *pAijOverAT;
   double *pBT;
   double *pAOverBT;
   double *pDAdtOverAT;
   double *pDBdtOverBT;
   double *pHelp3T;
   double *pHelp4T;
   double *pHelp5T;
   double *pHelp6T;
   double *pDAidtOverAiT;
   double *pD2Adt2OverAT;
   double *pD2Bdt2OverBT;
   double *pHelp1T;
   double *pHelp2T;
   double *pDBidtOverBiT;
   double *pAiOverAT;
   double *pBiOverBT;
   double *pHelp7T;
   int     iEnergy;
   int     iNcm;

   /* Set up work terms */
   iNcm = m_iEosComp * iM;
   pAijOverAT = pDPotdz;
   pBT = pAT + iM;
   pAOverBT = pBT + iM;
   pDAdtOverAT = pAOverBT + iM;
   pDBdtOverBT = pDAdtOverAT + iM;
   pD2Adt2OverAT = pDBdtOverBT + iM;
   pD2Bdt2OverBT = pD2Adt2OverAT + iM;
   pHelp1T = pD2Bdt2OverBT + iM;
   pHelp2T = pHelp1T + iM;
   pHelp3T = pHelp2T + iM;
   pHelp4T = pHelp3T + iM;
   pHelp5T = pHelp4T + iM;
   pHelp6T = pHelp5T + iM;
   pDAidtOverAiT = pHelp6T + iM;
   pDBidtOverBiT = pDAidtOverAiT + iNcm;
   pAiOverAT = pDBidtOverBiT + iNcm;
   pBiOverBT = pAiOverAT + iNcm;
   pHelp7T = pAT;

   /* Calculate a and b terms */
   /* TODO: The next line is too long. Please rewrite to make it shorter. */
   GetABTerms( iM, iGetF, iGetH, iDrv, iMultipleAbcT, pAbcOffsetT, pP, pT, pX, pAT, pBT, pAOverBT, pAiOverAT, pBiOverBT, pAijOverAT, pD2Adt2OverAT, pD2Bdt2OverBT, pDAidtOverAiT, pDBidtOverBiT, pDAdtOverAT, pDBdtOverBT, pHelp1T, pHelp2T, pHelp3T, pHelp4T, pHelp5T, pHelp6T );

   /* Get z factor */
   iEnergy = iGetF || iGetH;

   /* TODO: The next line is too long. Please rewrite to make it shorter. */
   ZFactor( iM, iDrv, iEnergy, iPhase, pP, pPhase, pZ, pDZdp, pDZdt, pDZdz, pAT, pBT, pAOverBT, pAiOverAT, pBiOverBT, pDAdtOverAT, pDBdtOverBT, pHelp1T, pHelp2T, pHelp3T, pHelp4T, pHelp5T );

   /* Get heat */
   if ( iGetH )
   {
      /* TODO: The next line is too long. Please rewrite to make it shorter. */
      NonIdealHeat( iM, iDrv, pP, pT, pZ, pDZdp, pDZdt, pDZdz, pHeat, pDHeatdp, pDHeatdt, pDHeatdz, pBT, pAOverBT, pAiOverAT, pBiOverBT, pDAdtOverAT, pDBdtOverBT, pD2Adt2OverAT, pD2Bdt2OverBT, pDAidtOverAiT, pDBidtOverBiT, pHelp1T, pHelp2T, pHelp3T, pHelp4T, pHelp5T, pHelp6T );

      /* Get the ideal heat terms */
      if ( m_iHeat )
      {
         IdealHeat( iM, iDrv, pT, pX, pHeat, pDHeatdt, pDHeatdz, pHelp1T );
      }
   }

   /* Calculate the chemical potentials */
   if ( iGetF )
   {
      /* TODO: The next line is too long. Please rewrite to make it shorter. */
      Potential( iM, iDrv, iGetP, pP, pT, pZ, pDZdp, pDZdt, pDZdz, pPot, pDPotdp, pDPotdt, pDPotdz, pBT, pAOverBT, pAiOverAT, pBiOverBT, pAijOverAT, pDAdtOverAT, pDBdtOverBT, pDAidtOverAiT, pDBidtOverBiT, pHelp1T, pHelp2T, pHelp3T, pHelp4T, pHelp5T, pHelp6T, pHelp7T );

      /* Get the ideal terms */
      if ( m_iHeat && iGetP )
      {
         IdealPotential( iM, iDrv, pT, pPot, pDPotdt, pHelp1T );
      }
   }

   /* Volume translation used */
   if ( m_iTranslate && ( ( iGetF && iGetP ) || iGetH ) )
   {
      /* Get terms */
      VolumeTranslationC( iM, iDrv, iGetH, iMultipleAbcT, pAbcOffsetT, pT, pAiOverAT, pBiOverBT, pDAidtOverAiT );

      /* Add contribution to potential */
      if ( iGetP && iGetF )
      {
         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         VolumeTranslationPotential( iM, iDrv, pAiOverAT, pBiOverBT, pP, pX, pZ, pDZdp, pDZdt, pDZdz, pPot, pDPotdp, pDPotdt, pHelp1T, pHelp2T, pHelp3T );
      }

      /* Add contribution to heat terms */
      if ( iGetH )
      {
         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         VolumeTranslationHeat( iM, iDrv, pAiOverAT, pBiOverBT, pDAidtOverAiT, pP, pT, pX, pHeat, pDHeatdp, pDHeatdt, pDHeatdz, pHelp1T, pHelp2T, pHelp3T );
      }
   }
}


/* 
// GetABTerms
//
// Driver for calculating a and b terms for cubic equation of state
//
// iM ** constant **
//    Number of objects
// iGetF ** constant **
//    Indicator whether to get fugacity
//       EOS_FUGACITY
//          Calculate the fugacity
//       EOS_NOFUGACITY
//          Do not calculate the fugacity
// iGetH ** constant **
//    Indicator whether to get enthalpy
//       EOS_HEAT
//          Calculate the enthalpy
//       EOS_NOFUGACITY
//          Do not calculate the enthalpy
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
// iMultipleAbcT ** constant **
//    Indicator whether there is only a single entry in temperature
//    tables involved or whether there are multiple
// pAbcOffsetT ** constant **
//    Pointer for each object giving its location in the temperature
//    tables
// pP ** constant **
//    Pressure for each object
// pT ** constant **
//    Temperature for each object
// pX ** constant **
//    Composition for each object normalized to one
//       first index corresponds to object
//       second index corresponds to component
// pAT
//    "A" coefficient for equation-of-state
// pBT 
//    "B" coefficient for equation-of-state
// pAOverBT
//    A over B
// pAiOverAT
//    Ai over A
// pBiOverBT
//    Bi over B
// pAijOverAT
//    Aij over A
// pD2Adt2OverAT
//    Second derivative of A wrt temperature over A
// pD2Bdt2OverBT
//    Second derivative of B wrt temperature over B
// pDAidtOverAiT
//    Derivative of Ai wrt temperature over Ai
// pDBidtOverBT
//    Derivative of Bi wrt temperature over Bi
// pDAdtOverAT
//    Derivative of A wrt temperature over A
// pDBdtOverBT
//    Derivative of B wrt temperature over B
// pHelp1T
//    Help vector
// pHelp2T
//    Help vector
//
// 1) Call the appropriate routine
//    GetIdeal for ideal gas
/     GetAB for ABC tables
//    GetRK for Redlich Kwong
//    GetEos for other forms
*/
void EosPvtTable::GetABTerms( int iM, int iGetF, int iGetH, int iDrv, int iMultipleAbcT, int *pAbcOffsetT, double *pP,
                              double *pT, double *pX, double *pAT, double *pBT, double *pAOverBT, double *pAiOverAT,
                              double *pBiOverBT, double *pAijOverAT, double *pD2Adt2OverAT, double *pD2Bdt2OverBT,
                              double *pDAidtOverAiT, double *pDBidtOverBiT, double *pDAdtOverAT, double *pDBdtOverBT,
                              double *pHelp1T, double *pHelp2T, double *pHelp3T, double *pHelp4T, double *pHelp5T,
                              double *pHelp6T )
{
   /* Calculate a and b terms for ideal gas */
   if ( m_iMethod == EOS_PVT_IDEAL )
   {
      /* TODO: The next line is too long. Please rewrite to make it shorter. */
      GetIdeal( iM, iGetF, iGetH, iDrv, pAT, pBT, pAOverBT, pAiOverAT, pBiOverBT, pAijOverAT, pD2Adt2OverAT, pD2Bdt2OverBT, pDAidtOverAiT, pDBidtOverBiT, pDAdtOverAT, pDBdtOverBT );
   }

   /* Calculate a and b terms for tables */
   else if ( m_iMethod == EOS_PVT_TABLE )
   {
      /* TODO: The next line is too long. Please rewrite to make it shorter. */
      GetAB( iM, iGetF, iGetH, iDrv, iMultipleAbcT, pAbcOffsetT, pP, pT, pX, pAT, pBT, pAOverBT, pAiOverAT, pBiOverBT, pAijOverAT, pD2Adt2OverAT, pD2Bdt2OverBT, pDAidtOverAiT, pDBidtOverBiT, pDAdtOverAT, pDBdtOverBT, pHelp1T, pHelp2T, pHelp3T, pHelp4T );
   }

   /* Calculate a and b terms for Redlich Kwong */
   else if ( m_iMethod == EOS_PVT_RK )
   {
      /* TODO: The next line is too long. Please rewrite to make it shorter. */
      GetRK( iM, iGetF, iGetH, iDrv, pP, pT, pX, pAT, pBT, pAOverBT, pAiOverAT, pBiOverBT, pAijOverAT, pD2Adt2OverAT, pD2Bdt2OverBT, pDAidtOverAiT, pDBidtOverBiT, pDAdtOverAT, pDBdtOverBT, pHelp1T, pHelp2T );
   }

   /* Calculate a and b terms for other forms */
   else
   {
      /* TODO: The next line is too long. Please rewrite to make it shorter. */
      GetEos( iM, iGetF, iGetH, iDrv, iMultipleAbcT, pP, pT, pX, pAT, pBT, pAOverBT, pAiOverAT, pBiOverBT, pAijOverAT, pD2Adt2OverAT, pD2Bdt2OverBT, pDAidtOverAiT, pDBidtOverBiT, pDAdtOverAT, pDBdtOverBT, pHelp1T, pHelp2T, pHelp3T, pHelp4T, pHelp5T, pHelp6T );
   }
}


/* 
// GetIdeal
//
// Calculate a and b terms for ideal gas
// iM ** constant **
//    Number of objects
// iGetF ** constant **
//    Indicator whether to get fugacity
//       EOS_FUGACITY
//          Calculate the fugacity
//       EOS_NOFUGACITY
//          Do not calculate the fugacity
// iGetH ** constant **
//    Indicator whether to get enthalpy
//       EOS_HEAT
//          Calculate the enthalpy
//       EOS_NOFUGACITY
//          Do not calculate the enthalpy
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
// pAT
//    "A" coefficient for equation-of-state
// pBT 
//    "B" coefficient for equation-of-state
// pAOverBT
//    A over B
// pAiOverAT
//    Ai over A
// pBiOverBT
//    Bi over B
// pAijOverAT
//    Aij over A
// pD2Adt2OverAT
//    Second derivative of A wrt temperature over A
// pD2Bdt2OverBT
//    Second derivative of B wrt temperature over B
// pDAidtOverAiT
//    Derivative of Ai wrt temperature over Ai
// pDBidtOverBT
//    Derivative of Bi wrt temperature over Bi
// pDAdtOverAT
//    Derivative of A wrt temperature over A
// pDBdtOverBT
//    Derivative of B wrt temperature over B
//
// 1) Set all terms to zero for an ideal gas
*/
void EosPvtTable::GetIdeal( int iM, int iGetF, int iGetH, int iDrv, double *pAT, double *pBT, double *pAOverBT,
                            double *pAiOverAT, double *pBiOverBT, double *pAijOverAT, double *pD2Adt2OverAT,
                            double *pD2Bdt2OverBT, double *pDAidtOverAiT, double *pDBidtOverBiT, double *pDAdtOverAT,
                            double *pDBdtOverBT )
{
   int i;
   int iNc2;
   int iNcm;
   int iNc2m;

   /* Terms */
   iNc2 = m_iEosComp * m_iEosComp;
   iNcm = m_iEosComp * iM;
   iNc2m = iNc2 * iM;

   /* Set A, B, and A/B */
   for ( i = 0; i < iM; i++ )
   {
      pAT[i] = 0;
      pBT[i] = 0;
      pAOverBT[i] = 0;
   }

   /* Set Ai and Bi */
   for ( i = 0; i < iNcm; i++ )
   {
      pAiOverAT[i] = 0;
      pBiOverBT[i] = 0;
   }

   /* Set Aij */
   for ( i = 0; i < iNc2m; i++ )
   {
      pAijOverAT[i] = 0;
   }

   /* Form temperature derivatives */
   if ( iGetH || ( iDrv >= EOS_DRV_T ) )
   {
      for ( i = 0; i < iM; i++ )
      {
         pDAdtOverAT[i] = 0.0;
         pDBdtOverBT[i] = 0.0;
      }

      /* Extra terms for fugaticity */
      if ( iGetF || ( iGetH && iDrv ) )
      {
         for ( i = 0; i < iNcm; i++ )
         {
            pDAidtOverAiT[i] = 0.0;
            pDBidtOverBiT[i] = 0.0;
         }

         /* Extra terms for heat */
         if ( iGetH && ( iDrv >= EOS_DRV_T ) )
         {
            for ( i = 0; i < iM; i++ )
            {
               pD2Adt2OverAT[i] = 0.0;
               pD2Bdt2OverBT[i] = 0.0;
            }
         }
      }
   }
}


/* 
// GetAB
//
// Calculate a and b terms for cubic equation of state
//
// iM ** constant **
//    Number of objects
// iGetF ** constant **
//    Indicator whether to get fugacity
//       EOS_FUGACITY
//          Calculate the fugacity
//       EOS_NOFUGACITY
//          Do not calculate the fugacity
// iGetH ** constant **
//    Indicator whether to get enthalpy
//       EOS_HEAT
//          Calculate the enthalpy
//       EOS_NOFUGACITY
//          Do not calculate the enthalpy
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
// iMultipleAbcT ** constant **
//    Indicator whether there is only a single entry in temperature
//    tables involved or whether there are multiple
// pAbcOffsetT ** constant **
//    Pointer for each object giving its location in the temperature
//    tables
// pP ** constant **
//    Pressure for each object
// pT ** constant **
//    Temperature for each object
// pX ** constant **
//    Composition for each object normalized to one
//       first index corresponds to object
//       second index corresponds to component
// pAT
//    "A" coefficient for equation-of-state
// pBT 
//    "B" coefficient for equation-of-state
// pAOverBT
//    A over B
// pAiOverAT
//    Ai over A
// pBiOverBT
//    Bi over B
// pAijOverAT
//    Aij over A
// pD2Adt2OverAT
//    Second derivative of A wrt temperature over A
// pD2Bdt2OverBT
//    Second derivative of B wrt temperature over B
// pDAidtOverAiT
//    Derivative of Ai wrt temperature over Ai
// pDBidtOverBT
//    Derivative of Bi wrt temperature over Bi
// pDAdtOverAT
//    Derivative of A wrt temperature over A
// pDBdtOverBT
//    Derivative of B wrt temperature over B
// pHelp1T
//    Help vector
// pHelp2T
//    Help vector
// pHelp3T
//    Help vector
// pHelp4T
//    Help vector
//
// 1) There are several variations on the same theme,
//    namely, 
//       a) multiple objects, multiple temperature entries
//       b) single object, multiple temperature entries
//       c) multiple objects, single temperature entry
//       d) single object, single temperature entry
//    For the sake of brevity only the most general will
//    be described
// 2) One will note that derivatives are in general
//    normalized.  This is for computational efficiency
//    in later routines
// 3) First get the temperature dependent part of A and B
//    which are stored in pDADTdata and pDBDTdata
//    For A, store in pAijOverAT.  For B, store in pDBidtOverBiT
//    There are two variations on this part depending upon
//    whether or not all objects have the same temperature entry
//    in the pvt tables 
// 4) If enthalpy derivatives will later be required or temperature
//    derivatives needed then start forming needed terms
//
//       pDAidtOverAiT = Sum pAijOverAT * Xj
//                        j
//
//       pDAdtOverAT = Sum pDAidtOverAT * Xi
//                      i
//
//       pDBdtOverBT = Sum pDBidtOverBT * Xi
//                      i
//
// 5) Then start forming form main terms
//
//       pAijOverAT = pAijOverAT + pAdataij
//
//       pBiOverBT = pBiOverBT + pBdatai
// 
//    There are two variations on this part depending upon
//    whether or not all objects have the same temperature entry
//    in the pvt tables 
//
// 6) Form additional terms
//
//       pAiOverAT = Sum pAijOverAT * Xj
//                    j
//
//       pA = Sum pAiOverAT * Xi
//             i
//
//       pB = Sum pBiOverBT * Xi
//             i
//
// 7) When either temperature derivatives or enthalpy
//    calculations will later be required, form derivatives of 
//    A and B
//
//       pDAdtOverAT = pDAdtOverAT / pAT  - 2 / T
//
//       pDBdtOverBT = pDBdtOverBT / pBT  - 1 / T
//
//    Note that the first term arises since A is a linear
//    function of temperature over each interval
//
// 8) When either temperature derivatives of fugacity or enthalpy
//    calculations with derivatives will later be required form 
//    derivatives of Ai and Bi
//
//       pDAidtOverAiT = pDAidtOverAiT / pDAdtOverAT - 2 / T
//
//       pDBidtOverBiT = pDBidtOverBiT / pDBdtOverBT - 1 / T
//
//    Note that the first term arises since A is a linear
//    function of temperature over each interval
//
// 9) When temperature derivatives of enthalpy required, yet
//    two more terms
//
//       pD2Adt2OverAT = - ( 4 * pDAdtOverAT - 2 / T ) / T
//
//       pD2Bdt2OverBT = - 2 * pDBdtOverBT / T
//
// 10) When derivatives or fugacities required, normalize
//     the terms
//
//       pAiOverAT = pAiOverAT / pAT
//                    
//       pBiOverBT = pBiOverBT / pBT
//
// 11) When derivatives and fugacities required, normalize
//     the term
//
//       pAijOverAT = pAijOverAT / pAT
//
//     Strictly speaking this will provide no additional
//     efficiency but is simply done for consistency
//
// 12) For main terms
//
//       pAT = pAT * P / T / T
//
//       pBT = pBT * P / T
//
//       pAOverBT = pAT / pBT
*/
void EosPvtTable::GetAB( int iM, int iGetF, int iGetH, int iDrv, int iMultipleAbcT, int *pAbcOffsetT, double *pP,
                         double *pT, double *pX, double *pAT, double *pBT, double *pAOverBT, double *pAiOverAT,
                         double *pBiOverBT, double *pAijOverAT, double *pD2Adt2OverAT, double *pD2Bdt2OverBT,
                         double *pDAidtOverAiT, double *pDBidtOverBiT, double *pDAdtOverAT, double *pDBdtOverBT,
                         double *pHelp1T, double *pHelp2T, double *pHelp3T, double *pHelp4T )
{
   double *pTa;
   double *pTb;
   double *pTc;
   double *pTd;
   double *pTe;
   double *pTf;
   double  dA;
   double  dB;
   double  dC;
   double  dD;
   double  dE;
   double  dF;
   double  dG;
   double  dH;
   int     i;
   int     iNi;
   int     iNj;
   int     iNc2;
   int     iTemp;

   /* Terms */
   iNc2 = m_iEosComp * m_iEosComp;

   /* Multiple grid block single table: Store terms */
   if ( iM > 1 && m_iTables == 0 )
   {
      pTb = pBiOverBT;
      for ( iNi = 0; iNi < m_iEosComp; iNi++ )
      {
         dA = m_pBdata[iNi];
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
         for ( i = 0; i < iM; i++ )
         {
            pTb[i] = dA;
         }

         pTb += iM;
      }

      pTa = pAijOverAT;
      pTe = m_pAdata;
      for ( iNi = 0; iNi < iNc2; iNi++ )
      {
         dA = *pTe++;
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
         for ( i = 0; i < iM; i++ )
         {
            pTa[i] = dA;
         }

         pTa += iM;
      }

      /* Form ai terms */
      pTa = pAijOverAT;
      pTb = pAiOverAT;
      for ( iNi = 0; iNi < m_iEosComp; iNi++ )
      {
         pTc = pX;
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
         for ( i = 0; i < iM; i++ )
         {
            pTb[i] = pTc[i] * pTa[i];
         }

         pTa += iM;

         for ( iNj = 1; iNj < m_iEosComp; iNj++ )
         {
            pTc += iM;
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
            for ( i = 0; i < iM; i++ )
            {
               pTb[i] += pTc[i] * pTa[i];
            }

            pTa += iM;
         }

         pTb += iM;
      }

      /* Form a and b terms */
      pTc = pX;
      pTd = pAiOverAT;
      pTe = pBiOverBT;
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
      for ( i = 0; i < iM; i++ )
      {
         dC = pTc[i];
         pAT[i] = dC * pTd[i];
         pBT[i] = dC * pTe[i];
      }

      for ( iNi = 1; iNi < m_iEosComp; iNi++ )
      {
         pTc += iM;
         pTd += iM;
         pTe += iM;
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
         for ( i = 0; i < iM; i++ )
         {
            dC = pTc[i];
            pAT[i] += dC * pTd[i];
            pBT[i] += dC * pTe[i];
         }
      }

      /* Store inverse */
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
      for ( i = 0; i < iM; i++ )
      {
         pHelp2T[i] = 1.0 / pAT[i];
         pHelp3T[i] = 1.0 / pBT[i];
         pHelp4T[i] = 1.0 / pT[i];
      }

      /* Form temperature derivatives */
      if ( iGetH || ( iDrv >= EOS_DRV_T ) )
      {
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
         for ( i = 0; i < iM; i++ )
         {
            dC = -1.0 / pT[i];
            pDAdtOverAT[i] = dC + dC;
            pDBdtOverBT[i] = dC;
         }

         /* Extra terms for fugaticity */
         if ( iGetF || ( iGetH && iDrv ) )
         {
            pTc = pDAidtOverAiT;
            pTd = pDBidtOverBiT;
            for ( iNi = 0; iNi < m_iEosComp; iNi++ )
            {
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
               for ( i = 0; i < iM; i++ )
               {
                  pTc[i] = pDAdtOverAT[i];
                  pTd[i] = pDBdtOverBT[i];
               }

               pTc += iM;
               pTd += iM;
            }
         }

         /* Extra terms for heat */
         if ( iGetH && ( iDrv >= EOS_DRV_T ) )
         {
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
            for ( i = 0; i < iM; i++ )
            {
               dA = -pHelp4T[i];
               pD2Adt2OverAT[i] = 3.0 * dA * pDAdtOverAT[i];
               pD2Bdt2OverBT[i] = ( dA + dA ) * pDBdtOverBT[i];
            }
         }
      }

      /* Normalize ai and bi terms */
      if ( iDrv || iGetF )
      {
         pTd = pAiOverAT;
         pTe = pBiOverBT;
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
         for ( i = 0; i < iM; i++ )
         {
            dA = pHelp2T[i] + pHelp2T[i];
            pAOverBT[i] = dA;
            pTd[i] *= dA;
            pTe[i] *= pHelp3T[i];
         }

         for ( iNi = 1; iNi < m_iEosComp; iNi++ )
         {
            pTd += iM;
            pTe += iM;
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
            for ( i = 0; i < iM; i++ )
            {
               pTd[i] *= pAOverBT[i];
               pTe[i] *= pHelp3T[i];
            }
         }
      }

      /* Normalize aij term */
      if ( iGetF && iDrv )
      {
         pTa = pAijOverAT;
         for ( iNi = 0; iNi < iNc2; iNi++ )
         {
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
            for ( i = 0; i < iM; i++ )
            {
               pTa[i] *= pHelp2T[i];
            }

            pTa += iM;
         }
      }

      /* Set main terms */
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
      for ( i = 0; i < iM; i++ )
      {
         dD = pHelp4T[i];
         dE = pP[i] * dD;
         dC = pAT[i] * dD;
         pAOverBT[i] = dC * pHelp3T[i];
         pAT[i] = dC * dE;
         pBT[i] *= dE;
      }
   }

   /* Single grid block single table: store main terms */
   else if ( m_iTables == 0 )
   {
      dD = *pT;
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
      for ( iNi = 0; iNi < m_iEosComp; iNi++ )
      {
         pBiOverBT[iNi] = m_pBdata[iNi];
      }

#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
      for ( iNi = 0; iNi < iNc2; iNi++ )
      {
         pAijOverAT[iNi] = m_pAdata[iNi];
      }

      /* Form ai terms */
      pTa = pAijOverAT;
      for ( iNi = 0; iNi < m_iEosComp; iNi++ )
      {
         dA = pX[0] * pTa[0];
         for ( iNj = 1; iNj < m_iEosComp; iNj++ )
         {
            dA += pX[iNj] * pTa[iNj];
         }

         pTa += m_iEosComp;
         pAiOverAT[iNi] = dA;
      }

      /* Form a and b terms */
      dC = pX[0];
      dA = dC * pAiOverAT[0];
      dB = dC * pBiOverBT[0];
      for ( iNi = 1; iNi < m_iEosComp; iNi++ )
      {
         dC = pX[iNi];
         dA += dC * pAiOverAT[iNi];
         dB += dC * pBiOverBT[iNi];
      }

      /* Store inverse */
      dF = 1.0 / dA;
      dG = 1.0 / dB;
      dH = 1.0 / dD;

      /* Form temperature derivatives */
      if ( iGetH || ( iDrv >= EOS_DRV_T ) )
      {
         dC = -dH;
         dE = dC + dC;
         *pDAdtOverAT = dE;
         *pDBdtOverBT = dC;

         /* Extra terms for fugaticity */
         if ( iGetF || ( iGetH && iDrv ) )
         {
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
            for ( iNi = 0; iNi < m_iEosComp; iNi++ )
            {
               pDAidtOverAiT[iNi] = dE;
               pDBidtOverBiT[iNi] = dC;
            }
         }

         /* Extra terms for heat */
         if ( iGetH && ( iDrv >= EOS_DRV_T ) )
         {
            dE *= dC;
            *pD2Adt2OverAT = 3.0 * dE;
            *pD2Bdt2OverBT = dE;
         }
      }

      /* Normalize ai and bi terms */
      if ( iDrv || iGetF )
      {
         dC = dF + dF;
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            pAiOverAT[iNi] *= dC;
            pBiOverBT[iNi] *= dG;
         }
      }

      /* Normalize aij term */
      if ( iGetF && iDrv )
      {
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
         for ( iNi = 0; iNi < iNc2; iNi++ )
         {
            pAijOverAT[iNi] *= dF;
         }
      }

      /* Set main terms */
      dE = *pP * dH;
      dC = dA * dH;
      *pAOverBT = dC * dG;
      *pAT = dC * dE;
      *pBT = dB * dE;
   }

   /* Code for multiple tables multiple blocks */
   else if ( iM > 1 )
   {
      /* Multiple abc tables */
      if ( iMultipleAbcT )
      {
         pTa = pAijOverAT;
         pTb = pDBidtOverBiT;
         for ( i = 0; i < iM; i++ )
         {
            iTemp = pAbcOffsetT[i];
            pTc = pTb++;
            pTd = m_pDBdtdata + iTemp;
            for ( iNi = 0; iNi < m_iEosComp; iNi++ )
            {
               *pTc = *pTd++;
               pTc += iM;
            }

            pTc = pTa++;
            pTd = m_pDAdtdata + iTemp;
            for ( iNi = 0; iNi < iNc2; iNi++ )
            {
               *pTc = *pTd++;
               pTc += iM;
            }
         }
      }

      /* Single abc table */
      else
      {
         iTemp = *pAbcOffsetT;
         pTb = pDBidtOverBiT;
         pTe = m_pDBdtdata + iTemp;
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            dA = *pTe++;
            for ( i = 0; i < iM; i++ )
            {
               *pTb++ = dA;
            }
         }

         pTa = pAijOverAT;
         pTe = m_pDAdtdata + iTemp;
         for ( iNi = 0; iNi < iNc2; iNi++ )
         {
            dA = *pTe++;
            for ( i = 0; i < iM; i++ )
            {
               *pTa++ = dA;
            }
         }
      }

      /* Form daidt terms */
      if ( iGetH || iDrv >= EOS_DRV_T )
      {
         pTa = pAijOverAT;
         pTb = pDAidtOverAiT;
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            pTc = pX;
            pTd = pTb;
            for ( i = 0; i < iM; i++ )
            {
               *pTd++ = *pTc++ **pTa++;
            }

            for ( iNj = 1; iNj < m_iEosComp; iNj++ )
            {
               pTd = pTb;
               for ( i = 0; i < iM; i++ )
               {
                  *pTd += *pTc++ **pTa++;
                  pTd++;
               }
            }

            pTb += iM;
         }

         /* Form dadt and dbdt terms */
         pTa = pDAdtOverAT;
         pTb = pDBdtOverBT;
         pTc = pDAidtOverAiT;
         pTd = pDBidtOverBiT;
         pTe = pX;
         for ( i = 0; i < iM; i++ )
         {
            dC = *pTe++;
            *pTa++ = *pTc++ *dC;
            *pTb++ = *pTd++ *dC;
         }

         for ( iNi = 1; iNi < m_iEosComp; iNi++ )
         {
            pTa = pDAdtOverAT;
            pTb = pDBdtOverBT;
            for ( i = 0; i < iM; i++ )
            {
               dC = *pTe++;
               *pTa += *pTc++ *dC;
               *pTb += *pTd++ *dC;
               pTa++;
               pTb++;
            }
         }
      }

      /* Now store constant terms */
      if ( iMultipleAbcT )
      {
         pTa = pAijOverAT;
         pTb = pBiOverBT;
         pTc = pDBidtOverBiT;
         for ( i = 0; i < iM; i++ )
         {
            iTemp = pAbcOffsetT[i];
            dD = pT[i];
            pTd = pTb++;
            pTf = pTc++;
            pTe = m_pBdata + iTemp;
            for ( iNi = 0; iNi < m_iEosComp; iNi++ )
            {
               *pTd = *pTe++ +*pTf * dD;
               pTf += iM;
               pTd += iM;
            }

            pTd = pTa++;
            pTe = m_pAdata + iTemp;
            for ( iNi = 0; iNi < iNc2; iNi++ )
            {
               *pTd = *pTe++ +*pTd * dD;
               pTd += iM;
            }
         }
      }

      /* Single abc table */
      else
      {
         iTemp = *pAbcOffsetT;
         pTb = pBiOverBT;
         pTc = pDBidtOverBiT;
         pTe = m_pBdata + iTemp;
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            dA = *pTe++;
            pTd = pT;
            for ( i = 0; i < iM; i++ )
            {
               *pTb++ = dA +*pTc++ **pTd++;
            }
         }

         pTa = pAijOverAT;
         pTe = m_pAdata + iTemp;
         for ( iNi = 0; iNi < iNc2; iNi++ )
         {
            dA = *pTe++;
            pTd = pT;
            for ( i = 0; i < iM; i++ )
            {
               *pTa = dA +*pTa **pTd++;
               pTa++;
            }
         }
      }

      /* Form ai terms */
      pTa = pAijOverAT;
      pTb = pAiOverAT;
      for ( iNi = 0; iNi < m_iEosComp; iNi++ )
      {
         pTc = pX;
         pTd = pTb;
         for ( i = 0; i < iM; i++ )
         {
            *pTd++ = *pTc++ **pTa++;
         }

         for ( iNj = 1; iNj < m_iEosComp; iNj++ )
         {
            pTd = pTb;
            for ( i = 0; i < iM; i++ )
            {
               *pTd += *pTc++ **pTa++;
               pTd++;
            }
         }

         pTb += iM;
      }

      /* Form a and b terms */
      pTa = pAT;
      pTb = pBT;
      pTc = pX;
      pTd = pAiOverAT;
      pTe = pBiOverBT;
      for ( i = 0; i < iM; i++ )
      {
         dC = *pTc++;
         *pTa++ = dC **pTd++;
         *pTb++ = dC **pTe++;
      }

      for ( iNi = 1; iNi < m_iEosComp; iNi++ )
      {
         pTa = pAT;
         pTb = pBT;
         for ( i = 0; i < iM; i++ )
         {
            dC = *pTc++;
            *pTa += dC **pTd++;
            *pTb += dC **pTe++;
            pTa++;
            pTb++;
         }
      }

      /* Store inverse */
      for ( i = 0; i < iM; i++ )
      {
         pHelp2T[i] = 1.0 / pAT[i];
         pHelp3T[i] = 1.0 / pBT[i];
         pHelp4T[i] = 1.0 / pT[i];
      }

      /* Form temperature derivatives for multiple abc tables */
      if ( iGetH || ( iDrv >= EOS_DRV_T ) )
      {
         pTa = pDAdtOverAT;
         pTb = pDBdtOverBT;
         pTc = pAOverBT;
         pTd = pHelp1T;
         for ( i = 0; i < iM; i++ )
         {
            *pTc = pHelp4T[i];
            *pTd = 2.0 **pTc;
            *pTa = *pTa * pHelp2T[i] -*pTd++;
            *pTb = *pTb * pHelp3T[i] -*pTc++;
            pTa++;
            pTb++;
         }

         /* Extra terms for fugaticity */
         if ( iGetF || ( iGetH && iDrv ) )
         {
            pTa = pDAidtOverAiT;
            pTb = pDBidtOverBiT;
            pTe = pAiOverAT;
            pTf = pBiOverBT;
            for ( iNi = 0; iNi < m_iEosComp; iNi++ )
            {
               pTc = pAOverBT;
               pTd = pHelp1T;
               for ( i = 0; i < iM; i++ )
               {
                  *pTa = *pTa / *pTe++ -*pTd++;
                  *pTb = *pTb / *pTf++ -*pTc++;
                  pTa++;
                  pTb++;
               }
            }
         }

         /* Extra terms for heat */
         if ( iGetH && ( iDrv >= EOS_DRV_T ) )
         {
            pTa = pD2Adt2OverAT;
            pTb = pD2Bdt2OverBT;
            pTc = pDAdtOverAT;
            pTd = pDBdtOverBT;
            pTe = pAOverBT;
            pTf = pHelp1T;
            for ( i = 0; i < iM; i++ )
            {
               dA = -*pTe++;
               *pTa++ = dA * ( 4.0 **pTc++ +*pTf++ );
               *pTb++ = ( dA + dA ) **pTd++;
            }
         }
      }

      /* Normalize ai and bi terms */
      if ( iDrv || iGetF )
      {
         pTc = pAOverBT;
         pTd = pAiOverAT;
         pTe = pBiOverBT;
         for ( i = 0; i < iM; i++ )
         {
            *pTc = pHelp2T[i] + pHelp2T[i];
            *pTd *= *pTc++;
            *pTe *= pHelp3T[i];
            pTd++;
            pTe++;
         }

         for ( iNi = 1; iNi < m_iEosComp; iNi++ )
         {
            pTa = pAOverBT;
            for ( i = 0; i < iM; i++ )
            {
               *pTd *= *pTa++;
               *pTe *= pHelp3T[i];
               pTd++;
               pTe++;
            }
         }
      }

      /* Normalize aij term */
      if ( iGetF && iDrv )
      {
         pTa = pAijOverAT;
         for ( iNi = 0; iNi < iNc2; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               *pTa = *pTa * pHelp2T[i];
               pTa++;
            }
         }
      }

      /* Set main terms */
      pTa = pAT;
      pTb = pBT;
      pTc = pAOverBT;
      pTd = pT;
      pTe = pP;
      for ( i = 0; i < iM; i++ )
      {
         dD = pHelp4T[i];
         dE = *pTe++ *dD;
         dC = *pTa * dD;
         *pTc++ = dC * pHelp3T[i];
         *pTa++ = dC * dE;
         *pTb *= dE;
         pTb++;
      }
   }

   /* Single grid block first store temperature independent terms */
   else
   {
      iTemp = *pAbcOffsetT;
      pTb = pDBidtOverBiT;
      pTc = m_pDBdtdata + iTemp;
      for ( iNi = 0; iNi < m_iEosComp; iNi++ )
      {
         *pTb++ = *pTc++;
      }

      pTa = pAijOverAT;
      pTc = m_pDAdtdata + iTemp;
      for ( iNi = 0; iNi < iNc2; iNi++ )
      {
         *pTa++ = *pTc++;
      }

      /* Form daidt terms */
      if ( iGetH || ( iDrv >= EOS_DRV_T ) )
      {
         pTa = pAijOverAT;
         pTb = pDAidtOverAiT;
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            pTc = pX;
            dA = *pTc++ **pTa++;
            for ( iNj = 1; iNj < m_iEosComp; iNj++ )
            {
               dA += *pTc++ **pTa++;
            }

            *pTb++ = dA;
         }

         /* Form dadt and dbdt terms */
         pTa = pDAidtOverAiT;
         pTb = pDBidtOverBiT;
         pTc = pX;
         dC = *pTc++;
         dA = dC **pTa++;
         dB = dC **pTb++;
         for ( iNi = 1; iNi < m_iEosComp; iNi++ )
         {
            dC = *pTc++;
            dA += dC **pTa++;
            dB += dC **pTb++;
         }

         *pDAdtOverAT = dA;
         *pDBdtOverBT = dB;
      }

      /* Now store constant terms */
      dD = *pT;
      pTa = pBiOverBT;
      pTb = pDBidtOverBiT;
      pTc = m_pBdata + iTemp;
      for ( iNi = 0; iNi < m_iEosComp; iNi++ )
      {
         *pTa++ = *pTc++ +*pTb++ *dD;
      }

      pTa = pAijOverAT;
      pTc = m_pAdata + iTemp;
      for ( iNi = 0; iNi < iNc2; iNi++ )
      {
         *pTa = *pTc++ +*pTa * dD;
         pTa++;
      }

      /* Form ai terms */
      pTa = pAijOverAT;
      pTb = pAiOverAT;
      for ( iNi = 0; iNi < m_iEosComp; iNi++ )
      {
         pTc = pX;
         dA = *pTc++ **pTa++;
         for ( iNj = 1; iNj < m_iEosComp; iNj++ )
         {
            dA += *pTc++ **pTa++;
         }

         *pTb++ = dA;
      }

      /* Form a and b terms */
      pTa = pAiOverAT;
      pTb = pBiOverBT;
      pTc = pX;
      dC = *pTc++;
      dA = dC **pTa++;
      dB = dC **pTb++;
      for ( iNi = 1; iNi < m_iEosComp; iNi++ )
      {
         dC = *pTc++;
         dA += dC **pTa++;
         dB += dC **pTb++;
      }

      /* Store inverse */
      dF = 1.0 / dA;
      dG = 1.0 / dB;
      dH = 1.0 / dD;

      /* Form temperature derivatives for multiple abc tables */
      if ( iGetH || ( iDrv >= EOS_DRV_T ) )
      {
         dC = dH;
         dE = dC + dC;
         *pDAdtOverAT = *pDAdtOverAT * dF - dE;
         *pDBdtOverBT = *pDBdtOverBT * dG - dC;

         /* Extra terms for fugaticity */
         if ( iGetF || ( iGetH && iDrv ) )
         {
            pTa = pDAidtOverAiT;
            pTb = pDBidtOverBiT;
            pTc = pAiOverAT;
            pTd = pBiOverBT;
            for ( iNi = 0; iNi < m_iEosComp; iNi++ )
            {
               *pTa = *pTa / *pTc++ -dE;
               *pTb = *pTb / *pTd++ -dC;
               pTa++;
               pTb++;
            }
         }

         /* Extra terms for heat */
         if ( iGetH && ( iDrv >= EOS_DRV_T ) )
         {
            dC = -dC;
            *pD2Adt2OverAT = dC * ( 4.0 **pDAdtOverAT + dE );
            *pD2Bdt2OverBT = ( dC + dC ) **pDBdtOverBT;
         }
      }

      /* Normalize ai and bi terms */
      if ( iDrv || iGetF )
      {
         dC = dF + dF;
         pTa = pAiOverAT;
         pTb = pBiOverBT;
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            *pTa *= dC;
            *pTb *= dG;
            pTa++;
            pTb++;
         }
      }

      /* Normalize aij term */
      if ( iGetF && iDrv )
      {
         pTa = pAijOverAT;
         for ( iNi = 0; iNi < iNc2; iNi++ )
         {
            *pTa = *pTa * dF;
            pTa++;
         }
      }

      /* Set main terms */
      dE = *pP * dH;
      dC = dA * dH;
      *pAOverBT = dC * dG;
      *pAT = dC * dE;
      *pBT = dB * dE;
   }
}


/* 
// GetEos
//
// Calculate a and b terms for cubic equation of state
//
// iM ** constant **
//    Number of objects
// iGetF ** constant **
//    Indicator whether to get fugacity
//       EOS_FUGACITY
//          Calculate the fugacity
//       EOS_NOFUGACITY
//          Do not calculate the fugacity
// iGetH ** constant **
//    Indicator whether to get enthalpy
//       EOS_HEAT
//          Calculate the enthalpy
//       EOS_NOFUGACITY
//          Do not calculate the enthalpy
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
// iMultipleAbcT
//    Indicator for multiple temperatures
// pP ** constant **
//    Pressure for each object
// pT ** constant **
//    Temperature for each object
// pX ** constant **
//    Composition for each object normalized to one
//       first index corresponds to object
//       second index corresponds to component
// pAT
//    "A" coefficient for equation-of-state
// pBT 
//    "B" coefficient for equation-of-state
// pAOverBT
//    A over B
// pAiOverAT
//    Ai over A
// pBiOverBT
//    Bi over B
// pAijOverAT
//    Aij over A
// pD2Adt2OverAT
//    Second derivative of A wrt temperature over A
// pD2Bdt2OverBT
//    Second derivative of B wrt temperature over B
// pDAidtOverAiT
//    Derivative of Ai wrt temperature over Ai
// pDBidtOverBT
//    Derivative of Bi wrt temperature over Bi
// pDAdtOverAT
//    Derivative of A wrt temperature over A
// pDBdtOverBT
//    Derivative of B wrt temperature over B
// pHelp1T
//    Help vector
// pHelp2T
//    Help vector
//
// 1) One will note that derivatives are in general
//    normalized.  This is for computational efficiency
//    in later routines
// 2) First get Aij, which is written as
//
//    pAijOverA = pAdataij / T + pDAdsdataij / T ^ 0.5 + pDAdtdataij
//
// 3) Form additional terms
//
//       pAiOverAT = Sum pAijOverAT * Xj
//                    j
//
//       pBiOverBT = pBdatai
//
//       pA = Sum pAiOverAT * Xi
//             i
//
//       pB = Sum pBiOverBT * Xi
//             i
//
// 4) If enthalpy derivatives will later be required or temperature
//    derivatives needed then start forming needed terms
//
//       pDAidtOverAiT = Sum d(pAijOverAT)/dT * Xj
//                        j
//
//       pDBidtOverBiT = d(pBdata)/dT
//
//       pDAdtOverAT = Sum pDAidtOverAT * Xi
//                      i
//
//       pDBdtOverBT = Sum pDBidtOverBT * Xi
//                      i
//
//    Also form second derivatives of A and B if needed, and perform
//    normalizations
// 5) For main terms
//
//       pAT = pAT * P / T
//
//       pBT = pBT * P / T
//
//       pAOverBT = pAT / pBT
*/
void EosPvtTable::GetEos( int iM, int iGetF, int iGetH, int iDrv, int iMultipleAbcT, double *pP, double *pT, double *pX,
                          double *pAT, double *pBT, double *pAOverBT, double *pAiOverAT, double *pBiOverBT,
                          double *pAijOverAT, double *pD2Adt2OverAT, double *pD2Bdt2OverBT, double *pDAidtOverAiT,
                          double *pDBidtOverBiT, double *pDAdtOverAT, double *pDBdtOverBT, double *pHelp1T,
                          double *pHelp2T, double *pHelp3T, double *pHelp4T, double *pHelp5T, double *pHelp6T )
{
   double  dA;
   double  dB;
   double  dC;
   double  dD;
   double  dE;
   double  dF;
   double  dS;
   double  dT;
   double  dU;
   double  dG = 0.0;
   double  dH = 0.0;
   double *pTa;
   double *pTb;
   double *pTc;
   double *pTd;
   double *pTe;
   double *pTf;
   double *pTg;
   int     i;
   int     iNi;
   int     iNj;
   int     iNc2;
   int     iTemp;
   int     iTest;

   /* Test for needing multiple temperature terms */
   iTest = iMultipleAbcT || iGetH || iDrv >= EOS_DRV_T;

   /* Multiple grid blocks with multiple temperatures */
   if ( iM > 1 && iTest )
   {
      /* Initialize needed terms */
      for ( i = 0; i < iM; i++ )
      {
         pHelp5T[i] = 1.0 / pT[i];
         pHelp1T[i] = sqrt( pHelp5T[i] );
         pHelp6T[i] = 1.0 / pHelp1T[i];
         pAT[i] = 0.0;
         pBT[i] = 0.0;
      }

      /* Derivative terms */
      if ( iGetH || ( iDrv >= EOS_DRV_T ) )
      {
         for ( i = 0; i < iM; i++ )
         {
            pDAdtOverAT[i] = 0.0;
            pDBdtOverBT[i] = 0.0;
         }

         /* Second derivative terms */
         if ( iGetH && ( iDrv >= EOS_DRV_T ) )
         {
            for ( i = 0; i < iM; i++ )
            {
               pD2Adt2OverAT[i] = 0.0;
               pD2Bdt2OverBT[i] = 0.0;
            }
         }
      }

      /* Set pointers */
      pTa = pAiOverAT;
      pTb = pAijOverAT;
      pTc = pDAidtOverAiT;
      pTd = pBiOverBT;
      pTe = pDBidtOverBiT;
      pTf = pX;
      iTemp = 0;

      /* Form A and B term */
      for ( iNi = 0; iNi < m_iEosComp; iNi++ )
      {
         pTg = pX;
         dA = m_pAdata[iTemp];
         dB = m_pDAdsdata[iTemp];
         dC = m_pDAdtdata[iTemp];

         /* Linear binary terms */
         if ( m_iTypeOfBij == EOS_LINEAR_C )
         {
            dG = m_pDAdrdata[iTemp];
            dH = m_pDAdldata[iTemp];

            /* First component in Ai */
            for ( i = 0; i < iM; i++ )
            {
               dD = dA * pHelp5T[i] + dB * pHelp1T[i] + dC + dG * pHelp6T[i] + dH * pT[i];
               pTa[i] = dD * pTg[i];
               *pTb++ = dD;
            }

            /* Form daidt terms */
            if ( iGetH || ( iDrv >= EOS_DRV_T ) )
            {
               dD = dA + dA;
               dE = 1.5 * dB;
               dH = 0.5 * dG;
               for ( i = 0; i < iM; i++ )
               {
                  pTc[i] = -pTg[i] * ( dD * pHelp5T[i] + dE * pHelp1T[i] + dC + dH * pHelp6T[i] );
               }

               /* Form d2adt2 terms */
               if ( iGetH && ( iDrv >= EOS_DRV_T ) )
               {
                  dD = 6.0 * dA;
                  dE = 3.75 * dB;
                  dF = dC + dC;
                  dH = 0.75 * dG;
                  for ( i = 0; i < iM; i++ )
                  {
                     pHelp2T[i] = pTg[i] * ( dD * pHelp5T[i] + dE * pHelp1T[i] + dF + dH * pHelp6T[i] );
                  }
               }
            }
         }

         /* Constant binary terms */
         else
         {
            /* First component in Ai */
            for ( i = 0; i < iM; i++ )
            {
               dD = dA * pHelp5T[i] + dB * pHelp1T[i] + dC;
               pTa[i] = dD * pTg[i];
               *pTb++ = dD;
            }

            /* Form daidt terms */
            if ( iGetH || ( iDrv >= EOS_DRV_T ) )
            {
               dD = dA + dA;
               dE = 1.5 * dB;
               for ( i = 0; i < iM; i++ )
               {
                  pTc[i] = -pTg[i] * ( dD * pHelp5T[i] + dE * pHelp1T[i] + dC );
               }

               /* Form d2adt2 terms */
               if ( iGetH && ( iDrv >= EOS_DRV_T ) )
               {
                  dD = 6.0 * dA;
                  dE = 3.75 * dB;
                  dF = dC + dC;
                  for ( i = 0; i < iM; i++ )
                  {
                     pHelp2T[i] = pTg[i] * ( dD * pHelp5T[i] + dE * pHelp1T[i] + dF );
                  }
               }
            }
         }

         iTemp++;

         /* Other Components */
         for ( iNj = 1; iNj < m_iEosComp; iNj++ )
         {
            pTg += iM;
            dA = m_pAdata[iTemp];
            dB = m_pDAdsdata[iTemp];
            dC = m_pDAdtdata[iTemp];

            /* Linear binary terms */
            if ( m_iTypeOfBij == EOS_LINEAR_C )
            {
               dG = m_pDAdrdata[iTemp];
               dH = m_pDAdldata[iTemp];

               /* Ai */
               for ( i = 0; i < iM; i++ )
               {
                  dD = dA * pHelp5T[i] + dB * pHelp1T[i] + dC + dG * pHelp6T[i] + dH * pT[i];
                  pTa[i] += dD * pTg[i];
                  *pTb++ = dD;
               }

               /* Form daidt terms */
               if ( iGetH || ( iDrv >= EOS_DRV_T ) )
               {
                  dD = dA + dA;
                  dE = 1.5 * dB;
                  dH = 0.5 * dG;
                  for ( i = 0; i < iM; i++ )
                  {
                     pTc[i] -= pTg[i] * ( dD * pHelp5T[i] + dE * pHelp1T[i] + dC + dH * pHelp6T[i] );
                  }

                  /* Form d2adt2 terms */
                  if ( iGetH && ( iDrv >= EOS_DRV_T ) )
                  {
                     dD = 6.0 * dA;
                     dE = 3.75 * dB;
                     dF = dC + dC;
                     dH = 0.75 * dG;
                     for ( i = 0; i < iM; i++ )
                     {
                        pHelp2T[i] += pTg[i] * ( dD * pHelp5T[i] + dE * pHelp1T[i] + dF + dH * pHelp6T[i] );
                     }
                  }
               }
            }

            /* Constant binary terms */
            else
            {
               /* Ai */
               for ( i = 0; i < iM; i++ )
               {
                  dD = dA * pHelp5T[i] + dB * pHelp1T[i] + dC;
                  pTa[i] += dD * pTg[i];
                  *pTb++ = dD;
               }

               /* Form daidt terms */
               if ( iGetH || ( iDrv >= EOS_DRV_T ) )
               {
                  dD = dA + dA;
                  dE = 1.5 * dB;
                  for ( i = 0; i < iM; i++ )
                  {
                     pTc[i] -= pTg[i] * ( dD * pHelp5T[i] + dE * pHelp1T[i] + dC );
                  }

                  /* Form d2adt2 terms */
                  if ( iGetH && ( iDrv >= EOS_DRV_T ) )
                  {
                     dD = 6.0 * dA;
                     dE = 3.75 * dB;
                     dF = dC + dC;
                     for ( i = 0; i < iM; i++ )
                     {
                        pHelp2T[i] += pTg[i] * ( dD * pHelp5T[i] + dE * pHelp1T[i] + dF );
                     }
                  }
               }
            }

            iTemp++;
         }

         /* Update A and B terms */
         dA = m_pBdata[iNi];
         for ( i = 0; i < iM; i++ )
         {
            dB = pTf[i];
            *pTd = dA;
            pAT[i] += dB **pTa++;
            pBT[i] += dB **pTd++;
         }

         /* Derivatives */
         if ( iGetH || ( iDrv >= EOS_DRV_T ) )
         {
            dB = -dA;
            for ( i = 0; i < iM; i++ )
            {
               dC = pTf[i];
               dD = pHelp5T[i];
               *pTc = *pTc * dD;
               *pTe = dB * dD;
               pDAdtOverAT[i] += dC **pTc++;
               pDBdtOverBT[i] += dC **pTe++;
            }

            /* Second derivatives */
            if ( iGetH && ( iDrv >= EOS_DRV_T ) )
            {
               dB = dA + dA;
               for ( i = 0; i < iM; i++ )
               {
                  dC = pHelp5T[i];
                  dD = pTf[i] * dC * dC;
                  pD2Adt2OverAT[i] += dD * pHelp2T[i];
                  pD2Bdt2OverBT[i] += dD * dB;
               }
            }
         }

         /* Update composition pointer */
         pTf += iM;
      }

      /* Take inverse */
      for ( i = 0; i < iM; i++ )
      {
         pHelp3T[i] = 1.0 / pAT[i];
         pHelp4T[i] = 1.0 / pBT[i];
      }

      /* Normalize derivatives */
      if ( iGetH || ( iDrv >= EOS_DRV_T ) )
      {
         pTa = pDAidtOverAiT;
         pTb = pDBidtOverBiT;
         pTc = pAiOverAT;
         pTd = pBiOverBT;
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               *pTa = *pTa / *pTc++;
               *pTb = *pTb / *pTd++;
               pTa++;
               pTb++;
            }
         }

         for ( i = 0; i < iM; i++ )
         {
            pDAdtOverAT[i] = pDAdtOverAT[i] * pHelp3T[i];
            pDBdtOverBT[i] = pDBdtOverBT[i] * pHelp4T[i];
         }

         /* Normalize second derivatives */
         if ( iGetH && ( iDrv >= EOS_DRV_T ) )
         {
            for ( i = 0; i < iM; i++ )
            {
               pD2Adt2OverAT[i] = pD2Adt2OverAT[i] * pHelp3T[i];
               pD2Bdt2OverBT[i] = pD2Bdt2OverBT[i] * pHelp4T[i];
            }
         }
      }

      /* Normalize ai and bi terms */
      if ( iDrv || iGetF )
      {
         pTa = pAiOverAT;
         pTb = pBiOverBT;
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               dA = *pTa;
               *pTa++ = ( dA + dA ) * pHelp3T[i];
               *pTb = *pTb * pHelp4T[i];
               pTb++;
            }
         }

         /* Normalize aij term */
         if ( iGetF && iDrv )
         {
            pTa = pAijOverAT;
            iNc2 = m_iEosComp * m_iEosComp;
            for ( iNi = 0; iNi < iNc2; iNi++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  *pTa = *pTa * pHelp3T[i];
                  pTa++;
               }
            }
         }
      }

      /* Set main terms */
      for ( i = 0; i < iM; i++ )
      {
         dB = pP[i] * pHelp5T[i];
         dC = pAT[i];
         pAOverBT[i] = dC * pHelp4T[i];
         pAT[i] = dC * dB;
         pBT[i] *= dB;
      }
   }

   /* Multiple grid blocks with single temperature */
   else if ( iM > 1 )
   {
      /* Initialize needed terms */
      for ( i = 0; i < iM; i++ )
      {
         pAT[i] = 0.0;
         pBT[i] = 0.0;
      }

      /* Set pointers */
      pTa = pAiOverAT;
      pTb = pAijOverAT;
      pTd = pBiOverBT;
      pTf = pX;
      iTemp = 0;

      /* Form A and B term */
      for ( iNi = 0; iNi < m_iEosComp; iNi++ )
      {
         pTg = pX;
         dA = m_pADefault[iTemp];
         iTemp++;

         /* First component in Ai */
#ifdef IPFtune
         int cnt = iM;
#pragma ivdep
         for ( i = 0; i < cnt; i++ )
         {
            pTa[i] = dA * pTg[i];
            pTb[i] = dA;
         }
#else
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
         for ( i = 0; i < iM; i++ )
         {
            pTa[i] = dA * pTg[i];
            pTb[i] = dA;
         }
#endif

         pTb += iM;

         /* Other Components */
         for ( iNj = 1; iNj < m_iEosComp; iNj++ )
         {
            pTg += iM;
            dA = m_pADefault[iTemp];
            iTemp++;

            /* Ai */
#ifdef IPFtune
            int cnt2 = iM;
#pragma ivdep
            for ( i = 0; i < cnt2; i++ )
            {
               pTa[i] += dA * pTg[i];
               pTb[i] = dA;
            }
#else
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
            for ( i = 0; i < iM; i++ )
            {
               pTa[i] += dA * pTg[i];
               pTb[i] = dA;
            }
#endif

            pTb += iM;
         }

         /* Update A and B terms */
         dA = m_pBdata[iNi];
#ifdef IPFtune
         int cnt2 = iM;
#pragma ivdep
         for ( i = 0; i < cnt2; i++ )
         {
            dB = pTf[i];
            pTd[i] = dA;
            pAT[i] += dB * pTa[i];
            pBT[i] += dB * dA;
         }
#else
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
         for ( i = 0; i < iM; i++ )
         {
            dB = pTf[i];
            pTd[i] = dA;
            pAT[i] += dB * pTa[i];
            pBT[i] += dB * dA;
         }
#endif

         /* Update composition pointer */
         pTf += iM;
         pTd += iM;
         pTa += iM;
      }

      /* Take inverse */
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
#ifdef IPFtune
#pragma ivdep
#endif
      for ( i = 0; i < iM; i++ )
      {
         pHelp3T[i] = 1.0 / pAT[i];
         pHelp4T[i] = 1.0 / pBT[i];
      }

      /* Normalize ai and bi terms */
      if ( iDrv || iGetF )
      {
         pTa = pAiOverAT;
         pTb = pBiOverBT;
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
#ifdef IPFtune
            int cnt = iM;
#pragma ivdep
            for ( i = 0; i < cnt; i++ )
            {
               dA = pTa[i];
               pTa[i] = ( dA + dA ) * pHelp3T[i];
               pTb[i] *= pHelp4T[i];
            }
#else
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
            for ( i = 0; i < iM; i++ )
            {
               dA = pTa[i];
               pTa[i] = ( dA + dA ) * pHelp3T[i];
               pTb[i] *= pHelp4T[i];
            }
#endif

            pTa += iM;
            pTb += iM;
         }

         /* Normalize aij term */
         if ( iGetF && iDrv )
         {
            pTa = pAijOverAT;
            iNc2 = m_iEosComp * m_iEosComp;
            for ( iNi = 0; iNi < iNc2; iNi++ )
            {
#ifdef IPFtune
               int cnt = iM;
#pragma ivdep
               for ( i = 0; i < cnt; i++ )
               {
                  pTa[i] *= pHelp3T[i];
               }
#else
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
               for ( i = 0; i < iM; i++ )
               {
                  pTa[i] *= pHelp3T[i];
               }
#endif

               pTa += iM;
            }
         }
      }

      /* Set main terms */
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
#ifdef IPFtune
#pragma ivdep
#endif
      for ( i = 0; i < iM; i++ )
      {
         dB = pP[i] / pT[i];
         dC = pAT[i];
         pAOverBT[i] = dC * pHelp4T[i];
         pAT[i] = dC * dB;
         pBT[i] *= dB;
      }
   }

   /* Single grid block with thermal effects */
   else if ( iTest )
   {
      /* Initialize needed terms */
      dT = 1.0 / *pT;
      dS = sqrt( dT );
      dU = 1.0 / dS;
      dF = 0.0;
      *pAT = 0.0;
      *pBT = 0.0;

      /* Derivative terms */
      if ( iGetH || ( iDrv >= EOS_DRV_T ) )
      {
         *pDAdtOverAT = 0.0;
         *pDBdtOverBT = 0.0;

         /* Second derivative terms */
         if ( iGetH && ( iDrv >= EOS_DRV_T ) )
         {
            *pD2Adt2OverAT = 0.0;
            *pD2Bdt2OverBT = 0.0;
         }
      }

      /* Set pointers */
      pTa = pAijOverAT;
      iTemp = 0;

      /* Form A and B term */
#ifdef IPFtune
#pragma ivdep
#endif
      for ( iNi = 0; iNi < m_iEosComp; iNi++ )
      {
         dA = m_pAdata[iTemp] * dT;
         dB = m_pDAdsdata[iTemp] * dS;
         dC = m_pDAdtdata[iTemp];

         /* Binary temperature terms */
         if ( m_iTypeOfBij == EOS_LINEAR_C )
         {
            dG = m_pDAdrdata[iTemp] * dU;
            dH = m_pDAdldata[iTemp] **pT;

            /* First component in Ai */
            dD = dA + dB + dC + dG + dH;
            dE = pX[0];
            pAiOverAT[iNi] = dD * dE;
            *pTa++ = dD;

            /* Form daidt terms */
            if ( iGetH || ( iDrv >= EOS_DRV_T ) )
            {
               pDAidtOverAiT[iNi] = -dE * ( dA + dA + 1.5 * dB + dC + 0.5 * dG );

               /* Form d2adt2 terms */
               if ( iGetH && ( iDrv >= EOS_DRV_T ) )
               {
                  dF = dE * ( 6.0 * dA + 3.75 * dB + dC + dC + 0.75 * dG );
               }
            }
         }

         /* Linear temperature terms */
         else
         {
            /* First component in Ai */
            dD = dA + dB + dC;
            dE = pX[0];
            pAiOverAT[iNi] = dD * dE;
            *pTa++ = dD;

            /* Form daidt terms */
            if ( iGetH || ( iDrv >= EOS_DRV_T ) )
            {
               pDAidtOverAiT[iNi] = -dE * ( dA + dA + 1.5 * dB + dC );

               /* Form d2adt2 terms */
               if ( iGetH && ( iDrv >= EOS_DRV_T ) )
               {
                  dF = dE * ( 6.0 * dA + 3.75 * dB + dC + dC );
               }
            }
         }

         iTemp++;

         /* Other Components */
#ifdef IPFtune
         double *pAd  = m_pAdata;
         double *pDAs = m_pDAdsdata;
         double *pDAt = m_pDAdtdata;
         /* Binary temperature terms */
         if ( m_iTypeOfBij == EOS_LINEAR_C )
         {
            double *pDAr = m_pDAdrdata;
            double *pDAl = m_pDAdldata;
#pragma ivdep
            for ( iNj = 1; iNj < m_iEosComp; iNj++ )
            {
               dA = pAd[iTemp] * dT;
               dB = pDAs[iTemp] * dS;
               dC = pDAt[iTemp];

               dG = pDAr[iTemp] * dU;
               dH = pDAl[iTemp] **pT;

               /* Ai */
               dD = dA + dB + dC + dG + dH;
               dE = pX[iNj];
               pAiOverAT[iNi] += dD * dE;
               *pTa++ = dD;

               /* Form daidt terms */
               if ( iGetH || ( iDrv >= EOS_DRV_T ) )
               {
                  pDAidtOverAiT[iNi] -= dE * ( dA + dA + 1.5 * dB + dC + 0.5 * dG );

                  /* Form d2adt2 terms */
                  if ( iGetH && ( iDrv >= EOS_DRV_T ) )
                  {
                     dF += dE * ( 6.0 * dA + 3.75 * dB + dC + dC + 0.75 * dG );
                  }
               }

               iTemp++;
            }
         }

         /* Linear temperature terms */
         else
         {
            if ( iGetH && ( iDrv >= EOS_DRV_T ) )
            {
#pragma ivdep
               for ( iNj = 1; iNj < m_iEosComp; iNj++ )
               {
                  dA = pAd[iTemp] * dT;
                  dB = pDAs[iTemp] * dS;
                  dC = pDAt[iTemp];
                  /* Ai */
                  dD = dA + dB + dC;
                  dE = pX[iNj];
                  pAiOverAT[iNi] += dD * dE;
                  *pTa++ = dD;

                  /* Form daidt terms */
                  pDAidtOverAiT[iNi] -= dE * ( dA + dA + 1.5 * dB + dC );

                  /* Form d2adt2 terms */
                  dF += dE * ( 6.0 * dA + 3.75 * dB + dC + dC );
                  iTemp++;
               }
            }
            else 
            {
               bool   fTs = iGetH || ( iDrv >= EOS_DRV_T );
               double tAi = pAiOverAT[iNi];
               double tDA;
               if ( fTs )
               {
                  tDA = pDAidtOverAiT[iNi];
               }
#pragma ivdep
               for ( iNj = 1; iNj < m_iEosComp; iNj++ )
               {
                  dA = pAd[iTemp] * dT;
                  dB = pDAs[iTemp] * dS;
                  dC = pDAt[iTemp];
                  /* Ai */
                  dD = dA + dB + dC;
                  dE = pX[iNj];
                  tAi += dD * dE;
                  *pTa++ = dD;

                  /* Form daidt terms */
                  if ( fTs )
                  {
                     tDA -= dE * ( dA + dA + 1.5 * dB + dC );
                  }
                  iTemp++;
               }
               pAiOverAT[iNi] = tAi;
               if ( fTs )
               {
                   pDAidtOverAiT[iNi] = tDA;
               }
            }
         }
#else
         for ( iNj = 1; iNj < m_iEosComp; iNj++ )
         {
            dA = m_pAdata[iTemp] * dT;
            dB = m_pDAdsdata[iTemp] * dS;
            dC = m_pDAdtdata[iTemp];

            /* Binary temperature terms */
            if ( m_iTypeOfBij == EOS_LINEAR_C )
            {
               dG = m_pDAdrdata[iTemp] * dU;
               dH = m_pDAdldata[iTemp] **pT;

               /* Ai */
               dD = dA + dB + dC + dG + dH;
               dE = pX[iNj];
               pAiOverAT[iNi] += dD * dE;
               *pTa++ = dD;

               /* Form daidt terms */
               if ( iGetH || ( iDrv >= EOS_DRV_T ) )
               {
                  pDAidtOverAiT[iNi] -= dE * ( dA + dA + 1.5 * dB + dC + 0.5 * dG );

                  /* Form d2adt2 terms */
                  if ( iGetH && ( iDrv >= EOS_DRV_T ) )
                  {
                     dF += dE * ( 6.0 * dA + 3.75 * dB + dC + dC + 0.75 * dG );
                  }
               }
            }

            /* Linear temperature terms */
            else
            {
               /* Ai */
               dD = dA + dB + dC;
               dE = pX[iNj];
               pAiOverAT[iNi] += dD * dE;
               *pTa++ = dD;

               /* Form daidt terms */
               if ( iGetH || ( iDrv >= EOS_DRV_T ) )
               {
                  pDAidtOverAiT[iNi] -= dE * ( dA + dA + 1.5 * dB + dC );

                  /* Form d2adt2 terms */
                  if ( iGetH && ( iDrv >= EOS_DRV_T ) )
                  {
                     dF += dE * ( 6.0 * dA + 3.75 * dB + dC + dC );
                  }
               }
            }

            iTemp++;
         }
#endif

         /* Update A and B terms */
         dA = pX[iNi];
         dB = m_pBdata[iNi];
         pBiOverBT[iNi] = dB;
         *pAT += dA * pAiOverAT[iNi];
         *pBT += dA * dB;
         dG = 1.0 / *pAT;
         dH = 1.0 / *pBT;

         /* Derivatives */
         if ( iGetH || ( iDrv >= EOS_DRV_T ) )
         {
            dC = pDAidtOverAiT[iNi] * dT;
            dD = -dB * dT;
            pDAidtOverAiT[iNi] = dC;
            pDBidtOverBiT[iNi] = dD;
            *pDAdtOverAT += dA * dC;
            *pDBdtOverBT += dA * dD;

            /* Second derivatives */
            if ( iGetH && ( iDrv >= EOS_DRV_T ) )
            {
               dC = dB + dB;
               dD = dA * dT * dT;
               *pD2Adt2OverAT += dD * dF;
               *pD2Bdt2OverBT += dD * dC;
            }
         }
      }

      /* Normalize derivatives */
      if ( iGetH || ( iDrv >= EOS_DRV_T ) )
      {
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            pDAidtOverAiT[iNi] = pDAidtOverAiT[iNi] / pAiOverAT[iNi];
            pDBidtOverBiT[iNi] = pDBidtOverBiT[iNi] / pBiOverBT[iNi];
         }

         *pDAdtOverAT = *pDAdtOverAT * dG;
         *pDBdtOverBT = *pDBdtOverBT * dH;

         /* Normalize second derivatives */
         if ( iGetH && ( iDrv >= EOS_DRV_T ) )
         {
            *pD2Adt2OverAT = *pD2Adt2OverAT * dG;
            *pD2Bdt2OverBT = *pD2Bdt2OverBT * dH;
         }
      }

      /* Normalize ai and bi terms */
      if ( iDrv || iGetF )
      {
#ifdef IPFtune
#pragma ivdep
#endif
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            dA = pAiOverAT[iNi];
            pAiOverAT[iNi] = ( dA + dA ) * dG;
            pBiOverBT[iNi] *= dH;
         }

         /* Normalize aij term */
         if ( iGetF && iDrv )
         {
            pTa = pAijOverAT;
            iNc2 = m_iEosComp * m_iEosComp;
#ifdef IPFtune
#pragma ivdep
#endif
            for ( iNi = 0; iNi < iNc2; iNi++ )
            {
               *pTa *= dG;
               pTa++;
            }
         }
      }

      /* Set main terms */
      dB = *pP * dT;
      dC = *pAT;
      dD = *pBT;
      *pAOverBT = dC * dH;
      *pAT = dC * dB;
      *pBT = dD * dB;
   }

   /* Single grid block */
   else
   {
      /* Initialize needed terms */
      *pAT = 0.0;
      *pBT = 0.0;

      /* Set pointers */
      pTa = pAijOverAT;
      pTb = m_pADefault;

      /* Form A and B term */
      for ( iNi = 0; iNi < m_iEosComp; iNi++ )
      {
         /* First component in Ai */
         dA = pTb[0];
         pAiOverAT[iNi] = dA * pX[0];
         pTa[0] = dA;

         /* Other Components */
         for ( iNj = 1; iNj < m_iEosComp; iNj++ )
         {
            dA = pTb[iNj];
            pAiOverAT[iNi] += dA * pX[iNj];
            pTa[iNj] = dA;
         }

         /* Update A and B terms */
         dA = pX[iNi];
         dB = m_pBdata[iNi];
         pBiOverBT[iNi] = dB;
         *pAT += dA * pAiOverAT[iNi];
         *pBT += dA * dB;
         pTa += m_iEosComp;
         pTb += m_iEosComp;
      }

      /* Save the inverse */
      dG = 1.0 / *pAT;
      dH = 1.0 / *pBT;

      /* Normalize ai and bi terms */
      if ( iDrv || iGetF )
      {
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            dA = pAiOverAT[iNi];
            pAiOverAT[iNi] = ( dA + dA ) * dG;
            pBiOverBT[iNi] *= dH;
         }

         /* Normalize aij term */
         if ( iGetF && iDrv )
         {
            iNc2 = m_iEosComp * m_iEosComp;
            for ( iNi = 0; iNi < iNc2; iNi++ )
            {
               pAijOverAT[iNi] *= dG;
            }
         }
      }

      /* Set main terms */
      dB = *pP / *pT;
      dC = *pAT;
      dD = *pBT;
      *pAOverBT = dC * dH;
      *pAT = dC * dB;
      *pBT = dD * dB;
   }
}


/* 
// GetRK
//
// Calculate a and b terms for Redlich Kwong equation of state
//
// iM ** constant **
//    Number of objects
// iGetF ** constant **
//    Indicator whether to get fugacity
//       EOS_FUGACITY
//          Calculate the fugacity
//       EOS_NOFUGACITY
//          Do not calculate the fugacity
// iGetH ** constant **
//    Indicator whether to get enthalpy
//       EOS_HEAT
//          Calculate the enthalpy
//       EOS_NOFUGACITY
//          Do not calculate the enthalpy
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
// pP ** constant **
//    Pressure for each object
// pT ** constant **
//    Temperature for each object
// pX ** constant **
//    Composition for each object normalized to one
//       first index corresponds to object
//       second index corresponds to component
// pAT
//    "A" coefficient for equation-of-state
// pBT 
//    "B" coefficient for equation-of-state
// pAOverBT
//    A over B
// pAiOverAT
//    Ai over A
// pBiOverBT
//    Bi over B
// pAijOverAT
//    Aij over A
// pD2Adt2OverAT
//    Second derivative of A wrt temperature over A
// pD2Bdt2OverBT
//    Second derivative of B wrt temperature over B
// pDAidtOverAiT
//    Derivative of Ai wrt temperature over Ai
// pDBidtOverBT
//    Derivative of Bi wrt temperature over Bi
// pDAdtOverAT
//    Derivative of A wrt temperature over A
// pDBdtOverBT
//    Derivative of B wrt temperature over B
// pHelp1T
//    Help vector
// pHelp2T
//    Help vector
//
// 1) One will note that derivatives are in general
//    normalized.  This is for computational efficiency
//    in later routines
// 2) First get Aij, which is written as
//
//    pAijOverA = pAdataij / T ^ 1.5
//
// 3) Form additional terms
//
//       pAiOverAT = Sum pAijOverAT * Xj
//                    j
//
//       pBiOverBT = pBdatai
//
//       pA = Sum pAiOverAT * Xi
//             i
//
//       pB = Sum pBiOverBT * Xi
//             i
//
// 4) If enthalpy derivatives will later be required or temperature
//    derivatives needed then start forming needed terms
//
//       pDAidtOverAiT = Sum d(pAijOverAT)/dT * Xj
//                        j
//
//       pDBidtOverBiT = d(pBdata)/dT
//
//       pDAdtOverAT = Sum pDAidtOverAT * Xi
//                      i
//
//       pDBdtOverBT = Sum pDBidtOverBT * Xi
//                      i
//
//    Also form second derivatives of A and B if needed, and perform
//    normalizations
// 5) For main terms
//
//       pAT = pAT * P / T
//
//       pBT = pBT * P / T
//
//       pAOverBT = pAT / pBT
*/
void EosPvtTable::GetRK( int iM, int iGetF, int iGetH, int iDrv, double *pP, double *pT, double *pX, double *pAT,
                         double *pBT, double *pAOverBT, double *pAiOverAT, double *pBiOverBT, double *pAijOverAT,
                         double *pD2Adt2OverAT, double *pD2Bdt2OverBT, double *pDAidtOverAiT, double *pDBidtOverBiT,
                         double *pDAdtOverAT, double *pDBdtOverBT, double *pHelp1T, double *pHelp2T )
{
   double *pTa;
   double *pTb;
   double *pTc;
   double *pTd;
   double *pTe;
   double *pTf;
   double *pTg;
   double  dA;
   double  dB;
   double  dC;
   double  dD;
   double  dE;
   double  dF;
   double  dS;
   double  dT;
   int     i;
   int     iNi;
   int     iNj;
   int     iNc2;
   int     iTemp;

   /* Multiple grid blocks */
   if ( iM > 1 )
   {
      /* Initialize needed terms */
      for ( i = 0; i < iM; i++ )
      {
         dA = pT[i];
         pHelp1T[i] = 1.0 / ( dA * sqrt( dA ) );
         pAT[i] = 0.0;
         pBT[i] = 0.0;
      }

      /* Derivative terms */
      if ( iGetH || ( iDrv >= EOS_DRV_T ) )
      {
         for ( i = 0; i < iM; i++ )
         {
            pDAdtOverAT[i] = 0.0;
            pDBdtOverBT[i] = 0.0;
         }

         /* Second derivative terms */
         if ( iGetH && ( iDrv >= EOS_DRV_T ) )
         {
            for ( i = 0; i < iM; i++ )
            {
               pD2Adt2OverAT[i] = 0.0;
               pD2Bdt2OverBT[i] = 0.0;
            }
         }
      }

      /* Set pointers */
      pTa = pAiOverAT;
      pTb = pAijOverAT;
      pTc = pDAidtOverAiT;
      pTd = pBiOverBT;
      pTe = pDBidtOverBiT;
      pTf = pX;
      iTemp = 0;

      /* Form A and B term */
      for ( iNi = 0; iNi < m_iEosComp; iNi++ )
      {
         pTg = pX;
         dA = m_pAdata[iTemp];

         /* Linear binary terms */
         if ( m_iTypeOfBij == EOS_LINEAR_C )
         {
            dB = m_pDAdldata[iTemp];

            /* First component in Ai */
            for ( i = 0; i < iM; i++ )
            {
               dD = ( dA + dB * pT[i] ) * pHelp1T[i];
               pTa[i] = dD * pTg[i];
               *pTb++ = dD;
            }

            /* Form daidt terms */
            if ( iGetH || ( iDrv >= EOS_DRV_T ) )
            {
               dD = 2.5 * dA;
               dC = 1.5 * dB;
               for ( i = 0; i < iM; i++ )
               {
                  pTc[i] = -pTg[i] * ( dD + dC * pT[i] ) * pHelp1T[i];
               }

               /* Form d2adt2 terms */
               if ( iGetH && ( iDrv >= EOS_DRV_T ) )
               {
                  dD = 8.75 * dA;
                  dC = 3.75 * dB;
                  for ( i = 0; i < iM; i++ )
                  {
                     pHelp2T[i] = pTg[i] * ( dD + dC * pT[i] ) * pHelp1T[i];
                  }
               }
            }
         }

         /* Constant binary terms */
         else
         {
            /* First component in Ai */
            for ( i = 0; i < iM; i++ )
            {
               dD = dA * pHelp1T[i];
               pTa[i] = dD * pTg[i];
               *pTb++ = dD;
            }

            /* Form daidt terms */
            if ( iGetH || ( iDrv >= EOS_DRV_T ) )
            {
               dD = 2.5 * dA;
               for ( i = 0; i < iM; i++ )
               {
                  pTc[i] = -pTg[i] * dD * pHelp1T[i];
               }

               /* Form d2adt2 terms */
               if ( iGetH && ( iDrv >= EOS_DRV_T ) )
               {
                  dD = 8.75 * dA;
                  for ( i = 0; i < iM; i++ )
                  {
                     pHelp2T[i] = pTg[i] * dD * pHelp1T[i];
                  }
               }
            }
         }

         iTemp++;

         /* Other Components */
         for ( iNj = 1; iNj < m_iEosComp; iNj++ )
         {
            pTg += iM;
            dA = m_pAdata[iTemp];

            /* Linear binary terms */
            if ( m_iTypeOfBij == EOS_LINEAR_C )
            {
               dB = m_pDAdldata[iTemp];

               /* Ai */
               for ( i = 0; i < iM; i++ )
               {
                  dD = ( dA + dB * pT[i] ) * pHelp1T[i];
                  pTa[i] += dD * pTg[i];
                  *pTb++ = dD;
               }

               /* Form daidt terms */
               if ( iGetH || ( iDrv >= EOS_DRV_T ) )
               {
                  dD = 2.5 * dA;
                  dC = 1.5 * dB;
                  for ( i = 0; i < iM; i++ )
                  {
                     pTc[i] -= pTg[i] * ( dD + dC * pT[i] ) * pHelp1T[i];
                  }

                  /* Form d2adt2 terms */
                  if ( iGetH && ( iDrv >= EOS_DRV_T ) )
                  {
                     dD = 8.75 * dA;
                     dC = 3.75 * dB;
                     for ( i = 0; i < iM; i++ )
                     {
                        pHelp2T[i] += pTg[i] * ( dD + dC * pT[i] ) * pHelp1T[i];
                     }
                  }
               }
            }

            /* Constant binary terms */
            else
            {
               /* Ai */
               for ( i = 0; i < iM; i++ )
               {
                  dD = dA * pHelp1T[i];
                  pTa[i] += dD * pTg[i];
                  *pTb++ = dD;
               }

               /* Form daidt terms */
               if ( iGetH || ( iDrv >= EOS_DRV_T ) )
               {
                  dD = 2.5 * dA;
                  for ( i = 0; i < iM; i++ )
                  {
                     pTc[i] -= pTg[i] * dD * pHelp1T[i];
                  }

                  /* Form d2adt2 terms */
                  if ( iGetH && ( iDrv >= EOS_DRV_T ) )
                  {
                     dD = 8.75 * dA;
                     for ( i = 0; i < iM; i++ )
                     {
                        pHelp2T[i] += pTg[i] * dD * pHelp1T[i];
                     }
                  }
               }
            }

            iTemp++;
         }

         /* Update A and B terms */
         dA = m_pBdata[iNi];
         for ( i = 0; i < iM; i++ )
         {
            dB = pTf[i];
            *pTd = dA;
            pAT[i] += dB **pTa++;
            pBT[i] += dB **pTd++;
         }

         /* Derivatives */
         if ( iGetH || ( iDrv >= EOS_DRV_T ) )
         {
            dB = -dA;
            for ( i = 0; i < iM; i++ )
            {
               dC = pTf[i];
               dD = pT[i];
               *pTc = *pTc / dD;
               *pTe = dB / dD;
               pDAdtOverAT[i] += dC **pTc++;
               pDBdtOverBT[i] += dC **pTe++;
            }

            /* Second derivatives */
            if ( iGetH && ( iDrv >= EOS_DRV_T ) )
            {
               dB = dA + dA;
               for ( i = 0; i < iM; i++ )
               {
                  dC = pT[i];
                  dD = pTf[i] / dC / dC;
                  pD2Adt2OverAT[i] += dD * pHelp2T[i];
                  pD2Bdt2OverBT[i] += dD * dB;
               }
            }
         }

         /* Update composition pointer */
         pTf += iM;
      }

      /* Normalize derivatives */
      if ( iGetH || ( iDrv >= EOS_DRV_T ) )
      {
         pTa = pDAidtOverAiT;
         pTb = pDBidtOverBiT;
         pTc = pAiOverAT;
         pTd = pBiOverBT;
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               *pTa = *pTa / *pTc++;
               *pTb = *pTb / *pTd++;
               pTa++;
               pTb++;
            }
         }

         for ( i = 0; i < iM; i++ )
         {
            pDAdtOverAT[i] = pDAdtOverAT[i] / pAT[i];
            pDBdtOverBT[i] = pDBdtOverBT[i] / pBT[i];
         }

         /* Normalize second derivatives */
         if ( iGetH && ( iDrv >= EOS_DRV_T ) )
         {
            for ( i = 0; i < iM; i++ )
            {
               pD2Adt2OverAT[i] = pD2Adt2OverAT[i] / pAT[i];
               pD2Bdt2OverBT[i] = pD2Bdt2OverBT[i] / pBT[i];
            }
         }
      }

      /* Normalize ai and bi terms */
      if ( iDrv || iGetF )
      {
         pTa = pAiOverAT;
         pTb = pBiOverBT;
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               dA = *pTa;
               *pTa++ = ( dA + dA ) / pAT[i];
               *pTb = *pTb / pBT[i];
               pTb++;
            }
         }

         /* Normalize aij term */
         if ( iGetF && iDrv )
         {
            pTa = pAijOverAT;
            iNc2 = m_iEosComp * m_iEosComp;
            for ( iNi = 0; iNi < iNc2; iNi++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  *pTa = *pTa / pAT[i];
                  pTa++;
               }
            }
         }
      }

      /* Set main terms */
      for ( i = 0; i < iM; i++ )
      {
         dB = pP[i] / pT[i];
         dC = pAT[i];
         dD = pBT[i];
         pAOverBT[i] = dC / dD;
         pAT[i] = dC * dB;
         pBT[i] = dD * dB;
      }
   }

   /* Single grid block */
   else
   {
      /* Initialize needed terms */
      dT = *pT;
      dS = 1.0 / ( dT * sqrt( dT ) );
      dF = 0.0;
      *pAT = 0.0;
      *pBT = 0.0;

      /* Derivative terms */
      if ( iGetH || ( iDrv >= EOS_DRV_T ) )
      {
         *pDAdtOverAT = 0.0;
         *pDBdtOverBT = 0.0;

         /* Second derivative terms */
         if ( iGetH && ( iDrv >= EOS_DRV_T ) )
         {
            *pD2Adt2OverAT = 0.0;
            *pD2Bdt2OverBT = 0.0;
         }
      }

      /* Set pointers */
      pTa = pAijOverAT;
      iTemp = 0;

      /* Form A and B term */
      for ( iNi = 0; iNi < m_iEosComp; iNi++ )
      {
         dA = m_pAdata[iTemp] * dS;

         /* Linear Bij terms */
         if ( m_iTypeOfBij == EOS_LINEAR_C )
         {
            dB = m_pDAdldata[iTemp] * dS * dT;

            /* First component in Ai */
            dE = dA * pX[0];
            dD = dB * pX[0];
            pAiOverAT[iNi] = dE + dD;
            *pTa++ = dA + dB;

            /* Form daidt terms */
            if ( iGetH || ( iDrv >= EOS_DRV_T ) )
            {
               pDAidtOverAiT[iNi] = -( dE * 2.5 + dD * 1.5 );

               /* Form d2adt2 terms */
               if ( iGetH && ( iDrv >= EOS_DRV_T ) )
               {
                  dF = dE * 8.75 + dD * 3.75;
               }
            }
         }

         /* Constant Bij terms */
         else
         {
            /* First component in Ai */
            dE = dA * pX[0];
            pAiOverAT[iNi] = dE;
            *pTa++ = dA;

            /* Form daidt terms */
            if ( iGetH || ( iDrv >= EOS_DRV_T ) )
            {
               pDAidtOverAiT[iNi] = -dE * 2.5;

               /* Form d2adt2 terms */
               if ( iGetH && ( iDrv >= EOS_DRV_T ) )
               {
                  dF = dE * 8.75;
               }
            }
         }

         iTemp++;

         /* Other Components */
         for ( iNj = 1; iNj < m_iEosComp; iNj++ )
         {
            dA = m_pAdata[iTemp] * dS;

            /* Linear Bij terms */
            if ( m_iTypeOfBij == EOS_LINEAR_C )
            {
               dB = m_pDAdldata[iTemp] * dS * dT;

               /* Ai */
               dE = dA * pX[iNj];
               dD = dB * pX[iNj];
               pAiOverAT[iNi] += ( dE + dD );
               *pTa++ = dA + dB;

               /* Form daidt terms */
               if ( iGetH || ( iDrv >= EOS_DRV_T ) )
               {
                  pDAidtOverAiT[iNi] -= ( dE * 2.5 + dD * 1.5 );

                  /* Form d2adt2 terms */
                  if ( iGetH && ( iDrv >= EOS_DRV_T ) )
                  {
                     dF += ( dE * 8.75 + dD * 3.75 );
                  }
               }
            }

            /* Constant Bij terms */
            else
            {
               /* Ai */
               dE = dA * pX[iNj];
               pAiOverAT[iNi] += dE;
               *pTa++ = dA;

               /* Form daidt terms */
               if ( iGetH || ( iDrv >= EOS_DRV_T ) )
               {
                  pDAidtOverAiT[iNi] -= dE * 2.5;

                  /* Form d2adt2 terms */
                  if ( iGetH && ( iDrv >= EOS_DRV_T ) )
                  {
                     dF += dE * 8.75;
                  }
               }
            }

            iTemp++;
         }

         /* Update A and B terms */
         dA = pX[iNi];
         dB = m_pBdata[iNi];
         pBiOverBT[iNi] = dB;
         *pAT += dA * pAiOverAT[iNi];
         *pBT += dA * dB;

         /* Derivatives */
         if ( iGetH || ( iDrv >= EOS_DRV_T ) )
         {
            dC = pDAidtOverAiT[iNi] / dT;
            dD = -dB / dT;
            pDAidtOverAiT[iNi] = dC;
            pDBidtOverBiT[iNi] = dD;
            *pDAdtOverAT += dA * dC;
            *pDBdtOverBT += dA * dD;

            /* Second derivatives */
            if ( iGetH && ( iDrv >= EOS_DRV_T ) )
            {
               dC = dB + dB;
               dD = dA / dT / dT;
               *pD2Adt2OverAT += dD * dF;
               *pD2Bdt2OverBT += dD * dC;
            }
         }
      }

      /* Normalize derivatives */
      if ( iGetH || ( iDrv >= EOS_DRV_T ) )
      {
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            pDAidtOverAiT[iNi] = pDAidtOverAiT[iNi] / pAiOverAT[iNi];
            pDBidtOverBiT[iNi] = pDBidtOverBiT[iNi] / pBiOverBT[iNi];
         }

         *pDAdtOverAT = *pDAdtOverAT / *pAT;
         *pDBdtOverBT = *pDBdtOverBT / *pBT;

         /* Normalize second derivatives */
         if ( iGetH && ( iDrv >= EOS_DRV_T ) )
         {
            *pD2Adt2OverAT = *pD2Adt2OverAT / *pAT;
            *pD2Bdt2OverBT = *pD2Bdt2OverBT / *pBT;
         }
      }

      /* Normalize ai and bi terms */
      if ( iDrv || iGetF )
      {
         pTa = pAiOverAT;
         pTb = pBiOverBT;
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            dA = pAiOverAT[iNi];
            pAiOverAT[iNi] = ( dA + dA ) / *pAT;
            pBiOverBT[iNi] = pBiOverBT[iNi] / *pBT;
         }

         /* Normalize aij term */
         if ( iGetF && iDrv )
         {
            dA = *pAT;
            pTa = pAijOverAT;
            iNc2 = m_iEosComp * m_iEosComp;
            for ( iNi = 0; iNi < iNc2; iNi++ )
            {
               *pTa = *pTa / dA;
               pTa++;
            }
         }
      }

      /* Set main terms */
      dB = *pP / dT;
      dC = *pAT;
      dD = *pBT;
      *pAOverBT = dC / dD;
      *pAT = dC * dB;
      *pBT = dD * dB;
   }
}


/* 
// ZFactor
//
// Calculate the z factor for a cubic equation of state 
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
// iEnergy ** constant **
//    Indicator whether any terms needed for energetics
//    will be needed, whether enthalpy or chemical potentials
// iPhase ** constant **
//    Indicator for getting phase identification flag
//       EOS_PHASEID
//          Get the phase id
//       EOS_NOPHASEID
//          Do not get the phase id
// pP ** constant **
//    Pressure for each object
// pPhase
//    Phase indicator, if requested
//       EOS_SINGLE_PHASE_GAS 
//          Mixture is a gas
//       EOS_SINGLE_PHASE_OIL 
//          Mixture is an oil
// pT ** constant **
//    Temperature for each object
// pX ** constant **
//    Composition for each object normalized to one
//       first index corresponds to object
//       second index corresponds to component
// pZ 
//    Z factor for each object
// pDZdp
//    Pressure derivative of z factor of each object
// pDZdt
//    Temperature derivative of z factor of each object
// pDZdz
//    Overall mole derivative of z factor of each object
//       first index corresponds to object
//       second index corresponds to component
// pAT ** constant **
//    "A" coefficient for equation-of-state
// pBT ** constant **
//    "B" coefficient for equation-of-state
// pAOverBT
//    A over B.  This term will later be overwritten
//    for fugacity calculations
// pAiOverAT
//    Ai over A
// pBiOverBT
//    Bi over B
// pDAdtOverAT
//    Derivative of A wrt temperature over A
// pDBdtOverBT
//    Derivative of B wrt temperature over B
// pHelp1T
//    Help vector
// pHelp2T
//    Help vector
// pHelp3T
//    Help vector
// pHelp4T
//    Help vector 
// pHelp5T
//    Help vector 
//
// 1) A generalized cubic equation of state is of the form
//
//    Z * ( Z * ( Z - ( 1 - dKb7 * B ) ) + A - B * ( dKb4 +
//          dKb6 * B ) ) - B * ( A + dKb5 * B * ( 1 + B ) ) = F(Z)
//
//    or
//
//    Z^3 + S * Z^2 + T * Z + U
//
//    This routine is to compute the root and its derivatives
//    and has two variations, first for a single object and
//    then for multiple objects
//
// 2) Reduce the equation to the form
//
//    W^3 + Q * W + R
// 
//    by making the substitution W = Z * Y, where
//
//    Y = ( 1 - dKb7 * B ) / 3 = S / 3
//
//    which yields as coefficients
//
//    R = ( T / 2 - S^2 / 9 ) * S / 3 - U / 2
//
//    Q = T / 3 - S^2 / 9
//
// 3) Form the descriminant
//
//    G = R * R + Q * Q * Q
//
// 4) If the descriminant is greater than zero, only
//    a single real root which is computed by
//
//    Z = Y + ( R + G^0.5 )^1/3 + ( R - G^0.5 )^1/3
//
// 5) If the descriminant is zero there is only one real root
//
//    Z = Y + 2 * (-dQ)^0.5 
//
// 6) If A > 2B then there are three real roots, the
//    middle of which is not important.  Then the
//    fugacity needs to be checked.  The two roots
//    are first calculated from the roots of a general
//    cubic equation.  There is some checking done
//    here to avoid rounding problems, as the arccos
//    function must lie between -1 and 1.  Also, when
//    the sine is required, this is calculated from the
//    cosine function between -pi/4 and pi/4 directly, 
//    since the internal sine and cosine functions do this 
//    anyway, and one of these wonderful UNIX tools called
//    Purify produced a core dump.  Anyway, given the
//    two extreme roots, choose the only with the lowest 
//    free energy.  The precise formula for the roots is
//
//    X = acos ( - R / ( -Q )^1.5 ) / 3
//
//    Z1 = S / 3 +  2 * (-Q)^0.5 * sin ( X ) 
//
//    Z2 = S / 3 +  2 * (-Q)^0.5 * sin ( X + 2 * PI / 3) 
//
//    For the fugacity, the term is calculated from
//
//    FU = Z - 1 - Log(Z-B) - A * Log(FX) / B / ( dKb0 - dKb1 )
//
//    where
//
//    FX = (Z + dKb0 * B) / (Z + dKb2 * B)
//    
// 6) If A < 2B then there are three real roots but one
//    of them is not important and the other one is 
//    negative.  Calculate the positive root.  The same
//    trick with the cosine function is also used here
//
// 7) Take one Newton iteration on the cubic equation to
//    eliminate any roundoff errors that have crept in while
//    forming the exact solution.
//
// 8) If phase identification is required, set the type of
//    phase. The options are
//       EOS_SINGLE_PHASE_GAS 
//          Single phase forced to be a gas
//       EOS_SINGLE_PHASE_OIL 
//          Single phase forced to be an oil
//       EOS_SINGLE_PHASE_ZMINUSB 
//          Single phase is an oil if
//
//          Z - B < CritZMinusB
// 
//          which by default is 0.25.  The problem with this
//          approach is that as the pressure varies the phase
//          identification can change from oil to gas without
//          forming a two phase system
//       EOS_SINGLE_PHASE_AOVERB
//          Phase identification selected from either
//          that obtained by a fugacity test with multiple
//          roots or by a test on the value of A over B.
//          The user defined critical A over B should be
//          decreased if an oil is wrongly identified as
//          a gas, and increased if a gas is wrongly identified
//          as an oil
//       EOS_SINGLE_PHASE_DEFAULT
//          Phase identification selected from either
//          that obtained by a fugacity test with multiple
//          roots or by assuming that the mixture is a
//          single component system and descreasing the
//          pressure below the critical pressure.  The problem
//          with this approach is that while it may be
//          consistent with variations in pressure
//          it may also be consistantly wrong.  
//
// 9) Form derivatives from implicit function theorem.
//
//    DZDA = - DFDA(Z) / DFDZ(Z) AND DZDB = - DFDB(Z) / DFDZ(Z)
//
//    pDZdp = DZDA * pDAdp + DZDB * pDBdp
//
//    pDZdt = DZDA * pDAdt + DZDB * pDBdt
//
//    pDZdz  = DZDA * pDAdz  + DZDB * pDBdz
//         i               i               i
//
//    These computations have been rearranged for computational
//    efficiency
//
// 10) If energetics are required, store the logarithm term
//     for the fugacity, etc., in A over B.  For the Redlich
//     Kwong equation of state this corresponds to
//
//     A/B * LOG ( 1 + B/Z )
*/
void EosPvtTable::ZFactor( int iM, int iDrv, int iEnergy, int iPhase, double *pP, int *pPhase, double *pZ,
                           double *pDZdp, double *pDZdt, double *pDZdz, double *pAT, double *pBT, double *pAOverBT,
                           double *pAiOverAT, double *pBiOverBT, double *pDAdtOverAT, double *pDBdtOverBT,
                           double *pHelp1T, double *pHelp2T, double *pHelp3T, double *pHelp4T, double *pHelp5T )
{
   double        dA;
   double        dB;
   double        dC;
   double        dD;
   double        dE;
   double        dF;
   double        dG;
   double       *pTa;
   double       *pTb;
   double       *pTc;
   double        dQ;
   double        dR;
   double        dS;
   double        dT;
   double        dU;
   double        dV;
   double        dW;
   double        dX;
   double        dY;
   double        dZ;
   int           iNi;
   int           i;

   /* Set numerical terms */
#ifdef EOS_NEW_CUBIC
   const double  dSmall = 1.0e-10;
#else
   const double  dS3 = sqrt( 3.0 );
#endif
   const double  dThird = 1.0 / 3.0;

   /* Form the terms for a single grid block */
   if ( iM == 1 )
   {
      dA = *pAT;
      dB = *pBT;
      dE = *pAOverBT;
      dC = dB * m_dKb0;
      dD = dB * m_dKb1;
      *pHelp4T = dC;
      *pHelp5T = dD;

      /* Solve the cubic equation */
      dU = dB * ( dA + m_dKb5 * dB * ( 1.0 + dB ) );
      dV = dA - dB * ( m_dKb6 * dB + m_dKb4 );
      dW = 1.0 - dB * m_dKb7;
      dY = dW * dThird;
      dX = dY * dY;
      dR = dX * dY + ( dU - dV * dY ) * 0.5;
      dQ = dV * dThird - dX;
      dG = dR * dR + dQ * dQ * dQ;
      dF = 0.0;

#ifdef EOS_NEW_CUBIC
      /* Starting point for Newton iterations */
      dS = dQ > 0.0 ? dQ : ( -dQ );
      dT = dR > 0.0 ? dR : ( -dR );
      dZ = sqrt( 3.0 * ( dS > dT ? dS : dT ) ) + dSmall;
      if ( dR < 0.0 )
      {
         dZ = -dZ;
      }

      /* Solve for the root using Newton's method */
      dR += dR;
      dQ *= 3.0;
      dT = dZ * dZ;
      dX = dT + dQ;
      dS = ( dZ * dX - dR ) / ( dX + dT + dT );
      for ( int it = 0; ( ( dS > 0.0 ? dS : ( -dS ) ) > dSmall ) && ( it < 20 ); it++ )
      {
         dZ -= dS;
         dT = dZ * dZ;
         dX = dT + dQ;
         dS = ( dZ * dX - dR ) / ( dX + dT + dT );
      }

      /* Add the transformation */
      dZ -= dS;

      /* Get the other roots */
      if ( dG < 0.0 )
      {
         dS = -3.0 * dZ * dZ - 4.0 * dQ;
         dS = dS < 0.0 ? 0.0 : dS;
         if ( dR < 0.0 )
         {
            dT = dZ;
            dZ = dY + 0.5 * ( sqrt( dS ) - dT );
            dT += dY;
         }
         else
         {
            dT = dY - 0.5 * ( dZ + sqrt( dS ) );
            dZ += dY;
         }

         /* Select the correct root */
         if ( ( dT > dB ) && ( dE > 2.0 ) && ( dB <= m_dKb3 ) )
         {
            /* TODO: The next line is too long. Please rewrite to make it shorter. */
            if ( ( dZ - dT ) - log( ( dZ - dB ) / ( dT - dB ) ) > dE * ( m_dKb8 * log( ( ( dZ + dC ) / ( dZ + dD ) ) * ( ( dT + dD ) / ( dT + dC ) ) ) + m_dKb9 * ( dB / ( dZ + dC ) - dB / ( dT + dC ) ) ) )
            {
               dZ = dT;
               dF = 1.0;
            }
         }
      }
      else
      {
         dZ += dY;
      }

#else
      /* Single real root */
      if ( dG > 0.0 )
      {
         dZ = sqrt( dG );
         dX = dR + dZ;
         dS = ( dX < 0.0 ) ? -1.0 : 1.0;
         dX *= dS;
         dS *= pow( dX, dThird );
         dX = dR - dZ;
         dT = ( dX < 0.0 ) ? -1.0 : 1.0;
         dX *= dT;
         dT *= pow( dX, dThird );
         dZ = dY + dS + dT;
      }

      /* Inflection single root */
      else if ( dG == 0.0 )
      {
         dZ = sqrt( -dQ );
         dZ = dY + dZ + dZ;
      }

      /* Choose the correct root */
      else if ( dA >= 2.0 * dB )
      {
         dZ = sqrt( -dQ );
         dX = -dR / dQ / dZ;
         dX = ( dX > 1.0 ) ? 1.0 : ( ( dX < -1.0 ) ? -1.0 : dX );
         dT = dZ + dZ;
         dS = acos( dX ) / 6.0;
         dZ = cos( dS );
         dS = dZ * ( dS3 * sin( dS ) + dZ ) - 0.5;
         dZ = dY + dT * ( 2.0 * dZ * dZ - 1.0 );
         dT = dY - dT * dS;
         if ( dT > dB )
         {
            /* TODO: The next line is too long. Please rewrite to make it shorter. */
            if ( ( dZ - dT ) - log( ( dZ - dB ) / ( dT - dB ) ) > dE * ( m_dKb8 * log( ( ( dZ + dC ) / ( dZ + dD ) ) * ( ( dT + dD ) / ( dT + dC ) ) ) + m_dKb9 * ( dB / ( dZ + dC ) - dB / ( dT + dC ) ) ) )
            {
               dZ = dT;
               dF = 1.0;
            }
         }
      }

      /* Two roots but one negative */
      else
      {
         dZ = sqrt( -dQ );
         dX = -dR / dQ / dZ;
         dX = ( dX > 1.0 ) ? 1.0 : ( ( dX < -1.0 ) ? -1.0 : dX );
         dS = cos( acos( dX ) / 6.0 );
         dZ = dY + ( dZ + dZ ) * ( 2.0 * dS * dS - 1.0 );
      }

      /* Take one Newton iteration */
      dZ = ( dZ * dZ * ( dZ + dZ - dW ) + dU ) / ( dZ * ( 3.0 * dZ - dW - dW ) + dV );
#endif
      *pZ = dZ;

      /* Phase indicator for a single block */
      if ( iPhase == EOS_PHASEID )
      {
         switch ( m_iTypeOfSingle )
         {
            case EOS_SINGLE_PHASE_GAS:
               {
                  *pPhase = EOS_SINGLE_PHASE_GAS;
                  break;
               }

            case EOS_SINGLE_PHASE_OIL:
               {
                  *pPhase = EOS_SINGLE_PHASE_OIL;
                  break;
               }

            case EOS_SINGLE_PHASE_ZMINUSB:
               {
                  *pPhase = ( dZ - dB < m_dCritZMinusB ) ? 1 : 0;
                  break;
               }

            case EOS_SINGLE_PHASE_AOVERB:
               {
                  *pPhase = ( ( dF > 0.0 ) || ( dB <= m_dKb3 && dR < 0.0 ) || ( dB > m_dKb3 && dE > m_dCritAOverB ) ) ? 1 : 0;
                  break;
               }

            default:
               {
                  *pPhase = ( ( dF > 0.0 ) || ( dB <= m_dKb3 && dR < 0.0 ) || ( dB > m_dKb3 && dE > m_dKb2 ) ) ? 1 : 0;
                  break;
               }
         }
      }

      /* A and B derivatives */
      if ( iDrv >= EOS_DRV_N )
      {
         dG = 1.0 / ( dZ * ( 3.0 * dZ - dW - dW ) + dV );
         dX = dB * ( dA + dB * m_dKb5 * ( 2.0 + 3.0 * dB ) + dZ * ( 2.0 * dB * m_dKb6 + m_dKb4 - m_dKb7 * dZ ) ) * dG;
         dY = dA * ( dB - dZ ) * dG;
         dE = dY + dY + dX;

         /* Composition derivatives */
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
#ifdef IPFtune
#pragma ivdep
#endif
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            pDZdz[iNi] = dY * pAiOverAT[iNi] + dX * pBiOverBT[iNi] - dE;
         }

         /* Pressure derivatives */
         if ( iDrv >= EOS_DRV_P )
         {
            *pDZdp = ( dY + dX ) / *pP;

            /* Temperature derivatives */
            if ( iDrv >= EOS_DRV_T )
            {
               *pDZdt = *pDAdtOverAT * dY +*pDBdtOverBT * dX;
            }
         }
      }

      /* Save a over b for energy calculations */
      if ( iEnergy )
      {
         dA = dZ + dC;
         *pAOverBT *= ( m_dKb8 * log( dA / ( dZ + dD ) ) + m_dKb9 * dB / dA );
      }
   }

   /* Form the terms for multiple grid blocks */
   else
   {
      for ( i = 0; i < iM; i++ )
      {
         dA = pAT[i];
         dB = pBT[i];
         dE = pAOverBT[i];
         dC = dB * m_dKb0;
         dD = dB * m_dKb1;
         pHelp4T[i] = dC;
         pHelp5T[i] = dD;

         /* Solve the cubic equation */
         dU = dB * ( dA + m_dKb5 * dB * ( 1.0 + dB ) );
         dV = dA - dB * ( m_dKb6 * dB + m_dKb4 );
         dW = 1.0 - dB * m_dKb7;
         dY = dW * dThird;
         dX = dY * dY;
         dR = dX * dY + ( dU - dV * dY ) * 0.5;
         dQ = dV * dThird - dX;
         dG = dR * dR + dQ * dQ * dQ;
         dF = 0.0;

#ifdef EOS_NEW_CUBIC
         /* Starting point for Newton iterations */
         dS = dQ > 0.0 ? dQ : ( -dQ );
         dT = dR > 0.0 ? dR : ( -dR );
         dZ = sqrt( 3.0 * ( dS > dT ? dS : dT ) ) + dSmall;
         if ( dR < 0.0 )
         {
            dZ = -dZ;
         }

         /* Solve for the root using Newton's method */
         dR += dR;
         dQ *= 3.0;
         dT = dZ * dZ;
         dX = dT + dQ;
         dS = ( dZ * dX - dR ) / ( dX + dT + dT );
         for ( int it = 0; ( ( dS > 0.0 ? dS : ( -dS ) ) > dSmall ) && ( it < 20 ); it++ )
         {
            dZ -= dS;
            dT = dZ * dZ;
            dX = dT + dQ;
            dS = ( dZ * dX - dR ) / ( dX + dT + dT );
         }

         /* Add the transformation */
         dZ -= dS;

         /* Get the other roots */
         if ( dG < 0.0 )
         {
            dS = -3.0 * dZ * dZ - 4.0 * dQ;
            dS = dS < 0.0 ? 0.0 : dS;
            if ( dR < 0.0 )
            {
               dT = dZ;
               dZ = dY + 0.5 * ( sqrt( dS ) - dT );
               dT += dY;
            }
            else
            {
               dT = dY - 0.5 * ( dZ + sqrt( dS ) );
               dZ += dY;
            }

            /* Select the correct root */
            if ( ( dT > dB ) && ( dE > 2.0 ) && ( dB <= m_dKb3 ) )
            {
               /* TODO: The next line is too long. Please rewrite to make it shorter. */
               if ( ( dZ - dT ) - log( ( dZ - dB ) / ( dT - dB ) ) > dE * ( m_dKb8 * log( ( ( dZ + dC ) / ( dZ + dD ) ) * ( ( dT + dD ) / ( dT + dC ) ) ) + m_dKb9 * ( dB / ( dZ + dC ) - dB / ( dT + dC ) ) ) )
               {
                  dZ = dT;
                  dF = 1.0;
               }
            }
         }
         else
         {
            dZ += dY;
         }

#else
         /* Single real root */
         if ( dG > 0.0 )
         {
            dZ = sqrt( dG );
            dX = dR + dZ;
            dS = ( dX < 0.0 ) ? -1.0 : 1.0;
            dX *= dS;
            dS *= pow( dX, dThird );
            dX = dR - dZ;
            dT = ( dX < 0.0 ) ? -1.0 : 1.0;
            dX *= dT;
            dT *= pow( dX, dThird );
            dZ = dY + dS + dT;
         }

         /* Inflection single root */
         else if ( dG == 0.0 )
         {
            dZ = sqrt( -dQ );
            dZ = dY + dZ + dZ;
         }

         /* Choose the correct root */
         else if ( dA >= 2.0 * dB )
         {
            dZ = sqrt( -dQ );
            dX = -dR / dQ / dZ;
            dX = ( dX > 1.0 ) ? 1.0 : ( ( dX < -1.0 ) ? -1.0 : dX );
            dT = dZ + dZ;
            dS = acos( dX ) / 6.0;
            dZ = cos( dS );
            dS = dZ * ( dS3 * sin( dS ) + dZ ) - 0.5;
            dZ = dY + dT * ( 2.0 * dZ * dZ - 1.0 );
            dT = dY - dT * dS;
            if ( dT > dB )
            {
               /* TODO: The next line is too long. Please rewrite to make it shorter. */
               if ( ( dZ - dT ) - log( ( dZ - dB ) / ( dT - dB ) ) > dE * ( m_dKb8 * log( ( ( dZ + dC ) / ( dZ + dD ) ) * ( ( dT + dD ) / ( dT + dC ) ) ) + m_dKb9 * ( dB / ( dZ + dC ) - dB / ( dT + dC ) ) ) )
               {
                  dZ = dT;
                  dF = 1.0;
               }
            }
         }

         /* Two roots but one negative */
         else
         {
            dZ = sqrt( -dQ );
            dX = -dR / dQ / dZ;
            dX = ( dX > 1.0 ) ? 1.0 : ( ( dX < -1.0 ) ? -1.0 : dX );
            dS = cos( acos( dX ) / 6.0 );
            dZ = dY + ( dZ + dZ ) * ( 2.0 * dS * dS - 1.0 );
         }

         /* Take one Newton iteration */
         dZ = ( dZ * dZ * ( dZ + dZ - dW ) + dU ) / ( dZ * ( 3.0 * dZ - dW - dW ) + dV );
#endif
         pZ[i] = dZ;

         /* Save needed terms */
         pHelp1T[i] = ( dF > 0.0 ) || ( dB <= m_dKb3 && dR < 0.0 );
         pHelp2T[i] = dZ * ( 3.0 * dZ - dW - dW ) + dV;
      }

      /* Phase indicator for a single block */
      if ( iPhase == EOS_PHASEID )
      {
         switch ( m_iTypeOfSingle )
         {
            case EOS_SINGLE_PHASE_GAS:
               {
                  for ( i = 0; i < iM; i++ )
                  {
                     pPhase[i] = EOS_SINGLE_PHASE_GAS;
                  }

                  break;
               }

            case EOS_SINGLE_PHASE_OIL:
               {
                  for ( i = 0; i < iM; i++ )
                  {
                     pPhase[i] = EOS_SINGLE_PHASE_OIL;
                  }

                  break;
               }

            case EOS_SINGLE_PHASE_ZMINUSB:
               {
   #ifdef USE_VECTOR_ALWAYS
   #pragma vector always
   #endif
                  for ( i = 0; i < iM; i++ )
                  {
                     pPhase[i] = ( pZ[i] - pBT[i] < m_dCritZMinusB ) ? 1 : 0;
                  }

                  break;
               }

            case EOS_SINGLE_PHASE_AOVERB:
               {
   #ifdef USE_VECTOR_ALWAYS
   #pragma vector always
   #endif
                  for ( i = 0; i < iM; i++ )
                  {
                     pPhase[i] = ( pHelp1T[i] || ( ( pBT[i] > m_dKb3 ) && ( pAOverBT[i] > m_dCritAOverB ) ) ) ? 1 : 0;
                  }

                  break;
               }

            default:
               {
   #ifdef USE_VECTOR_ALWAYS
   #pragma vector always
   #endif
                  for ( i = 0; i < iM; i++ )
                  {
                     pPhase[i] = ( pHelp1T[i] || ( ( pBT[i] > m_dKb3 ) && ( pAOverBT[i] > m_dKb2 ) ) ) ? 1 : 0;
                  }

                  break;
               }
         }
      }

      /* A and B derivatives */
      if ( iDrv >= EOS_DRV_N )
      {
#ifdef IPFtune
         double tk4 = m_dKb4;
         double tk5 = m_dKb5;
         double tk6 = m_dKb6;
         double tk7 = m_dKb7;
#pragma ivdep
         for ( i = 0; i < iM; i++ )
         {
            dB = pBT[i];
            dA = pAT[i];
            dZ = pZ[i];
            dG = 1.0 / pHelp2T[i];
            dC = dB * ( dA + dB * tk5 * ( 2.0 + 3.0 * dB ) + dZ * ( 2.0 * dB * tk6 + tk4 - tk7 * dZ ) ) * dG;
            dD = dA * ( dB - dZ ) * dG;
            pHelp1T[i] = dC;
            pHelp2T[i] = dD;
            pHelp3T[i] = dD + dD + dC;
         }
#else
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
         for ( i = 0; i < iM; i++ )
         {
            dB = pBT[i];
            dA = pAT[i];
            dZ = pZ[i];
            dG = 1.0 / pHelp2T[i];
            dC = dB * ( dA + dB * m_dKb5 * ( 2.0 + 3.0 * dB ) + dZ * ( 2.0 * dB * m_dKb6 + m_dKb4 - m_dKb7 * dZ ) ) * dG;
            dD = dA * ( dB - dZ ) * dG;
            pHelp1T[i] = dC;
            pHelp2T[i] = dD;
            pHelp3T[i] = dD + dD + dC;
         }
#endif

         /* Composition derivatives */
         pTa = pAiOverAT;
         pTb = pBiOverBT;
         pTc = pDZdz;
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
#ifdef IPFtune
            int cnt = iM;
#pragma ivdep
            for ( i = 0; i < cnt; i++ )
            {
               pTc[i] = pHelp2T[i] * pTa[i] + pHelp1T[i] * pTb[i] - pHelp3T[i];
            }
#else 
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
            for ( i = 0; i < iM; i++ )
            {
               pTc[i] = pHelp2T[i] * pTa[i] + pHelp1T[i] * pTb[i] - pHelp3T[i];
            }
#endif

            pTa += iM;
            pTb += iM;
            pTc += iM;
         }

         /* Pressure derivatives */
         if ( iDrv >= EOS_DRV_P )
         {
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
            for ( i = 0; i < iM; i++ )
            {
               pDZdp[i] = ( pHelp1T[i] + pHelp2T[i] ) / pP[i];
            }

            /* Temperature derivatives */
            if ( iDrv >= EOS_DRV_T )
            {
               for ( i = 0; i < iM; i++ )
               {
                  pDZdt[i] = pDAdtOverAT[i] * pHelp2T[i] + pDBdtOverBT[i] * pHelp1T[i];
               }
            }
         }
      }

      /* Set a over b term */
      if ( iEnergy )
      {
#ifdef IPFtune      
#pragma ivdep 
#endif     
         for ( i = 0; i < iM; i++ )
         {
            dZ = pZ[i];
            dA = dZ + pHelp4T[i];
            pHelp4T[i] = dA;
            pHelp5T[i] = dA / ( dZ + pHelp5T[i] );
         }

         EosUtil::VectorLog( iM, pHelp5T, pHelp5T );
#ifdef IPFtune      
         double tk8 = m_dKb8;
         double tk9 = m_dKb9;
#pragma ivdep 
         for ( i = 0; i < iM; i++ )
         {
            pAOverBT[i] *= ( tk8 * pHelp5T[i] + tk9 * pBT[i] / pHelp4T[i] );
         }
#else 
         for ( i = 0; i < iM; i++ )
         {
            pAOverBT[i] *= ( m_dKb8 * pHelp5T[i] + m_dKb9 * pBT[i] / pHelp4T[i] );
         }
#endif     
      }
   }
}


/* 
// NonIdealHeat
//
// Routine to get the non ideal contribution to the heat terms 
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
// pP ** constant **
//    Pressure for each object
// pT ** constant **
//    Temperature for each object
// pX ** constant **
//    Composition for each object normalized to one
//       first index corresponds to object
//       second index corresponds to component
// pZ ** constant ** 
//    Z factor for each object
// pDZdp ** constant **
//    Pressure derivative of z factor of each object
// pDZdt ** constant **
//    Temperature derivative of z factor of each object
// pDZdz ** constant **
//    Overall mole derivative of z factor of each object
//       first index corresponds to object
//       second index corresponds to component
// pHeat
//    Enthalpy for each object
// pDHeatdp
//    Pressure derivative of enthalpy of each object
// pDHeatdt
//    Temperature derivative of enthalpy of each object
// pDHeatdz
//    Overall mole derivative of enthalpy of each object
//       first index corresponds to object
//       second index corresponds to component
// pBT ** constant **
//    "B" coefficient for equation-of-state
// pAOverBT ** constant **
//    A over B.  This term will later be overwritten
//    for fugacity calculations
// pAiOverAT ** constant **
//    Ai over A
// pBiOverBT ** constant **
//    Bi over B
// pDAdtOverAT ** constant **
//    Derivative of A wrt temperature over A
// pDBdtOverBT ** constant **
//    Derivative of B wrt temperature over B
// pD2Adt2OverAT
//    Second derivative of A wrt temperature over A
// pD2Bdt2OverBT
//    Second derivative of B wrt temperature over B
// pDAidtOverAiT
//    Derivative of Ai wrt temperature over Ai
// pDBidtOverBT
//    Derivative of Bi wrt temperature over Bi
// pHelp1T
//    Help vector
// pHelp2T
//    Help vector
// pHelp3T
//    Help vector
// pHelp4T
//    Help vector
// pHelp5T
//    Help vector
// pHelp6T
//    Help vector
//
// 1) The non ideal contribution to the enthalpy is given
//    by
//
//    DH = INT ( V - T dV/dT ) dP
//
//    which reduces to
//
//    A/B LOG ( ( Z + Kb0 * B ) / ( Z + Kb1 * B ) ) * RT * T * 
//            ( dA/dT / A - dB/dT / B ) - ( Z - 1 ) * RT * T * 
//            ( dB/dT / B ) + RT
//
//    This term, of course, must be converted to internal
//    energy units
// 2) If requested, take derivatives, which are tedious
//    but straight forward
// 3) Evaluation of the integral
//  
//    DH = INT ( V - T dV/dT ) dP
//
//    is thus straightforward but tedious
*/
void EosPvtTable::NonIdealHeat( int iM, int iDrv, double *pP, double *pT, double *pZ, double *pDZdp, double *pDZdt,
                                double *pDZdz, double *pHeat, double *pDHeatdp, double *pDHeatdt, double *pDHeatdz,
                                double *pBT, double *pAOverBT, double *pAiOverAT, double *pBiOverBT,
                                double *pDAdtOverAT, double *pDBdtOverBT, double *pD2Adt2OverAT, double *pD2Bdt2OverBT,
                                double *pDAidtOverAiT, double *pDBidtOverBiT, double *pHelp1T, double *pHelp2T,
                                double *pHelp3T, double *pHelp4T, double *pHelp5T, double *pHelp6T )
{
   double  dA;
   double  dB;
   double  dC;
   double  dD;
   double  dE;
   double  dF;
   double  dG;
   double  dH;
   double  dQ;
   double  dR;
   double  dS;
   double  dT;
   double  dU;
   double  dV;
   double  dW;
   double *pTa;
   double *pTb;
   double *pTc;
   double *pTd;
   double *pTe;
   double *pTf;
   double  dX;
   int     i;
   int     iNi;

   /* For multiple blocks */
   if ( iM > 1 )
   {
      /* Terms */
      dG = m_dConvHeat * m_dGascon / m_dConvPressure / m_dConvVolume;
      for ( i = 0; i < iM; i++ )
      {
         dE = pT[i];
         dA = dE * dE;
         dB = pZ[i];
         dC = dA * pDBdtOverBT[i];
         pHelp1T[i] = dG - dG / ( dB - pBT[i] );
         pHelp2T[i] = dG * pAOverBT[i];
         pHelp3T[i] = dG * ( dB - 1.0 );
         pHelp4T[i] = dC * dG;
         pHelp5T[i] = dA * pDAdtOverAT[i] - dC;
         pHelp6T[i] = dA;
         pHeat[i] = pHelp2T[i] * pHelp5T[i] - pHelp3T[i] * dC;
      }

      /* Derivatives */
      if ( iDrv )
      {
         pTa = pDHeatdz;
         pTb = pBiOverBT;
         pTc = pAiOverAT;
         pTd = pDZdz;
         pTe = pDAidtOverAiT;
         pTf = pDBidtOverBiT;
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               dA = *pTc++;
               dB = *pTb++;
               dC = pZ[i];
               dD = pHelp2T[i];
               dE = pHelp6T[i];
               dF = dE * dB * ( *pTf++ -pDBdtOverBT[i] );
               dQ = *pTd++;

               /* TODO: The next line is too long. Please rewrite to make it shorter. */
               *pTa++ = pHelp5T[i] * ( pHelp1T[i] * ( dQ - dC * dB + dC ) + dD * ( dA - dB - 1.0 ) ) - dQ * pHelp4T[i] + dD * ( dE * dA * ( *pTe++ -pDAdtOverAT[i] ) - dF ) - pHelp3T[i] * dF;
            }
         }

         /* Form the fugacity temperature and pressure derivative */
         if ( iDrv >= EOS_DRV_P )
         {
            if ( iDrv >= EOS_DRV_T )
            {
               for ( i = 0; i < iM; i++ )
               {
                  dC = pT[i];
                  dQ = pDAdtOverAT[i];
                  dR = pDBdtOverBT[i];
                  dA = dC * dQ;
                  dB = dC * dR;
                  dD = pHelp2T[i];
                  dE = pHelp6T[i];
                  dF = dB * ( 2.0 - dB ) + dE * pD2Bdt2OverBT[i];
                  dH = pDZdt[i];

                  /* TODO: The next line is too long. Please rewrite to make it shorter. */
                  pDHeatdt[i] = pHelp5T[i] * ( pHelp1T[i] * ( dH - pZ[i] * dR ) + dD * ( dQ - dR ) ) - pHelp4T[i] * dH + dD * ( dA * ( 2.0 - dA ) + dE * pD2Adt2OverAT[i] - dF ) - pHelp3T[i] * dF;
               }
            }

            for ( i = 0; i < iM; i++ )
            {
               dA = pDZdp[i];
               pDHeatdp[i] = pHelp5T[i] * pHelp1T[i] * ( dA - pZ[i] / pP[i] ) - pHelp4T[i] * dA;
            }
         }
      }
   }

   /* For single block */
   else
   {
      /* Terms */
      dG = m_dConvHeat * m_dGascon / m_dConvPressure / m_dConvVolume;
      dA = *pT;
      dE = dA * dA;
      dB = *pZ;
      dW = *pDAdtOverAT;
      dX = *pDBdtOverBT;
      dC = dE * dX;
      dD = dG **pAOverBT;
      dS = dG - dG / ( dB -*pBT );
      dT = dG * ( dB - 1.0 );
      dU = dC * dG;
      dV = dE * dW - dC;
      *pHeat = dD * dV - dT * dC;

      /* Derivatives */
      if ( iDrv )
      {
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            dR = pAiOverAT[iNi];
            dH = pBiOverBT[iNi];
            dF = dE * dH * ( pDBidtOverBiT[iNi] - dX );
            dQ = pDZdz[iNi];

            /* TODO: The next line is too long. Please rewrite to make it shorter. */
            pDHeatdz[iNi] = dV * ( dS * ( dQ - dB * dH + dB ) + dD * ( dR - dH - 1.0 ) ) - dQ * dU + dD * ( ( pDAidtOverAiT[iNi] - dW ) * dE * dR - dF ) - dT * dF;
         }

         /* Form the fugacity temperature and pressure derivative */
         if ( iDrv >= EOS_DRV_P )
         {
            if ( iDrv >= EOS_DRV_T )
            {
               dQ = dA * dW;
               dR = dA * dX;
               dF = dR * ( 2.0 - dR ) + dE **pD2Bdt2OverBT;
               dH = *pDZdt;

               /* TODO: The next line is too long. Please rewrite to make it shorter. */
               *pDHeatdt = dV * ( dS * ( dH - dB * dX ) + dD * ( dW - dX ) ) - dU * dH + dD * ( dQ * ( 2.0 - dQ ) + dE **pD2Adt2OverAT - dF ) - dT * dF;
            }

            dA = *pDZdp;
            *pDHeatdp = dV * dS * ( dA - dB / *pP ) - dU * dA;
         }
      }
   }
}


/* 
// IdealHeat
//
// Routine to get the ideal heat terms 
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
//    Temperature for each object
// pX ** constant **
//    Composition for each object normalized to one
//       first index corresponds to object
//       second index corresponds to component
// pHeat
//    Enthalpy for each object
// pDHeatdt
//    Temperature derivative of enthalpy of each object
// pDHeatdz
//    Overall mole derivative of enthalpy of each object
//       first index corresponds to object
//       second index corresponds to component
//
// 1) The ideal heat contribution of each component
//    is assumed to be of the form
//
//       Hi = H1i * T + H2i * T^2 + H3i * T^3 +
//            H4i * T^4 + H5i * T^5 + H0
//
//    This is taken from the correlations of Charles Passut
//    and Ronald Danner, Ind. Eng. Chem. Process Des. Develop,
//    v 11, no 4, 1972, p 543 ff.
// 2) The total contribution is thus
//
//       H = Sum Hi
//            i
// 3) Differentiation of the formula is straightforward
*/
void EosPvtTable::IdealHeat( int iM, int iDrv, double *pT, double *pX, double *pHeat, double *pDHeatdt,
                             double *pDHeatdz, double *pHelp1T )
{
   double  dA;
   double  dB;
   double  dC;
   double *pTa;
   double *pTb;
   int     i;
   int     iNi;

   /* Ideal terms multiple blocks */
   if ( iM > 1 )
   {
      if ( iDrv )
      {
         pTa = pX;
         pTb = pDHeatdz;
         for ( i = 0; i < iM; i++ )
         {
            dA = pT[i];

            /* TODO: The next line is too long. Please rewrite to make it shorter. */
            dB = m_pHeat0[0] + dA * ( m_pHeat1[0] + dA * ( m_pHeat2[0] + dA * ( m_pHeat3[0] + dA * ( m_pHeat4[0] + dA * m_pHeat5[0] ) ) ) );
            pHelp1T[i] = dB **pTa++;
            *pTb += dB;
            pTb++;
         }

         for ( iNi = 1; iNi < m_iEosComp; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               dA = pT[i];

               /* TODO: The next line is too long. Please rewrite to make it shorter. */
               dB = m_pHeat0[iNi] + dA * ( m_pHeat1[iNi] + dA * ( m_pHeat2[iNi] + dA * ( m_pHeat3[iNi] + dA * ( m_pHeat4[iNi] + dA * m_pHeat5[iNi] ) ) ) );
               pHelp1T[i] += dB **pTa++;
               *pTb += dB;
               pTb++;
            }
         }
      }
      else
      {
         pTa = pX;
         for ( i = 0; i < iM; i++ )
         {
            dA = pT[i];

            /* TODO: The next line is too long. Please rewrite to make it shorter. */
            dB = m_pHeat0[0] + dA * ( m_pHeat1[0] + dA * ( m_pHeat2[0] + dA * ( m_pHeat3[0] + dA * ( m_pHeat4[0] + dA * m_pHeat5[0] ) ) ) );
            pHelp1T[i] = dB **pTa++;
         }

         for ( iNi = 1; iNi < m_iEosComp; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               dA = pT[i];

               /* TODO: The next line is too long. Please rewrite to make it shorter. */
               dB = m_pHeat0[iNi] + dA * ( m_pHeat1[iNi] + dA * ( m_pHeat2[iNi] + dA * ( m_pHeat3[iNi] + dA * ( m_pHeat4[iNi] + dA * m_pHeat5[iNi] ) ) ) );
               pHelp1T[i] += dB **pTa++;
            }
         }
      }

      /* Add to function and derivatives */
      for ( i = 0; i < iM; i++ )
      {
         pHeat[i] += pHelp1T[i];
      }

      if ( iDrv )
      {
         pTa = pDHeatdz;
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               *pTa -= pHelp1T[i];
               pTa++;
            }
         }

         if ( iDrv >= EOS_DRV_T )
         {
            pTa = pX;
            for ( iNi = 0; iNi < m_iEosComp; iNi++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  dA = pT[i];

                  /* TODO: The next line is too long. Please rewrite to make it shorter. */
                  pDHeatdt[i] += *pTa++ *( m_pHeat1[iNi] + dA * ( m_pHeat2[iNi] * 2.0 + dA * ( m_pHeat3[iNi] * 3.0 + dA * ( m_pHeat4[iNi] * 4.0 + dA * m_pHeat5[iNi] * 5.0 ) ) ) );
               }
            }
         }
      }
   }

   /* Ideal terms single block */
   else
   {
      if ( iDrv )
      {
         dA = *pT;

         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         dB = m_pHeat0[0] + dA * ( m_pHeat1[0] + dA * ( m_pHeat2[0] + dA * ( m_pHeat3[0] + dA * ( m_pHeat4[0] + dA * m_pHeat5[0] ) ) ) );
         dC = dB * pX[0];
         pDHeatdz[0] += dB;
         for ( iNi = 1; iNi < m_iEosComp; iNi++ )
         {
            /* TODO: The next line is too long. Please rewrite to make it shorter. */
            dB = m_pHeat0[iNi] + dA * ( m_pHeat1[iNi] + dA * ( m_pHeat2[iNi] + dA * ( m_pHeat3[iNi] + dA * ( m_pHeat4[iNi] + dA * m_pHeat5[iNi] ) ) ) );
            dC += dB * pX[iNi];
            pDHeatdz[iNi] += dB;
         }
      }
      else
      {
         dA = *pT;

         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         dB = m_pHeat0[0] + dA * ( m_pHeat1[0] + dA * ( m_pHeat2[0] + dA * ( m_pHeat3[0] + dA * ( m_pHeat4[0] + dA * m_pHeat5[0] ) ) ) );
         dC = dB * pX[0];
         for ( iNi = 1; iNi < m_iEosComp; iNi++ )
         {
            /* TODO: The next line is too long. Please rewrite to make it shorter. */
            dB = m_pHeat0[iNi] + dA * ( m_pHeat1[iNi] + dA * ( m_pHeat2[iNi] + dA * ( m_pHeat3[iNi] + dA * ( m_pHeat4[iNi] + dA * m_pHeat5[iNi] ) ) ) );
            dC += dB * pX[iNi];
         }
      }

      /* Add to function and derivatives */
      *pHeat += dC;
      if ( iDrv )
      {
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            pDHeatdz[iNi] -= dC;
         }

         if ( iDrv >= EOS_DRV_T )
         {
            dC = 0.0;
            for ( iNi = 0; iNi < m_iEosComp; iNi++ )
            {
               /* TODO: The next line is too long. Please rewrite to make it shorter. */
               dC += pX[iNi] * ( m_pHeat1[iNi] + dA * ( m_pHeat2[iNi] * 2.0 + dA * ( m_pHeat3[iNi] * 3.0 + dA * ( m_pHeat4[iNi] * 4.0 + dA * m_pHeat5[iNi] * 5.0 ) ) ) );
            }

            *pDHeatdt += dC;
         }
      }
   }
}


/* 
// VolumeTranslationHeat
// 
// Routine to get the heat from volume translation
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
// pCi ** constant **
//    Volume translation term for each component
// pDCidt ** constant **
//    Temperature derivative of volume term for each component
// pD2Cidt2 ** constant **
//    Second temperature derivative of volume term for each component
// pP ** constant **
//    Pressure for each object
// pT ** constant **
//    Temperature for each object
// pX ** constant **
//    Composition for each object normalized to one
//       first index corresponds to object
//       second index corresponds to component
// pHeat 
//    Enthalpy for each object
// pDHeatdp 
//    Pressure derivative of enthalpy of each object
// pDHeatdt 
//    Temperature derivative of enthalpy of each object
// pDHeatdz 
//    Overall mole derivative of enthalpy of each object
//       first index corresponds to object
//       second index corresponds to component
// pHelp1T
//    Help vector
// pHelp2T
//    Help vector
// pHelp3T
//    Help vector
//
// 1) If volume translation is used an additional term
//    appears in the equations.  Evaluation of the integral
//  
//    DMU = INT ( V - T * dVdT ) dP
//
//    is thus straightforward but tedious
*/
void EosPvtTable::VolumeTranslationHeat( int iM, int iDrv, double *pCi, double *pDCidt, double *pD2Cidt2, double *pP,
                                         double *pT, double *pX, double *pHeat, double *pDHeatdp, double *pDHeatdt,
                                         double *pDHeatdz, double *pHelp1T, double *pHelp2T, double *pHelp3T )
{
   double *pTa;
   double *pTb;
   double *pTc;
   double *pTd;
   double *pTe;
   double  dA;
   double  dB;
   double  dC;
   double  dD;
   double  dE;
   double  dZ;
   int     i;
   int     iNi;

   /* Needed term */
   dZ = m_dConvHeat / m_dConvPressure / m_dConvVolume;

   /* Multiple grid blocks */
   if ( iM > 1 )
   {
      for ( i = 0; i < iM; i++ )
      {
         pHelp3T[i] = 0.0;
      }

      pTa = pDHeatdz;
      pTb = pX;
      pTc = pCi;
      pTd = pDCidt;
      pTe = pD2Cidt2;
      for ( iNi = 0; iNi < m_iEosComp; iNi++ )
      {
         for ( i = 0; i < iM; i++ )
         {
            dB = ( *pTc++ -pT[i] **pTd++ ) * dZ;
            dA = dB **pTb++;
            pHelp2T[i] = dB;
            pHelp1T[i] = dA;
            pHelp3T[i] += dA;
         }

         if ( iDrv )
         {
            for ( i = 0; i < iM; i++ )
            {
               *pTa -= pHelp2T[i] * pP[i];
               pTa++;
            }

            if ( iDrv >= EOS_DRV_P )
            {
               for ( i = 0; i < iM; i++ )
               {
                  pDHeatdp[i] -= pHelp1T[i];
               }

               if ( iDrv >= EOS_DRV_T )
               {
                  pTb -= iM;
                  for ( i = 0; i < iM; i++ )
                  {
                     pDHeatdt[i] += dZ * pP[i] * pT[i] **pTe++ **pTb++;
                  }
               }
            }
         }
      }

      /* Change z factors and derivatives */
      for ( i = 0; i < iM; i++ )
      {
         pHelp3T[i] *= pP[i];
         pHeat[i] -= pHelp3T[i];
      }

      if ( iDrv )
      {
         pTa = pDHeatdz;
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               *pTa += pHelp3T[i];
               pTa++;
            }
         }
      }
   }

   /* Single grid block */
   else
   {
      dC = 0.0;
      dD = *pT;
      dE = *pP;

      /* With derivatives */
      if ( iDrv )
      {
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            dB = ( pCi[iNi] - dD * pDCidt[iNi] ) * dZ;
            dA = dB * pX[iNi];
            dC += dA;
            pDHeatdz[iNi] -= dB * dE;
            if ( iDrv >= EOS_DRV_P )
            {
               *pDHeatdp -= dA;
               if ( iDrv >= EOS_DRV_T )
               {
                  *pDHeatdt += dZ * dD * dE * pX[iNi] * pD2Cidt2[iNi];
               }
            }
         }
      }

      /* No derivatives */
      else
      {
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            dC += ( pCi[iNi] - dD * pDCidt[iNi] ) * dZ * pX[iNi];
         }
      }

      /* Change z factors and derivatives */
      dC *= dE;
      *pHeat -= dC;
      if ( iDrv )
      {
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            pDHeatdz[iNi] += dC;
         }
      }
   }
}


/* 
// Potential
// 
// Routine to get the potentials/fugacities
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
// iGetP ** constant **
//    Indicator for whether to calculate the chemical potential
//    or only the fugacity coefficient
//       EOS_POTENTIAL   
//          Calculate the full chemical potential     
//       EOS_NOPOTENTIAL     
//          Only calculate the fugacity coefficient
// pP ** constant **
//    Pressure for each object
// pT ** constant **
//    Temperature for each object
// pX ** constant **
//    Composition for each object normalized to one
//       first index corresponds to object
//       second index corresponds to component
// pZ 
//    Z factor for each object
// pDZdp 
//    Pressure derivative of z factor of each object
// pDZdt 
//    Temperature derivative of z factor of each object
// pDZdz 
//    Overall mole derivative of z factor of each object
//       first index corresponds to object
//       second index corresponds to component
// pPot
//    Fugacity or chemical potential for each object
//       first index corresponds to object
//       second index corresponds to component
// pDPotdp
//    Pressure derivative of fugacity or chemical potential of 
//    each object
//       first index corresponds to object
//       second index corresponds to component
// pDPotdt
//    Temperature derivative of fugacity or chemical potential of 
//    each object
//       first index corresponds to object
//       second index corresponds to component
// pDPotdz
//    Overall mole derivative of fugacity or chemical potential of 
//    each object
//       first index corresponds to object
//       second index corresponds to component
//       third index corresponds to component derivative
// pBT ** constant **
//    "B" coefficient for equation-of-state
// pAOverBT ** constant **
//    A over B.  This term will later be overwritten
//    for fugacity calculations
// pAiOverAT ** constant **
//    Ai over A
// pBiOverBT ** constant **
//    Bi over B
// pAijOverAT 
//    Aij over A.  The way that memory is arranged in the
//    flasher 
// pDAdtOverAT ** constant **
//    Derivative of A wrt temperature over A
// pDBdtOverBT ** constant **
//    Derivative of B wrt temperature over B
// pDAidtOverAiT ** constant **
//    Derivative of Ai wrt temperature over Ai
// pDBidtOverBT ** constant **
//    Derivative of Bi wrt temperature over Bi
// pHelp1T
//    Help vector
// pHelp2T
//    Help vector
// pHelp3T
//    Help vector
// pHelp4T
//    Help vector
// pHelp5T
//    Help vector
// pHelp6T
//    Help vector
//
// 1) The fugacity coefficient is first calculated.  It
//    is defined as
//
//    ln (FUG) = INT V dP
//
//    and for a generalized cubic equation of state is
//    for each component
//
//    LN ( FUG  ) =  B  / B * ( Z - 1 ) - LOG ( Z - B ) -
//            i       i
//
//          A * ( 2 * A  / A - B  / B ) / B *
//                     i        i
//
//          LOG ( ( Z + kB0 * B ) / LOG ( Z + kB1 * B ) )
//
//    When deriving this form REMEMBER that the 
//    sum of the mole fractions appears before each term!
//
// 2) The derivatives are tedious yet straight forward, with
//    one exception, namely, that the pressure derivative
//    of the fugacity is by definition the pressure derivative
//    of the integral, which in turn just the partial volume
//
// 3) The composition derivatives for each component are symmetric 
//    since they are the second derivatives of the same function
//
// 4) If getting the actual chemical potentials some additional
//    calculations are made.  First the Z factor is converted
//    to energy
//
//    Z = Z * R * T 
//
//    The potential must be modified as
//
//    pPot = pPot * R * T
//
//    The derivatives are straight forward
//
// 5) Evaluation of the integral
//  
//    DMU = INT V dP
//
//    is thus straightforward but tedious
*/
void EosPvtTable::Potential( int iM, int iDrv, int iGetP, double *pP, double *pT, double *pZ, double *pDZdp,
                             double *pDZdt, double *pDZdz, double *pPot, double *pDPotdp, double *pDPotdt,
                             double *pDPotdz, double *pBT, double *pAOverBT, double *pAiOverAT, double *pBiOverBT,
                             double *pAijOverAT, double *pDAdtOverAT, double *pDBdtOverBT, double *pDAidtOverAiT,
                             double *pDBidtOverBiT, double *pHelp1T, double *pHelp2T, double *pHelp3T, double *pHelp4T,
                             double *pHelp5T, double *pHelp6T, double *pHelp7T )
{
   double  dA;
   double  dB;
   double  dC;
   double  dD;
   double  dE;
   double  d1;
   double  d2;
   double  d3;
   double  d4;
   double  d5;
   double  d6;
   double  d7;
   double  d8;
   double  d9;
   double  dZ;
   double *pTa;
   double *pTb;
   double *pTc;
   double *pTd;
   double *pTe;
   double *pTf;
   double *pTg;
   double *pTh;
   double *pTi;
   double *pTj;
   double  dZt;
   double *pTk;
   double *pTl;
   double *pTm;
   int     i;
   int     iNi;
   int     iNj;
   int     iNk;
   int     iNcm;

   /* Save log term for fugacity */
   if ( iM == 1 )
   {
      dZ = *pZ;
      dE = *pBT;
      d7 = *pAOverBT;
      if ( iDrv >= EOS_DRV_T )
      {
         dZt = *pDZdt;
      }
      else
      {
         dZt = 0.0;
      }

      /* Other terms */
      dA = dZ - dE;
      d2 = dZ - 1.0;
      d3 = log( dA );
      d4 = 1.0 / dA;
      d5 = 1.0 - d4;
      d6 = dE * d4;

      /* For each component get fugacity and derivatives */
      pTa = pDPotdz;
      pTb = pAijOverAT;
      for ( iNi = 0; iNi < m_iEosComp; iNi++ )
      {
         dA = pAiOverAT[iNi];
         dB = pBiOverBT[iNi];
         d1 = dA - dB;
         dC = d1 * d7;

         /* Save terms for derivatives */
         if ( iDrv >= EOS_DRV_N )
         {
            dD = d5 * d1;
            d8 = dB - d4 - dD;
            d9 = d6 + dZ * dD - dB * d2;

            /* Symmetric derivatives wrt each component */
            iNk = iNi;
#ifdef IPFtune
            int cnt = m_iEosComp;
#pragma ivdep
            for ( iNj = 0; iNj < iNi; iNj++ )
            {
               pTa[iNj] = pDPotdz[iNk];
               iNk += cnt;
            }
#else   
            for ( iNj = 0; iNj < iNi; iNj++ )
            {
               pTa[iNj] = pDPotdz[iNk];
               iNk += m_iEosComp;
            }
#endif

            /* Form term */
#ifdef IPFtune
            int cnt2 = m_iEosComp;
#pragma ivdep
            for ( iNj = iNi; iNj < cnt2; iNj++ )
            {
               dD = pBiOverBT[iNj];
               dE = pTb[iNj];
               pTa[iNj] = d7 * ( dA * dD + dB * pAiOverAT[iNj] - 2.0 * ( dB * dD + dE ) ) + d9 * ( dD - 1.0 ) + d8 * pDZdz[iNj];
            }
#else
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
            for ( iNj = iNi; iNj < m_iEosComp; iNj++ )
            {
               dD = pBiOverBT[iNj];
               dE = pTb[iNj];
               pTa[iNj] = d7 * ( dA * dD + dB * pAiOverAT[iNj] - 2.0 * ( dB * dD + dE ) ) + d9 * ( dD - 1.0 ) + d8 * pDZdz[iNj];
            }
#endif

            pTa += m_iEosComp;
            pTb += m_iEosComp;

            /* Form the fugacity temperature pressure derivatives */
            if ( iDrv >= EOS_DRV_P )
            {
               if ( iDrv >= EOS_DRV_T )
               {
                  dD = d7 * dA;

                  /* TODO: The next line is too long. Please rewrite to make it shorter. */
                  pDPotdt[iNi] = dZt * d8 +*pDAdtOverAT * ( dD - dC ) - pDAidtOverAiT[iNi] * dD +*pDBdtOverBT * ( d9 + dC - d7 * dB ) + pDBidtOverBiT[iNi] * ( dB * ( d2 + d7 ) );
               }

               pDPotdp[iNi] = d2 + pDZdz[iNi];
            }
         }

         /* Set the fugacity */
         pPot[iNi] = dB * d2 - d3 - dC;
      }

      if ( iDrv >= EOS_DRV_P )
      {
         dC = 1.0 / *pP;
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            pDPotdp[iNi] *= dC;
         }
      }
   }

   /* For multiple blocks */
   else
   {
      iNcm = m_iEosComp * iM;

      /* Other terms */
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
#ifdef IPFtune
#pragma ivdep
#endif
      for ( i = 0; i < iM; i++ )
      {
         dC = pZ[i];
         dB = dC - pBT[i];
         dA = 1.0 / dB;
         pHelp2T[i] = dC - 1.0;
         pHelp3T[i] = dB;
         pHelp4T[i] = dA;
         pHelp5T[i] = 1.0 - dA;
         pBT[i] *= dA;
      }

      EosUtil::VectorLog( iM, pHelp3T, pHelp3T );

      /* For each component get fugacity and derivatives */
      pTa = pPot;
      pTb = pBiOverBT;
      pTc = pDPotdp;
      pTd = pDPotdt;
      pTe = pAiOverAT;
      pTf = pDAidtOverAiT;
      pTg = pDBidtOverBiT;
      pTh = pDZdz;
      pTi = pDPotdz;
      pTj = pAijOverAT;
      for ( iNi = 0; iNi < m_iEosComp; iNi++ )
      {
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
#ifdef IPFtune
#pragma ivdep
#endif
         for ( i = 0; i < iM; i++ )
         {
            pHelp1T[i] = pTe[i] - pTb[i];
         }

         /* Save terms for derivatives */
         if ( iDrv )
         {
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
#ifdef IPFtune
#pragma ivdep
#endif
            for ( i = 0; i < iM; i++ )
            {
               dA = pTb[i];
               dB = pHelp5T[i] * pHelp1T[i];
               pHelp6T[i] = dA - pHelp4T[i] - dB;
               pHelp7T[i] = pBT[i] + pZ[i] * dB - dA * pHelp2T[i];
            }

            /* Symmetric derivatives wrt each component */
            iNk = iNi * iM;
            pTk = pDPotdz + iNk;
            for ( iNj = 0; iNj < iNi; iNj++ )
            {
#ifdef IPFtune
               int cnt = iM;
#pragma ivdep
               for ( i = 0; i < cnt; i++ )
               {
                  pTi[i] = pTk[i];
               }
#else
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
               for ( i = 0; i < iM; i++ )
               {
                  pTi[i] = pTk[i];
               }
#endif

               pTi += iM;
               pTk += iNcm;
            }

            /* Form term */
            pTk = pBiOverBT + iNk;
            pTl = pAiOverAT + iNk;
            pTm = pDZdz + iNk;
            pTj += iNk;
            for ( iNj = iNi; iNj < m_iEosComp; iNj++ )
            {
#ifdef IPFtune
               int cnt = iM;
#pragma ivdep
               for ( i = 0; i < cnt; i++ )
               {
                  dA = pTb[i];
                  dB = pTk[i];

                  /* TODO: The next line is too long. Please rewrite to make it shorter. */
                  pTi[i] = pAOverBT[i] * ( pTe[i] * dB + dA * pTl[i] - 2.0 * ( dA * dB + pTj[i] ) ) + ( dB - 1.0 ) * pHelp7T[i] + pHelp6T[i] * pTm[i];
               }
#else
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
               for ( i = 0; i < iM; i++ )
               {
                  dA = pTb[i];
                  dB = pTk[i];

                  /* TODO: The next line is too long. Please rewrite to make it shorter. */
                  pTi[i] = pAOverBT[i] * ( pTe[i] * dB + dA * pTl[i] - 2.0 * ( dA * dB + pTj[i] ) ) + ( dB - 1.0 ) * pHelp7T[i] + pHelp6T[i] * pTm[i];
               }
#endif

               pTi += iM;
               pTj += iM;
               pTk += iM;
               pTl += iM;
               pTm += iM;
            }

            /* Form the fugacity temperature pressure derivatives */
            if ( iDrv >= EOS_DRV_P )
            {
               if ( iDrv >= EOS_DRV_T )
               {
                  for ( i = 0; i < iM; i++ )
                  {
                     dA = pAOverBT[i];
                     dB = pTb[i];
                     dC = dA * pTe[i];
                     dD = dB * ( pHelp2T[i] + dA );
                     dZ = pHelp1T[i] * dA;

                     /* TODO: The next line is too long. Please rewrite to make it shorter. */
                     *pTd++ = pDZdt[i] * pHelp6T[i] + pDAdtOverAT[i] * ( dC - dZ ) -*pTf++ *dC +*pTg++ *dD + pDBdtOverBT[i] * ( pHelp7T[i] + dZ - dA * dB );
                  }
               }

#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
#ifdef IPFtune
#pragma ivdep
#endif
               for ( i = 0; i < iM; i++ )
               {
                  pTc[i] = pHelp2T[i] + pTh[i];
               }

               pTc += iM;
               pTh += iM;
            }
         }

         /* Set the fugacity */
#ifdef IPFtune
         int cnt = iM;
#pragma ivdep
         for ( i = 0; i < cnt; i++ )
         {
            pTa[i] = pTb[i] * pHelp2T[i] - pHelp3T[i] - pHelp1T[i] * pAOverBT[i];
         }
#else
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
         for ( i = 0; i < iM; i++ )
         {
            pTa[i] = pTb[i] * pHelp2T[i] - pHelp3T[i] - pHelp1T[i] * pAOverBT[i];
         }
#endif

         pTa += iM;
         pTb += iM;
         pTe += iM;
      }

      if ( iDrv >= EOS_DRV_P )
      {
         pTa = pDPotdp;
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
         for ( i = 0; i < iM; i++ )
         {
            dZ = 1.0 / pP[i];
            pTa[i] *= dZ;
            pHelp2T[i] = dZ;
         }

         for ( iNi = 1; iNi < m_iEosComp; iNi++ )
         {
            pTa += iM;
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
            for ( i = 0; i < iM; i++ )
            {
               pTa[i] *= pHelp2T[i];
            }
         }
      }
   }

   /* Compute the actual potential */
   if ( iGetP )
   {
      /* Multiple grid blocks */
      if ( iM > 1 )
      {
         /* Add the pressure term */
         dZ = m_dConvHeat * m_dGascon / m_dConvPressure / m_dConvVolume;
         for ( i = 0; i < iM; i++ )
         {
            pHelp2T[i] = dZ * pT[i];
         }

         /* Derivatives */
         if ( iDrv )
         {
            pTa = pDPotdz;
            pTb = pDZdz;
            for ( iNi = 0; iNi < m_iEosComp; iNi++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  *pTb *= pHelp2T[i];
                  pTb++;
               }

               for ( iNj = 0; iNj < m_iEosComp; iNj++ )
               {
                  for ( i = 0; i < iM; i++ )
                  {
                     *pTa *= pHelp2T[i];
                     pTa++;
                  }
               }
            }

            if ( iDrv >= EOS_DRV_P )
            {
               for ( i = 0; i < iM; i++ )
               {
                  pDZdp[i] *= pHelp2T[i];
               }

               pTa = pDPotdp;
               for ( iNi = 0; iNi < m_iEosComp; iNi++ )
               {
                  for ( i = 0; i < iM; i++ )
                  {
                     *pTa *= pHelp2T[i];
                     pTa++;
                  }
               }

               if ( iDrv >= EOS_DRV_T )
               {
                  for ( i = 0; i < iM; i++ )
                  {
                     pDZdt[i] = pHelp2T[i] * pDZdt[i] + dZ * pZ[i];
                  }

                  pTa = pDPotdt;
                  pTb = pPot;
                  for ( iNi = 0; iNi < m_iEosComp; iNi++ )
                  {
                     for ( i = 0; i < iM; i++ )
                     {
                        *pTa = pHelp2T[i] **pTa + dZ **pTb++;
                        pTa++;
                     }
                  }
               }
            }
         }

         /* Finally update the function */
         for ( i = 0; i < iM; i++ )
         {
            pZ[i] *= pHelp2T[i];
         }

         pTa = pPot;
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               *pTa *= pHelp2T[i];
               pTa++;
            }
         }
      }

      /* Single grid block */
      else
      {
         /* Add the pressure term */
         dC = *pT;
         dZ = m_dConvHeat * m_dGascon / m_dConvPressure / m_dConvVolume;
         dB = dZ * dC;

         /* Derivatives */
         if ( iDrv )
         {
            for ( iNi = 0; iNi < m_iEosComp; iNi++ )
            {
               pDZdz[iNi] *= dB;
            }

            pTa = pDPotdz;
            for ( iNi = 0; iNi < m_iEosComp; iNi++ )
            {
               for ( iNj = 0; iNj < m_iEosComp; iNj++ )
               {
                  *pTa *= dB;
                  pTa++;
               }
            }

            if ( iDrv >= EOS_DRV_P )
            {
               *pDZdp *= dB;
               for ( iNi = 0; iNi < m_iEosComp; iNi++ )
               {
                  pDPotdp[iNi] *= dB;
               }

               if ( iDrv >= EOS_DRV_T )
               {
                  *pDZdt = *pDZdt * dB + dZ **pZ;
                  for ( iNi = 0; iNi < m_iEosComp; iNi++ )
                  {
                     pDPotdt[iNi] = dB * pDPotdt[iNi] + dZ * pPot[iNi];
                  }
               }
            }
         }

         /* Finally update the function */
         *pZ *= dB;
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            pPot[iNi] *= dB;
         }
      }
   }
}


/* 
//
// Routine to get the ideal contribution to the potentials 
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
//    Temperature for each object
// pX ** constant **
//    Composition for each object normalized to one
//       first index corresponds to object
//       second index corresponds to component
// pPot
//    Fugacity or chemical potential for each object
//       first index corresponds to object
//       second index corresponds to component
// pDPotdt
//    Temperature derivative of fugacity or chemical potential of 
//    each object
//       first index corresponds to object
//       second index corresponds to component
// pHelp1T
//    Help vector
//
// 1) The ideal entropy contribution of each component
//    is assumed to be of the form
//
//    EntropyI[T] = Heat1I * ln ( T ) + 2 * Heat2I * T + 
//                  3 * Heat3I * T * T / 2 + ...
//                  5 * Heat5I * T ^ 4 / 4 + Heat6I
//
//    This is taken from the correlations of Charles Passut
//    and Ronald Danner, Ind. Eng. Chem. Process Des. Develop,
//    v 11, no 4, 1972, p 543 ff.
// 2) Since
//
//    dG = V dp - S dt
//    
//    the contribution for each component becomes
//
//    pPoti = pPoti - Heat1i * ( ln ( T ) - 1 ) + Heat2i * T^2 + 
//                    Heat3i * T^3 / 2 + Heat3i * T^4 / 3 +
//                    Heat5i * T^5 / 4 + Heat6i * T
// 3) Differentiation of the formula is straightforward
*/
void EosPvtTable::IdealPotential( int iM, int iDrv, double *pT, double *pPot, double *pDPotdt, double *pHelp1T )
{
   double  d1;
   double  d2;
   double  d3;
   double  d4;
   double  d5;
   double  d6;
   double  dA;
   double  dB;
   double  dC;
   double *pTa;
   int     i;
   int     iNi;

   /* Multiple blocks */
   if ( iM > 1 )
   {
      /* Save log term */
      for ( i = 0; i < iM; i++ )
      {
         pHelp1T[i] = log( pT[i] ) - 1.0;
      }

      /* Form heats */
      pTa = pPot;
      for ( iNi = 0; iNi < m_iEosComp; iNi++ )
      {
         d1 = m_pHeat1[iNi];
         d2 = m_pHeat2[iNi];
         d3 = m_pHeat3[iNi] * 0.5;
         d4 = m_pHeat4[iNi] / 3.0;
         d5 = m_pHeat5[iNi] * 0.25;
         d6 = m_pHeat6[iNi];
         for ( i = 0; i < iM; i++ )
         {
            dA = pT[i];
            *pTa -= dA * ( d1 * pHelp1T[i] + d6 + dA * ( d2 + dA * ( d3 + dA * ( d4 + dA * d5 ) ) ) );
            pTa++;
         }
      }

      /* Form derivatives */
      if ( iDrv >= EOS_DRV_T )
      {
         for ( i = 0; i < iM; i++ )
         {
            pHelp1T[i] += 1.0;
         }

         pTa = pDPotdt;
         dC = 4.0 / 3.0;
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            d1 = m_pHeat1[iNi];
            d2 = m_pHeat2[iNi] * 2.00;
            d3 = m_pHeat3[iNi] * 1.50;
            d4 = m_pHeat4[iNi] * dC;
            d5 = m_pHeat5[iNi] * 1.25;
            d6 = m_pHeat6[iNi];
            for ( i = 0; i < iM; i++ )
            {
               dA = pT[i];
               *pTa -= ( d1 * pHelp1T[i] + d6 + dA * ( d2 + dA * ( d3 + dA * ( d4 + dA * d5 ) ) ) );
               pTa++;
            }
         }
      }
   }

   /* Single blocks */
   else
   {
      dC = *pT;
      dB = log( dC ) - 1.0;
      for ( iNi = 0; iNi < m_iEosComp; iNi++ )
      {
         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         pPot[iNi] -= dC * ( m_pHeat1[iNi] * dB + m_pHeat6[iNi] + dC * ( m_pHeat2[iNi] + dC * ( m_pHeat3[iNi] * 0.5 + dC * ( m_pHeat4[iNi] / 3.0 + dC * m_pHeat5[iNi] * 0.25 ) ) ) );
      }

      /* Derivatives */
      if ( iDrv >= EOS_DRV_T )
      {
         dB += 1.0;
         dA = 4.0 / 3.0;
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            /* TODO: The next line is too long. Please rewrite to make it shorter. */
            pDPotdt[iNi] -= ( m_pHeat1[iNi] * dB + m_pHeat6[iNi] + dC * ( m_pHeat2[iNi] * 2.0 + dC * ( m_pHeat3[iNi] * 1.5 + dC * ( m_pHeat4[iNi] * dA + dC * m_pHeat5[iNi] * 1.25 ) ) ) );
         }
      }
   }
}


/* 
// VolumeTranslatePotential
// 
// Routine to get the potentials from volume translation
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
// pCi ** constant **
//    Volume translation term for each component
// pDCidt ** constant **
//    Temperature derivative of volume term for each component
// pP ** constant **
//    Pressure for each object
// pT ** constant **
//    Temperature for each object
// pX ** constant **
//    Composition for each object normalized to one
//       first index corresponds to object
//       second index corresponds to component
// pZ 
//    Z factor for each object
// pDZdp 
//    Pressure derivative of z factor of each object
// pDZdt 
//    Temperature derivative of z factor of each object
// pDZdz 
//    Overall mole derivative of z factor of each object
//       first index corresponds to object
//       second index corresponds to component
// pPot
//    Fugacity or chemical potential for each object
//       first index corresponds to object
//       second index corresponds to component
// pDPotdp
//    Pressure derivative of fugacity or chemical potential of 
//    each object
//       first index corresponds to object
//       second index corresponds to component
// pDPotdt
//    Temperature derivative of fugacity or chemical potential of 
//    each object
//       first index corresponds to object
//       second index corresponds to component
// pHelp1T
//    Help vector
// pHelp2T
//    Help vector
// pHelp3T
//    Help vector
//
// 1) If volume translation is used an additional term
//    appears in the equations.  Evaluation of the integral
//  
//    DMU = INT V dP
//
//    is thus straightforward but tedious
// 2) Note that when the potential is calculated that we
//    assume that we are returning VP rather than P
*/
void EosPvtTable::VolumeTranslationPotential( int iM, int iDrv, double *pCi, double *pDCidt, double *pP, double *pX,
                                              double *pZ, double *pDZdp, double *pDZdt, double *pDZdz, double *pPot,
                                              double *pDPotdp, double *pDPotdt, double *pHelp1T, double *pHelp2T,
                                              double *pHelp3T )
{
   double *pTa;
   double *pTb;
   double *pTc;
   double *pTd;
   double *pTe;
   double *pTf;
   double *pTg;
   double  dA;
   double  dB;
   double  dC;
   double  dD;
   double  dE;
   double  dZ;
   int     i;
   int     iNi;

   /* Needed term */
   dZ = m_dConvHeat / m_dConvPressure / m_dConvVolume;

   /* Multiple grid blocks */
   if ( iM > 1 )
   {
      for ( i = 0; i < iM; i++ )
      {
         pHelp3T[i] = 0.0;
      }

      pTa = pPot;
      pTb = pDPotdp;
      pTc = pX;
      pTd = pDPotdt;
      pTe = pDZdz;
      pTf = pCi;
      pTg = pDCidt;
      for ( iNi = 0; iNi < m_iEosComp; iNi++ )
      {
         for ( i = 0; i < iM; i++ )
         {
            dB = dZ **pTf++;
            dA = pTc[i] * dB;
            dC = dB * pP[i];
            *pTa -= dC;
            pHelp1T[i] = dB;
            pHelp2T[i] = dC;
            pHelp3T[i] += dA;
            pTa++;
         }

         if ( iDrv )
         {
            for ( i = 0; i < iM; i++ )
            {
               *pTe -= pHelp2T[i];
               pTe++;
            }

            if ( iDrv >= EOS_DRV_P )
            {
               for ( i = 0; i < iM; i++ )
               {
                  *pTb -= pHelp1T[i];
                  pTb++;
               }

               if ( iDrv >= EOS_DRV_T )
               {
                  for ( i = 0; i < iM; i++ )
                  {
                     dA = *pTg++ *dZ * pP[i];
                     pDZdt[i] -= dA * pTc[i];
                     *pTd -= dA;
                     pTd++;
                  }
               }
            }
         }

         pTc += iM;
      }

      /* Change z factors and derivatives */
      if ( iDrv >= EOS_DRV_P )
      {
         for ( i = 0; i < iM; i++ )
         {
            pDZdp[i] -= pHelp3T[i];
         }
      }

      for ( i = 0; i < iM; i++ )
      {
         pHelp3T[i] *= pP[i];
         pZ[i] -= pHelp3T[i];
      }

      if ( iDrv )
      {
         pTa = pDZdz;
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               *pTa += pHelp3T[i];
               pTa++;
            }
         }
      }
   }

   /* Single grid block */
   else
   {
      dD = *pP;
      dC = 0.0;

      /* With derivatives */
      if ( iDrv )
      {
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            dB = pCi[iNi] * dZ;
            dA = dB * pX[iNi];
            dE = dB * dD;
            dC += dA;
            pPot[iNi] -= dE;
            pDZdz[iNi] -= dE;
            if ( iDrv >= EOS_DRV_P )
            {
               pDPotdp[iNi] -= dB;
               *pDZdp -= dA;
               if ( iDrv >= EOS_DRV_T )
               {
                  dA = pDCidt[iNi] * dZ * dD;
                  *pDZdt -= dA * pX[iNi];
                  pDPotdt[iNi] -= dA;
               }
            }
         }

         if ( iDrv >= EOS_DRV_P )
         {
            *pDZdp -= dC;
         }

         dC *= dD;
      }

      /* No derivatives */
      else
      {
         dD *= dZ;
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            dB = pCi[iNi] * dD;
            pPot[iNi] -= dB;
            dC += dB * pX[iNi];
         }
      }

      /* Change z factors and derivatives */
      *pZ -= dC;
      if ( iDrv )
      {
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            pDZdz[iNi] += dC;
         }
      }
   }
}


/* 
// Density
//
// Routine to convert to volume to density 
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
// iMolar ** constant **
//    Indicator for molar (1) quantities or mass (2) quantities
// pMw ** constant **   
//    Phase molecular weight 
// pZ 
//    On input the molar volume of each object
//    On output the molar or mass density
// pDZdp 
//    On input pressure derivative of molar volume of each object
//    On output pressure derivative of molar or mass density of each 
//    object
// pDZdt 
//    On input temperature derivative of molar volume of each object
//    On output temperature derivative of molar or mass density of 
//    each object
// pDZdx 
//    On input overall mole derivative of molar volume of each object
//    On output overall mole derivative of molar or mass density of 
//    each object
//       first index corresponds to object
//       second index corresponds to component
//
// 1) The density is simply the inverse of the volume.  When
//    mass quantities are required divide by the molecular
//    weight.  
*/
void EosPvtTable::Density( int iM, int iDrv, int iMolar, double *pMw, double *pZ, double *pDZdp, double *pDZdt,
                           double *pDZdx )
{
   double *pTa;
   double *pTb;
   double  dA;
   double  dB;
   int     iNi;
   int     i;

   /* Derivatives for multiple grid blocks */
   if ( iM > 1 )
   {
      /* Density: derivatives first */
      if ( iDrv )
      {
         if ( iMolar )
         {
            pTa = pDZdx;
            for ( iNi = 0; iNi < m_iEosComp; iNi++ )
            {
               pTb = pZ;
               for ( i = 0; i < iM; i++ )
               {
                  *pTa = -*pTa / pZ[i];
                  pTa++;
               }
            }
         }
         else
         {
            pTa = pDZdx;
            pTb = m_pMolecularWeight;
            for ( iNi = 0; iNi < m_iEosComp; iNi++ )
            {
               dA = *pTb++;
               for ( i = 0; i < iM; i++ )
               {
                  *pTa = dA / pMw[i] - 1.0 -*pTa / pZ[i];
                  pTa++;
               }
            }
         }

         /* Take other derivatives */
         if ( iDrv >= EOS_DRV_P )
         {
            for ( i = 0; i < iM; i++ )
            {
               pDZdp[i] = -pDZdp[i] / pZ[i];
            }

            if ( iDrv >= EOS_DRV_T )
            {
               for ( i = 0; i < iM; i++ )
               {
                  pDZdt[i] = -pDZdt[i] / pZ[i];
               }
            }
         }
      }

      /* Set z factor */
      if ( iMolar )
      {
         for ( i = 0; i < iM; i++ )
         {
            pZ[i] = 1.0 / pZ[i];
         }
      }
      else
      {
         for ( i = 0; i < iM; i++ )
         {
            pZ[i] = pMw[i] / pZ[i];
         }
      }

      /* Finish the derivatives */
      if ( iDrv )
      {
         pTa = pDZdx;
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               *pTa *= pZ[i];
               pTa++;
            }
         }

         if ( iDrv >= EOS_DRV_P )
         {
            for ( i = 0; i < iM; i++ )
            {
               pDZdp[i] *= pZ[i];
            }

            if ( iDrv >= EOS_DRV_T )
            {
               for ( i = 0; i < iM; i++ )
               {
                  pDZdt[i] *= pZ[i];
               }
            }
         }
      }
   }

   /* Single grid block derivatives */
   else
   {
      dA = *pZ;
      dB = *pMw;

      /* Derivatives: density first */
      if ( iDrv )
      {
         if ( iMolar )
         {
            for ( iNi = 0; iNi < m_iEosComp; iNi++ )
            {
               pDZdx[iNi] = -pDZdx[iNi] / dA;
            }
         }
         else
         {
            pTa = m_pMolecularWeight;
            for ( iNi = 0; iNi < m_iEosComp; iNi++ )
            {
               pDZdx[iNi] = pTa[iNi] / dB - 1.0 - pDZdx[iNi] / dA;
            }
         }

         /* Other derivatives */
         if ( iDrv >= EOS_DRV_P )
         {
            *pDZdp = -*pDZdp / dA;
            if ( iDrv >= EOS_DRV_T )
            {
               *pDZdt = -*pDZdt / dA;
            }
         }
      }

      /* Set z factor */
      *pZ = ( iMolar ? 1.0 : dB ) / dA;

      /* Finish the derivatives */
      if ( iDrv )
      {
         dA = *pZ;
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            pDZdx[iNi] *= dA;
         }

         if ( iDrv >= EOS_DRV_P )
         {
            *pDZdp *= dA;
            if ( iDrv >= EOS_DRV_T )
            {
               *pDZdt *= dA;
            }
         }
      }
   }
}


/* 
// GetComps
//
// Get the pseudo components for viscosity calculations
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
// iMolar ** constant **
//    Indicator for moles or mass
// iLights ** constant **
//    Indicator whether to gather heavies or lights
// dTiny ** constant **
//    A tiny number
// pZ ** constant ** 
//    Mole or mass fraction of each object
//       first index corresponds to object
//       second index corresponds to component
// pDZdp ** constant ** 
//    Pressure derivative of mole/mass fraction of each object
//       first index corresponds to object
//       second index corresponds to component
// pDZdt ** constant ** 
//    Temperature derivative of mole/mass fraction of each object
//       first index corresponds to object
//       second index corresponds to component
// pDZda ** constant ** 
//    Accumulation derivative of mole/mass fraction of each object
//       first index corresponds to object
//       second index corresponds to component
//       third index corresponds to component derivative
// pX  
//    Light mole or mass fraction of each object
//       first index corresponds to object
//       second index corresponds to component
// pDXdp 
//    Pressure derivative of light mole/mass fraction of each object
//       first index corresponds to object
//       second index corresponds to component
// pDXdt  
//    Temperature derivative of light mole/mass fraction of each 
//    object
//       first index corresponds to object
//       second index corresponds to component
// pDXda 
//    Accumulation derivative of light mole/mass fraction of each 
//    object
//       first index corresponds to object
//       second index corresponds to component
//       third index corresponds to component derivative
// pNComp
//    Number of components
// pRho ** constant ** 
//    Mole/mass density of each object
// pDRhodp ** constant **
//    Pressure derivative of mole/mass density of each object
// pDRhodt ** constant **
//    Temperature derivative of mole/mass density of each object
// pDRhoda ** constant ** 
//    Accumulation derivative of mole/mass density of each object
//       first index corresponds to object
//       second index corresponds to component
// pRo ** constant ** 
//    Mole density of each object
// pDRodp ** constant **
//    Pressure derivative of mole density of each object
// pDRodt ** constant **
//    Temperature derivative of mole density of each object
// pDRoda ** constant ** 
//    Accumulation derivative of mole density of each object
//       first index corresponds to object
//       second index corresponds to component
// pMWC
//    Molecular weight of components
// pPCC
//    Critical pressure of components
// pTCC
//    Critical temperature of components
// pVCC
//    Critical volume of components
// pTmp
//    Beginning of work array used in the computations
//
// 1) First determine which components are proper type and store
//    critical data
// 2) Compute the normalized compositions and derivatives
*/
void EosPvtTable::GetComps( int iM, int iDrv, int iMolar, int iLights, double dTiny, double *pZ, double *pDZdp,
                            double *pDZdt, double *pDZda, double *pX, double *pDXdp, double *pDXdt, double *pDXda,
                            int *pNComp, double *pRho, double *pDRhodp, double *pDRhodt, double *pDRhoda, double *pRo,
                            double *pDRodp, double *pDRodt, double *pDRoda, double *pMWC, double *pPCC, double *pTCC,
                            double *pVCC, double *pTmp )
{
   double *pTa;
   double *pTb;
   double *pTc;
   double *pTd;
   double *pTe;
   double *pTf;
   double *pDrv;
   double  dA;
   double  dB;
   int     iNi;
   int     iNj;
   int     iNComp;
   int     i;
   int     iTemp;

   /* Set number of Comps to zero and pointers */
   iNComp = 0;
   pTa = pZ;
   pTb = pDZdp;
   pTc = pDZdt;
   pTd = pX;
   pTe = pDXdp;
   pTf = pDXdt;

   /* For each component */
   for ( iNi = 0; iNi < m_iEosComp; iNi++ )
   {
      /* If molecular weight small enough consider a "Comp" */
      dA = m_pMolecularWeight[iNi];
      iTemp = iLights ? dA < EOS_JOSSI_MW_SPLIT : dA >= EOS_JOSSI_MW_SPLIT;
      if ( iTemp )
      {
         /* Set the critical properties */
         pMWC[iNComp] = dA;
         pPCC[iNComp] = m_pCriticalPressure[iNi];
         pTCC[iNComp] = m_pCriticalTemperature[iNi];
         pVCC[iNComp] = m_pCriticalVolume[iNi];
         iNComp++;

         /* Convert to moles */
         dA = iMolar ? 1.0 : 1.0 / dA;

         /* Set the fractions and save sum */
         for ( i = 0; i < iM; i++ )
         {
            *pTd++ = dA **pTa++;
         }

         /* Pressure derivatives */
         if ( iDrv >= EOS_DRV_P )
         {
            for ( i = 0; i < iM; i++ )
            {
               *pTe++ = dA **pTb++;
            }

            /* Pressure derivatives */
            if ( iDrv >= EOS_DRV_T )
            {
               for ( i = 0; i < iM; i++ )
               {
                  *pTf++ = dA **pTc++;
               }
            }
         }
      }

      /* Increment counters */
      else
      {
         pTa += iM;
         pTb += iM;
         pTc += iM;
      }
   }

   /* If Comps, need to normalize */
   if ( iNComp > 0 )
   {
      pDrv = pTmp + iM;

      /* Set the normalization factor */
      pTa = pX;
      for ( i = 0; i < iM; i++ )
      {
         pTmp[i] = *pTa++;
      }

      for ( iNi = 1; iNi < iNComp; iNi++ )
      {
         for ( i = 0; i < iM; i++ )
         {
            pTmp[i] += *pTa++;
         }
      }

      /* Check the normalization factor */
      pTa = pX;
      for ( i = 0; i < iM; i++ )
      {
         dA = pTmp[i];
         iTemp = dA < dTiny;
         dB = iTemp ? 0.0 : ( 1.0 / dA );
         *pTa = iTemp ? 1.0 : *pTa * dB;
         pTmp[i] = dB;
         pTa++;
      }

      /* Normalize */
      for ( iNi = 1; iNi < iNComp; iNi++ )
      {
         for ( i = 0; i < iM; i++ )
         {
            *pTa *= pTmp[i];
            pTa++;
         }
      }

      /* Derivatives */
      if ( iDrv )
      {
         /* For each component */
         pTa = pDZda;
         pTb = pDXda;
         for ( iNj = 0; iNj < m_iEosComp; iNj++ )
         {
            /* Zero out derivative */
            for ( i = 0; i < iM; i++ )
            {
               pDrv[i] = 0.0;
            }

            /* Sum over components */
            pTc = pTb;
            for ( iNi = 0; iNi < m_iEosComp; iNi++ )
            {
               /* If molecular weight small enough a "Comp" */
               dA = m_pMolecularWeight[iNi];
               iTemp = iLights ? dA < EOS_JOSSI_MW_SPLIT : dA >= EOS_JOSSI_MW_SPLIT;
               if ( iTemp )
               {
                  dA = iMolar ? 1.0 : 1.0 / dA;
                  for ( i = 0; i < iM; i++ )
                  {
                     dB = dA **pTa++;
                     pDrv[i] += dB;
                     *pTc++ = dB;
                  }
               }

               /* Else increment counter */
               else
               {
                  pTa += iM;
               }
            }

            /* Now form the derivatives */
            pTc = pX;
            for ( iNi = 0; iNi < iNComp; iNi++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  *pTb = pTmp[i] * ( *pTb - pDrv[i] **pTc++ );
                  pTb++;
               }
            }
         }

         /* Pressure derivatives */
         if ( iDrv >= EOS_DRV_P )
         {
            /* Sum of the derivatives */
            pTa = pDXdp;
            for ( i = 0; i < iM; i++ )
            {
               pDrv[i] = *pTa++;
            }

            for ( iNi = 1; iNi < iNComp; iNi++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  pDrv[i] += *pTa++;
               }
            }

            /* Take derivatives */
            pTa = pDXdp;
            pTb = pX;
            for ( iNi = 0; iNi < iNComp; iNi++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  *pTa = pTmp[i] * ( *pTa - pDrv[i] **pTb++ );
                  pTa++;
               }
            }

            /* Temperature derivatives */
            if ( iDrv >= EOS_DRV_T )
            {
               /* Sum of the derivatives */
               pTa = pDXdt;
               for ( i = 0; i < iM; i++ )
               {
                  pDrv[i] = *pTa++;
               }

               for ( iNi = 1; iNi < iNComp; iNi++ )
               {
                  for ( i = 0; i < iM; i++ )
                  {
                     pDrv[i] += *pTa++;
                  }
               }

               /* Take derivatives */
               pTa = pDXdt;
               pTb = pX;
               for ( iNi = 0; iNi < iNComp; iNi++ )
               {
                  for ( i = 0; i < iM; i++ )
                  {
                     *pTa = pTmp[i] * ( *pTa - pDrv[i] **pTb++ );
                     pTa++;
                  }
               }
            }
         }
      }
   }

   /* Set the number of Comps */
   *pNComp = iNComp;

   /* Transform to molar density */
   if ( iLights && iNComp )
   {
      /* Molar values; just copy */
      if ( iMolar )
      {
         for ( i = 0; i < iM; i++ )
         {
            pRo[i] = pRho[i];
         }

         /* Derivatives */
         if ( iDrv )
         {
            iNi = iM * m_iEosComp;
            for ( i = 0; i < iNi; i++ )
            {
               pDRoda[i] = pDRhoda[i];
            }

            if ( iDrv >= EOS_DRV_P )
            {
               for ( i = 0; i < iM; i++ )
               {
                  pDRodp[i] = pDRhodp[i];
               }

               if ( iDrv >= EOS_DRV_T )
               {
                  for ( i = 0; i < iM; i++ )
                  {
                     pDRodt[i] = pDRhodt[i];
                  }
               }
            }
         }
      }

      /* Mass values */
      else
      {
         /* Compute the molecular weight */
         pTa = pZ;
         dA = 1.0 / m_pMolecularWeight[0];
         for ( i = 0; i < iM; i++ )
         {
            pTmp[i] = dA **pTa++;
         }

         for ( iNi = 1; iNi < m_iEosComp; iNi++ )
         {
            dA = 1.0 / m_pMolecularWeight[iNi];
            for ( i = 0; i < iM; i++ )
            {
               pTmp[i] += dA **pTa++;
            }
         }

         /* Derivatives */
         if ( iDrv )
         {
            pTa = pDZda;
            pTb = pDRhoda;
            pTc = pDRoda;
            for ( iNj = 0; iNj < m_iEosComp; iNj++ )
            {
               dA = 1.0 / m_pMolecularWeight[0];
               for ( i = 0; i < iM; i++ )
               {
                  pTc[i] = dA **pTa++;
               }

               for ( iNi = 1; iNi < m_iEosComp; iNi++ )
               {
                  dA = 1.0 / m_pMolecularWeight[iNi];
                  for ( i = 0; i < iM; i++ )
                  {
                     pTc[i] += dA **pTa++;
                  }
               }

               for ( i = 0; i < iM; i++ )
               {
                  *pTc = pTmp[i] **pTb++ +pRho[i] **pTc;
                  *pTc++;
               }
            }

            if ( iDrv >= EOS_DRV_P )
            {
               pTa = pDZdp;
               dA = 1.0 / m_pMolecularWeight[0];
               for ( i = 0; i < iM; i++ )
               {
                  pDRodp[i] = dA **pTa++;
               }

               for ( iNi = 1; iNi < m_iEosComp; iNi++ )
               {
                  dA = 1.0 / m_pMolecularWeight[iNi];
                  for ( i = 0; i < iM; i++ )
                  {
                     pDRodp[i] += dA **pTa++;
                  }
               }

               for ( i = 0; i < iM; i++ )
               {
                  pDRodp[i] = pTmp[i] * pDRhodp[i] + pRho[i] * pDRodp[i];
               }

               if ( iDrv >= EOS_DRV_T )
               {
                  pTa = pDZdt;
                  dA = 1.0 / m_pMolecularWeight[0];
                  for ( i = 0; i < iM; i++ )
                  {
                     pDRodt[i] = dA **pTa++;
                  }

                  for ( iNi = 1; iNi < m_iEosComp; iNi++ )
                  {
                     dA = 1.0 / m_pMolecularWeight[iNi];
                     for ( i = 0; i < iM; i++ )
                     {
                        pDRodt[i] += dA **pTa++;
                     }
                  }

                  for ( i = 0; i < iM; i++ )
                  {
                     pDRodt[i] = pTmp[i] * pDRhodt[i] + pRho[i] * pDRodt[i];
                  }
               }
            }
         }

         /* Form function */
         for ( i = 0; i < iM; i++ )
         {
            pRo[i] = pTmp[i] * pRho[i];
         }
      }
   }
}


/* 
// LohrenzBC
//
// Compute the Lohrenz viscosity 
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
// iMolar ** constant **
//    Indicator as to whether molar (1) or mass (0) quantities used
//    If EOS_OPTION_UNKNOWN, then molar fractions but mass density
// iNLight ** constant **
//    Number of light components
// pMWC ** constant **
//    Molecular weights
// pPCC ** constant **
//    Critical pressure
// pTCC ** constant **
//    Critical temperature
// pVCC ** constant **
//    Critical volume
// pT ** constant **
//    Temperature for each object
// pZ ** constant ** 
//    Mole or mass fraction of each object
//       first index corresponds to object
//       second index corresponds to component
// pDZdp ** constant ** 
//    Pressure derivative of mole/mass fraction of each object
//       first index corresponds to object
//       second index corresponds to component
// pDZdt ** constant ** 
//    Temperature derivative of mole/mass fraction of each object
//       first index corresponds to object
//       second index corresponds to component
// pDZda ** constant ** 
//    Accumulation derivative of mole/mass fraction of each object
//       first index corresponds to object
//       second index corresponds to component
//       third index corresponds to component derivative
// pRho ** constant ** 
//    Mole/mass density of each object
// pDRhodp ** constant **
//    Pressure derivative of mole/mass density of each object
// pDRhodt ** constant **
//    Temperature derivative of mole/mass density of each object
// pDRhoda ** constant ** 
//    Accumulation derivative of mole/mass density of each object
//       first index corresponds to object
//       second index corresponds to component
// pMu 
//    The viscosity for each object
// pDMudp
//    The pressure derivative of viscosity for each object
// pDMudt
//    The temperature derivative of viscosity for each object. 
// pDMuda
//    The accumulation derivative of viscosity for each object.
//       first index corresponds to object
//       second index corresponds to component
// pSumT
//    Beginning of work array used in the computations
//
// Yes, this routine is YUGLY!!!!
//
// 1) This routine calculates the Lohrenz-Bray-Clark 
//    viscosity.  See Lohrenz, J., Bray, B. G., and Clark, C. R.
//    (1964), Calculating Viscosities of Reservoir Fluids from
//    Their Compositions, JPT, 1171.
// 2) The pseudo reduced density is defined as
//
//    RhoR = Rho * Sum X  Vc
//                      i   i
//
// 3) Since, when using mass fractions, 
//
//    pRho = pRhomass / pMW
//
//    and
//
//    pX = pMW * pXmass  / M   
//                          i
//
//    the term becomes 
//
//    RhoR = Rhomass * Sum Xmass  Vc  / M
//                              i   i    i
//    
// 4) The following functions are then calculated
//
//    F = ( ( ( 9.3324E-3 * RhoR - 4.0758E-2 ) * RhoR + 5.8533E-2 ) *
//                          RhoR + 2.3364E-2 ) * RhoR + 1.0230E-1
//
//    G = ( F * F - 0.01 ) * ( F * F + 0.01 )
//
//
// 5) We also calculate a zeta term
//
//                        2/3             -1/6                 1/2
//    ZETA = ( Sum PC X  )    * ( Sum TC X     ) * ( Sum M X  )
//              i    i i           i    i i           i   i i
//
// 6) For mass fractions Zeta is 
//
//                                2/3                      -1/6
//    ZETA = ( Sum PC Xmass  / M )    * ( Sum TC Xmass / M )    / pMw 
//              i    i     i    i          i    i     i   i  
//
// 7) There is another temperature dependent term.  Define
//
//    Term  = 3.4E-4 * TR  ** 0.94 , TR  < 1.5 ,
//        i              i             i
//
//          = 4.6E-4 * ( TR  - 0.365 ) ** 0.625 otherwise
//                         i
//
//    and then form the term
//
//                            2/3     -1/6                   1/2
//    Upsilon = Sum Term  * PC    * TC     * M  X  / Sum X  M
//               i      i     i       i       i  i        i  i
//
// 8) For mass fractions one obtains
//
//                             2/3     -1/6                    1/2
//    Upsilon = Sum Term  * PC    * TC     Xmass  / Sum ( X / M    )
//               i      i     i       i         i    i     i   i
//
// 9) Finally we get the viscosity
//
//    pMu = ( Upsilon + G * Zeta / Sqrt(MW) ) / 5.43984
//                
// 10) The derivatives are tedious yet straightforward
*/
void EosPvtTable::LohrenzBC( int iM, int iDrv, int iMolar, int iNLight, double *pMWC, double *pPCC, double *pTCC,
                             double *pVCC, double *pT, double *pZ, double *pDZdp, double *pDZdt, double *pDZda,
                             double *pRho, double *pDRhodp, double *pDRhodt, double *pDRhoda, double *pMu,
                             double *pDMudp, double *pDMudt, double *pDMuda, double *pSumT )
{
   double *pSummT;
   double *pSumpT;
   double *pSumtT;
   double *pBotT;
   double *pSaveT;
   double *pTTermT;
   double  dSixth;
   double  dCvp;
   double  dCvm;
   double  dCvt;
   double  dA;
   double  dB;
   double  dC;
   double  dD;
   double  dE;
   double  dF;
   double  dSave;
   double  dSum;
   double  dSumm;
   double  dSump;
   double  dSumt;
   double  dTop;
   double  dBot;
   double *pTa;
   double *pTb;
   double *pTc;
   double *pTd;
   double *pTe;
   double *pTf;
   double *pTg;
   double *pDTTermT;
   double *pTopT;
   double *pSTermT;
   int     iNi;
   int     iNj;
   int     i;

   /* Save pointers */
   iNi = iNLight * iM;
   pSummT = pSumT + iM;
   pSumpT = pSummT + iM;
   pSumtT = pSumpT + iM;
   pTopT = pSumtT + iM;
   pBotT = pTopT + iM;
   pSaveT = pBotT + iM;
   pTTermT = pSaveT + iM;
   pDTTermT = pTTermT + iNi;
   pSTermT = pDTTermT + iNi;

   /* Saved terms */
   dSixth = 1.0 / 6.0;
   dCvp = 1.0 / 101325.0 / m_dConvPressure;
   dCvt = 1.0 / m_dConvTemperature;
   dCvm = 0.001 * m_dConvViscosity;

   /* Multiple grid blocks */
   if ( iM > 1 )
   {
      /* Density term */
      if ( iMolar )
      {
         pTa = pZ;
         pTb = pTTermT;
         pTc = pVCC;
         dA = *pTc++;
         for ( i = 0; i < iM; i++ )
         {
            pSaveT[i] = dA **pTa++;
            *pTb++ = dA * pRho[i];
         }

         for ( iNi = 1; iNi < iNLight; iNi++ )
         {
            dA = *pTc++;
            for ( i = 0; i < iM; i++ )
            {
               pSaveT[i] += dA **pTa++;
               *pTb++ = dA * pRho[i];
            }
         }
      }

      /* Mass fractions */
      else
      {
         pTa = pZ;
         pTb = pTTermT;
         pTc = pVCC;
         pTd = pMWC;
         dA = *pTc++ / *pTd++;
         for ( i = 0; i < iM; i++ )
         {
            pSaveT[i] = dA **pTa++;
            *pTb++ = dA * pRho[i];
         }

         for ( iNi = 1; iNi < iNLight; iNi++ )
         {
            dA = *pTc++ / *pTd++;
            for ( i = 0; i < iM; i++ )
            {
               pSaveT[i] += dA **pTa++;
               *pTb++ = dA * pRho[i];
            }
         }
      }

      /* Density correlation */
      dD = m_dVTune2 + m_dVTune2;
      dE = m_dVTune3 * 3.0;
      dF = m_dVTune4 * 4.0;
      for ( i = 0; i < iM; i++ )
      {
         dA = pSaveT[i] * pRho[i];
         dB = m_dVTune0 + dA * ( m_dVTune1 + dA * ( m_dVTune2 + dA * ( m_dVTune3 + dA * m_dVTune4 ) ) );
         dC = dB * ( m_dVTune1 + dA * ( dD + dA * ( dE + dA * dF ) ) );
         dB *= dB;
         pBotT[i] = 4.0 * dB * dC;
         pTopT[i] = dB * dB - 0.0001;
      }

      /* Composite term for mole fractions */
      if ( iMolar )
      {
         pTa = pDTTermT;
         pTb = pSTermT;
         pTf = pZ;
         pTc = pMWC;
         pTd = pPCC;
         pTe = pTCC;
         dC = *pTc++;
         dA = *pTd++ *dCvp;
         dB = *pTe++ *dCvt;
         for ( i = 0; i < iM; i++ )
         {
            dD = *pTf++;
            *pTa++ = dA;
            *pTb++ = dB;
            pSumpT[i] = dA * dD;
            pSumtT[i] = dB * dD;
            pSummT[i] = dC * dD;
         }

         for ( iNi = 1; iNi < iNLight; iNi++ )
         {
            dC = *pTc++;
            dA = *pTd++ *dCvp;
            dB = *pTe++ *dCvt;
            for ( i = 0; i < iM; i++ )
            {
               dD = *pTf++;
               *pTa++ = dA;
               *pTb++ = dB;
               pSumpT[i] += dA * dD;
               pSumtT[i] += dB * dD;
               pSummT[i] += dC * dD;
            }
         }

         /* Reset terms for chain rule */
         for ( i = 0; i < iM; i++ )
         {
            dB = pSumpT[i];
            dA = dB * dB;
            pMu[i] = dA * dA / pSumtT[i];
         }

         EosUtil::VectorPow( iM, pMu, dSixth, pMu );
         for ( i = 0; i < iM; i++ )
         {
            dC = dCvm * pMu[i] * sqrt( pSummT[i] );
            pMu[i] = pTopT[i] * dC;
            pSumT[i] = dC;
         }
      }

      /* Composite term for mass fractions */
      else
      {
         pTa = pDTTermT;
         pTb = pSTermT;
         pTf = pZ;
         pTc = pMWC;
         pTd = pPCC;
         pTe = pTCC;
         dC = 1.0 / *pTc++;
         dA = *pTd++ *dCvp * dC;
         dB = *pTe++ *dCvt * dC;
         for ( i = 0; i < iM; i++ )
         {
            dD = *pTf++;
            *pTa++ = dA;
            *pTb++ = dB;
            pSumpT[i] = dA * dD;
            pSumtT[i] = dB * dD;
            pSummT[i] = dD * dC;
         }

         for ( iNi = 1; iNi < iNLight; iNi++ )
         {
            dC = 1.0 / *pTc++;
            dA = *pTd++ *dCvp * dC;
            dB = *pTe++ *dCvt * dC;
            for ( i = 0; i < iM; i++ )
            {
               dD = *pTf++;
               *pTa++ = dA;
               *pTb++ = dB;
               pSumpT[i] += dA * dD;
               pSumtT[i] += dB * dD;
               pSummT[i] += dD * dC;
            }
         }

         /* Reset terms for chain rule */
         for ( i = 0; i < iM; i++ )
         {
            dB = pSumpT[i];
            dA = dB * dB;
            pMu[i] = dA * dA / pSumtT[i];
         }

         EosUtil::VectorPow( iM, pMu, dSixth, pMu );
         for ( i = 0; i < iM; i++ )
         {
            dC = dCvm * pMu[i] / pSummT[i];
            pMu[i] = pTopT[i] * dC;
            pSumT[i] = dC;
         }
      }

      /* Derivatives */
      if ( iDrv )
      {
         /* Add together the terms for mole fractions */
         if ( iMolar )
         {
            for ( i = 0; i < iM; i++ )
            {
               dA = pMu[i];
               dB = pSumT[i] * pBotT[i];
               pSumT[i] = dB;
               pSaveT[i] *= dB;
               pSumpT[i] = ( dA + dA ) / pSumpT[i] / 3.0;
               pSumtT[i] = dA / pSumtT[i] / 6.0;
               pSummT[i] = dA / pSummT[i] / 2.0;
            }

            pTa = pDTTermT;
            pTb = pSTermT;
            pTc = pTTermT;
            pTd = pMWC;
            for ( iNi = 0; iNi < iNLight; iNi++ )
            {
               dA = *pTd++;
               for ( i = 0; i < iM; i++ )
               {
                  *pTc = pSumpT[i] **pTa++ -pSumtT[i] **pTb++ +pSummT[i] * dA + pSumT[i] **pTc;
                  pTc++;
               }
            }
         }

         /* Add together the terms for mass fractions */
         else
         {
            for ( i = 0; i < iM; i++ )
            {
               dA = pMu[i];
               dB = pSumT[i] * pBotT[i];
               pSumT[i] = dB;
               pSaveT[i] *= dB;
               pSumpT[i] = ( dA + dA ) / pSumpT[i] / 3.0;
               pSumtT[i] = dA / pSumtT[i] / 6.0;
               pSummT[i] = dA / pSummT[i];
            }

            pTa = pDTTermT;
            pTb = pSTermT;
            pTc = pTTermT;
            pTd = pMWC;
            for ( iNi = 0; iNi < iNLight; iNi++ )
            {
               dA = *pTd++;
#ifdef IPFtune
#pragma ivdep
#endif
               for ( i = 0; i < iM; i++ )
               {
                  *pTc = pSumpT[i] **pTa++ -pSumtT[i] **pTb++ -pSummT[i] / dA + pSumT[i] **pTc;
                  pTc++;
               }
            }
         }

         /* Density term pressure derivative */
         if ( iDrv >= EOS_DRV_P )
         {
            for ( i = 0; i < iM; i++ )
            {
               pDMudp[i] = pSaveT[i] * pDRhodp[i];
            }

            pTa = pTTermT;
            pTb = pDZdp;
            for ( iNi = 0; iNi < iNLight; iNi++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  pDMudp[i] += *pTa++ **pTb++;
               }
            }

            /* Density term temperature derivative */
            if ( iDrv >= EOS_DRV_T )
            {
               for ( i = 0; i < iM; i++ )
               {
                  pDMudt[i] = pSaveT[i] * pDRhodt[i];
               }

               pTa = pTTermT;
               pTb = pDZdt;
               for ( iNi = 0; iNi < iNLight; iNi++ )
               {
                  for ( i = 0; i < iM; i++ )
                  {
                     pDMudt[i] += *pTa++ **pTb++;
                  }
               }
            }
         }

         /* Density term composition derivative */
         pTa = pDMuda;
         pTb = pDRhoda;
         pTc = pDZda;
         for ( iNj = 0; iNj < m_iEosComp; iNj++ )
         {
            pTd = pTTermT;
            for ( i = 0; i < iM; i++ )
            {
               pTopT[i] = *pTd++ **pTc++;
            }

            for ( iNi = 1; iNi < iNLight; iNi++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  pTopT[i] += *pTd++ **pTc++;
               }
            }

            for ( i = 0; i < iM; i++ )
            {
               *pTa++ = pTopT[i] + pSaveT[i] **pTb++;
            }
         }
      }

      /* Compute the temperature dependent terms for mole fractions */
      if ( iMolar )
      {
         pTa = pPCC;
         pTb = pTCC;
         pTc = pMWC;
         pTd = pZ;
         pTe = pDTTermT;
         pTf = pTTermT;
         pTg = pSTermT;
         dA = *pTa++ *dCvp;
         dE = *pTb++;
         dD = *pTc++;
         dA *= dA;
         dF = dCvm * pow( dA * dA / dE / dCvt, dSixth ) * dD;
         dC = sqrt( dD );
         for ( i = 0; i < iM; i++ )
         {
            dA = pT[i] / dE;
            pTe[i] = dA;
            if ( dA < 1.5 )
            {
               pTf[i] = dA;
               pTg[i] = 0.94;
            }
            else
            {
               pTf[i] = dA - 0.365;
               pTg[i] = 0.625;
            }
         }

         EosUtil::VectorPowX( iM, pTf, pTg, pTg );
         for ( i = 0; i < iM; i++ )
         {
            dB = ( *pTe++ < 1.5 ? 0.00034 : 0.00046 ) **pTg * dF;
            dD = *pTd++;
            *pTf++ = dB;
            *pTg++ = dC;
            pTopT[i] = dB * dD;
            pSaveT[i] = dC * dD;
         }

         for ( iNi = 1; iNi < iNLight; iNi++ )
         {
            dA = *pTa++ *dCvp;
            dE = *pTb++;
            dD = *pTc++;
            dA *= dA;
            dF = dCvm * pow( dA * dA / dE / dCvt, dSixth ) * dD;
            dC = sqrt( dD );
            for ( i = 0; i < iM; i++ )
            {
               dA = pT[i] / dE;
               pTe[i] = dA;
               if ( dA < 1.5 )
               {
                  pTf[i] = dA;
                  pTg[i] = 0.94;
               }
               else
               {
                  pTf[i] = dA - 0.365;
                  pTg[i] = 0.625;
               }
            }

            EosUtil::VectorPowX( iM, pTf, pTg, pTg );
            for ( i = 0; i < iM; i++ )
            {
               dB = ( *pTe++ < 1.5 ? 0.00034 : 0.00046 ) **pTg * dF;
               dD = *pTd++;
               *pTf++ = dB;
               *pTg++ = dC;
               pTopT[i] += dB * dD;
               pSaveT[i] += dC * dD;
            }
         }
      }

      /* Compute the temperature dependent terms for mass fractions */
      else
      {
         pTa = pPCC;
         pTb = pTCC;
         pTc = pMWC;
         pTd = pZ;
         pTe = pDTTermT;
         pTf = pTTermT;
         pTg = pSTermT;
         dA = *pTa++ *dCvp;
         dA *= dA;
         dE = *pTb++;
         dF = dCvm * pow( dA * dA / dE / dCvt, dSixth );
         dC = 1.0 / sqrt( *pTc++ );
         for ( i = 0; i < iM; i++ )
         {
            dA = pT[i] / dE;
            pTe[i] = dA;
            if ( dA < 1.5 )
            {
               pTf[i] = dA;
               pTg[i] = 0.94;
            }
            else
            {
               pTf[i] = dA - 0.365;
               pTg[i] = 0.625;
            }
         }

         EosUtil::VectorPowX( iM, pTf, pTg, pTg );
         for ( i = 0; i < iM; i++ )
         {
            dB = ( *pTe++ < 1.5 ? 0.00034 : 0.00046 ) **pTg * dF;
            dD = *pTd++;
            *pTf++ = dB;
            *pTg++ = dC;
            pTopT[i] = dB * dD;
            pSaveT[i] = dC * dD;
         }

         for ( iNi = 1; iNi < iNLight; iNi++ )
         {
            dA = *pTa++ *dCvp;
            dA *= dA;
            dE = *pTb++;
            dF = dCvm * pow( dA * dA / dE / dCvt, dSixth );
            dC = 1.0 / sqrt( *pTc++ );
            for ( i = 0; i < iM; i++ )
            {
               dA = pT[i] / dE;
               pTe[i] = dA;
               if ( dA < 1.5 )
               {
                  pTf[i] = dA;
                  pTg[i] = 0.94;
               }
               else
               {
                  pTf[i] = dA - 0.365;
                  pTg[i] = 0.625;
               }
            }

            EosUtil::VectorPowX( iM, pTf, pTg, pTg );
            for ( i = 0; i < iM; i++ )
            {
               dB = ( *pTe++ < 1.5 ? 0.00034 : 0.00046 ) **pTg * dF;
               dD = *pTd++;
               *pTf++ = dB;
               *pTg++ = dC;
               pTopT[i] += dB * dD;
               pSaveT[i] += dC * dD;
            }
         }
      }

      /* Set the viscosity */
      for ( i = 0; i < iM; i++ )
      {
         pSaveT[i] = 1.0 / pSaveT[i];
         pSumT[i] = pTopT[i] * pSaveT[i];
         pMu[i] += pSumT[i];
      }

      /* Derivatives */
      if ( iDrv )
      {
         /* Pressure derivatives */
         if ( iDrv >= EOS_DRV_P )
         {
            pTa = pDZdp;
            pTb = pTTermT;
            pTc = pSTermT;
            for ( i = 0; i < iM; i++ )
            {
               dA = *pTa++;
               pTopT[i] = *pTb++ *dA;
               pBotT[i] = *pTc++ *dA;
            }

            for ( iNi = 1; iNi < iNLight; iNi++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  dA = *pTa++;
                  pTopT[i] += *pTb++ *dA;
                  pBotT[i] += *pTc++ *dA;
               }
            }

            for ( i = 0; i < iM; i++ )
            {
               pDMudp[i] += ( pTopT[i] - pSumT[i] * pBotT[i] ) * pSaveT[i];
            }

            /* Temperature derivatives */
            if ( iDrv >= EOS_DRV_T )
            {
               pTa = pTCC;
               pTb = pDTTermT;
               pTc = pTTermT;
               pTd = pDZdt;
               pTe = pZ;
               pTf = pSTermT;
               dE = 1.0 / *pTa++;
               for ( i = 0; i < iM; i++ )
               {
                  dB = *pTb++;
                  dC = *pTc++;
                  dD = *pTd++;
                  dA = ( dB < 1.5 ? 0.94 * dC / dB : 0.625 * dC / ( dB - 0.365 ) );
                  pTopT[i] = dC * dD + dA **pTe++ *dE;
                  pBotT[i] = *pTf++ *dD;
               }

               for ( iNi = 1; iNi < iNLight; iNi++ )
               {
                  dE = 1.0 / *pTa++;
                  for ( i = 0; i < iM; i++ )
                  {
                     dB = *pTb++;
                     dC = *pTc++;
                     dD = *pTd++;
                     dA = ( dB < 1.5 ? 0.94 * dC / dB : 0.625 * dC / ( dB - 0.365 ) );
                     pTopT[i] += dC * dD + dA **pTe++ *dE;
                     pBotT[i] += *pTf++ *dD;
                  }
               }

               for ( i = 0; i < iM; i++ )
               {
                  pDMudt[i] += ( pTopT[i] - pSumT[i] * pBotT[i] ) * pSaveT[i];
               }
            }
         }

         /* Composition derivatives */
         pTa = pDMuda;
         pTb = pDZda;
         for ( iNj = 0; iNj < m_iEosComp; iNj++ )
         {
            pTc = pTTermT;
            pTd = pSTermT;
            for ( i = 0; i < iM; i++ )
            {
               dA = *pTb++;
               pTopT[i] = *pTc++ *dA;
               pBotT[i] = *pTd++ *dA;
            }

            for ( iNi = 1; iNi < iNLight; iNi++ )
            {
#ifdef IPFtune
#pragma ivdep
#endif
               for ( i = 0; i < iM; i++ )
               {
                  dA = *pTb++;
                  pTopT[i] += *pTc++ *dA;
                  pBotT[i] += *pTd++ *dA;
               }
            }

#ifdef IPFtune
#pragma ivdep
#endif
            for ( i = 0; i < iM; i++ )
            {
               *pTa += ( pTopT[i] - pSumT[i] * pBotT[i] ) * pSaveT[i];
               pTa++;
            }
         }
      }
   }

   /* Single grid block */
   else
   {
      /* Density term */
      if ( iMolar )
      {
         pTa = pZ;
         pTb = pTTermT;
         pTc = pVCC;
         dA = *pTc++;
         dB = *pRho;
         dSave = dA **pTa++;
         *pTb++ = dA * dB;
         for ( iNi = 1; iNi < iNLight; iNi++ )
         {
            dA = *pTc++;
            dSave += dA **pTa++;
            *pTb++ = dA * dB;
         }
      }

      /* Mass fractions */
      else
      {
         pTa = pZ;
         pTb = pTTermT;
         pTc = pVCC;
         pTd = pMWC;
         dA = *pTc++ / *pTd++;
         dB = *pRho;
         dSave = dA **pTa++;
         *pTb++ = dA * dB;
         for ( iNi = 1; iNi < iNLight; iNi++ )
         {
            dA = *pTc++ / *pTd++;
            dSave += dA **pTa++;
            *pTb++ = dA * dB;
         }
      }

      /* Density correlation */
      dA = dSave **pRho;
      dB = m_dVTune0 + dA * ( m_dVTune1 + dA * ( m_dVTune2 + dA * ( m_dVTune3 + dA * m_dVTune4 ) ) );
      dC = dB * ( m_dVTune1 + dA * ( 2.0 * m_dVTune2 + dA * ( 3.0 * m_dVTune3 + dA * 4.0 * m_dVTune4 ) ) );
      dB *= dB;
      dBot = 4.0 * dB * dC;
      dTop = dB * dB - 0.0001;

      /* Composite term for mole fractions */
      if ( iMolar )
      {
         pTa = pDTTermT;
         pTb = pSTermT;
         pTf = pZ;
         pTc = pMWC;
         pTd = pPCC;
         pTe = pTCC;
         dC = *pTc++;
         dA = *pTd++ *dCvp;
         dB = *pTe++ *dCvt;
         dD = *pTf++;
         *pTa++ = dA;
         *pTb++ = dB;
         dSump = dA * dD;
         dSumt = dB * dD;
         dSumm = dC * dD;
         for ( iNi = 1; iNi < iNLight; iNi++ )
         {
            dC = *pTc++;
            dA = *pTd++ *dCvp;
            dB = *pTe++ *dCvt;
            dD = *pTf++;
            *pTa++ = dA;
            *pTb++ = dB;
            dSump += dA * dD;
            dSumt += dB * dD;
            dSumm += dC * dD;
         }

         /* Reset terms for chain rule */
         dA = dSump * dSump;
         dC = dCvm * pow( dA * dA / dSumt, dSixth ) * sqrt( dSumm );
         *pMu = dTop * dC;
         dSum = dC;
      }

      /* Composite term for mass fractions */
      else
      {
         pTa = pDTTermT;
         pTb = pSTermT;
         pTf = pZ;
         pTc = pMWC;
         pTd = pPCC;
         pTe = pTCC;
         dC = *pTc++;
         dA = *pTd++ *dCvp / dC;
         dB = *pTe++ *dCvt / dC;
         dD = *pTf++;
         *pTa++ = dA;
         *pTb++ = dB;
         dSump = dA * dD;
         dSumt = dB * dD;
         dSumm = dD / dC;
         for ( iNi = 1; iNi < iNLight; iNi++ )
         {
            dC = *pTc++;
            dA = *pTd++ *dCvp / dC;
            dB = *pTe++ *dCvt / dC;
            dD = *pTf++;
            *pTa++ = dA;
            *pTb++ = dB;
            dSump += dA * dD;
            dSumt += dB * dD;
            dSumm += dD / dC;
         }

         /* Reset terms for chain rule */
         dA = dSump * dSump;
         dC = dCvm * pow( dA * dA / dSumt, dSixth ) / dSumm;
         *pMu = dTop * dC;
         dSum = dC;
      }

      /* Derivatives */
      if ( iDrv )
      {
         /* Add together the terms for mole fractions */
         if ( iMolar )
         {
            dA = *pMu;
            dB = dSum * dBot;
            dSum = dB;
            dSave *= dB;
            dSump = ( dA + dA ) / dSump / 3.0;
            dSumt = dA / dSumt / 6.0;
            dSumm = dA / dSumm / 2.0;
            pTa = pDTTermT;
            pTb = pSTermT;
            pTc = pTTermT;
            pTd = pMWC;
            for ( iNi = 0; iNi < iNLight; iNi++ )
            {
               *pTc = dSump **pTa++ -dSumt **pTb++ +dSumm **pTd++ +dSum **pTc;
               pTc++;
            }
         }

         /* Add together the terms for mass fractions */
         else
         {
            dA = *pMu;
            dB = dSum * dBot;
            dSum = dB;
            dSave *= dB;
            dSump = ( dA + dA ) / dSump / 3.0;
            dSumt = dA / dSumt / 6.0;
            dSumm = dA / dSumm;
            pTa = pDTTermT;
            pTb = pSTermT;
            pTc = pTTermT;
            pTd = pMWC;
            for ( iNi = 0; iNi < iNLight; iNi++ )
            {
               *pTc = dSump **pTa++ -dSumt **pTb++ -dSumm / *pTd++ +dSum **pTc;
               pTc++;
            }
         }

         /* Density term pressure derivative */
         if ( iDrv >= EOS_DRV_P )
         {
            dA = dSave **pDRhodp;
            pTa = pTTermT;
            pTb = pDZdp;
            for ( iNi = 0; iNi < iNLight; iNi++ )
            {
               dA += *pTa++ **pTb++;
            }

            *pDMudp = dA;

            /* Density term temperature derivative */
            if ( iDrv >= EOS_DRV_T )
            {
               dA = dSave **pDRhodt;
               pTa = pTTermT;
               pTb = pDZdt;
               for ( iNi = 0; iNi < iNLight; iNi++ )
               {
                  dA += *pTa++ **pTb++;
               }

               *pDMudt = dA;
            }
         }

         /* Density term composition derivative */
         pTa = pDMuda;
         pTb = pDRhoda;
         pTc = pDZda;
         for ( iNj = 0; iNj < m_iEosComp; iNj++ )
         {
            pTd = pTTermT;
            dA = *pTd++ **pTc++;
            for ( iNi = 1; iNi < iNLight; iNi++ )
            {
               dA += *pTd++ **pTc++;
            }

            *pTa++ = dA + dSave **pTb++;
         }
      }

      /* Compute the temperature dependent terms for mole fractions */
      if ( iMolar )
      {
         dBot = *pT;
         pTa = pPCC;
         pTb = pTCC;
         pTc = pMWC;
         pTd = pZ;
         pTe = pDTTermT;
         pTf = pTTermT;
         pTg = pSTermT;
         dA = *pTa++ *dCvp;
         dE = *pTb++;
         dD = *pTc++;
         dA *= dA;
         dF = dCvm * pow( dA * dA / dE / dCvt, dSixth ) * dD;
         dC = sqrt( dD );
         dA = dBot / dE;
         dB = ( dA < 1.5 ? 0.00034 * pow( dA, 0.94 ) : 0.00046 * pow( dA - 0.365, 0.625 ) ) * dF;
         dD = *pTd++;
         *pTe++ = dA;
         *pTf++ = dB;
         *pTg++ = dC;
         dTop = dB * dD;
         dSave = dC * dD;
         for ( iNi = 1; iNi < iNLight; iNi++ )
         {
            dA = *pTa++ *dCvp;
            dE = *pTb++;
            dD = *pTc++;
            dA *= dA;
            dF = dCvm * pow( dA * dA / dE / dCvt, dSixth ) * dD;
            dC = sqrt( dD );
            dA = dBot / dE;
            dB = ( dA < 1.5 ? 0.00034 * pow( dA, 0.94 ) : 0.00046 * pow( dA - 0.365, 0.625 ) ) * dF;
            dD = *pTd++;
            *pTe++ = dA;
            *pTf++ = dB;
            *pTg++ = dC;
            dTop += dB * dD;
            dSave += dC * dD;
         }
      }

      /* Compute the temperature dependent terms for mass fractions */
      else
      {
         dBot = *pT;
         pTa = pPCC;
         pTb = pTCC;
         pTc = pMWC;
         pTd = pZ;
         pTe = pDTTermT;
         pTf = pTTermT;
         pTg = pSTermT;
         dA = *pTa++ *dCvp;
         dA *= dA;
         dE = *pTb++;
         dF = dCvm * pow( dA * dA / dE / dCvt, dSixth );
         dC = 1.0 / sqrt( *pTc++ );
         dA = dBot / dE;
         dB = ( dA < 1.5 ? 0.00034 * pow( dA, 0.94 ) : 0.00046 * pow( dA - 0.365, 0.625 ) ) * dF;
         dD = *pTd++;
         *pTe++ = dA;
         *pTf++ = dB;
         *pTg++ = dC;
         dTop = dB * dD;
         dSave = dC * dD;
         for ( iNi = 1; iNi < iNLight; iNi++ )
         {
            dA = *pTa++ *dCvp;
            dA *= dA;
            dE = *pTb++;
            dF = dCvm * pow( dA * dA / dE / dCvt, dSixth );
            dC = 1.0 / sqrt( *pTc++ );
            dA = dBot / dE;
            dB = ( dA < 1.5 ? 0.00034 * pow( dA, 0.94 ) : 0.00046 * pow( dA - 0.365, 0.625 ) ) * dF;
            dD = *pTd++;
            *pTe++ = dA;
            *pTf++ = dB;
            *pTg++ = dC;
            dTop += dB * dD;
            dSave += dC * dD;
         }
      }

      /* Set the viscosity */
      dSum = dTop / dSave;
      *pMu += dSum;

      /* Derivatives */
      if ( iDrv )
      {
         /* Pressure derivatives */
         if ( iDrv >= EOS_DRV_P )
         {
            pTa = pDZdp;
            pTb = pTTermT;
            pTc = pSTermT;
            dA = *pTa++;
            dTop = *pTb++ *dA;
            dBot = *pTc++ *dA;
            for ( iNi = 1; iNi < iNLight; iNi++ )
            {
               dA = *pTa++;
               dTop += *pTb++ *dA;
               dBot += *pTc++ *dA;
            }

            *pDMudp += ( dTop - dSum * dBot ) / dSave;

            /* Temperature derivatives */
            if ( iDrv >= EOS_DRV_T )
            {
               pTa = pTCC;
               pTb = pDTTermT;
               pTc = pTTermT;
               pTd = pDZdt;
               pTe = pZ;
               pTf = pSTermT;
               dE = *pTa++;
               dB = *pTb++;
               dC = *pTc++;
               dD = *pTd++;
               dA = ( dB < 1.5 ? 0.94 * dC / dB : 0.625 * dC / ( dB - 0.365 ) );
               dTop = dC * dD + dA **pTe++ / dE;
               dBot = *pTf++ *dD;
               for ( iNi = 1; iNi < iNLight; iNi++ )
               {
                  dE = *pTa++;
                  dB = *pTb++;
                  dC = *pTc++;
                  dD = *pTd++;
                  dA = ( dB < 1.5 ? 0.94 * dC / dB : 0.625 * dC / ( dB - 0.365 ) );
                  dTop += dC * dD + dA **pTe++ / dE;
                  dBot += *pTf++ *dD;
               }

               *pDMudt += ( dTop - dSum * dBot ) / dSave;
            }
         }

         /* Composition derivatives */
         pTa = pDMuda;
         pTb = pDZda;
         for ( iNj = 0; iNj < m_iEosComp; iNj++ )
         {
            pTc = pTTermT;
            pTd = pSTermT;
            dA = *pTb++;
            dTop = *pTc++ *dA;
            dBot = *pTd++ *dA;
            for ( iNi = 1; iNi < iNLight; iNi++ )
            {
               dA = *pTb++;
               dTop += *pTc++ *dA;
               dBot += *pTd++ *dA;
            }

            *pTa += ( dTop - dSum * dBot ) / dSave;
            pTa++;
         }
      }
   }
}


/* 
// MassDensity
//
// Routine to convert density to mass density 
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
// pDXdp ** constant ** 
//    Pressure derivative of mole fraction of each object
//       first index corresponds to object
//       second index corresponds to component
// pDXdt ** constant ** 
//    Temperature derivative of mole fraction of each object
//       first index corresponds to object
//       second index corresponds to component
// pDXda ** constant ** 
//    Accumulation derivative of mole fraction of each object
//       first index corresponds to object
//       second index corresponds to component
//       third index corresponds to component derivative
// pRho 
//    On input the molar density of each object
//    On output the mass density
// pDRhodp 
//    On input pressure derivative of molar density of each object
//    On output pressure derivative of mass density of each object
// pDRhodt 
//    On input temperature derivative of molar volume of each object
//    On output temperature derivative of mass density of each object
// pDRhoda 
//    On input overall mole derivative of molar volume of each object
//    On output overall mole derivative of mass density of each object
//       first index corresponds to object
//       second index corresponds to component
// pMw ** constant **   
//    Phase molecular weight 
// pWorkT  
//    Work array
//
// 1) This routine is invoked after all computations (including
//    derivatives) are finished.  Even if derivatives with 
//    respect to moles rather than mass are required, we
//    sometimes would like molar densities.  Thus this routine
// 2) Divide the density by the molecular weight.  The derivatives
//    must make use of the composition derivatives since
//    we are no longer in the realm of pure mole numbers, as
//    in other routines.
*/
void EosPvtTable::MassDensity( int iM, int iDrv, double *pDXdp, double *pDXdt, double *pDXda, double *pRho,
                               double *pDRhodp, double *pDRhodt, double *pDRhoda, double *pMw, double *pWorkT )
{
   double *pTa;
   double *pTb;
   double  dA;
   double  dB;
   double  dC;
   int     iNi;
   int     iNj;
   int     i;

   /* Reset the derivatives for multiple grid blocks */
   if ( iM > 1 )
   {
      if ( iDrv )
      {
         pTa = pDXda;
         pTb = pDRhoda;
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            dA = m_pMolecularWeight[0];
            for ( i = 0; i < iM; i++ )
            {
               pWorkT[i] = *pTa++ *dA;
            }

            for ( iNj = 1; iNj < m_iEosComp; iNj++ )
            {
               dA = m_pMolecularWeight[iNj];
               for ( i = 0; i < iM; i++ )
               {
                  pWorkT[i] += *pTa++ *dA;
               }
            }

            for ( i = 0; i < iM; i++ )
            {
               *pTb = *pTb * pMw[i] + pRho[i] * pWorkT[i];
               pTb++;
            }
         }

         /* Pressure derivatives */
         if ( iDrv >= EOS_DRV_P )
         {
            pTb = pDXdp;
            dA = m_pMolecularWeight[0];
            for ( i = 0; i < iM; i++ )
            {
               pWorkT[i] = *pTb++ *dA;
            }

            for ( iNi = 1; iNi < m_iEosComp; iNi++ )
            {
               dA = m_pMolecularWeight[iNi];
               for ( i = 0; i < iM; i++ )
               {
                  pWorkT[i] += *pTb++ *dA;
               }
            }

            for ( i = 0; i < iM; i++ )
            {
               pDRhodp[i] = pDRhodp[i] * pMw[i] + pRho[i] * pWorkT[i];
            }

            /* Temperature derivatives */
            if ( iDrv >= EOS_DRV_T )
            {
               pTb = pDXdt;
               dA = m_pMolecularWeight[0];
               for ( i = 0; i < iM; i++ )
               {
                  pWorkT[i] = *pTb++ *dA;
               }

               for ( iNi = 1; iNi < m_iEosComp; iNi++ )
               {
                  dA = m_pMolecularWeight[iNi];
                  for ( i = 0; i < iM; i++ )
                  {
                     pWorkT[i] += *pTb++ *dA;
                  }
               }

               for ( i = 0; i < iM; i++ )
               {
                  pDRhodt[i] = pDRhodt[i] * pMw[i] + pRho[i] * pWorkT[i];
               }
            }
         }
      }

      /* Set the function */
      for ( i = 0; i < iM; i++ )
      {
         pRho[i] *= pMw[i];
      }
   }

   /* Get molecular weight for single grid block */
   else
   {
      dC = *pRho;
      dA = *pMw;

      /* Reset the derivatives */
      if ( iDrv )
      {
         pTa = pDXda;
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            dB = m_pMolecularWeight[0] **pTa++;
            for ( iNj = 1; iNj < m_iEosComp; iNj++ )
            {
               dB += m_pMolecularWeight[iNj] **pTa++;
            }

            pDRhoda[iNi] = pDRhoda[iNi] * dA + dC * dB;
         }

         /* Pressure derivatives */
         if ( iDrv >= EOS_DRV_P )
         {
            dB = m_pMolecularWeight[0] * pDXdp[0];
            for ( iNi = 1; iNi < m_iEosComp; iNi++ )
            {
               dB += m_pMolecularWeight[iNi] * pDXdp[iNi];
            }

            *pDRhodp = *pDRhodp * dA + dC * dB;

            /* Temperature derivatives */
            if ( iDrv >= EOS_DRV_T )
            {
               dB = m_pMolecularWeight[0] * pDXdt[0];
               for ( iNi = 1; iNi < m_iEosComp; iNi++ )
               {
                  dB += m_pMolecularWeight[iNi] * pDXdt[iNi];
               }

               *pDRhodt = *pDRhodt * dA + dC * dB;
            }
         }
      }

      /* Set the function */
      *pRho *= dA;
   }
}


/* 
// MassHeat
//
// Routine to modify heat terms for mass 
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
// pMW ** constant **   
//    Phase molecular weight 
// pHeat 
//    On input the enthalpy per unit mole of each object
//    On output the enthalpy per unit mass
// pDHeatdp 
//    On input pressure derivative of the enthalpy per unit mole
//    of each object.  On output pressure derivative of enthalpy
//    per unit mass of each object
// pDHeatdt 
//    On input temperature derivative of the enthalpy per unit mole
//    of each object.  On output temperature derivative of enthalpy
//    per unit mass of each object
// pDHeatda 
//    On input accumulation derivative of the enthalpy per unit mole
//    of each object.  On output accumulation derivative of enthalpy
//    per unit mass of each object
//       first index corresponds to object
//       second index corresponds to component
//
// 1) Divide the enthalpy by the molecular weight
*/
void EosPvtTable::MassHeat( int iM, int iDrv, double *pMW, double *pHeat, double *pDHeatdp, double *pDHeatdt,
                            double *pDHeatda )
{
   double *pTa;
   double  dA;
   double  dB;
   int     iNi;
   int     i;

   /* Modify heats multiple blocks */
   if ( iM > 1 )
   {
      for ( i = 0; i < iM; i++ )
      {
         pHeat[i] = pHeat[i] / pMW[i];
      }

      if ( iDrv )
      {
         pTa = pDHeatda;
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            dA = m_pMolecularWeight[iNi];
            for ( i = 0; i < iM; i++ )
            {
               *pTa = ( *pTa + pHeat[i] * ( pMW[i] - dA ) ) / pMW[i];
               pTa++;
            }
         }

         if ( iDrv >= EOS_DRV_P )
         {
            for ( i = 0; i < iM; i++ )
            {
               pDHeatdp[i] = pDHeatdp[i] / pMW[i];
            }

            if ( iDrv >= EOS_DRV_T )
            {
               for ( i = 0; i < iM; i++ )
               {
                  pDHeatdt[i] = pDHeatdt[i] / pMW[i];
               }
            }
         }
      }
   }

   /* Modify heat single block */
   else
   {
      dA = *pMW;
      dB = *pHeat / dA;
      *pHeat = dB;
      if ( iDrv )
      {
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            pDHeatda[iNi] = ( pDHeatda[iNi] + dB * ( dA - m_pMolecularWeight[iNi] ) ) / dA;
         }

         if ( iDrv >= EOS_DRV_P )
         {
            *pDHeatdp = *pDHeatdp / dA;
            if ( iDrv >= EOS_DRV_T )
            {
               *pDHeatdt = *pDHeatdt / dA;
            }
         }
      }
   }
}


/* 
// Mobility ratio
//
// Compute the mobility ratio of heavy and light viscosities
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
// pML 
//    The light viscosity for each object
// pDMLdp
//    The pressure derivative of light viscosity for each object
// pDMLdt
//    The temperature derivative of light viscosity for each object. 
// pDMLda
//    The accumulation derivative of light viscosity for each object.
//       first index corresponds to object
//       second index corresponds to component
// pMH 
//    The heavy viscosity for each object
// pDMHdp
//    The pressure derivative of heavy viscosity for each object
// pDMHdt
//    The temperature derivative of heavy viscosity for each object. 
// pDMHda
//    The accumulation derivative of heavy viscosity for each object.
//       first index corresponds to object
//       second index corresponds to component
// pMR 
//    The mobility ratio for each object
// pDMRdp
//    The pressure derivative of mobility ratio for each object
// pDMRdt
//    The temperature derivative of mobility ratio for each object. 
// pDMRda
//    The accumulation derivative of mobility ratio for each object.
//       first index corresponds to object
//       second index corresponds to component
// pTmp
//    Beginning of work array used in the computations
//
// 1) The mobility ratio is simply the heavy viscosity divided
//    by the light viscosity
// 2) Take derivatives if needed....
*/
void EosPvtTable::MobilityRatio( int iM, int iDrv, double *pML, double *pDMLdp, double *pDMLdt, double *pDMLda,
                                 double *pMH, double *pDMHdp, double *pDMHdt, double *pDMHda, double *pMR,
                                 double *pDMRdp, double *pDMRdt, double *pDMRda, double *pTmp )
{
   double  dA;
   double *pTa;
   double *pTb;
   double *pTc;
   int     i;
   int     iNi;

   /* Derivatives */
   if ( iDrv )
   {
      /* Form mobility ratio */
      for ( i = 0; i < iM; i++ )
      {
         dA = 1.0 / pML[i];
         pMR[i] = dA * pMH[i];
         pTmp[i] = dA;
      }

      /* Composition derivatives */
      pTa = pDMHda;
      pTb = pDMLda;
      pTc = pDMRda;
      for ( iNi = 0; iNi < m_iEosComp; iNi++ )
      {
         for ( i = 0; i < iM; i++ )
         {
            *pTc++ = pTmp[i] * ( *pTa++ -pMR[i] **pTb++ );
         }
      }

      /* Pressure derivatives */
      if ( iDrv >= EOS_DRV_P )
      {
         for ( i = 0; i < iM; i++ )
         {
            pDMRdp[i] = pTmp[i] * ( pDMHdp[i] - pMR[i] * pDMLdp[i] );
         }

         /* Temperature derivatives */
         if ( iDrv >= EOS_DRV_T )
         {
            for ( i = 0; i < iM; i++ )
            {
               pDMRdt[i] = pTmp[i] * ( pDMHdt[i] - pMR[i] * pDMLdt[i] );
            }
         }
      }
   }

   /* No derivatives just form mobility ratio */
   else
   {
      for ( i = 0; i < iM; i++ )
      {
         pMR[i] = pMH[i] / pML[i];
      }
   }
}


/* 
// SetTension
//
// Subroutine to compute the two phase interfacial tension 
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
// pIfx 
//    On input the parachor times density for x phase of each object
//    On output the two phase interfacial tension for each object
// pDIfxdp
//    On input the pressure derivative parachor times density for x 
//    phase of each object.  On output the pressure derivative of
//    two phase interfacial tension for each object
// pDIfxdt
//    On input the temperature derivative parachor times density for x 
//    phase of each object.  On output the temperature derivative of
//    two phase interfacial tension for each object
// pDIfxda
//    On input the accumulation derivative parachor times density for 
//    x phase of each object.  On output the accumulation derivative 
//    of two phase interfacial tension for each object
//       first index corresponds to object
//       second index corresponds to component
// pIfy 
//    On input the parachor times density for y phase of each object
//    On output the two phase interfacial tension for each object
// pDIfydp
//    On input the pressure derivative parachor times density for y 
//    phase of each object.  On output the pressure derivative of
//    two phase interfacial tension for each object
// pDIfydt
//    On input the temperature derivative parachor times density for y 
//    phase of each object.  On output the temperature derivative of
//    two phase interfacial tension for each object
// pDIfyda
//    On input the accumulation derivative parachor times density for 
//    y phase of each object.  On output the accumulation derivative 
//    of two phase interfacial tension for each object
//       first index corresponds to object
//       second index corresponds to component
// pWorkT
//    Work vector
//
// 1) Given the two single phase tension terms the interfacial
//    tension is simply the fourth power of their difference
// 2) Conversion constant needed to convert from MKS to
//    local units
// 3) Note that on output the interfacial tension for both
//    phases set to identical values
*/
void EosPvtTable::SetTension( int iM, int iDrv, double *pIftx, double *pDIftxdp, double *pDIftxdt, double *pDIftxda,
                              double *pIfty, double *pDIftydp, double *pDIftydt, double *pDIftyda, double *pWorkT )
{
   double *pTa;
   double *pTb;
   double  dA;
   double  dB;
   double  dC;
   int     iNi;
   int     i;

   /* More than one item with derivatives */
   if ( iM > 1 )
   {
      if ( iDrv )
      {
         for ( i = 0; i < iM; i++ )
         {
            dA = pIftx[i] - pIfty[i];
            dB = dA * dA;
            dC = m_dConvTension * 0.001 * dB * dB;
            pIftx[i] = dC;
            pIfty[i] = dC;
            pWorkT[i] = m_dConvTension * 0.004 * dA * dB;
         }

         pTa = pDIftxda;
         pTb = pDIftyda;
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               *pTa = pWorkT[i] * ( *pTa -*pTb );
               *pTb++ = *pTa++;
            }
         }

         if ( iDrv >= EOS_DRV_P )
         {
            for ( i = 0; i < iM; i++ )
            {
               pDIftxdp[i] = pWorkT[i] * ( pDIftxdp[i] - pDIftydp[i] );
               pDIftydp[i] = pDIftxdp[i];
            }

            if ( iDrv >= EOS_DRV_T )
            {
               for ( i = 0; i < iM; i++ )
               {
                  pDIftxdt[i] = pWorkT[i] * ( pDIftxdt[i] - pDIftydt[i] );
                  pDIftydt[i] = pDIftxdt[i];
               }
            }
         }
      }

      /* More than one block without derivatives */
      else
      {
         for ( i = 0; i < iM; i++ )
         {
            dA = pIftx[i] - pIfty[i];
            dB = dA * dA;
            dC = m_dConvTension * 0.001 * dB * dB;
            pIftx[i] = dC;
            pIfty[i] = dC;
         }
      }
   }

   /* One item with derivatives */
   else if ( iDrv )
   {
      dA = *pIftx -*pIfty;
      dB = dA * dA;
      dC = m_dConvTension * 0.001 * dB * dB;
      *pIftx = dC;
      *pIfty = dC;
      dC = m_dConvTension * 0.004 * dA * dB;
      for ( iNi = 0; iNi < m_iEosComp; iNi++ )
      {
         pDIftxda[iNi] = dC * ( pDIftxda[iNi] - pDIftyda[iNi] );
         pDIftyda[iNi] = pDIftxda[iNi];
      }

      if ( iDrv >= EOS_DRV_P )
      {
         *pDIftxdp = dC * ( *pDIftxdp -*pDIftydp );
         *pDIftydp = *pDIftxdp;
         if ( iDrv >= EOS_DRV_T )
         {
            *pDIftxdt = dC * ( *pDIftxdt -*pDIftydt );
            *pDIftydt = *pDIftxdt;
         }
      }
   }

   /* More than one block without derivatives */
   else
   {
      dA = *pIftx -*pIfty;
      dB = dA * dA;
      dC = m_dConvTension * 0.001 * dB * dB;
      *pIftx = dC;
      *pIfty = dC;
   }
}


/* 
// Tension
//
// Subroutine to compute the interfacial tension parameters 
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
// iMolar
//    Indicator as to whether molar (1) or mass (0) quantities used
// iCompute
//    Indicator as to whether terms to be set to zero
//       0 - Terms are to be set to zero
//       1 - Terms are to be computed
// pX ** constant ** 
//    Mole or mass fraction of each object
//       first index corresponds to object
//       second index corresponds to component
// pDXdp ** constant ** 
//    Pressure derivative of mole/mass fraction of each object
//       first index corresponds to object
//       second index corresponds to component
// pDXdt ** constant ** 
//    Temperature derivative of mole/mass fraction of each object
//       first index corresponds to object
//       second index corresponds to component
// pDXda ** constant ** 
//    Accumulation derivative of mole/mass fraction of each object
//       first index corresponds to object
//       second index corresponds to component
//       third index corresponds to component derivative
// pRho ** constant ** 
//    Mole/mass density of each object
// pDRhodp ** constant **
//    Pressure derivative of mole/mass density of each object
// pDRhodt ** constant **
//    Temperature derivative of mole/mass density of each object
// pDRhoda ** constant ** 
//    Accumulation derivative of mole/mass density of each object
//       first index corresponds to object
//       second index corresponds to component
// pIft 
//    The parachor times density for each object
// pDIftdp
//    The pressure derivative of parachor times density for each 
//    object
// pDIftdt
//    The temperature derivative of parachor times density each 
//    object. 
// pDIftda
//    The accumulation derivative of parachor times density for 
//    each object.
//       first index corresponds to object
//       second index corresponds to component
//
// 1) When iCompute is zero, the terms are set to zero.  This
//    option is used for single phase mixtures
// 2) The parachor model is, when using mole fractions
//
//    pIft = sum pX  pRho pParachor
//            i    i               i
//
// 3) Since, when using mass fractions, 
//
//    pRho = pRhomass / pMW
//
//    and
//
//    pX = pMW * pXmass  / M   
//      i              i    i      
//
//    thus
//
//    pIft = sum pXmass  pRhomass pParachor  / M
//            i        i                   i    i
*/
void EosPvtTable::Tension( int iM, int iDrv, int iMolar, int iCompute, double *pX, double *pDXdp, double *pDXdt,
                           double *pDXda, double *pRho, double *pDRhodp, double *pDRhodt, double *pDRhoda, double *pIft,
                           double *pDIftdp, double *pDIftdt, double *pDIftda, double *pWorkT )
{
   double *pTa;
   double *pTb;
   double *pTc;
   double *pTd;
   double *pTe;
   double *pWrk1T;
   double  dA;
   double  dB;
   double  dC;
   double  dCnv;
   int     iNi;
   int     iNj;
   int     i;

   /* Set the conversion constant */
   pWrk1T = pWorkT + iM;
   dCnv = 0.001 * m_dConvVolume / m_dConvDensity;

   /* Zero out results for more than one item */
   if ( iCompute == 0 && iM > 1 )
   {
      for ( i = 0; i < iM; i++ )
      {
         pIft[i] = 0.0;
      }

      if ( iDrv )
      {
         pTa = pDIftda;
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
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

   /* Zero out results for one item */
   else if ( iCompute == 0 )
   {
      *pIft = 0.0;
      if ( iDrv )
      {
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
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

   /* Compute the parachor terms for molar */
   else if ( iM > 1 )
   {
      if ( iMolar )
      {
         pTa = pWrk1T;
         pTb = pX;
         dA = dCnv * m_pParachor[0];
         for ( i = 0; i < iM; i++ )
         {
            pWorkT[i] = *pTb++ *dA;
            *pTa++ = pRho[i] * dA;
         }

         for ( iNi = 1; iNi < m_iEosComp; iNi++ )
         {
            dA = dCnv * m_pParachor[iNi];
            for ( i = 0; i < iM; i++ )
            {
               pWorkT[i] += *pTb++ *dA;
               *pTa++ = pRho[i] * dA;
            }
         }
      }

      /* Compute the parachor terms for mass */
      else
      {
         pTa = pWrk1T;
         pTb = pX;
         dA = dCnv * m_pParachor[0] / m_pMolecularWeight[0];
         for ( i = 0; i < iM; i++ )
         {
            pWorkT[i] = *pTb++ *dA;
            *pTa++ = pRho[i] * dA;
         }

         for ( iNi = 1; iNi < m_iEosComp; iNi++ )
         {
            dA = dCnv * m_pParachor[iNi] / m_pMolecularWeight[iNi];
            for ( i = 0; i < iM; i++ )
            {
               pWorkT[i] += *pTb++ *dA;
               *pTa++ = pRho[i] * dA;
            }
         }
      }

      /* Set the ift */
      for ( i = 0; i < iM; i++ )
      {
         pIft[i] = pRho[i] * pWorkT[i];
      }

      /* Derivatives */
      if ( iDrv )
      {
         pTa = pDXda;
         pTc = pDRhoda;
         pTd = pDIftda;
         for ( iNj = 0; iNj < m_iEosComp; iNj++ )
         {
            pTb = pWrk1T;
            pTe = pTd;
            for ( i = 0; i < iM; i++ )
            {
               *pTe++ = pWorkT[i] **pTc++;
            }

            for ( iNi = 0; iNi < m_iEosComp; iNi++ )
            {
               pTe = pTd;
               for ( i = 0; i < iM; i++ )
               {
                  *pTe += *pTb++ **pTa++;
                  pTe++;
               }
            }

            pTd += iM;
         }

         /* Pressure derivatives */
         if ( iDrv >= EOS_DRV_P )
         {
            pTa = pDXdp;
            pTb = pWrk1T;
            for ( i = 0; i < iM; i++ )
            {
               pDIftdp[i] = pWorkT[i] * pDRhodp[i];
            }

            for ( iNi = 0; iNi < m_iEosComp; iNi++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  pDIftdp[i] += *pTb++ **pTa++;
               }
            }

            /* Temperature derivatives */
            if ( iDrv >= EOS_DRV_T )
            {
               pTa = pDXdt;
               pTb = pWrk1T;
               for ( i = 0; i < iM; i++ )
               {
                  pDIftdt[i] = pWorkT[i] * pDRhodt[i];
               }

               for ( iNi = 0; iNi < m_iEosComp; iNi++ )
               {
                  for ( i = 0; i < iM; i++ )
                  {
                     pDIftdt[i] += *pTb++ **pTa++;
                  }
               }
            }
         }
      }
   }

   /* Compute the parachor terms for molar */
   else
   {
      if ( iMolar )
      {
         pTa = pWrk1T;
         pTb = pX;
         dA = dCnv * m_pParachor[0];
         dB = *pRho;
         dC = *pTb++ *dA;
         *pTa++ = dB * dA;
         for ( iNi = 1; iNi < m_iEosComp; iNi++ )
         {
            dA = dCnv * m_pParachor[iNi];
            dC += *pTb++ *dA;
            *pTa++ = dB * dA;
         }
      }

      /* Compute the parachor terms for mass */
      else
      {
         pTa = pWrk1T;
         pTb = pX;
         dA = dCnv * m_pParachor[0] / m_pMolecularWeight[0];
         dB = *pRho;
         dC = *pTb++ *dA;
         *pTa++ = dB * dA;
         for ( iNi = 1; iNi < m_iEosComp; iNi++ )
         {
            dA = dCnv * m_pParachor[iNi] / m_pMolecularWeight[iNi];
            dC += *pTb++ *dA;
            *pTa++ = dB * dA;
         }
      }

      /* Set the ift */
      *pIft = dB * dC;

      /* Derivatives */
      if ( iDrv )
      {
         pTa = pDXda;
         pTc = pDRhoda;
         pTd = pDIftda;
         for ( iNj = 0; iNj < m_iEosComp; iNj++ )
         {
            pTb = pWrk1T;
            dB = dC **pTc++;
            for ( iNi = 0; iNi < m_iEosComp; iNi++ )
            {
               dB += *pTb++ **pTa++;
            }

            *pTd++ = dB;
         }

         /* Pressure derivatives */
         if ( iDrv >= EOS_DRV_P )
         {
            pTa = pDXdp;
            pTb = pWrk1T;
            dB = dC **pDRhodp;
            for ( iNi = 0; iNi < m_iEosComp; iNi++ )
            {
               dB += *pTb++ **pTa++;
            }

            *pDIftdp = dB;

            /* Temperature derivatives */
            if ( iDrv >= EOS_DRV_T )
            {
               pTa = pDXdt;
               pTb = pWrk1T;
               dB = dC **pDRhodt;
               for ( iNi = 0; iNi < m_iEosComp; iNi++ )
               {
                  dB += *pTb++ **pTa++;
               }

               *pDIftdt = dB;
            }
         }
      }
   }
}


/* 
// ThermalDiffusion
//
// Compute the thermal diffusion contribution to compositional
// grading
//
// iM ** constant **
//    Number of objects
// dT ** constant **
//    Logarithm of derivative
// dDiffusion ** constant **
//    Adjustable term for thermal diffusion
// pZx ** constant **
//    Volume times pressure for each object
// pDZxda ** constant **
//    Accumulation derivative of volume times pressure for each object
//       first index corresponds to object
//       second index corresponds to component
// pHx ** constant **
//    Enthalpy for each object
// pDHxda ** constant **
//    Accumulation derivative of enthalpy for each object
//       first index corresponds to object
//       second index corresponds to component
// pDiffusion
//    Additional diffusion term for each object
//
// 1) From Kemper's work,
//
//    Dij = dlnT ( H  V - V  H ) / V = dlnT ( H  - H V  / V )
//                  i      i                   i      i
//
//    Note that this term is antisymmetric, namely, Dij = -Dji
//
//    See L.J.T.M Kempers, "A thermodynamic theory of the Soret
//    effect in a multicomponent liquid", J. Chem. Phys, 90, 
//    June 1989.  
// 2) Within the flasher, the partial volume and partial
//    enthalpies are related to the accumulation derivatives by
//   
//    V  = V + dV/dA
//     i            i
//
//    with a similar formula applying for enthalpy
// 3) Expanding
//
//    Dij = dlnT ( H + dH/dA  - H - H dV/dA  / V )
//                          i             i
//
// 4) Rearranging and adding a tuning parameter D
//
//    Dij = D dlnT ( dH/dA  - H dV/dA  / V )
//                        i          i
//
// 5) Note that although the volume is multiplied by the
//    pressure when used in this routine, the pressure
//    term cancels 
*/
void EosPvtTable::ThermalDiffusion( int iM, double dT, double dDiffusion, double *pZx, double *pDZxda, double *pHx,
                                    double *pDHxda, double *pDiffusion )
{
   double  dA;
   double  dH;
   double *pTa;
   double *pTb;
   double *pTc;
   int     iNi;
   int     i;

   /* Save term */
   dH = dDiffusion * dT;

   /* Multiple grid blocks */
   if ( iM > 1 )
   {
      pTa = pDZxda;
      pTb = pDHxda;
      pTc = pDiffusion;
      for ( i = 0; i < iM; i++ )
      {
         dA = pHx[i] / pZx[i];
         pHx[i] = dA;
         *pTc++ = dH * ( *pTb++ -dA **pTa++ );
      }

      for ( iNi = 1; iNi < m_iEosComp; iNi++ )
      {
         for ( i = 0; i < iM; i++ )
         {
            *pTc++ = dH * ( *pTb++ -pHx[i] **pTa++ );
         }
      }
   }

   /* Single grid block */
   else
   {
      dA = *pHx / *pZx;
      for ( iNi = 0; iNi < m_iEosComp; iNi++ )
      {
         pDiffusion[iNi] = dH * ( pDHxda[iNi] - dA * pDZxda[iNi] );
      }
   }
}


/* 
// Viscosity
//
// Compute the viscosity 
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
// iMolar ** constant **
//    Indicator as to whether molar (1) or mass (0) quantities used
// dTiny ** constant **
//    A tiny number
// pP ** constant **
//    Pressure
// pT ** constant **
//    Temperature for each object
// pZ ** constant ** 
//    Mole or mass fraction of each object
//       first index corresponds to object
//       second index corresponds to component
// pDZdp ** constant ** 
//    Pressure derivative of mole/mass fraction of each object
//       first index corresponds to object
//       second index corresponds to component
// pDZdt ** constant ** 
//    Temperature derivative of mole/mass fraction of each object
//       first index corresponds to object
//       second index corresponds to component
// pDZda ** constant ** 
//    Accumulation derivative of mole/mass fraction of each object
//       first index corresponds to object
//       second index corresponds to component
//       third index corresponds to component derivative
// pZ ** constant ** 
//    Mole/mass density of each object
// pDZdp ** constant **
//    Pressure derivative of mole/mass density of each object
// pDZdt ** constant **
//    Temperature derivative of mole/mass density of each object
// pDZda ** constant ** 
//    Accumulation derivative of mole/mass density of each object
//       first index corresponds to object
//       second index corresponds to component
// pMu 
//    The viscosity for each object
// pDMudp
//    The pressure derivative of viscosity for each object
// pDMudt
//    The temperature derivative of viscosity for each object. 
// pDMuda
//    The accumulation derivative of viscosity for each object.
//       first index corresponds to object
//       second index corresponds to component
// pSumT
//    Beginning of work array used in the computations
//
// 1) If dVTune is zero, simply call the Lohrenz viscosity routine
// 2) Otherwise, begin by sorting out which components are lights
//    with the call to GetLights
// 3) If there are lights, use the Lohrenz correlation for the
//    light viscosity
// 4) If there are only heavies, call the Shealy viscosity routine
//    and this is the answer
*/
void EosPvtTable::Viscosity( int iM, int iDrv, int iMolar, double dTiny, double *pP, double *pT, double *pZ,
                             double *pDZdp, double *pDZdt, double *pDZda, double *pRho, double *pDRhodp,
                             double *pDRhodt, double *pDRhoda, double *pMu, double *pDMudp, double *pDMudt,
                             double *pDMuda, double *pSumT )
{
   double *pMWC;
   double *pPCC;
   double *pTCC;
   double *pVCC;
   double *pX;
   double *pDXda;
   double *pDXdp;
   double *pDXdt;
   double *pTmp;
   double *pHlp;
   double *pHlpdp;
   double *pHlpdt;
   double *pHlpda;
   int     iNLight;
   int     iNHeavy;

   /* Call the viscosity if dVTune is zero */
   if ( m_dVTune == 0.0 )
   {
      /* TODO: The next line is too long. Please rewrite to make it shorter. */
      LohrenzBC( iM, iDrv, iMolar, m_iEosComp, m_pMolecularWeight, m_pCriticalPressure, m_pCriticalTemperature, m_pCriticalVolume, pT, pZ, pDZdp, pDZdt, pDZda, pRho, pDRhodp, pDRhodt, pDRhoda, pMu, pDMudp, pDMudt, pDMuda, pSumT );
   }

   /* Corresponding states */
   else if ( m_dVTune < 0.0 )
   {
      ViscosityCorrespondingStates( iM, iMolar, iDrv, pP, pT, pZ, pDZdp, pDZdt, pDZda, pMu, pDMuda, pDMudp, pDMudt, pSumT );
   }

   /* Jossi viscosity */
   else
   {
      iNLight = iM * m_iEosComp;

      /* Set temporary pointers */
      pHlp = pSumT;
      pHlpdp = pHlp + iM;
      pHlpdt = pHlpdp + iM;
      pHlpda = pHlpdt + iM;
      pX = pHlpda + iNLight;
      pDXdp = pX + iNLight;
      pDXdt = pDXdp + iNLight;
      pDXda = pDXdt + iNLight;
      pMWC = pDXda + iNLight * m_iEosComp;
      pPCC = pMWC + m_iEosComp;
      pTCC = pPCC + m_iEosComp;
      pVCC = pTCC + m_iEosComp;
      pTmp = pVCC + m_iEosComp;

      /* Get light components */
      /* TODO: The next line is too long. Please rewrite to make it shorter. */
      GetComps( iM, iDrv, iMolar, EOS_OPTION_ON, dTiny, pZ, pDZdp, pDZdt, pDZda, pX, pDXdp, pDXdt, pDXda, &iNLight, pRho, pDRhodp, pDRhodt, pDRhoda, pHlp, pHlpdp, pHlpdt, pHlpda, pMWC, pPCC, pTCC, pVCC, pTmp );

      /* Call the Lohrenz routine */
      if ( iNLight > 0 )
      {
         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         LohrenzBC( iM, iDrv, EOS_OPTION_ON, iNLight, pMWC, pPCC, pTCC, pVCC, pT, pX, pDXdp, pDXdt, pDXda, pHlp, pHlpdp, pHlpdt, pHlpda, pMu, pDMudp, pDMudt, pDMuda, pTmp );
      }

      /* Get heavy composition */
      if ( iNLight != m_iEosComp )
      {
         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         GetComps( iM, iDrv, iMolar, EOS_OPTION_OFF, dTiny, pZ, pDZdp, pDZdt, pDZda, pX, pDXdp, pDXdt, pDXda, &iNHeavy, pRho, pDRhodp, pDRhodt, pDRhoda, pHlp, pHlpdp, pHlpdt, pHlpda, pMWC, pPCC, pTCC, pVCC, pTmp );
      }
      else
      {
         iNHeavy = 0;
      }

      /* All heavy components */
      if ( iNLight == 0 )
      {
         ViscosityGSS( iM, iDrv, iNHeavy, pMWC, pP, pT, pX, pDXdp, pDXdt, pDXda, pMu, pDMudp, pDMudt, pDMuda, pPCC, pTCC );
      }

      /* A mixture */
      else if ( iNLight != m_iEosComp )
      {
         /* Heavy component viscosity */
         ViscosityGSS( iM, iDrv, iNHeavy, pMWC, pP, pT, pX, pDXdp, pDXdt, pDXda, pHlp, pHlpdp, pHlpdt, pHlpda, pPCC, pTCC );

         /* Mobility ratio */
         MobilityRatio( iM, iDrv, pMu, pDMudp, pDMudt, pDMuda, pHlp, pHlpdp, pHlpdt, pHlpda, pHlp, pHlpdp, pHlpdt, pHlpda, pX );

         /* Mixture viscosity */
         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         ViscosityMix( iM, iDrv, iMolar, pZ, pDZdp, pDZdt, pDZda, pX, pDXdp, pDXdt, pDXda, pMu, pDMudp, pDMudt, pDMuda, pHlp, pHlpdp, pHlpdt, pHlpda, pMu, pDMudp, pDMudt, pDMuda, pMWC );
      }
   }
}


/* 
// Corresponding states viscosity model
//
// iM - Number of objects to calculate (input)
// iMolar - Indicator if using mole fractions (input)
//          1 means using mole fractions; else 0
// iDrv - Derivative level (input)
// pP - Pressure (input)
// pT - Temperature (input)
// pZ - Composition (input)
// pDZdp - Pressure derivative of composition (input)
// pDZdt - Temperature derivative of composition (input)
// pDZda - Accumulation derivative of composition (input)
// pViscosity - viscosity (output)
// pDViscosityDX - derivative of viscosity wrt composition (output)
//                 not calculated if iDrv < 1
// pDViscosityDP - derivative of viscosity wrt pressure (output)
//                 not calculated if iDrv < 2
// pDViscosityDT - derivative of viscosity wrt temperature (output)
//                 not calculated if iDrv < 3
// pWork - Work array 
*/
void EosPvtTable::ViscosityCorrespondingStates( int iM, int iMolar, int iDrv, double *pP, double *pT, double *pZ,
                                                double *pDZdp, double *pDZdt, double *pDZda, double *pViscosity,
                                                double *pDViscosityDX, double *pDViscosityDP, double *pDViscosityDT,
                                                double *pWork )
{
   double *pTa;
   double *pTb;
   double *pTc;
   int     iNi;
   int     iNj;
   int     i;

   /* Derivative locations */
   double *pDMudx = iDrv ? pWork + iM * ( 8 + 3 * m_iEosComp ) : (double *)0;
   double *pDMudp = iDrv >= EOS_DRV_P ? pDViscosityDP : (double *)0;
   double *pDMudt = iDrv >= EOS_DRV_T ? pDViscosityDT : (double *)0;

   /* Call the viscosity model */
   ViscosityCS( iM, iMolar, pP, pT, pZ, pViscosity, pDMudx, pDMudp, pDMudt, pWork );

   /* Derivatives */
   if ( iDrv )
   {
      if ( iDrv >= EOS_DRV_P )
      {
         /* Chain rule for temperature derivatives */
         if ( iDrv >= EOS_DRV_T )
         {
            pTa = pDZdt;
            pTb = pDMudx;
            for ( iNi = 0; iNi < m_iEosComp; iNi++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  pDViscosityDT[i] += pTa[i] * pTb[i];
               }

               pTa += iM;
               pTb += iM;
            }
         }

         /* Chain rule for pressure derivatives */
         pTa = pDZdp;
         pTb = pDMudx;
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               pDViscosityDP[i] += pTa[i] * pTb[i];
            }

            pTa += iM;
            pTb += iM;
         }
      }

      /* Chain rule for accumulation derivatives */
      pTa = pDViscosityDX;
      pTb = pDZda;
      for ( iNi = 0; iNi < m_iEosComp; iNi++ )
      {
         pTc = pDMudx;
         for ( i = 0; i < iM; i++ )
         {
            pTa[i] = pTb[i] * pTc[i];
         }

         for ( iNj = 1; iNj < m_iEosComp; iNj++ )
         {
            pTb += iM;
            pTc += iM;
            for ( i = 0; i < iM; i++ )
            {
               pTa[i] += pTb[i] * pTc[i];
            }
         }

         pTa += iM;
         pTb += iM;
      }
   }
}


/* 
// ViscosityGSS
//
// Compute the viscosity using method of Glen Shealy
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
// iNHeavy ** constant **
//    Number of Heavy components
// pMWC ** constant **
//    Molecular weights
// pP ** constant **
//    Pressures
// pT ** constant **
//    Temperature for each object
// pZ ** constant ** 
//    Mole or mass fraction of each object
//       first index corresponds to object
//       second index corresponds to component
// pDZdp ** constant ** 
//    Pressure derivative of mole/mass fraction of each object
//       first index corresponds to object
//       second index corresponds to component
// pDZdt ** constant ** 
//    Temperature derivative of mole/mass fraction of each object
//       first index corresponds to object
//       second index corresponds to component
// pDZda ** constant ** 
//    Accumulation derivative of mole/mass fraction of each object
//       first index corresponds to object
//       second index corresponds to component
//       third index corresponds to component derivative
// pMu 
//    The viscosity for each object
// pDMudp
//    The pressure derivative of viscosity for each object
// pDMudt
//    The temperature derivative of viscosity for each object. 
// pDMuda
//    The accumulation derivative of viscosity for each object.
//       first index corresponds to object
//       second index corresponds to component
// pATerm
//    A component term
// pTmp
//    Beginning of work array used in the computations
//
// Yes, this routine is YUGLY!!!!
//
// Calculates reciprocal viscosity of heavy components.  Correlations 
// developed by G.S. Shealy are used.  See Shealy, G. S. (1987), 
// A Compositional Viscosity Model for Petroleum Fluids,
// Technical Progress Report, BRC, Houston
//
// Note that the definitions below have been rearranged for
// computational efficiency
//
// For each heavy component we must first define for each 
// heavy component the following term
//
//    1) A  = - 1.17 + 0.0086 * MW , MW < 300 ,
//        I
//          = 1.41 + 0.0168 * VTUNEP * ( MW - 300 ) , OTHERWISE
//
// We also define
//
//               { ( 9.585 * MW - 430.5 ) * ( 1 / T - 1 / 536 ) }
//    2) P1 = 10
//
//                                               3.5        3.5
//               { ( 4.33E7*MW - 5.23E9 ) * ( 1/T    - 1/536    ) }
//    3) P2 = 10
//
//    4) SM = 0.5 + ( W - 300 ) / { 4 + 2 * ABS ( W - 300 ) }
//
// And finally
//
//    5) B  = A  + LOG   ( P1 + SM * { P2 - P1 } )
//        I    I      10
//
// From these coefficients the viscosity of the heavy components
// is correlated as, independent of pressure,
//
//                SUM CHI  B
//                 H     H  H
//    6) MU0 = 10
//
// This expression is corrected for pressure as
//
//    7) MU = MU0 * { 1 + ( 1 + SUM CHI  A  ) ( P - 14.696 ) / 10000 }
//                               H     H  H
*/
void EosPvtTable::ViscosityGSS( int iM, int iDrv, int iNHeavy, double *pMWC, double *pP, double *pT, double *pZ,
                                double *pDZdp, double *pDZdt, double *pDZda, double *pMu, double *pDMudp,
                                double *pDMudt, double *pDMuda, double *pATerm, double *pTmp )
{
   double *pT1;
   double *pT2;
   double *pT3;
   double *pT4;
   double *pT5;
   double *pT6;
   double *pTa;
   double *pTb;
   double *pTc;
   double *pSum;
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
   int     i;
   int     iNi;
   int     iNj;

   /* Temperature pointers */
   pSum = pTmp + iM * iNHeavy;
   pT1 = pSum + iM;
   pT2 = pT1 + iM;
   pT3 = pT2 + iM;
   pT4 = pT3 + iM;
   pT5 = pT4 + iM;
   pT6 = pT5 + iM;

   /* Set temperature terms with temperature derivatives */
   if ( iDrv >= EOS_DRV_T )
   {
      dA = m_dConvTemperature / 1.8;
      for ( i = 0; i < iM; i++ )
      {
         dB = 1.0 / pT[i];
         dC = dB * dA;
         dD = 22.070278116348 * dC;
         pT2[i] = -dD * dB;
         pT1[i] = dD - 0.04117589200811;
         pT4[i] = -3.5 * dB;
         pT3[i] = dC;
      }

      EosUtil::VectorPow( iM, pT3, 3.5, pT3 );
      for ( i = 0; i < iM; i++ )
      {
         dA = 9.97019345266e7 * pT3[i];
         pT4[i] *= dA;
         pT3[i] = dA - 0.02796577254887;
         pDMudt[i] = 0.0;
         pSum[i] = 1.0;
         pMu[i] = 0.0;
      }
   }
   else
   {
      dA = m_dConvTemperature / 1.8;
      for ( i = 0; i < iM; i++ )
      {
         dB = 1.0 / pT[i];
         dC = dB * dA;
         pT1[i] = 22.070278116348 * dC - 0.04117589200811;
         pT3[i] = dC;
      }

      EosUtil::VectorPow( iM, pT3, 3.5, pT3 );
      for ( i = 0; i < iM; i++ )
      {
         pT3[i] = 9.97019345266e7 * pT3[i] - 0.02796577254887;
         pSum[i] = 1.0;
         pMu[i] = 0.0;
      }
   }

   /* For each component */
   pTa = pTmp;
   pTb = pZ;
   for ( iNi = 0; iNi < iNHeavy; iNi++ )
   {
      dA = pMWC[iNi];
      dB = dA - 300.0;
      dC = ( dB < 0.0 ) ? ( 0.0086 * dA - 1.17 ) : ( 1.41 + 0.0168 * m_dVTune * dB );
      pATerm[iNi] = dC;
      dD = 0.5 * ( 1.0 + dB / ( 2.0 + fabs( dB ) ) );
      dE = dA - 44.9139280125;
      dF = dA - 120.7852193995;
      dJ = 2.30258509299405 * dC;

      /* Exponentiations */
      for ( i = 0; i < iM; i++ )
      {
         pT5[i] = pT1[i] * dE;
         pT6[i] = pT3[i] * dF;
      }

      EosUtil::VectorExp( iM, pT5, pT5 );
      EosUtil::VectorExp( iM, pT6, pT6 );

      /* Temperature terms */
      if ( iDrv >= EOS_DRV_T )
      {
         for ( i = 0; i < iM; i++ )
         {
            dA = pT5[i];
            dB = pT6[i];
            dG = dA + dD * ( dB - dA );
            dI = dJ + log( dG );
            dA *= dE * pT2[i];
            dB *= dF * pT4[i];
            dH = *pTb++;
            pDMudt[i] += dH * ( dA + dD * ( dB - dA ) ) / dG;
            pSum[i] += dH * dC;
            pMu[i] += dH * dI;
            *pTa++ = dI;
         }
      }
      else
      {
         for ( i = 0; i < iM; i++ )
         {
            dH = *pTb++;
            dA = pT5[i];
            dI = dJ + log( dA + dD * ( pT6[i] - dA ) );
            pSum[i] += dH * dC;
            pMu[i] += dH * dI;
            *pTa++ = dI;
         }
      }
   }

   /* Form exponential */
   dA = 0.001 * m_dConvViscosity;
   dB = 1.0 / 6.89476e7 / m_dConvPressure;
   EosUtil::VectorExp( iM, pMu, pMu );
   if ( iDrv >= EOS_DRV_P )
   {
      for ( i = 0; i < iM; i++ )
      {
         dC = pSum[i];
         dD = dB * pP[i] - 0.0014696;
         dE = dA * pMu[i];
         dD *= dE;
         pDMudp[i] = dB * dC * dE;
         pMu[i] = dE + dC * dD;
         pT1[i] = dD;
      }
   }
   else
   {
      for ( i = 0; i < iM; i++ )
      {
         dC = pSum[i];
         dD = dB * pP[i] - 0.0014696;
         dE = dA * pMu[i];
         dD *= dE;
         pMu[i] = dE + dC * dD;
         pT1[i] = dD;
      }
   }

   /* Modify derivatives */
   if ( iDrv )
   {
      pTa = pTmp;
      for ( iNi = 0; iNi < iNHeavy; iNi++ )
      {
         dC = pATerm[iNi];
         for ( i = 0; i < iM; i++ )
         {
            *pTa = *pTa * pMu[i] + dC * pT1[i];
            pTa++;
         }
      }

      /* Accumulation derivatives */
      pTa = pDZda;
      pTc = pDMuda;
      for ( iNj = 0; iNj < m_iEosComp; iNj++ )
      {
         pTb = pTmp;
         for ( i = 0; i < iM; i++ )
         {
            pTc[i] = *pTa++ **pTb++;
         }

         for ( iNi = 1; iNi < iNHeavy; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               pTc[i] += *pTa++ **pTb++;
            }
         }

         pTc += iM;
      }

      /* Pressure derivatives */
      if ( iDrv >= EOS_DRV_P )
      {
         pTa = pDZdp;
         pTb = pTmp;
         for ( iNi = 0; iNi < iNHeavy; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               pDMudp[i] += *pTa++ **pTb++;
            }
         }

         /* Pressure derivatives */
         if ( iDrv >= EOS_DRV_T )
         {
            pTa = pDZdt;
            pTb = pTmp;
            for ( i = 0; i < iM; i++ )
            {
               pDMudt[i] *= pMu[i];
            }

            for ( iNi = 0; iNi < iNHeavy; iNi++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  pDMudt[i] += *pTa++ **pTb++;
               }
            }
         }
      }
   }
}


/* 
// ViscosityMix
//
// Compute the mixture viscosity from the light viscosity and
// the mobility ratio
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
// iMolar ** constant **
//    Indicator if molar quantities
// pZ ** constant ** 
//    Mole or mass fraction of each object
//       first index corresponds to object
//       second index corresponds to component
// pDZdp ** constant ** 
//    Pressure derivative of mole/mass fraction of each object
//       first index corresponds to object
//       second index corresponds to component
// pDZdt ** constant ** 
//    Temperature derivative of mole/mass fraction of each object
//       first index corresponds to object
//       second index corresponds to component
// pDZda ** constant ** 
//    Accumulation derivative of mole/mass fraction of each object
//       first index corresponds to object
//       second index corresponds to component
//       third index corresponds to component derivative
// pX  
//    Heavy mole fraction of each object
//       first index corresponds to object
// pDXdp 
//    Pressure derivative of heavy mole fraction of each object
//       first index corresponds to object
// pDXdt  
//    Temperature derivative of heavy mole fraction of each object
//       first index corresponds to object
// pDXda 
//    Accumulation derivative of heavy mole fraction of each object
//       first index corresponds to object
//       third index corresponds to component derivative
// pML 
//    The light viscosity for each object
// pDMLdp
//    The pressure derivative of light viscosity for each object
// pDMLdt
//    The temperature derivative of light viscosity for each object. 
// pDMLda
//    The accumulation derivative of light viscosity for each object.
//       first index corresponds to object
//       second index corresponds to component
// pMR 
//    The mobility ratio for each object
// pDMRdp
//    The pressure derivative of mobility ratio for each object
// pDMRdt
//    The temperature derivative of mobility ratio for each object. 
// pDMRda
//    The accumulation derivative of mobility ratio for each object.
//       first index corresponds to object
//       second index corresponds to component
// pMu 
//    The viscosity for each object
// pDMudp
//    The pressure derivative of viscosity for each object
// pDMudt
//    The temperature derivative of viscosity for each object. 
// pDMuda
//    The accumulation derivative of viscosity for each object.
//       first index corresponds to object
//       second index corresponds to component
// pTmp
//    Beginning of work array used in the computations
//
// 1) First compute the mole fraction of heavy since we will
//    need that later on
// 2) Compute the following term as a function of the mobility ratio
//
//    TERM = LOG   ( MR ) * { LOG   ( MR ) - 2 }
//              10               10
//
// 3) Define the exponent as
//
//    E = 2.5 + 0.6 * MAX ( F  , 0.1 ) * TERM
//                           H
//
// 4) The viscosity is computed then using a power law blending term 
//    of the form
//
//                  1/E          1/E   E
//    MU = ( F  * MU    + F  * MU    )
//            L     L      H     H
//
// 5) The above expression can be written in terms of the 
//    mobility ratio
//
//                              1/E        E
//    MU = MU  * ( 1 + F  * ( MR    - 1 ) )
//           L          H     
//
// 6) Of course there have been some rearrangements to make
//    for more efficient calculations.
// 7) Take derivatives if requested
*/
void EosPvtTable::ViscosityMix( int iM, int iDrv, int iMolar, double *pZ, double *pDZdp, double *pDZdt, double *pDZda,
                                double *pX, double *pDXdp, double *pDXdt, double *pDXda, double *pML, double *pDMLdp,
                                double *pDMLdt, double *pDMLda, double *pMR, double *pDMRdp, double *pDMRdt,
                                double *pDMRda, double *pMu, double *pDMudp, double *pDMudt, double *pDMuda,
                                double *pTmp )
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
   double  dMR;
   double  dHV;
   double *pTa;
   double *pTb;
   double *pTc;
   double *pTd;
   double *pTmpP;
   double *pTmpT;
   int     i;
   int     iNi;
   int     iNj;
   int     iTemp;

   /* Set pointers */
   pTmpP = pTmp + iM;
   pTmpT = pTmpP + iM;

   /* Compute heavy mole fractions for mole fractions */
   if ( iMolar )
   {
      /* Zero out heavy fractions */
      for ( i = 0; i < iM; i++ )
      {
         pX[i] = 0.0;
      }

      if ( iDrv >= EOS_DRV_P )
      {
         for ( i = 0; i < iM; i++ )
         {
            pDXdp[i] = 0.0;
         }

         if ( iDrv >= EOS_DRV_T )
         {
            for ( i = 0; i < iM; i++ )
            {
               pDXdt[i] = 0.0;
            }
         }
      }

      /* For each component */
      pTa = pZ;
      pTb = pDZdp;
      pTc = pDZdt;
      for ( iNi = 0; iNi < m_iEosComp; iNi++ )
      {
         /* If molecular weight large enough consider a "heavy" */
         dA = m_pMolecularWeight[iNi];
         if ( dA >= EOS_JOSSI_MW_SPLIT )
         {
            /* Add to heavy */
            for ( i = 0; i < iM; i++ )
            {
               pX[i] += *pTa++;
            }

            if ( iDrv >= EOS_DRV_P )
            {
               for ( i = 0; i < iM; i++ )
               {
                  pDXdp[i] += *pTb++;
               }

               if ( iDrv >= EOS_DRV_T )
               {
                  for ( i = 0; i < iM; i++ )
                  {
                     pDXdt[i] += *pTc++;
                  }
               }
            }
         }
         else
         {
            pTa += iM;
            pTb += iM;
            pTc += iM;
         }
      }

      /* Derivatives */
      if ( iDrv )
      {
         pTa = pDZda;
         pTb = pDXda;
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               pTb[i] = 0.0;
            }

            for ( iNj = 0; iNj < m_iEosComp; iNj++ )
            {
               /* If molecular weight large enough a "heavy" */
               dA = m_pMolecularWeight[iNj];
               if ( dA >= EOS_JOSSI_MW_SPLIT )
               {
                  for ( i = 0; i < iM; i++ )
                  {
                     pTb[i] += *pTa++;
                  }
               }
               else
               {
                  pTa += iM;
               }
            }

            pTb += iM;
         }
      }
   }

   /* Compute heavy mole fractions for mass fractions */
   else
   {
      /* Zero out heavy fractions */
      for ( i = 0; i < iM; i++ )
      {
         pX[i] = 0.0;
         pTmp[i] = 0.0;
      }

      if ( iDrv >= EOS_DRV_P )
      {
         for ( i = 0; i < iM; i++ )
         {
            pDXdp[i] = 0.0;
            pTmpP[i] = 0.0;
         }

         if ( iDrv >= EOS_DRV_T )
         {
            for ( i = 0; i < iM; i++ )
            {
               pDXdt[i] = 0.0;
               pTmpT[i] = 0.0;
            }
         }
      }

      /* For each component */
      pTa = pZ;
      pTb = pDZdp;
      pTc = pDZdt;
      for ( iNi = 0; iNi < m_iEosComp; iNi++ )
      {
         /* If molecular weight large enough consider a "heavy" */
         dA = m_pMolecularWeight[iNi];
         if ( dA >= EOS_JOSSI_MW_SPLIT )
         {
            dA = 1.0 / dA;

            /* Add to heavy */
            for ( i = 0; i < iM; i++ )
            {
               pX[i] += dA **pTa++;
            }

            if ( iDrv >= EOS_DRV_P )
            {
               for ( i = 0; i < iM; i++ )
               {
                  pDXdp[i] += dA **pTb++;
               }

               if ( iDrv >= EOS_DRV_T )
               {
                  for ( i = 0; i < iM; i++ )
                  {
                     pDXdt[i] += dA **pTc++;
                  }
               }
            }
         }

         /* Need the sum for the quotient */
         else
         {
            dA = 1.0 / dA;

            /* Add to heavy */
            for ( i = 0; i < iM; i++ )
            {
               pTmp[i] += dA **pTa++;
            }

            if ( iDrv >= EOS_DRV_P )
            {
               for ( i = 0; i < iM; i++ )
               {
                  pTmpP[i] += dA **pTb++;
               }

               if ( iDrv >= EOS_DRV_T )
               {
                  for ( i = 0; i < iM; i++ )
                  {
                     pTmpT[i] += dA **pTc++;
                  }
               }
            }
         }
      }

      /* Compute heavy fractions */
      for ( i = 0; i < iM; i++ )
      {
         dA = pX[i];
         dB = 1.0 / ( dA + pTmp[i] );
         pX[i] = dA * dB;
         pTmp[i] = dB;
      }

      if ( iDrv >= EOS_DRV_P )
      {
         for ( i = 0; i < iM; i++ )
         {
            dA = pDXdp[i];
            pDXdp[i] = ( dA - pX[i] * ( dA + pTmpP[i] ) ) * pTmp[i];
         }

         if ( iDrv >= EOS_DRV_T )
         {
            for ( i = 0; i < iM; i++ )
            {
               dA = pDXdt[i];
               pDXdt[i] = ( dA - pX[i] * ( dA + pTmpT[i] ) ) * pTmp[i];
            }
         }
      }

      /* Derivatives */
      if ( iDrv )
      {
         pTa = pDZda;
         pTb = pDXda;
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               pTb[i] = 0.0;
               pTmpP[i] = 0.0;
            }

            for ( iNj = 0; iNj < m_iEosComp; iNj++ )
            {
               /* If molecular weight large enough a "heavy" */
               dA = m_pMolecularWeight[iNj];
               if ( dA >= EOS_JOSSI_MW_SPLIT )
               {
                  dA = 1.0 / dA;
                  for ( i = 0; i < iM; i++ )
                  {
                     pTb[i] += dA **pTa++;
                  }
               }
               else
               {
                  dA = 1.0 / dA;
                  for ( i = 0; i < iM; i++ )
                  {
                     pTmpP[i] += dA **pTa++;
                  }
               }
            }

            /* Final term */
            for ( i = 0; i < iM; i++ )
            {
               dA = pTb[i];
               pTb[i] = ( dA - pX[i] * ( dA + pTmpP[i] ) ) * pTmp[i];
            }

            pTb += iM;
         }
      }
   }

   /* Compute the terms with derivatives */
   if ( iDrv )
   {
      for ( i = 0; i < iM; i++ )
      {
         dHV = pX[i];
         dMR = pMR[i];
         dG = log( dMR );
         dB = dG / 2.30258509299405;
         dC = dB - 2.0;
         dD = dB * dC;
         iTemp = dHV > 0.1;
         dF = iTemp ? ( 0.6 * dHV ) : 0.6;
         dE = dF * ( dC + dB ) / dMR / 2.30258509299405;
         dB = iTemp ? ( 0.6 * dD ) : 0.0;
         dC = 2.5 + dF * dD;
         dA = dG / dC;
         dD = exp( dA );
         dF = dD - 1.0;
         dH = dD * dA / dC;
         dA = -dH * dB;
         dD = dD / dMR / dC - dH * dE;
         dG = 1.0 + dF * dHV;
         dH = log( dG );
         dI = exp( dH * dC );
         dG = dC / dG;
         dC = dI * pML[i];
         dH *= dC;
         dG *= dC;
         pMu[i] = dC;
         pTmp[i] = dI;
         pTmpP[i] = dE * dH + dG * dHV * dD;
         pTmpT[i] = dB * dH + dG * ( dF + dHV * dA );
      }

      /* Derivatives */
      pTa = pDMuda;
      pTb = pDMLda;
      pTc = pDMRda;
      pTd = pDXda;
      for ( iNi = 0; iNi < m_iEosComp; iNi++ )
      {
         for ( i = 0; i < iM; i++ )
         {
            *pTa++ = pTmp[i] **pTb++ +pTmpP[i] **pTc++ +pTmpT[i] **pTd++;
         }
      }

      if ( iDrv >= EOS_DRV_P )
      {
         for ( i = 0; i < iM; i++ )
         {
            pDMudp[i] = pTmp[i] * pDMLdp[i] + pTmpP[i] * pDMRdp[i] + pTmpT[i] * pDXdp[i];
         }

         if ( iDrv >= EOS_DRV_T )
         {
            for ( i = 0; i < iM; i++ )
            {
               pDMudt[i] = pTmp[i] * pDMLdt[i] + pTmpP[i] * pDMRdt[i] + pTmpT[i] * pDXdt[i];
            }
         }
      }
   }

   /* Compute the terms without derivatives */
   else
   {
      for ( i = 0; i < iM; i++ )
      {
         dHV = pX[i];
         dG = log( pMR[i] );
         dB = dG / 2.30258509299405;
         dC = 2.5 + dB * ( dB - 2.0 ) * ( ( dHV > 0.1 ) ? ( 0.6 * dHV ) : 0.6 );
         pMu[i] = pML[i] * exp( log( 1.0 + ( exp( dG / dC ) - 1.0 ) * dHV ) * dC );
      }
   }
}


/* 
// Volume
//
// Routine to convert z factor to volume 
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
// iMultipleAbcT ** constant **
//    Indicator for multiple entries present in temperature tables 
// pAbcOffsetT ** constant **
//    Offset for temperature term in table for each object
// pP ** constant ** 
//    Pressure of each object
// pT ** constant ** 
//    Temperature of each object
// pX ** constant ** 
//    Mole fraction of each object
//       first index corresponds to object
//       second index corresponds to component
// pZ 
//    On input the z factor of each object
//    On output the molar volume of each object
// pDZdp 
//    On input pressure derivative of z factor of each object
//    On output pressure derivative of molar volume of each object
// pDZdt 
//    On input temperature derivative of z factor of each object
//    On output temperature derivative of molar volume of each object
// pDZdx 
//    On input overall mole derivative of z factor of each object
//    On output overall mole derivative of molar volume of each object
//       first index corresponds to object
//       second index corresponds to component
// pWorkT  
//    Work array
//
// 1) Given the z factor, the volume is
//
//    V = Z R T / P
// 
// 2) The volume translation routine is then called if required
*/
void EosPvtTable::Volume( int iM, int iDrv, int iMultipleAbcT, int *pAbcOffsetT, double *pP, double *pT, double *pX,
                          double *pZ, double *pDZdp, double *pDZdt, double *pDZdx, double *pWorkT )
{
   double *pTa;
   double  dA;
   double  dB;
   int     iNi;
   int     i;

   /* Multiple grid block case */
   if ( iM > 1 )
   {
      if ( iDrv )
      {
         for ( i = 0; i < iM; i++ )
         {
            dA = m_dGascon * pT[i] / pP[i];
            pZ[i] *= dA;
            pWorkT[i] = dA;
         }

         pTa = pDZdx;
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               *pTa *= pWorkT[i];
               pTa++;
            }
         }

         if ( iDrv >= EOS_DRV_P )
         {
            for ( i = 0; i < iM; i++ )
            {
               pDZdp[i] = pDZdp[i] * pWorkT[i] - pZ[i] / pP[i];
            }

            if ( iDrv >= EOS_DRV_T )
            {
               for ( i = 0; i < iM; i++ )
               {
                  pDZdt[i] = pDZdt[i] * pWorkT[i] + pZ[i] / pT[i];
               }
            }
         }
      }
      else
      {
         for ( i = 0; i < iM; i++ )
         {
            pZ[i] *= m_dGascon * pT[i] / pP[i];
         }
      }
   }

   /* Single grid block case */
   else
   {
      if ( iDrv )
      {
         dA = m_dGascon **pT / *pP;
         *pZ *= dA;
         dB = *pZ;
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            pDZdx[iNi] *= dA;
         }

         if ( iDrv >= EOS_DRV_P )
         {
            *pDZdp = *pDZdp * dA - dB / *pP;
            if ( iDrv >= EOS_DRV_T )
            {
               *pDZdt = *pDZdt * dA + dB / *pT;
            }
         }
      }
      else
      {
         *pZ *= m_dGascon **pT / *pP;
      }
   }

   /* Perform the volume translation */
   if ( m_iTranslate )
   {
      VolumeTranslation( iM, iDrv, iMultipleAbcT, pAbcOffsetT, pT, pX, pZ, pDZdt, pDZdx, pWorkT );
   }
}


/* 
// VolumeTranslation
//
// Routine to perform the volume translation  
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
// iMultipleAbcT ** constant **
//    Indicator for multiple entries present in temperature tables 
// pAbcOffsetT ** constant **
//    Offset for temperature term in table for each object
// pT ** constant ** 
//    Temperature of each object
// pX ** constant ** 
//    Mole fraction of each object
//       first index corresponds to object
//       second index corresponds to component
// pZ 
//    On input the molar volume of each object
//    On output the molar volume of each object
// pDZdp 
//    On input pressure derivative of molar volume of each object
//    On output pressure derivative of molar volume of each object
// pDZdt 
//    On input temperature derivative of molar volume of each object
//    On output temperature derivative of molar volume of each object
// pDZdx 
//    On input overall mole derivative of molar volume of each object
//    On output overall mole derivative of molar volume of each object
//       first index corresponds to object
//       second index corresponds to component
// pWorkT  
//    Work array
//
// 1) Perform volume translation
//
//    V = V - Sum C  X
//             i   i  i
//
// 2) Call VolumeTranslationC to get the C terms
// 3) The deriatives are straight forward
*/
void EosPvtTable::VolumeTranslation( int iM, int iDrv, int iMultipleAbcT, int *pAbcOffsetT, double *pT, double *pX,
                                     double *pZ, double *pDZdt, double *pDZdx, double *pWorkT )
{
   double *pCi;
   double *pDCidt;
   double *pD2Cidt2;
   double *pTa;
   double *pTb;
   double *pTc;
   double  dA;
   double  dB;
   int     iNi;
   int     i;

   /* Work locations */
   pCi = pWorkT + iM;
   pDCidt = pCi + iM * m_iEosComp;
   pD2Cidt2 = (double *)0;

   /* Get Ci terms */
   VolumeTranslationC( iM, iDrv, EOS_NOHEAT, iMultipleAbcT, pAbcOffsetT, pT, pCi, pDCidt, pD2Cidt2 );

   /* Multiple grid block case */
   if ( iM > 1 )
   {
      /* Case with derivatives */
      if ( iDrv )
      {
         pTa = pX;
         pTb = pDZdx;
         pTc = pCi;
         for ( i = 0; i < iM; i++ )
         {
            dA = *pTc++;
            pWorkT[i] = dA **pTa++;
            *pTb -= dA;
            pTb++;
         }

         for ( iNi = 1; iNi < m_iEosComp; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               dA = *pTc++;
               pWorkT[i] += dA **pTa++;
               *pTb -= dA;
               pTb++;
            }
         }

         /* Update function */
         for ( i = 0; i < iM; i++ )
         {
            pZ[i] -= pWorkT[i];
         }

         /* Take derivatives */
         pTa = pDZdx;
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               *pTa += pWorkT[i];
               pTa++;
            }
         }

         /* Temperature derivatives */
         if ( iDrv >= EOS_DRV_T )
         {
            pTa = pX;
            pTb = pDCidt;
            for ( iNi = 0; iNi < m_iEosComp; iNi++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  pDZdt[i] -= *pTa++ **pTb++;
               }
            }
         }
      }

      /* Code without derivatives */
      else
      {
         pTa = pX;
         pTb = pCi;
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               pZ[i] -= *pTa++ **pTb++;
            }
         }
      }
   }

   /* Single block case */
   else
   {
      /* Case with derivatives */
      if ( iDrv )
      {
         dA = pCi[0];
         dB = dA * pX[0];
         pDZdx[0] -= dA;
         for ( iNi = 1; iNi < m_iEosComp; iNi++ )
         {
            dA = pCi[iNi];
            dB += dA * pX[iNi];
            pDZdx[iNi] -= dA;
         }

         /* Update function */
         *pZ -= dB;

         /* Take derivatives */
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            pDZdx[iNi] += dB;
         }

         /* Temperature derivatives */
         if ( iDrv >= EOS_DRV_T )
         {
            pTa = pX;
            pTb = pDCidt;
            dA = pDCidt[0] * pX[0];
            for ( iNi = 1; iNi < m_iEosComp; iNi++ )
            {
               dA += pDCidt[iNi] * pX[iNi];
            }

            *pDZdt -= dA;
         }
      }

      /* No derivatives */
      else
      {
         dA = pCi[0] * pX[0];
         for ( iNi = 1; iNi < m_iEosComp; iNi++ )
         {
            dA += pCi[iNi] * pX[iNi];
         }

         *pZ -= dA;
      }
   }
}


/* 
// VolumeTranslationC
//
// Routine to get volume translation coefficients  
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
// iGetH
/     Indicator for heat calculations
// iMultipleAbcT ** constant **
//    Indicator for multiple entries present in temperature tables 
// pAbcOffsetT ** constant **
//    Offset for temperature term in table for each object
// pT ** constant ** 
//    Temperature of each object
// pCi  
//    Term for each component
// pDCidt
//    Temperature derivative of term for each component
// pD2Cidt2
//    Second derivative of term for each component
//
// 1) The volume translation term can depend upon the
//    temperature in the form
//
//    C  = C0 + C1  T
//     i     i    i
//
//    There are separate loops depending upon whether or
//    not there are multiple PVT tables
*/
void EosPvtTable::VolumeTranslationC( int iM, int iDrv, int iGetH, int iMultipleAbcT, int *pAbcOffsetT, double *pT,
                                      double *pCi, double *pDCidt, double *pD2Cidt2 )
{
   double *pTa;
   double *pTb;
   double *pTc;
   double *pTd;
   double *pTe;
   double  dA;
   double  dB;
   int     iNi;
   int     iNk;
   int     i;
   int     iTemp;

   /* Linear interpolation table */
   if ( m_iTypeOfC == EOS_LINEAR_C )
   {
      /* Single grid block case */
      if ( iM == 1 )
      {
         pTa = m_pCdata;
         pTb = m_pDCdtdata;
         dA = *pT;

         /* Second derivatives */
         if ( iGetH && iDrv >= EOS_DRV_T )
         {
            for ( iNi = 0; iNi < m_iEosComp; iNi++ )
            {
               dB = pTb[iNi];
               pCi[iNi] = pTa[iNi] + dA * dB;
               pDCidt[iNi] = dB;
               pD2Cidt2[iNi] = 0.0;
            }
         }

         /* Derivatives */
         else if ( iGetH || iDrv >= EOS_DRV_T )
         {
            for ( iNi = 0; iNi < m_iEosComp; iNi++ )
            {
               dB = pTb[iNi];
               pCi[iNi] = pTa[iNi] + dA * dB;
               pDCidt[iNi] = dB;
            }
         }

         /* No derivatives */
         else
         {
            for ( iNi = 0; iNi < m_iEosComp; iNi++ )
            {
               pCi[iNi] = pTa[iNi] + dA * pTb[iNi];
            }
         }
      }

      /* Multiple grid blocks */
      else
      {
         /* Second derivatives */
         if ( iGetH && iDrv >= EOS_DRV_T )
         {
            pTa = m_pCdata;
            pTb = m_pDCdtdata;
            pTc = pCi;
            pTd = pDCidt;
            pTe = pD2Cidt2;
            for ( iNi = 0; iNi < m_iEosComp; iNi++ )
            {
               dA = pTa[iNi];
               dB = pTb[iNi];
               for ( i = 0; i < iM; i++ )
               {
                  *pTc++ = dA + dB * pT[i];
                  *pTd++ = dB;
                  *pTe++ = 0.0;
               }
            }
         }

         /* Derivatives */
         else if ( iGetH || iDrv >= EOS_DRV_T )
         {
            pTa = m_pCdata;
            pTb = m_pDCdtdata;
            pTc = pCi;
            pTd = pDCidt;
            for ( iNi = 0; iNi < m_iEosComp; iNi++ )
            {
               dA = pTa[iNi];
               dB = pTb[iNi];
               for ( i = 0; i < iM; i++ )
               {
                  *pTc++ = dA + dB * pT[i];
                  *pTd++ = dB;
               }
            }
         }

         /* No derivatives */
         else
         {
            pTa = m_pCdata;
            pTb = m_pDCdtdata;
            pTc = pCi;
            for ( iNi = 0; iNi < m_iEosComp; iNi++ )
            {
               dA = pTa[iNi];
               dB = pTb[iNi];
               for ( i = 0; i < iM; i++ )
               {
                  *pTc++ = dA + dB * pT[i];
               }
            }
         }
      }
   }

   /* Single table */
   else if ( m_iTables == 0 )
   {
      /* Multiple grid blocks */
      if ( iM > 1 )
      {
         pTa = pCi;
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            dA = m_pCdata[iNi];
            for ( i = 0; i < iM; i++ )
            {
               *pTa++ = dA;
            }
         }

         /* Derivatives */
         if ( iGetH || iDrv >= EOS_DRV_T )
         {
            iNi = iM * m_iEosComp;
            for ( i = 0; i < iNi; i++ )
            {
               pDCidt[i] = 0.0;
            }
         }

         /* Second derivatives */
         if ( iGetH && iDrv >= EOS_DRV_T )
         {
            iNi = iM * m_iEosComp;
            for ( i = 0; i < iNi; i++ )
            {
               pD2Cidt2[i] = 0.0;
            }
         }
      }

      /* Single grid block case */
      else
      {
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            pCi[iNi] = m_pCdata[iNi];
         }

         /* Derivatives */
         if ( iGetH || iDrv >= EOS_DRV_T )
         {
            for ( iNi = 0; iNi < m_iEosComp; iNi++ )
            {
               pDCidt[iNi] = 0.0;
            }
         }

         /* Second derivatives */
         if ( iGetH && iDrv >= EOS_DRV_T )
         {
            for ( iNi = 0; iNi < m_iEosComp; iNi++ )
            {
               pD2Cidt2[iNi] = 0.0;
            }
         }
      }
   }

   /* Temperature dependent case */
   else
   {
      /* Single grid block case */
      if ( iM == 1 )
      {
         iTemp = *pAbcOffsetT;
         pTa = m_pCdata + iTemp;
         pTb = m_pDCdtdata + iTemp;
         dA = *pT;

         /* Second derivatives */
         if ( iGetH && iDrv >= EOS_DRV_T )
         {
            for ( iNi = 0; iNi < m_iEosComp; iNi++ )
            {
               dB = pTb[iNi];
               pCi[iNi] = pTa[iNi] + dA * dB;
               pDCidt[iNi] = dB;
               pD2Cidt2[iNi] = 0.0;
            }
         }

         /* Derivatives */
         else if ( iGetH || iDrv >= EOS_DRV_T )
         {
            for ( iNi = 0; iNi < m_iEosComp; iNi++ )
            {
               dB = pTb[iNi];
               pCi[iNi] = pTa[iNi] + dA * dB;
               pDCidt[iNi] = dB;
            }
         }

         /* No derivatives */
         else
         {
            for ( iNi = 0; iNi < m_iEosComp; iNi++ )
            {
               pCi[iNi] = pTa[iNi] + dA * pTb[iNi];
            }
         }
      }

      /* Summation term for multiple abc tables */
      else if ( iMultipleAbcT )
      {
         /* Second derivatives */
         if ( iGetH && iDrv >= EOS_DRV_T )
         {
            for ( i = 0; i < iM; i++ )
            {
               iNk = i;
               dA = pT[i];
               iTemp = pAbcOffsetT[i];
               pTa = m_pCdata + iTemp;
               pTb = m_pDCdtdata + iTemp;
               for ( iNi = 0; iNi < m_iEosComp; iNi++ )
               {
                  dB = pTb[iNi];
                  pCi[iNk] = pTa[iNi] + dA * dB;
                  pDCidt[iNk] = dB;
                  pD2Cidt2[iNk] = 0.0;
               }
            }
         }

         /* Derivatives */
         else if ( iGetH || iDrv >= EOS_DRV_T )
         {
            for ( i = 0; i < iM; i++ )
            {
               iNk = i;
               dA = pT[i];
               iTemp = pAbcOffsetT[i];
               pTa = m_pCdata + iTemp;
               pTb = m_pDCdtdata + iTemp;
               for ( iNi = 0; iNi < m_iEosComp; iNi++ )
               {
                  dB = pTb[iNi];
                  pCi[iNk] = pTa[iNi] + dA * dB;
                  pDCidt[iNk] = dB;
               }
            }
         }

         /* No derivatives */
         else
         {
            for ( i = 0; i < iM; i++ )
            {
               iNk = i;
               dA = pT[i];
               iTemp = pAbcOffsetT[i];
               pTa = m_pCdata + iTemp;
               pTb = m_pDCdtdata + iTemp;
               for ( iNi = 0; iNi < m_iEosComp; iNi++ )
               {
                  pCi[iNk] = pTa[iNi] + dA * pTb[iNi];
               }
            }
         }
      }

      /* Single table */
      else
      {
         /* Second derivatives */
         if ( iGetH && iDrv >= EOS_DRV_T )
         {
            iTemp = *pAbcOffsetT;
            pTa = m_pCdata + iTemp;
            pTb = m_pDCdtdata + iTemp;
            pTc = pCi;
            pTd = pDCidt;
            pTe = pD2Cidt2;
            for ( iNi = 0; iNi < m_iEosComp; iNi++ )
            {
               dA = pTa[iNi];
               dB = pTb[iNi];
               for ( i = 0; i < iM; i++ )
               {
                  *pTc++ = dA + dB * pT[i];
                  *pTd++ = dB;
                  *pTe++ = 0.0;
               }
            }
         }

         /* Derivatives */
         else if ( iGetH || iDrv >= EOS_DRV_T )
         {
            iTemp = *pAbcOffsetT;
            pTa = m_pCdata + iTemp;
            pTb = m_pDCdtdata + iTemp;
            pTc = pCi;
            pTd = pDCidt;
            for ( iNi = 0; iNi < m_iEosComp; iNi++ )
            {
               dA = pTa[iNi];
               dB = pTb[iNi];
               for ( i = 0; i < iM; i++ )
               {
                  *pTc++ = dA + dB * pT[i];
                  *pTd++ = dB;
               }
            }
         }

         /* No derivatives */
         else
         {
            iTemp = *pAbcOffsetT;
            pTa = m_pCdata + iTemp;
            pTb = m_pDCdtdata + iTemp;
            pTc = pCi;
            for ( iNi = 0; iNi < m_iEosComp; iNi++ )
            {
               dA = pTa[iNi];
               dB = pTb[iNi];
               for ( i = 0; i < iM; i++ )
               {
                  *pTc++ = dA + dB * pT[i];
               }
            }
         }
      }
   }
}


/* 
// WilsonKValues
// 
// Wilson K-values  
//
// iM ** constant **
//    Number of objects
// iStretch ** constant **
//    Indicator whether to return ideal K values as is or whether
//    to modify them
// pT ** constant **
//    Temperature of each object
// pKValue
//    Ideal K value
// pWorkT
//    Work vector
//
// 1) The ideal Wilson K value is defined as
//
//    K  = Pc  * exp ( 5.42 * ( 1 - Tc  / T ) ) / P
//     i     i                        i
//
//    The pressure is not used since it is the same for
//    all components
//
// 2) Since these values are used for initial guesses
//    for the flasher, a problem may arise if they are
//    too close together.  Thus, if iStretch is one,
//    the K values are adjusted so that the ratio between
//    the minimum and maximum is at least EOS_KVALUE_SPECTRUM
*/
void EosPvtTable::WilsonKValues( int iM, int iStretch, double *pT, double *pKValue, double *pWorkT )
{
   double  dA;
   double  dB;
   double  dC;
   double  dD;
   double *pTa;
   double *pTb;
   double *pMin;
   double *pMax;
   int     iNi;
   int     i;

   /* Multiple grid block store ideal K values */
   if ( iM > 1 )
   {
      pMin = pWorkT;
      pMax = pMin + iM;
      pTa = pKValue;
      pTb = pKValue + iM * ( m_iEosComp - 1 );
      dA = m_pCriticalPressure[0];
      dB = m_pCriticalTemperature[0];
      for ( i = 0; i < iM; i++ )
      {
         pTb[i] = 5.42 / pT[i];
         pTa[i] = 5.42 - dB * pTb[i];
      }

      EosUtil::VectorExp( iM, pTa, pTa );
      for ( i = 0; i < iM; i++ )
      {
         dC = dA **pTa;
         pMin[i] = dC;
         pMax[i] = dC;
         *pTa++ = dC;
      }

      for ( iNi = 1; iNi < m_iEosComp; iNi++ )
      {
         dA = m_pCriticalPressure[iNi];
         dB = m_pCriticalTemperature[iNi];
         for ( i = 0; i < iM; i++ )
         {
            pTa[i] = 5.42 - dB * pTb[i];
         }

         EosUtil::VectorExp( iM, pTa, pTa );
         for ( i = 0; i < iM; i++ )
         {
            dC = dA **pTa;
            dD = pMin[i];
            pMin[i] = dD < dC ? dD : dC;
            dD = pMax[i];
            pMax[i] = dD > dC ? dD : dC;
            *pTa++ = dC;
         }
      }

      /* Get ratio */
      if ( iStretch )
      {
         for ( i = 0; i < iM; i++ )
         {
            dA = pMax[i] / pMin[i] / EOS_KVALUE_SPECTRUM;
            pMin[i] = dA > 1.0 ? 1.0 : 1.0 / dA;
         }

         /* Adjust k values */
         pTa = pKValue;
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               dA = *pTa;
               *pTa++ = ( dA < pMax[i] ? dA : dA * pMin[i] );
            }
         }
      }
   }

   /* Single grid block */
   else
   {
      dA = 5.42 / *pT;
      for ( iNi = 0; iNi < m_iEosComp; iNi++ )
      {
         pKValue[iNi] = 5.42 - dA * m_pCriticalTemperature[iNi];
      }

      EosUtil::VectorExp( m_iEosComp, pKValue, pKValue );
      dB = m_pCriticalPressure[0] * pKValue[0];
      dC = dB;
      pKValue[0] = dB;
      for ( iNi = 1; iNi < m_iEosComp; iNi++ )
      {
         dD = m_pCriticalPressure[iNi] * pKValue[iNi];
         dB = dB < dD ? dB : dD;
         dC = dC > dD ? dC : dD;
         pKValue[iNi] = dD;
      }

      /* Get ratio */
      if ( iStretch )
      {
         dA = dC / dB / EOS_KVALUE_SPECTRUM;
         dB = dA > 1.0 ? 1.0 : 1.0 / dA;

         /* Adjust k values */
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            dA = pKValue[iNi];
            pKValue[iNi] = ( dA < dC ? dA : dA * dB );
         }
      }
   }
}


/*
// Set the work array size
//
// iM - Number of objects to calculate (input)
*/
int EosPvtTable::ViscosityWorkArraySize( int iM )
{
   return ( iM * ( ( m_iEosComp > 41 ? m_iEosComp + 8 : 49 ) + 3 * m_iEosComp ) );
}


/* 
// Viscosity calculations for the corresponding state model
//
// iM - Number of objects to calculate (input)
// iMolar - Indicator if using mole fractions (input)
//          1 means using mole fractions; else 0
// pP - Pressure (input)
// pT - Temperature (input)
// pZ - Composition (input)
// pViscosity - viscosity (output)
// pDViscosityDX - derivative of viscosity wrt composition (output)
//                 not calculated if NULL
// pDViscosityDP - derivative of viscosity wrt pressure (output)
//                 not calculated if NULL
// pDViscosityDT - derivative of viscosity wrt temperature (output)
//                 not calculated if NULL
// pWork - Work array 
// Depending on value of m_CSVscMethod, different correlations are invoked
// m_CSVscMethod = 0, originally implemented 2 parameter CS model
// m_CSVscMethod = 1, newer 4 parameter CS model using PVTSim/Shell heavy oil JIP formulation
// m_CSVscMethod = 2, newer 4 parameter CS model using PVTSim heavy oil non JIP formulation
// Annotated formulae uses PVTSim's methods documentation notation
*/
void EosPvtTable::ViscosityCS( int iM, int iMolar, double *pP, double *pT, double *pZ, double *pViscosity,
                               double *pDViscosityDX, double *pDViscosityDP, double *pDViscosityDT, double *pWork )
{
   double *pTa;
   double *pTb;
   double *pTc;
   double *pTd;
   double  dA;
   double  dB;
   double  dC;
   double  dD;
   double  dE;
   double  dF;
   double  dG;
   int     iNi;
   int     iNj;
   int     i = iM * m_iEosComp;

   /* Work arrays */
   double *pWrk1 = pWork;
   double *pWrk2 = pWrk1 + iM;
   double *pWrk3 = pWrk2 + iM;
   double *pSum1 = pWrk3 + iM;
   double *pSum2 = pSum1 + iM;
   double *pSum3 = pSum2 + iM;
   double *pDSum1 = pSum3 + iM;
   double *pDSum2 = pDSum1 + i;
   double *pDSum3 = pDSum2 + i;
   double *pSum4 = pDSum3 + i;
   double *pWrk4 = pSum4 + iM;

   double *Mn = NULL;
   double *Mw = NULL;      
   double *VscH = NULL;    
   double *dVscHdMn = NULL;
   double *dVscHdMw = NULL;
   double *dVscHdZ = NULL;
   double *dVscHdP = NULL;
   double *dVscHdT = NULL;
   double *Tmeth = NULL;   
   double *dTmethdT = NULL;   
   double *dTmethdP = NULL;   
   double *dTmethdMWmix = NULL;   
   double *dTmethdZ = NULL;   

   /* Derivative value */
   int     iDrv = pDViscosityDX ? ( pDViscosityDP ? ( pDViscosityDT ? 3 : 2 ) : 1 ) : 0;

   if ( m_CSVscMethod )
   {
      Mn = new double[iM];
      Mw = new double[iM];      
      Tmeth = new double[iM];   
       
      if (iDrv >= EOS_DRV_N )
      {
         dTmethdMWmix = new double[iM]; 
         dTmethdZ = new double[i]; 
         dTmethdP = new double[iM];  
         dTmethdT = new double[iM];  
      }
   }

   /* Modified PC and TC terms */
   double *pTy = m_pPCMod; // ( 0.5*( Tci / Pci )^(1/3) + 0.5*( Tcj / Pcj )^(1/3)  )^3
   double *pTz = m_pTCMod; // sqrt(Tci * Tcj) 

   /* Calculate the sum with mole fractions */
   if ( iMolar )
   {
      /* Calculate the sum with derivatives */
      if ( iDrv )
      {
         pTa = pDSum1;
         pTb = pDSum2;
         pTc = pZ;                             // Zi 
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            pTd = pZ;                          // Zi 

            /* Initialization */
            if ( iNi == 0 )
            {
               dA = *pTy++;                    // ( 0.5*( Tci / Pci )^(1/3) + 0.5*( Tcj / Pcj )^(1/3)  )^3:
               dB = *pTz++;                    // sqrt(Tci * Tcj) 
               dF = m_pMolecularWeight[0]; // MW0
               dG = dF * dF;                   // Mw0*MW0
               for ( i = 0; i < iM; i++ )
               {
                  dC = pTd[i];                 // Zi
                  dD = dA * dC;                // ( 0.5*( Tci / Pci )^(1/3) + 0.5*( Tcj / Pcj )^(1/3)  )^3 * Zi
                  dE = dB * dD;                         
                  pSum1[i] = dE * dC;
                  pSum2[i] = dD * dC;
                  pSum3[i] = dG * dC;          // MW0*MW0*Zi
                  pSum4[i] = dF * dC;
                  pTa[i] = dE;
                  pTb[i] = dD;
               }
            }
            else
            {
               dA = *pTy++;
               dB = *pTz++;
               dF = m_pMolecularWeight[iNi];  // MWi
               dG = dF * dF;                      // MWi*MWi
               for ( i = 0; i < iM; i++ )
               {
                  dC = pTd[i];                    // Zi 
                  dD = dA * dC;
                  dE = dB * dD;
                  dC = pTc[i];
                  pSum1[i] += dE * dC;
                  pSum2[i] += dD * dC;
                  pSum3[i] += dG * dC;            // sum ( MWi*MWi*Zi )
                  pSum4[i] += dF * dC;            // Mn = sum ( MWi*Zi )
                  pTa[i] = dE;
                  pTb[i] = dD;
               }
            }

            /* Inner loop */
            for ( iNj = 1; iNj < m_iEosComp; iNj++ )
            {
               dA = *pTy++;
               dB = *pTz++;
               pTd += iM;
               for ( i = 0; i < iM; i++ )
               {
                  dC = pTd[i];
                  dD = dA * dC;
                  dE = dB * dD;
                  dC = pTc[i];
                  pSum1[i] += dE * dC;
                  pSum2[i] += dD * dC;
                  pTa[i] += dE;
                  pTb[i] += dD;
               }
            }

            /* Update pointers */
            pTa += iM;
            pTb += iM;
            pTc += iM;
         }

         /* Set the terms */
         for ( i = 0; i < iM; i++ )
         {
            dA = 1.0 / pSum2[i];
            dB = 1.0 / pSum4[i];         // 1 / sum( MWi*Zi )
            pSum1[i] *= dA;
            pSum2[i] = dA * pSum1[i];
            pSum3[i] *= dB;              // Mw = sum ( MWi*MWi*Zi ) / sum( MWi*Zi )
            pWrk4[i] = dA * dA;
            pWrk1[i] = dB;               // 1 / Mn
            pWrk2[i] = dA;
         }

         /* Derivative terms */
         pTa = pDSum1;
         pTb = pDSum2;
         pTc = pDSum3;
         pTd = pDViscosityDX;
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            dF = m_pMolecularWeight[iNi];              // MWi
            dG = dF * dF;                                  // MWi*MWi
            for ( i = 0; i < iM; i++ )
            {
               dA = pTb[i] * pSum1[i];
               dB = pTa[i] - dA;
               dC = dB * pWrk2[i];
               dD = ( dB - dA ) * pWrk4[i];
               pTa[i] = dC + dC;  
               pTb[i] = dD + dD;
               pTc[i] = ( dG - dF * pSum3[i] ) * pWrk1[i]; // dMwdzi = ( MWi^2 - MWi*Mw ) / Mn
               pTd[i] = dF;                                // dMndZ = MWi
            }

            pTa += iM;
            pTb += iM;
            pTc += iM;
            pTd += iM;
         }
      }

      /* Calculate the sum without derivatives */
      else
      {
         pTc = pZ;                                        // Zi
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            pTd = pZ;                                     // Zi

            /* Initialization */
            if ( iNi == 0 )
            {
               dA = *pTy++;                               // pPCMod, i=j=0
               dB = *pTz++;                               // pTCMod i=j=0 
               dF = m_pMolecularWeight[0];            // MW0
               dG = dF * dF;                              // MW0^2
               for ( i = 0; i < iM; i++ )
               {
                  dC = pTc[i];                            // Z0
                  dD = dA * dC * dC;                      // pPCMod * Z0 * Z0, i=j=0
                  pSum1[i] = dD * dB;                     // pPCMod * Z0 * Z0 * pTCMod, i=j=0
                  pSum2[i] = dD;                          // pPCMod * Z0 * Z0, i=j=0
                  pSum3[i] = dG * dC;                     // MW0^2 * Z0
                  pSum4[i] = dF * dC;                     // MW0 * Z0
               }
            }
            else
            {
               dA = *pTy++;                               // pPCMod
               dB = *pTz++;                               // pTCMod 
               dF = m_pMolecularWeight[iNi];          // MWi
               dG = dF * dF;                              // MWi^2
               for ( i = 0; i < iM; i++ )
               {
                  dC = pTc[i];                            // Zi
                  dD = dA * dC * pTd[i];                  // pPCMod * Zi * Zi
                  pSum1[i] += dD * dB;                    // Sum_i( pPCMod * Zi * Zi * pTCMod )
                  pSum2[i] += dD;                         // Sum_i( pPCMod * Zi * Zi )
                  pSum3[i] += dG * dC;                    // Sum_i( MWi^2 * Zi )
                  pSum4[i] += dF * dC;                    // Mn = Sum_i( MWi * Zi )
               }
            }

            /* Inner loop */
            for ( iNj = 1; iNj < m_iEosComp; iNj++ )
            {
               dA = *pTy++;                               // pPCMod
               dB = *pTz++;                               // pTCMod
               pTd += iM;                                  
               for ( i = 0; i < iM; i++ )
               {
                  dD = dA * pTd[i] * pTc[i];              // pPCMod * Zj *  Zi
                  pSum1[i] += dD * dB;                    // Sum_ij(pPCMod * Zj * Zi * pTCMod), Tcmix numerator
                  pSum2[i] += dD;                         // Sum_ij(pPCMod * Zj * Zi), Tcmix denominator  
               }
            }

            /* Update pointers */
            pTc += iM;                                    
         }

         /* Set the terms */
         for ( i = 0; i < iM; i++ )
         {
            dA = 1.0 / pSum2[i];               // 1 / den Tcmix 
            dB = dA * pSum1[i];                // Tcmix = num Tcmix / den Tcmix  
            pSum2[i] = dA * dB;                // Pcmix = Tcmix / den Tcmix
            pSum1[i] = dB;                     // Tcmix 
            pSum3[i] = pSum3[i] / pSum4[i];    // Mw = Sum_i(Mwi^2 * Zi) / Mwn
         }
      }
   }

   /* Calculate the sum with mass fractions */
   else
   {
      /* Calculate the sum with derivatives */
      if ( iDrv )
      {
         pTa = pDSum1;
         pTb = pDSum2;
         pTc = pZ;
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            pTd = pZ;
            dF = m_pMolecularWeight[iNi];
            dG = 1.0 / dF;

            /* Initialization */
            if ( iNi == 0 )
            {
               dA = dG * dG **pTy++;
               dB = *pTz++;
               for ( i = 0; i < iM; i++ )
               {
                  dC = pTd[i];
                  dD = dA * dC;
                  dE = dB * dD;
                  pSum1[i] = dE * dC;
                  pSum2[i] = dD * dC;
                  pSum3[i] = dF * dC;
                  pSum4[i] = dG * dC;
                  pTa[i] = dE;
                  pTb[i] = dD;
               }
            }
            else
            {
               dA = dG **pTy++ / m_pMolecularWeight[0];
               dB = *pTz++;
               for ( i = 0; i < iM; i++ )
               {
                  dC = pTd[i];
                  dD = dA * dC;
                  dE = dB * dD;
                  dC = pTc[i];
                  pSum1[i] += dE * dC;
                  pSum2[i] += dD * dC;
                  pSum3[i] += dF * dC;
                  pSum4[i] += dG * dC;
                  pTa[i] = dE;
                  pTb[i] = dD;
               }
            }

            /* Inner loop */
            for ( iNj = 1; iNj < m_iEosComp; iNj++ )
            {
               dA = dG **pTy++ / m_pMolecularWeight[iNj];
               dB = *pTz++;
               pTd += iM;
               for ( i = 0; i < iM; i++ )
               {
                  dC = pTd[i];
                  dD = dA * dC;
                  dE = dB * dD;
                  dC = pTc[i];
                  pSum1[i] += dE * dC;
                  pSum2[i] += dD * dC;
                  pTa[i] += dE;
                  pTb[i] += dD;
               }
            }

            /* Update pointers */
            pTa += iM;
            pTb += iM;
            pTc += iM;
         }

         /* Set the terms */
         for ( i = 0; i < iM; i++ )
         {
            dA = 1.0 / pSum2[i];
            dC = pSum4[i];
            dB = 1.0 / dC;
            dD = pSum1[i] * dA;
            dE = dC * dC * dA;
            pSum1[i] = dD;
            pSum2[i] = dE * dD;
            pSum4[i] = dB;
            pWrk4[i] = dA;
            pWrk1[i] = dE * dA;
            pWrk2[i] = dB * dB;
            pWrk3[i] = dC * dA * dD;
         }

         /* Derivative terms */
         pTa = pDSum1;
         pTb = pDSum2;
         pTc = pDSum3;
         pTd = pDViscosityDX;
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            dE = m_pMolecularWeight[iNi];
            dF = 1.0 / dE;
            dG = dF + dF;
            for ( i = 0; i < iM; i++ )
            {
               dA = pTb[i] * pSum1[i];
               dB = pTa[i] - dA;
               dC = dB * pWrk4[i];
               dD = ( dB - dA ) * pWrk1[i];
               pTa[i] = dC + dC;
               pTb[i] = dD + dD + dG * pWrk3[i];
               pTc[i] = dE;                       // dMwdWi = MWi
               pTd[i] = -dF * pWrk2[i];           // dMndWi = -1 / MWi / Mn^2
            }

            pTa += iM;
            pTb += iM;
            pTc += iM;
            pTd += iM;
         }
      }

      /* Calculate the sum without derivatives */
      else
      {
         pTc = pZ;                                         // Wi (mass)
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            pTd = pZ;                                      // Wi 
            dF = m_pMolecularWeight[iNi];              // MWi 
            dG = 1.0 / dF;                                 // 1/MWi 

            /* Initialization */
            if ( iNi == 0 )
            {
               dA = dG * dG **pTy++;                       // pPCMod / MWi^2, i=j=0
               dB = *pTz++;                                // pTCMod, i=j=0
               for ( i = 0; i < iM; i++ )
               {
                  dC = pTd[i];                             // W0
                  dD = dA * dC * dC;                       // pPCMod * W0 * W0
                  pSum1[i] = dD * dB;
                  pSum2[i] = dD;
                  pSum3[i] = dF * dC;
                  pSum4[i] = dG * dC;                      // W0 / MW0
               }
            }
            else
            {
               dA = dG **pTy++ / m_pMolecularWeight[0];
               dB = *pTz++;
               for ( i = 0; i < iM; i++ )
               {
                  dC = pTc[i];                              // Wi
                  dD = dA * dC * pTd[i];
                  pSum1[i] += dD * dB;
                  pSum2[i] += dD;
                  pSum3[i] += dF * dC;                      // Mw = sum ( MWi * Wi )
                  pSum4[i] += dG * dC;                      // 1/Mn = sum( Wi / MWi )
               }
            }

            /* Inner loop */
            for ( iNj = 1; iNj < m_iEosComp; iNj++ )
            {
               dA = dG **pTy++ / m_pMolecularWeight[iNj];
               dB = *pTz++;
               pTd += iM;
               for ( i = 0; i < iM; i++ )
               {
                  dC = pTc[i];
                  dD = dA * dC * pTd[i];
                  pSum1[i] += dD * dB;
                  pSum2[i] += dD;
               }
            }

            /* Update pointers */
            pTc += iM;
         }

         /* Set the terms */
         for ( i = 0; i < iM; i++ )
         {
            dA = 1.0 / pSum2[i];
            dC = pSum4[i];
            dD = pSum1[i] * dA;
            pSum1[i] = dD;
            pSum2[i] = dC * dC * dA * dD;
            pSum4[i] = 1.0 / dC; // Mn = 1 / sum( Wi / MWi )
         }
      }
   }
   // copy Mn and Mw into arrays for use with heavy viscosity model later
   if ( m_CSVscMethod ) 
   {
      for ( i = 0; i < iM; i++ )
      {
         Mw[i] = pSum3[i];
         Mn[i] = pSum4[i];
      }
   }

   /* Form the effective molecular weight of the mixture */
   if ( iDrv )
   {
      dG = m_dVTerm2;                          // Visfac2
      dE = dG - 1.0;                               // Visfac2 - 1
      dF = m_dVTerm1;                          // Visfac1
      dG *= dF;                                    // Visfac2 * Visfac1
      EosUtil::VectorPow( iM, pSum4, dE, pWrk1 ); // pWrk1 = (Mn)^(Visfac2-1)
      EosUtil::VectorPow( iM, pSum3, dE, pWrk2 ); // pWrk2 = (Mw)^(Visfac2-1) 
      for ( i = 0; i < iM; i++ )
      {
         dA = pSum4[i];                            // Mn[i]
         // In the line below, pSum3 = MWmix = Mn + Visfac1*( Mw*Mw^(Visfac2-1) -Mn*(Mn)^(Visfac2-1) )
         pSum3[i] = dA + dF * ( pSum3[i] * pWrk2[i] - dA * pWrk1[i] ); 
         pWrk1[i] *= dG;                           // (Mn)^(Visfac2-1) * Visfac2 * Visfac1 
         pWrk2[i] *= dG;                           // (Mw)^(Visfac2-1) * Visfac2 * Visfac1
      }

      pTa = pDSum3;                // dMwdZi or dMwdWi
      pTb = pDViscosityDX;         // dMndZi or dMndWi 
      for ( iNi = 0; iNi < m_iEosComp; iNi++ )
      {
         for ( i = 0; i < iM; i++ )
         {
            dA = pTb[i];
            pTa[i] = dA + ( pWrk2[i] * pTa[i] - pWrk1[i] * dA ); // pDSum3 = dMWmixdZi or dMWmixdWi
         }

         pTa += iM;
         pTb += iM;
      }
   }
   else
   {
      EosUtil::VectorPow( iM, pSum4, m_dVTerm2, pWrk1 );   // Mn^2.303
      EosUtil::VectorPow( iM, pSum3, m_dVTerm2, pWrk2 );   // Mw^2.303
      for ( i = 0; i < iM; i++ )
      {
         // In the line below, MWmix = 1.3404e-4 * ( Mw^2.303 - Mn^2.303 ) + Mn  
         pSum3[i] = pSum4[i] + m_dVTerm1 * ( pWrk2[i] - pWrk1[i] );
      }
   }

   /* Set terms for the derivatives */
   pTa = pDViscosityDX ? pDViscosityDP : NULL;  // pTa = DVscDP
   pTb = pTa ? pDViscosityDT : NULL;            // pTb = DVscDT
   pTc = pDViscosityDX ? pWrk3 : NULL;          // pTc = pWrk3

   /* Perform the calculation */
   //  pTa = DVscDp, pTb = DVscDT, pWrk2 = DVscDPcmix, pWrk1 = DVscDTcmix, pTc = DVscDMWmix
   CalculateViscosity( iM, pP, pT, pSum2, pSum1, pSum3, pViscosity, pTa, pTb, 
                       pWrk2, pWrk1, pTc, pSum4, Tmeth, dTmethdT, dTmethdP, dTmethdMWmix );

   /* Set the derivative terms */
   if ( iDrv )
   {
      pTa = pDSum1; // dTcmixdzi
      pTb = pDSum2; // dPcmixdZi
      pTc = pDSum3; // dMWmixdZi
      pTd = pDViscosityDX;
      
      for ( iNi = 0; iNi < m_iEosComp; iNi++ )
      {
         for ( i = 0; i < iM; i++ )
         {
            // In the line below, dVscdX = dVscdTcmix * dTcmixdZ + dVscdPcmix * dPcmixdZ + dVscdMWmix * dMWmixdZ 
            pTd[i] = pWrk1[i] * pTa[i] + pWrk2[i] * pTb[i] + pWrk3[i] * pTc[i];

            if ( m_CSVscMethod && iDrv >= EOS_DRV_N )
            {
               // In the line below, dTmethdZ = dTmethdMWmix * dMWmixdZ 
               //                             + dTmethdTmeth0 * dTmeth0/dTcmix * dTcmix/dzi
               //                             + dTmethdPmeth0 * dPmeth0/dPcmix * dPcmix/dzi 
               dTmethdZ[i+iNi*iM] = dTmethdMWmix[i] * pTc[i]
                                  - dTmethdT[i] * pT[i]*EOS_VCSTC*pSum1[i]*pSum1[i] * pTa[i]
                                  - dTmethdP[i] * pP[i]*EOS_VCSPC*pSum2[i]*pSum2[i] * pTb[i];
            }
         }
         pTa += iM;
         pTb += iM;
         pTc += iM;
         pTd += iM;
      }
   }

   if ( m_CSVscMethod )
   {

      // convert dTmethdTmeth0 and dTmethdPmeth0 into dTmethdT and dTmethdP
      if ( iDrv >= EOS_DRV_P )
      {
         for ( i = 0; i < iM; i++ )
         {
            dTmethdP[i] = dTmethdP[i]*EOS_VCSPC*pSum2[i];   // dTmethdP = dTmethdPmeth0 * dPmeth0dP (pSum2 = 1/Pcmix)
            if ( iDrv >= EOS_DRV_T)
            {
               dTmethdT[i] = dTmethdT[i]*EOS_VCSTC*pSum1[i]; // dTmethdT = dTmethdTmeth0 * dTmeth0dP (pSum1 = 1/Tcmix)
            }
         }
      }

      VscH = pWrk1; 
      dVscHdT = (iDrv >= EOS_DRV_T) ? pWrk2 : NULL; 
      dVscHdP = (iDrv >= EOS_DRV_P) ? pWrk3 : NULL; 
      dVscHdMn = (iDrv >= EOS_DRV_N) ? pSum1 : NULL;
      dVscHdMw = (iDrv >= EOS_DRV_N) ? pSum2 : NULL;
      dVscHdZ = (iDrv >= EOS_DRV_N) ? pDSum1 : NULL;

      VscCorrHeavy( iM, iDrv, pT, pP, Mn, Mw, VscH, dVscHdT, dVscHdP, dVscHdMn, dVscHdMw);

      if ( iDrv >= EOS_DRV_N )  
      {
         double dMndZ=0.0, dMwdZ = 0.0;
         for ( iNi = 0; iNi < m_iEosComp; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               if (iMolar) // Z is mole fraction
               {
                  dMndZ = m_pMolecularWeight[iNi]; 
                  dMwdZ = dMndZ / Mn[i] * ( dMndZ - Mw[i] );
               }
               else // Z is mass fraction
               {
                  dMwdZ = m_pMolecularWeight[iNi];
                  dMndZ = -1.0 / dMwdZ * pow(Mn[i],-2.0);
               }
               dVscHdZ[i+iNi*iM] = dVscHdMw[i]*dMwdZ + dVscHdMn[i]*dMndZ;
            }
         }
      }

      // now form complete viscosity from CS and Heavy parts
      for ( i = 0; i < iM; i++ )
      {

         if ( Tmeth[i] < 50.0 ) // switch to only heavy model
         {
            pViscosity[i] = VscH[i];
            if ( iDrv >= EOS_DRV_N )
            {
               for ( iNi = 0; iNi < m_iEosComp; iNi++ )
               {
                  pDViscosityDX[i+iNi*iM] = dVscHdZ[i+iNi*iM];
               }
               if ( iDrv >= EOS_DRV_P )
               {
                  pDViscosityDP[i] = dVscHdP[i];
                  if ( iDrv >= EOS_DRV_T )
                  {
                     pDViscosityDT[i] = dVscHdT[i];
                  }
               }
            }
         }

         else if ( Tmeth[i]>=50.0 && Tmeth[i]<= 75.0 ) // take mix of heavy and CS models
         {
            const double Coeff = 0.2;
            const double DT = Coeff*(Tmeth[i]-70);
            const double HTAN = ( exp(DT) - exp(-DT) ) / ( exp(DT) + exp(-DT) );
            const double F1 = 0.5*(HTAN + 1.0);
            const double F2 = 0.5*(1.0 - HTAN);
            pViscosity[i] = F1 * pViscosity[i] + F2 * VscH[i];
            if ( iDrv >= EOS_DRV_N )
            {
               const double dHTANdDT = 1.0 - HTAN*HTAN;  
               const double dF1dTmeth = 0.5 * dHTANdDT * Coeff; // dF2dTmeth = - dF1dTmeth

               for ( iNi = 0; iNi < m_iEosComp; iNi++ )
               {
                  int index = i + iNi*iM;
                  pDViscosityDX[index] = F1 * pDViscosityDX[index]  
                                         + dF1dTmeth * dTmethdZ[index] * pViscosity[i]  
                                         + F2 * dVscHdZ[index] 
                                         - dF1dTmeth * dTmethdZ[index] * VscH[i];
               }
               if ( iDrv >= EOS_DRV_P)
               {
                  pDViscosityDP[i] = F1 * pDViscosityDP[i] 
                                     + dF1dTmeth * dTmethdP[i] * pViscosity[i]  
                                     + F2 * dVscHdP[i] 
                                     - dF1dTmeth * dTmethdP[i] * VscH[i];
                  if ( iDrv >= EOS_DRV_T)
                  {
                     pDViscosityDT[i] = F1 * pDViscosityDT[i] 
                                        + dF1dTmeth * dTmethdT[i] *pViscosity[i] 
                                        + F2 * dVscHdT[i] 
                                        - dF1dTmeth * dTmethdT[i] * VscH[i];
                  }
               }
            }
         }

      }

      delete [] Mn; 
      delete [] Mw;
      delete [] Tmeth;
      delete [] dTmethdT;
      delete [] dTmethdP;
      delete [] dTmethdMWmix;
      delete [] dTmethdZ;
   } //    end heavy oil viscosity 
}


/*
// Initialize the terms for the corresponding state viscosity model
*/
void EosPvtTable::InitializeViscosityTerms( void )
{
   double        dA;
   double       *pTy;
   double       *pTz;
   double       *pPCTemp;
   double       *pTCTemp;
   const double  d1 = 1.0 / 3.0;
   int           iNi;
   int           iNj;

   /* Allocate the memory */
   pPCTemp = CNEW ( double, m_iEosComp +m_iEosComp );
   pTCTemp = pPCTemp + m_iEosComp;

   /* Store molecular weights and compute other terms */
   for ( iNi = 0; iNi < m_iEosComp; iNi++ )
   {
      dA = m_pCriticalTemperature[iNi];
      pPCTemp[iNi] = 0.5 * pow( ( dA / m_pCriticalPressure[iNi] ), d1 ); // 0.5*( Tc / Pc )^(1/3)
      pTCTemp[iNi] = sqrt( dA );     // sqrt(Tc)
   }

   /* Store the viscosity terms */
   pTy = m_pPCMod;
   pTz = m_pTCMod;
   for ( iNi = 0; iNi < m_iEosComp; iNi++ )
   {
      for ( iNj = 0; iNj < m_iEosComp; iNj++ )
      {
         // In the line below, m_pPCMod = ( 0.5*( Tci / Pci )^(1/3) + 0.5*( Tcj / Pcj )^(1/3)  )^3
         *pTy++ = pow( ( pPCTemp[iNi] + pPCTemp[iNj] ), 3.0 ); 
         *pTz++ = pTCTemp[iNi] * pTCTemp[iNj]; // sqrt(Tci * Tcj)
      }
   }

   /* Delete the memory */
   CDELETE( pPCTemp );
}


/* 
// Get the reference viscosity and its derivatives
//
// iM - Number of objects to calculate (input)
// pP - Pressure (input)
// pT - Temperature (input)
// pPMix - Mixture critical pressure (input)
// pTMix - Mixture critical temperature (input)
// pMWMix - Mixture molecular weight (input)
// pViscosity - Viscosity (output)
// pDViscosityDP - Pressure derivative of viscosity (output)
// pDViscosityDT - Temperature derivative of viscosity (output)
// pDViscosityDPMix - Critical pressure derivative of viscosity (output)
// pDViscosityDTMix - Critical temperature derivative of viscosity (output)
// pDViscosityDMWMix - Molecular weight derivative of viscosity (output)
// pWork - Work array
// Tmeth - reference temperature for viscosity calculation, K (output)
// dTmethdTmeth0 - Derivative of Tmeth w.r.t. Tmeth0 needed for heavy oil extension, dimensionless (output)
// dTmethdPmeth0 - Derivative of Tmeth w.r.t. Pmeth0 needed for heavy oil extension, K/Pa (output)
// dTmethdMWmix - Derivative of Tmeth w.r.t. MWmix needed for heavy oil extension, K/(Kg/Kmol) (output)
*/
void EosPvtTable::CalculateViscosity( int iM, double *pP, double *pT, double *pPMix, double *pTMix, double *pMWMix,
                                      double *pViscosity, double *pDViscosityDP, double *pDViscosityDT,
                                      double *pDViscosityDPMix, double *pDViscosityDTMix, double *pDViscosityDMWMix,
                                      double *pWork, double *Tmeth, double *dTmethdTmeth0,
                                      double *dTmethdPmeth0, double *dTmethdMWmix )
{
   double  dA;
   double  dB;
   double  dC;
   double  dD;
   double  dE;
   int     i;

   /* Pointers */
   double *pPZero = pWork;
   double *pTZero = pPZero + iM;
   double *pRZero = pTZero + iM;
   double *pConvertP = pRZero + iM;
   double *pConvertT = pConvertP + iM;
   double *pRatio = pConvertT + iM;
   double *pDRZeroDT = pRatio + iM;
   double *pDRZeroDP = pDRZeroDT + iM;
   double *pDRZeroDMW = pDRZeroDP + iM;
   double *pDTZeroDT = pDRZeroDMW + iM;
   double *pDTZeroDP = pDTZeroDT + iM;
   double *pDTZeroDMW = pDTZeroDP + iM;
   double *pDRatioDP = pDTZeroDMW + iM;
   double *pDRatioDT = pDRatioDP + iM;
   double *pDRatioDMW = pDRatioDT + iM;
   double *pTZeroRed = pDRatioDMW + iM;
   double *pTa = pTZeroRed + iM;
   double *pTb = pTa + iM;
   double *pTc = pTb + iM;
   double *pTd = pTc + iM;
   double *pTe = pTd + iM;

   /* Get the reduced terms */
   for ( i = 0; i < iM; i++ )
   {
      dA = 1.0 / pPMix[i];
      dB = 1.0 / pTMix[i];
      pConvertP[i] = EOS_VCSPC * dA;    // EOS_VCSPC = CH4 critical pressure
      pConvertT[i] = EOS_VCSTC * dB;    // EOS_VCSTC = CH4 critical temperature
      pPZero[i] = pP[i] * pConvertP[i]; // Pmeth0 = Pco/Pcmix * P
      pTZero[i] = pT[i] * pConvertT[i]; // Tmeth0 = Tco/Tcmix * T
      pPMix[i] = dA;                    // now pPMix = 1/Pcmix
      pTMix[i] = dB;                    // now pTMix = 1/Tcmix
   }

   /* Get the initial density */
   MethaneDensity( iM, pTZero, pPZero, pRZero, pDRZeroDP, pDRZeroDT, pTa );

   /* Transformation of this density */
   dA = 1.0 / EOS_VCSRC;                           // EOS_VCSRC is critical density of CH4
   for ( i = 0; i < iM; i++ )
   {
      pTd[i] = pRZero[i] * dA;                     // now reduced CH4 densit
   }

   EosUtil::VectorPow( iM, pTd, 1.847, pTa );     // pTa = rhor^1.847
   EosUtil::VectorPow( iM, pMWMix, 0.5173, pTb ); // pTb = MWmix^0.5173
   for ( i = 0; i < iM; i++ )
   {
      pTc[i] = pTa[i] * pTb[i];
      pTe[i] = 1.0 / ( 1.0 + 0.031 * pTa[i] );          // 1/alpha0
      pRatio[i] = ( 1.0 + 0.007378 * pTc[i] ) * pTe[i]; // alpha/alpha0=(1 + 0.007378rhor^1.847*MWmix^0.5173)/alpha0
   }

   /* Derivatives */
   if ( pDViscosityDMWMix )
   {
      for ( i = 0; i < iM; i++ )
      {
         dB = pTe[i];
         // dA is d/drho = 1/rhoc * d/drhor, apparently rhoc = 10 is used instead of EOS_VCSRC
         dA = pTa[i] * ( 0.0013627166 * pTb[i] - 0.0057257 * pRatio[i] ) * dB / pTd[i]; // d(alpha/alpha0) dRho L/mol 
         pDRatioDP[i] = pDRZeroDP[i] * dA; // d(alpha/alpha0)dPmeth0, (mol/L/Pa) * (L/mol) [=] 1/Pa
         pDRatioDT[i] = pDRZeroDT[i] * dA; // d(alpha/alpha0)dTmeth0, (mol/L/K) * (L/mol) [=] 1/K
         pDRatioDMW[i] = 0.0038166394 * pTc[i] * dB / pMWMix[i]; // d(alpha/alpha0)dMWmix [=] 1/(kg/kmol)
      }
   }

   /* Get the reduced temperature and pressure */
   for ( i = 0; i < iM; i++ )
   {
      pTa[i] = 1.0 / pRatio[i];          // alpha0/alpha
      pTZeroRed[i] = pTZero[i] * pTa[i]; // Tmeth = Tmeth0*alpha0/alpha 
      pTb[i] = pPZero[i] * pTa[i];       // Pmeth = Pmeth0*alpha0/alpha
      if ( Tmeth )
      {
         Tmeth[i] = pTZeroRed[i]; 
      }
      if ( dTmethdMWmix )
      {
         dTmethdMWmix[i] = -pTZeroRed[i]*pTa[i]*pDRatioDMW[i]; // -Tmeth*alpha0/alpha*d(alpha/alpha0)dMWmix, K/(kg/kmol)
      }
      if ( dTmethdPmeth0 )
      {
         dTmethdPmeth0[i] = -pTZeroRed[i]*pTa[i]*pDRatioDP[i]; // -Tmeth*alpha0/alpha*d(alpha/alpha0)dPmeth0, K/Pa
      }
      if ( dTmethdTmeth0 )
      {
         // In the line below, dTmethdTmeth0 = alpha0/alpha - Tmeth*alpha0/alpha*d(alpha/alpha0)dTmeth0, dimensionless  
         dTmethdTmeth0[i] = pTa[i] - pTZeroRed[i]*pTa[i]*pDRatioDT[i]; 
      }
   }

   /* Get the density */
   MethaneDensity( iM, pTZeroRed, pTb, pRZero, pDRZeroDP, pDRZeroDT, pTc );

   /* Convert to mass density */
   for ( i = 0; i < iM; i++ )
   {
      pRZero[i] *= EOS_VCSMW; // EOS_VCSMW is MW of CH4, now in g / liter
   }

   /* Derivatives */
   if ( pDViscosityDMWMix ) 
   {
      for ( i = 0; i < iM; i++ )
      {
         dA = pTa[i];                        // alpha0/alpha
         dB = -dA * pTZeroRed[i];            // -alpha0/alpha*Tmeth
         dC = pDRZeroDP[i] * pTb[i] + pDRZeroDT[i] * pTZeroRed[i]; // dRhordPmeth * Pmeth + dRhodTmeth * Tmeth 
         pDTZeroDMW[i] = dB * pDRatioDMW[i]; // dTmethdMWmix =  -alpha0/alpha*Tmeth * d(alpha/alpha0)dMWmix
         pDTZeroDP[i] = dB * pDRatioDP[i];   // dTmethdPmeth0 =  -alpha0/alpha*Tmeth * d(alpha/alpha0)dPmeth0
         // In the line below, dTmethdTmeth0 = alpha0/alpha - alpha0/alpha*Tmeth * d(alpha/alpha0)dTmeth0  
         pDTZeroDT[i] = dA + dB * pDRatioDT[i];
         dA *= EOS_VCSMW;                    // MW[CH4] * alpha0/alpha
         // In the line below, dRhordMW = -MW[CH4]*alpha0/alpha * d(alpha/alpha0)dMmix*(dRhordPmeth * Pmeth + dRhodTmeth * Tmeth) 
         pDRZeroDMW[i] = -dA * pDRatioDMW[i] * dC;
         pDRZeroDP[i] = dA * ( pDRZeroDP[i] - dC * pDRatioDP[i] ); // dRhordPmeth
         pDRZeroDT[i] = dA * ( pDRZeroDT[i] - dC * pDRatioDT[i] ); // dRhordTmeth
      }
   }

   /* Apply the correlation */
   ViscosityCorrelation( iM, pTZeroRed, pRZero, pViscosity, pTa, pDViscosityDMWMix, pTb );

   /* Conversion factor for viscosity */
   dE = m_dConvViscosity * 1.0e-7;
   for ( i = 0; i < iM; i++ )
   {
      pViscosity[i] *= dE; // now in Pa s
   }

   /* Chain rule for the derivatives */
   if ( pDViscosityDMWMix )
   {
      for ( i = 0; i < iM; i++ )
      {
         dA = pRatio[i] * dE;                                                // alpha/alpha0
         dB = pViscosity[i];                                                 // Vsc
         dC = dA * pDViscosityDMWMix[i];                                     // alpha/alpha0 * dVscdRhor
         dD = dA * pTa[i];                                                   // alpha/alpha0 * dVscdTmeth
         pTb[i] = dC * pDRZeroDP[i] + dD * pDTZeroDP[i] + dB * pDRatioDP[i]; // dVscdPmeth0
         pTc[i] = dC * pDRZeroDT[i] + dD * pDTZeroDT[i] + dB * pDRatioDT[i]; // dVscdTmeth0
         pDViscosityDPMix[i] = -pPZero[i] * pTb[i] * pPMix[i]; // dVscdPmeth * dPmethdPcmix = -Pmeth0 * () / Pcmix
         pDViscosityDTMix[i] = -pTZero[i] * pTc[i] * pTMix[i]; // dVscdTmeth * dTmethdTcmix = -Tmeth0 * () / Tcmix
         pDViscosityDMWMix[i] = dC * pDRZeroDMW[i] + dD * pDTZeroDMW[i] + pViscosity[i] * pDRatioDMW[i];
      }

      if ( pDViscosityDP )
      {
         for ( i = 0; i < iM; i++ )
         {
            pDViscosityDP[i] = pConvertP[i] * pTb[i];
         }

         if ( pDViscosityDT )
         {
            for ( i = 0; i < iM; i++ )
            {
               pDViscosityDT[i] = pConvertT[i] * pTc[i];
            }
         }
      }
   }

   /* Additional correction  Vsc = (Tcmix/Tco)^(-1/6)*(Pcmix/Pco)^(2/3)*sqrt(MWmix/MWo)*alpha/alpha0*Vsco */
   dA = EOS_VCSTC * m_dConvTemperature;
   dB = EOS_VCSPC * m_dConvPressure;
   dC = 1.0 / 6.0;
   dD = -2.0 / 3.0;
   dE = 1.0 / EOS_VCSMW;
   for ( i = 0; i < iM; i++ )
   {
      pTa[i] = dA * pTMix[i];
      pTb[i] = dB * pPMix[i];
      pTc[i] = dE * pMWMix[i];
   }

   EosUtil::VectorPow( iM, pTa, dC, pTa );
   EosUtil::VectorPow( iM, pTb, dD, pTb );
   EosUtil::VectorPow( iM, pTc, 0.5, pTc );
   for ( i = 0; i < iM; i++ )
   {
      pTd[i] = pTa[i] * pTb[i] * pTc[i];
      pViscosity[i] *= pTd[i] * pRatio[i];
   }

   /* Derivatives */
   if ( pDViscosityDMWMix )
   {
      for ( i = 0; i < iM; i++ )
      {
         dA = pTd[i];
         dB = pViscosity[i];
         pDViscosityDPMix[i] = dA * pDViscosityDPMix[i] - dD * dB * pPMix[i];
         pDViscosityDTMix[i] = dA * pDViscosityDTMix[i] - dC * dB * pTMix[i];
         pDViscosityDMWMix[i] = dA * pDViscosityDMWMix[i] + 0.5 * dB / pMWMix[i];
      }

      if ( pDViscosityDP )
      {
         for ( i = 0; i < iM; i++ )
         {
            pDViscosityDP[i] *= pTd[i];
         }

         if ( pDViscosityDT )
         {
            for ( i = 0; i < iM; i++ )
            {
               pDViscosityDT[i] *= pTd[i];
            }
         }
      }
   }
}


/* 
// Apply the correlation and derivatives
//
// iM - Number of objects to calculate (input)
// pTZero - Reference temperature (input)
// pRZero - Reference density (input)
// pViscosity - Viscosity (output)
// pDViscosityDT - Temperature derivative of viscosity (output)
// pDViscosityDR - Density derivative of viscosity (output)
// pWork - Work array
// comments reflect notation used in PVTSIM documentation and original publications
*/
void EosPvtTable::ViscosityCorrelation( int iM, double *pTZero, double *pRZero, double *pViscosity,
                                        double *pDViscosityDT, double *pDViscosityDR, double *pWork )
{
   double        dA;
   double        dB;
   double        dC;
   double        dE;
   double        dF;
   double        dH;
   double        dI;
   double        dJ;
   double        dK;
   double        dM;
   double        dN;
   double        dO;
   double        dP;
   double        dQ;
   double        dR;
   double        dS;
   double        dT;
   double        dU;
   double        dV;
   double        dX;
   double        dY;
   double        dZ;
   double        d1;
   double        d2;
   double        d3;
   double        d4;
   const double  d5 = 1.0 / 3.0;
   const double  d6 = 1.0 / EOS_VCSC4; // eta1 expression, 1/F
   const double  d7 = 1.0 / EOS_VCSDC; // 1/rhoco, rhoco critical methane density in g/L
   int           i;

   /* Temporary memory */
   double       *pTa = pWork;
   double       *pTb = pTa + iM;
   double       *pTc = pTb + iM;
   double       *pTd = pTc + iM;
   double       *pTe = pTd + iM;
   double       *pTf = pTe + iM;
   double       *pTg = pTf + iM;
   double       *pTh = pTg + iM;
   double       *pTi = pTh + iM;
   double       *pTj = pTi + iM;
   double       *pTk = pTj + iM;
   double       *pTl = pTk + iM;
   double       *pTm = pTl + iM;
   double       *pTn = pTm + iM;
   double       *pTo = pTn + iM;
   double       *pTp = pTo + iM;
   double       *pTq = pTp + iM;
   double       *pTr = pTq + iM;
   double       *pTs = pTr + iM;
   double       *pTt = pTs + iM;
   double       *pTu = pTt + iM;
   double       *pTv = pTu + iM;
   double       *pTw = pTv + iM;
   double       *pTx = pTw + iM;
   double       *pTy = pTx + iM;
   double       *pTz = pTy + iM;

   /* Start the function */
   for ( i = 0; i < iM; i++ )
   {
      dT = pTZero[i];              // T
      dJ = 1.0 / dT;               // 1/T
      dK = EOS_VCSJ4 * dJ;         // j4/T
      dC = EOS_VCSK4 * dJ;         // k4/T
      pTb[i] = dT * d6;            // T/F
      pTc[i] = dJ;                 // 1/T
      pTi[i] = dK;                 // j4/T
      pTj[i] = EOS_VCSJ1 + dK;     // j1+j4/T
      pTo[i] = dC;                 // k4/T
      pTp[i] = EOS_VCSK1 + dC;     // k1+k4/T
   }

   /* Log terms */
   EosUtil::VectorLog( iM, pTb, pTb );         // log(T/F) 
   EosUtil::VectorPow( iM, pTZero, d5, pTa );  // T^(1/3)
   EosUtil::VectorPow( iM, pRZero, 0.5, pTg ); // sqrt(rho)
   EosUtil::VectorPow( iM, pTc, 1.5, pTe );    // (1/T)^(3/2)
   EosUtil::VectorPow( iM, pRZero, 0.1, pTf ); // rho^(0.1)
   EosUtil::VectorExp( iM, pTj, pTj );         // exp(j1+j4/T)
   EosUtil::VectorExp( iM, pTp, pTp );         // exp(k1+k4/T)

   double Coeff = 1.0;
   if ( m_CSVscMethod == 1 ) 
   {
      Coeff = 0.2;
   }

   /* Compute the function */
   for ( i = 0; i < iM; i++ )
   {
      dR = ( pRZero[i] * d7 - 1.0 ) * pTg[i];                  // theta * sqrt(rho)   
      dJ = pTc[i];                                             // 1/T 
      dM = pTe[i];                                             // (1/T)^(3/2)
      dN = pTf[i];                                             // rho^0.1
      dO = EOS_VCSJ2 + EOS_VCSJ3 * dM;                         // j2 + j3*(1/T)^(3/2)
      dQ = EOS_VCSJ5 + dJ * ( EOS_VCSJ6 + dJ * EOS_VCSJ7 );    // j5 + j6/T + j7/T^2
      dE = EOS_VCSK2 + EOS_VCSK3 * dM;                         // k2 + k3*(1/T)^(3/2)
      dF = EOS_VCSK5 + dJ * ( EOS_VCSK6 + dJ * EOS_VCSK7 );    // k5 + k6/T + k7/T^2
      dT = Coeff * ( pTZero[i] - EOS_VCSTT );                  // T - Tf (Tf freezing point CH4)
      pTu[i] = dT > 22.0 ? 22.0 : ( dT < -22.0 ? -22.0 : dT ); // abs(DT)<=22 K
      pTh[i] = dR;                                             // theta * sqrt(rho) 
      pTk[i] = dO;                                             // j2 + j3*(1/T)^(3/2)
      pTl[i] = dQ;                                             // j5 + j6/T + j7/T^2
      pTm[i] = dN * dO + dQ * dR;    // rho^0.1*( j2 + j3*(1/T)^(3/2) ) + ( j5 + j6/T + j7/T^2 ) * theta * sqrt(rho)              
      pTq[i] = dE;                   // k2 + k3*(1/T)^(3/2) 
      pTr[i] = dF;                   // k5 + k6/T + k7/T^2 
      pTs[i] = dN * dE + dF * dR;    // rho^0.1*( k2 + k3*(1/T)^(3/2) ) + ( k5 + k6/T + k7/T^2 ) * theta * sqrt(rho) 
   }

   /* Exponential */
   EosUtil::VectorExp( iM, pTm, pTm ); 
   EosUtil::VectorExp( iM, pTs, pTs );
   EosUtil::VectorExp( iM, pTu, pTu ); // exp(DT)

   /* More of the function */
   for ( i = 0; i < iM; i++ )
   {
      dU = EOS_VCSC3 - pTb[i];            // C - log(T/F) (C from eta1 expression)
      dV = pTa[i];                        // T^(1/3) 
      dI = 1.0 / dV;                      // T^(-1/3)
      dB = pTm[i] - 1.0;   // exp( rho^0.1*( j2 + j3*(1/T)^(3/2) ) + ( j5 + j6/T + j7/T^2 ) * theta * sqrt(rho) ) - 1
      dH = pTs[i] - 1.0;   // exp( rho^0.1*( k2 + k3*(1/T)^(3/2) ) + ( k5 + k6/T + k7/T^2 ) * theta * sqrt(rho) ) - 1 
      dA = pTu[i] * pTu[i];               // exp(DT)*exp(DT) 
      dX = ( dA - 1.0 ) / ( dA + 1.0 );   // ( exp(DT)*exp(DT) - 1 ) / ( exp(DT)*exp(DT) + 1 ) this is HTAN
      dY = 0.5 * ( 1.0 + dX );            // F1 = (HTAN+1)/2
      dZ = 0.5 * ( 1.0 - dX );            // F2 = (1-HTAN)/2
      d1 = dU * EOS_VCSC2;                // B * ( C - log(T/F) ) 
      d2 = EOS_VCSC1 + d1 * dU;           // A + B * ( C - log(T/F) )^2 = eta1  completely formed
      d3 = pTj[i] * dB;                   // now Deta1 = \delta eta^\prime completely formed 
      d4 = pTp[i] * dH;                   // now Deta2 = \delta eta^{\prime \prime} completely formed
      pTb[i] = d1;                        // B * ( C - log(T/F) )  
      pTd[i] = dI;                        // T^(-1/3) 
      pTn[i] = dB;         // rho^0.1*( j2 + j3*(1/T)^(3/2) ) + ( j5 + j6/T + j7/T^2 ) * theta * sqrt(rho) - 1 
      pTt[i] = dH;         // rho^0.1*( k2 + k3*(1/T)^(3/2) ) + ( k5 + k6/T + k7/T^2 ) * theta * sqrt(rho) - 1 
      pTu[i] = dX;                        // HTAN
      pTv[i] = dY;                        // F1 = (HTAN+1)/2
      pTw[i] = dZ;                        // F2 = (1-HTAN)/2
      pTx[i] = d2;                        // eta1
      pTy[i] = d3;                        // Deta1
      pTz[i] = d4;                        // Deta2

      // In the line below, Vsc = ( eta0 + eta1*rho + F1*Deta1 + F2*Deta2) [=] micro gram / cm / s 
      pViscosity[i] = EOS_VCSB4 + dV * ( EOS_VCSB5 + dV * ( EOS_VCSB6 + dV *  //  eta0 
                      ( EOS_VCSB7 + dV * ( EOS_VCSB8 + dV * EOS_VCSB9 ) ) ) ) + //  still eta0 
                      dI * ( EOS_VCSB3 + dI * ( EOS_VCSB2 + dI * EOS_VCSB1 ) ) + //  still eta0 
                      d2 * pRZero[i] + dY * d3 + dZ * d4; // eta1*rho + F1*Deta1 + F2*Deta2
   }

   /* Derivatives */
   if ( pDViscosityDR )
   {
      for ( i = 0; i < iM; i++ )
      {
         /* Temperature derivatives */
         dI = pTd[i];           // T^(-1/3) 
         dJ = pTc[i];           // 1/T 
         dK = pTv[i] * pTj[i];  // F1 * exp(j1+j4/T)
         dM = pTe[i];           // (1/T)^(3/2)   
         dN = pTf[i];           // rho^0.1
         dO = pTm[i];           // exp( rho^0.1*( j2 + j3*(1/T)^(3/2) ) + ( j5 + j6/T + j7/T^2 ) * theta * sqrt(rho) )
         dP = pRZero[i];        // rho
         dQ = pTw[i] * pTp[i];  // F2 * exp(k1+k4/T) 
         dR = pTh[i];           // theta * sqrt(rho) 
         dS = pTs[i];           // exp( rho^0.1*( k2 + k3*(1/T)^(3/2) ) + ( k5 + k6/T + k7/T^2 ) * theta * sqrt(rho) )
         dT = pTa[i];           // T^(1/3) 
         dV = pTb[i];           // B * ( C - log(T/F) )
         dX = pTu[i];           // HTAN

         pDViscosityDT[i] = dJ * ( ( dT * ( EOS_VCSB5 + dT * ( EOS_VCSB6 + EOS_VCSB6 +
                            dT * ( 3.0 * EOS_VCSB7 + dT * ( 4.0 * EOS_VCSB8 +
                            dT * 5.0 * EOS_VCSB9 ) ) ) ) - dI * ( EOS_VCSB3 +
                            dI * ( EOS_VCSB2 + EOS_VCSB2 + dI * 3.0 * EOS_VCSB1 ) ) ) / 3.0 -
                            ( dV + dV ) * dP - dK * ( dO * ( dN * 1.5 * EOS_VCSJ3 * dM +
                            dJ * ( dR * ( EOS_VCSJ6 + dJ * ( EOS_VCSJ7 + EOS_VCSJ7 ) ) ) ) +
                            pTi[i] * pTn[i] ) - dQ * ( dS * ( dN * 1.5 * EOS_VCSK3 * dM +
                            dJ * ( dR * ( EOS_VCSK6 + dJ * ( EOS_VCSK7 + EOS_VCSK7 ) ) ) ) +
                            pTo[i] * pTt[i] ) ) +
                            0.5 * ( 1.0 - dX * dX ) * Coeff * ( pTy[i] - pTz[i] ); 
                            // the above line  is dF1dHTAN * dHTANdDT * dDTdT * ( Deta1 - Deta2 )

         /* Density derivatives */
         dP = 1.0 / dP;                                     // 1/rho
         dB = pTg[i] * ( 1.0 / EOS_VCSDC ) + 0.5 * dR * dP; // sqrt(rho) / rhoc + 0.5 * theta*sqrt(rho) / rho 
         dC = dP * 0.1 * dN;                                // 0.1/rho * rho^0.1 
         pDViscosityDR[i] = pTx[i] + dK * dO * ( dC * pTk[i] + pTl[i] * dB )
                          + dQ * dS * ( dC * pTq[i] + dB * pTr[i] );
      }
   }
}

/* 
// Apply the correlation and derivatives
//
// iM - Number of objects to calculate (input)
// iDrv, derivative indicator (input)
// T - fluid temperature (input)
// P - fluid presure (input)
// MWn - number average molecular weight (input)
// MWw - weight average molecular weight (input)
// Vsc - Viscosity (output)
// dVscdT - Temperature derivative of viscosity (output)
// dVscdP - Pressure derivative of viscosity (output)
// dVscdMn - Mn derivative of viscosity (output)
// dVscdMw - Mw derivative of viscosity (output)
// 
// comments reflect notation used in PVTSIM documentation and original publications
// Calculatons are performed with pressure in atm because they appear in correlations as
// such.  Resulting viscosities are in cP.  A conversion back to internal units happens
// at end of routine.
*/
void EosPvtTable::VscCorrHeavy( const int iM, const int iDrv, const double *T, const double *P, 
                  const double *Mn, double *Mw, double *Vsc, double *dVscdT,
                  double *dVscdP, double *dVscdMn, double *dVscdMw )
{
   double Patm, V3, V4, M, Vsc0;
   double dV3dT, dV3dMn;
   double dV4dT, dV4dMn;
   double dMdMn, dMdMw, dMdT, dVsc0dT, dVsc0dM;
   const double atm2Pa = 101325.0; // Conversion factor from atm to Pa
   const double cp2Pas = 0.001;  // Conversion factor from cp to Pas
//   const double atm2Pa = Unit::getFullNamed("QNTDICT.ATM")->getConvFactor();
//   const double cp2Pas = Unit::getFullNamed("QNTDICT.CP")->getConvFactor();

   const double dVscdPConv = cp2Pas / atm2Pa; 
   const double Tune3 = m_dVTerm3;
   const double Tune4 = m_dVTerm4;
    
   
   for ( int i = 0; i < iM; i++ ) 
   {

      Patm = P[i] / atm2Pa;
      if ( Mw[i]/Mn[i] <= 1.5 )
      {
         Mw[i] = 1.5 * Mn[i];
      }

      // Determine correlation parameters
      if ( m_CSVscMethod == 2 )
      {
         V3 = 1.5 * Tune3;
         V4 = 0.5 * Tune4;
         dV3dT = 0.0;
         dV3dMn = 0.0;
         dV4dT = 0.0;
         dV4dMn = 0.0;
      }
      else 
      {
         const double c1 = 0.2252;
         const double c2 = 0.9738;
         const double c3 = 0.5354;
         const double c4 = -0.1170;
         V3 = ( c1*T[i]/Mn[i] + c2 ) * Tune3;
         V4 = ( c3*V3/Tune3 + c4 ) * Tune4;
         dV3dT = c1/Mn[i] * Tune3;
         dV4dT = c3 * Tune4 * dV3dT / Tune3;
         dV3dMn = -c1*T[i]/pow(Mn[i],2.0) * Tune3;
         dV4dMn = c3 * Tune4 * dV3dMn / Tune3;
      }

      M = Mn[i] * pow( Mw[i]/Mn[i] / V3, V4 );
      dMdMn = M/Mn[i] + M * ( dV4dMn*log( M/Mn[i] )/V4 - V4/Mn[i] - V4/V3*dV3dMn ); 
      dMdMw = M * V4 / Mw[i];
      dMdT = M * ( dV4dT/V4*log( M/Mn[i] ) -  V4/V3*dV3dT ); 

      // now Viscosity correlation at standard pressure and reservoir temperature
      const double a1 = -0.07995;
      double a2 = -0.01101;
      const double a3 = -371.8;
      double a4 = 6.215;
      if ( T[i] > 564.49 )
      {
         a2 = -a2;
         a4 = -a4;
      }
      Vsc0 = pow( 10.0, a1 + a2*M + a3/T[i] + a4*M/T[i] ); 
      dVsc0dT = Vsc0 * ( dMdT*(a2 + a4/T[i]) - (a3 + a4*M)/pow(T[i],2.0) ) * log(10.0); 
      dVsc0dM = Vsc0 * (a2 + a4/T[i]) * log(10.0);

      // Now adjust viscosity to reservoir P
      if ( m_CSVscMethod == 2 )
      {
         const double d1 = 0.008;
         const double factor =  1.0 + d1*(Patm - 1.0); 
         if ( factor > 0.0 )
         {
            Vsc[i] = Vsc0 * factor;
            if ( iDrv >= EOS_DRV_N )  
            {
               dVscdMn[i] = dVsc0dM * factor * dMdMn;  
               dVscdMw[i] = dVsc0dM * factor * dMdMw;
               if ( iDrv >= EOS_DRV_P )
               {
                  dVscdP[i] = d1 * Vsc0;
                  if ( iDrv >= EOS_DRV_T )
                  {
                     dVscdT[i] = dVsc0dT * factor;
                  }
               }
            }
         }
         else
         {
            Vsc[i] = Vsc0;  
            if ( iDrv >= EOS_DRV_N ) 
            {
               dVscdMn[i] = dVsc0dM * dMdMn;  
               dVscdMw[i] = dVsc0dM * dMdMw;
               if ( iDrv >= EOS_DRV_P )
               {
                  dVscdP[i] = 0.0;
                  if ( iDrv >= EOS_DRV_T )
                  {
                     dVscdT[i] = dVsc0dT;
                  }
               }
            }
         }
      }

      else
      {
         const double A = 0.0002649;
         const double B = 0.0010177;
         const double etalog = log(Vsc0) + A/B;
         double C = 1.0;

         if ( etalog <= 0.0 )
         {
            C = -1.0;
         }
         Vsc[i] = exp( etalog*exp( C*B*(Patm-1.0) ) - A/B );

         if ( iDrv >= EOS_DRV_N ) 
         {
            dVscdMn[i] = dVsc0dM * dMdMn * exp( C*B*(Patm-1.0) ) * Vsc[i]/Vsc0;
            dVscdMw[i] = dVsc0dM * dMdMw * exp( C*B*(Patm-1.0) ) * Vsc[i]/Vsc0;
            if ( iDrv >= EOS_DRV_P )
            {
               dVscdP[i] = Vsc[i] * ( log(Vsc0) + A/B ) * C* B * exp( C*B*(Patm-1.0) );
               if ( iDrv >= EOS_DRV_T )
               {
                  dVscdT[i] = dVsc0dT * exp( C*B*(Patm-1.0) ) * Vsc[i]/Vsc0;
               }
            }
         }
      }

      // convert units to internal, Viscosity: cP -> Pa s, Pressure: atm -> Pa
      Vsc[i] = Vsc[i] * cp2Pas;
      if ( iDrv >= EOS_DRV_N ) 
      {
         dVscdMn[i] = dVscdMn[i] * cp2Pas; 
         dVscdMw[i] = dVscdMw[i] * cp2Pas; 
         if ( iDrv >= EOS_DRV_P )
         {
            dVscdP[i] = dVscdP[i] * dVscdPConv; 
            if ( iDrv >= EOS_DRV_T )
            {
               dVscdT[i] = dVscdT[i] * cp2Pas; 
            }
         }
      }
 
   }
}


/*
// Initialize the temperature dependent terms
//
// iM - Number of objects to calculate (input)
// pTemperatureArg - Temperature (input)
// pSaturationPressure - Saturation pressure (output)
// pLiquidDensity - Saturated liquid density (output)
// pGasDensity - Saturated gas density (output)
// pWork - Work array
*/
void EosPvtTable::SaturationCurve( int iM, double *pTemperatureArg, double *pSaturationPressure, double *pLiquidDensity,
                                   double *pGasDensity, double *pWork )
{
   const double  dA = 1.0 / ( 1.0 - EOS_VCSTT / EOS_VCSTC );
   const double  dB = 1.0 / ( EOS_VCSTC - EOS_VCSTT );
   const double  dC = 1.01 * ( EOS_VCSRT - EOS_VCSRC );
   const double  dD = EOS_VCSLA + EOS_VCSLB + EOS_VCSLC;
   const double  dE = 0.99 * EOS_VCSRC;
   const double  dF = 1.01 * EOS_VCSRC;
   const double  dG = 1.0 / 3.0;
   double        dP;
   double        dR;
   double        dS;
   double        dT;
   double        dX;
   int           i;

   /* Work array */
   double       *pTa = pWork;
   double       *pTb = pTa + iM;
   double       *pTc = pTb + iM;
   double       *pTd = pTc + iM;

   /* Get the saturation pressure */
   for ( i = 0; i < iM; i++ )
   {
      dX = pTemperatureArg[i];
      dT = dX < EOS_VCSTC ? ( dX > EOS_VCSTT ? dX : EOS_VCSTT ) : EOS_VCSTC;
      dX = ( 1.0 - EOS_VCSTT / dT ) * dA;
      pTa[i] = ( EOS_VCSTC - dT ) * dB;
      pTb[i] = dX;
      pTd[i] = 1.0 - dX;
   }

   /* Vector terms */
   EosUtil::VectorPow( iM, pTd, 1.5, pSaturationPressure );
   EosUtil::VectorPow( iM, pTa, EOS_VCSLD, pLiquidDensity );
   EosUtil::VectorPow( iM, pTa, EOS_VCSA6, pGasDensity );
   EosUtil::VectorPow( iM, pTa, dG, pTc );

   /* Continue with calculations */
   for ( i = 0; i < iM; i++ )
   {
      dS = pTa[i];
      dX = pTb[i];
      dP = pTc[i];
      dR = dP * dP;
      pSaturationPressure[i] = dX * ( EOS_VCSSA + EOS_VCSSD * pSaturationPressure[i] + dX * ( EOS_VCSSB + dX * EOS_VCSSC ) );
      pTa[i] = dD - dR * ( EOS_VCSLA + dR * dR * ( EOS_VCSLB + dR * EOS_VCSLC ) );
      pTb[i] = EOS_VCSA1 * pGasDensity[i] + dS * ( EOS_VCSA2 + dP * ( EOS_VCSA3 + dP * EOS_VCSA4 ) + EOS_VCSA5 * pTd[i] );
   }

   /* More vector functions */
   EosUtil::VectorExp( iM, pSaturationPressure, pSaturationPressure );
   EosUtil::VectorExp( iM, pTa, pTa );
   EosUtil::VectorExp( iM, pTb, pGasDensity );

   /* Set the terms */
   for ( i = 0; i < iM; i++ )
   {
      pSaturationPressure[i] *= EOS_VCSPT;
      pLiquidDensity[i] = dF + dC * pLiquidDensity[i] * pTa[i];
      pGasDensity[i] *= dE;
   }
}


/*
// Get the methane density
//
// iM - Number of objects to calculate (input)
// pTemperatureArg - Temperature (input)
// pPressure - Pressure (input)
// pRho - Density (output)
// pDDensityDP - Pressure derivative of density (output)
// pDDensityDT - Temperature derivative of density (output)
// pWork - Work array
*/
void EosPvtTable::MethaneDensity( int iM, double *pTemperatureArg, double *pPressure, double *pRho, double *pDDensityDP,
                                  double *pDDensityDT, double *pWork )
{
   double  dA;
   double  dB;
   double  dC;
   double  dD;
   double *pTa = pWork;
   double *pTb = pTa + iM;
   double *pTc = pTb + iM;
   double *pTd = pTc + iM;
   double *pTe = pTd + iM;
   double *pTf = pTe + iM;
   double *pTg = pTf + iM;
   double *pTh = pTg + iM;
   double *pTi = pTh + iM;
   double *pTj = pTi + iM;
   double *pTk = pTj + iM;
   double *pTl = pTk + iM;
   double *pTm = pTl + iM;
   double *pTn = pTm + iM;
   double *pTo = pTn + iM;
   double *pTp = pTo + iM;
   double *pTq = pTp + iM;
   double *pTr = pTq + iM;
   double *pTs = pTr + iM;
   double *pTt = pTs + iM;
   int    *iInd = (int *)( pTt + iM + iM );
   int     iter;
   int     i;
   int     j;
   int     iActive;
   int     iActiveT;

   /* Calculate the saturation curve estimate */
   SaturationCurve( iM, pTemperatureArg, pTa, pTb, pTc, pTd );

   /* Initial guess for the density */
   for ( i = 0; i < iM; i++ )
   {
      pRho[i] = pPressure[i] < pTa[i] ? pTc[i] : pTb[i]; // units are mol / liter
   }

   /* Reset the coefficients */
   /* TODO: The next line is too long. Please rewrite to make it shorter. */
   BWRSCoefficients( iM, 0, pTemperatureArg, pTc, pTd, pTe, pTf, pTg, pTh, pTi, pTj, pTk, pTl, pTm, pTn, pTo, pTp, pTq, pTr, pTs, pTt );

   /* Prepare for the indirection */
   for ( i = 0; i < iM; i++ )
   {
      iInd[i] = i;
   }

   /* Newton Iterations */
   iter = 0;
   iActive = iM;
   while ( ( iter++ < EOS_VCSMI ) && iActive )
   {
      /* Get the function */
      /* TODO: The next line is too long. Please rewrite to make it shorter. */
      BWRSFunction( iActive, iInd, pPressure, pRho, pTa, pTb, pTc, pTd, pTe, pTf, pTg, pTh, pTi, pTj, pTk, pTl, pTm, pTn, pTo, pTp, pTq, pTr, pTs, pTt );

      /* Take the Newton step */
      iActiveT = iActive;
      for ( j = 0; j < iActive; j++ )
      {
         i = iInd[j];
         dB = pTb[i];
         dC = dB * EOS_DIVFAC ? 1.0 / dB : 0.0; // 1 / derivative of BWRS eqn w.r.t rho
         pTb[i] = dC;
         dA = pTa[i] * dC;
         dD = pRho[i];
         dB = dD + dD;
         dC = 0.1 * dD;
         dD -= dA;
         pRho[i] = ( dD < dC ) ? dC : ( ( dD > dB ) ? dB : dD ); // units are mole / liter
         if ( fabs( dA ) < EOS_VCSCV )
         {
            iActiveT--;
            iInd[j] = iInd[iActiveT];
         }
      }

      iActive = iActiveT;
   }

   /* Pressure derivative, w.r.t methane reference pressure */
   if ( pDDensityDP )
   {
      for ( i = 0; i < iM; i++ )
      {
         pDDensityDP[i] = pTs[i] * pTb[i];  // coeff16 / derivative of BWRS eqn w.r.t. rho, units are (atm/Pa) * (mol / liter / atm) = mol/L/Pa 
      }
   }

   /* Temperature derivative, w.r.t methane reference temperature */
   if ( pDDensityDT )
   {
      for ( i = 0; i < iM; i++ )
      {
         iInd[i] = i;
      }

      /* TODO: The next line is too long. Please rewrite to make it shorter. */
      BWRSCoefficients( iM, 1, pTemperatureArg, pTc, pTd, pTe, pTf, pTg, pTh, pTi, pTj, pTk, pTl, pTm, pTn, pTo, pTp, pTq, pTr, pTs, pTt );

      /* TODO: The next line is too long. Please rewrite to make it shorter. */
      BWRSFunction( iM, iInd, pPressure, pRho, pTa, NULL, pTc, pTd, pTe, pTf, pTg, pTh, pTi, pTj, pTk, pTl, pTm, pTn, pTo, pTp, pTq, pTr, pTs, pTt );
      for ( i = 0; i < iM; i++ )
      {
         pDDensityDT[i] = -pTa[i] * pTb[i]; //  units are (atm/K) * (mol / liter / atm) = mol/liter/K 
      }
   }
}


/* 
// Generate the coefficients for methane BWRS representation
//
// iM - Number of objects to calculate (input)
// iDrv - Indicator as to for normal or derivative calculations (input)
// pTemperature - Temperature (input)
// pCoef0, etc... - BWRS coefficients or their temperature derivatives (output)
// pWork - Work array
*/
void EosPvtTable::BWRSCoefficients( int iM, int iDrv, double *pTemperature, double *pCoef0, double *pCoef1,
                                    double *pCoef2, double *pCoef3, double *pCoef4, double *pCoef5, double *pCoef6,
                                    double *pCoef7, double *pCoef8, double *pCoef9, double *pCoef10, double *pCoef11,
                                    double *pCoef12, double *pCoef13, double *pCoef14, double *pCoef15, double *pCoef16,
                                    double *pWork )
{
   double dA;
   double dB;
   double dC;
   double dD;
   double dT;
   int    i;

   /* Compute the square root separately */
   EosUtil::VectorPow( iM, pTemperature, 0.5, pWork );

   /* Compute the derivatives of the coefficients */
   if ( iDrv )
   {
      for ( i = 0; i < iM; i++ )
      {
         dA = 1.0 / pTemperature[i];
         dB = dA * dA;
         dC = dA * dB;
         dD = dA * dC;
         dB = -dA * dB;
         dC = -dA * dC * 3.0;
         dD = -dA * dD * 4.0;
         dA = -dA * dA;
         dB += dB;
         pCoef0[i] = EOS_VCS00;
         pCoef1[i] = EOS_VCS01 + EOS_VCS04 * dA + EOS_VCS05 * dB + EOS_VCS02 * 0.5 / pWork[i];
         pCoef2[i] = EOS_VCS06 + EOS_VCS08 * dA + EOS_VCS09 * dB;
         pCoef3[i] = EOS_VCS10 + EOS_VCS12 * dA;
         pCoef4[i] = EOS_VCS14 * dA + EOS_VCS15 * dB;
         pCoef5[i] = EOS_VCS16 * dA;
         pCoef6[i] = EOS_VCS17 * dA + EOS_VCS18 * dB;
         pCoef7[i] = EOS_VCS19 * dB;
         pCoef8[i] = EOS_VCS20 * dB + EOS_VCS21 * dC;
         pCoef9[i] = EOS_VCS22 * dB + EOS_VCS23 * dD;
         pCoef10[i] = EOS_VCS24 * dB + EOS_VCS25 * dC;
         pCoef11[i] = EOS_VCS26 * dB + EOS_VCS27 * dD;
         pCoef12[i] = EOS_VCS28 * dB + EOS_VCS29 * dC;
         pCoef13[i] = EOS_VCS30 * dB + EOS_VCS31 * dC + EOS_VCS32 * dD;
         pCoef14[i] = 0.0;
         pCoef15[i] = 0.0;
         pCoef16[i] = 0.0;
      }
   }

   /* Compute the coefficients */
   else
   {
      for ( i = 0; i < iM; i++ )
      {
         dT = pTemperature[i];
         dA = 1.0 / dT;
         dB = dA * dA;
         dC = dA * dB;
         dD = dA * dC;
         pCoef0[i] = EOS_VCS00 * dT;
         pCoef1[i] = EOS_VCS01 * dT + EOS_VCS04 * dA + EOS_VCS05 * dB + EOS_VCS03 + EOS_VCS02 * pWork[i];
         pCoef2[i] = EOS_VCS06 * dT + EOS_VCS08 * dA + EOS_VCS09 * dB + EOS_VCS07;
         pCoef3[i] = EOS_VCS10 * dT + EOS_VCS12 * dA + EOS_VCS11;
         pCoef4[i] = EOS_VCS14 * dA + EOS_VCS15 * dB;
         pCoef5[i] = EOS_VCS16 * dA;
         pCoef6[i] = EOS_VCS17 * dA + EOS_VCS18 * dB;
         pCoef7[i] = EOS_VCS19 * dB;
         pCoef8[i] = EOS_VCS20 * dB + EOS_VCS21 * dC;
         pCoef9[i] = EOS_VCS22 * dB + EOS_VCS23 * dD;
         pCoef10[i] = EOS_VCS24 * dB + EOS_VCS25 * dC;
         pCoef11[i] = EOS_VCS26 * dB + EOS_VCS27 * dD;
         pCoef12[i] = EOS_VCS28 * dB + EOS_VCS29 * dC;
         pCoef13[i] = EOS_VCS30 * dB + EOS_VCS31 * dC + EOS_VCS32 * dD;
         pCoef14[i] = EOS_VCS13;
         pCoef15[i] = EOS_VCS33;
         pCoef16[i] = EOS_VCSCP;
      }
   }
}


/*
// BWRS equation of state for methane
//
// iM - Number of objects to calculate (input)
// iInd - Indirection indicator for objects (input)
// pPresure - Pressure (input)
// pRho - Current density estimate (input)
// pFunction - Value of the residual function (output)
// pDerivative - Derivative of the residual function (output)
// pCoef0, etc... - BWRS coefficients or their temperature derivatives (input)
// pWork - Work array
*/
void EosPvtTable::BWRSFunction( int iM, int *iInd, double *pPressure, double *pRho, double *pFunction,
                                double *pDerivative, double *pCoef0, double *pCoef1, double *pCoef2, double *pCoef3,
                                double *pCoef4, double *pCoef5, double *pCoef6, double *pCoef7, double *pCoef8,
                                double *pCoef9, double *pCoef10, double *pCoef11, double *pCoef12, double *pCoef13,
                                double *pCoef14, double *pCoef15, double *pCoef16, double *pWork )
{
   double *pWrk1 = pWork + iM;

   //double *pWrk2 = pWrk1 + iM;
   double  dA;

   //double *pWrk2 = pWrk1 + iM;
   double  dB;

   //double *pWrk2 = pWrk1 + iM;
   double  dC;

   //double *pWrk2 = pWrk1 + iM;
   double  dD;

   //double *pWrk2 = pWrk1 + iM;
   double  dE;
   int     i;
   int     j;

   /* Computations with derivative */
   if ( pDerivative )
   {
      for ( j = 0; j < iM; j++ )
      {
         i = iInd[j];
         dE = pRho[i];
         dA = dE * dE;
         pWork[j] = -EOS_VCS33 * dA;
         pWrk1[j] = dA;
      }

      EosUtil::VectorExp( iM, pWork, pWork );

      /* Compute the function */
      for ( j = 0; j < iM; j++ )
      {
         i = iInd[j];
         dE = pRho[i];
         dA = pWrk1[j];
         dC = pWork[j];
         dD = dE * dC;
         dB = dA * ( pCoef8[i] + dA * ( pCoef9[i] + dA * ( pCoef10[i] + dA * ( pCoef11[i] + dA * ( pCoef12[i] + dA * pCoef13[i] ) ) ) ) );

         /* Compute the function */
         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         pFunction[i] = dE * ( pCoef0[i] + dE * ( pCoef1[i] + dE * ( pCoef2[i] + dE * ( pCoef3[i] + dE * ( pCoef14[i] + dE * ( pCoef4[i] + dE * ( pCoef5[i] + dE * ( pCoef6[i] + dE * pCoef7[i] ) ) ) ) ) ) ) ) + dB * dD - pPressure[i] * pCoef16[i];

         /* Compute the derivative */
         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         pDerivative[i] = pCoef0[i] + dE * ( pCoef1[i] + pCoef1[i] + dE * ( 3.0 * pCoef2[i] + dE * ( 4.0 * pCoef3[i] + dE * ( 5.0 * pCoef14[i] + dE * ( 6.0 * pCoef4[i] + dE * ( 7.0 * pCoef5[i] + dE * ( 8.0 * pCoef6[i] + dE * 9.0 * pCoef7[i] ) ) ) ) ) ) + dD * ( pCoef8[i] + pCoef8[i] + dA * ( 4.0 * pCoef9[i] + dA * ( 6.0 * pCoef10[i] + dA * ( 8.0 * pCoef11[i] + dA * ( 10.0 * pCoef12[i] + dA * 12.0 * pCoef13[i] ) ) ) ) ) ) + dB * dC * ( 1.0 - ( pCoef15[i] + pCoef15[i] ) * dA );
      }
   }

   /* No derivatives */
   else
   {
      for ( i = 0; i < iM; i++ )
      {
         dE = pRho[i];
         dA = dE * dE;
         pWork[i] = -EOS_VCS33 * dA;
         pWrk1[i] = dA;
      }

      EosUtil::VectorExp( iM, pWork, pWork );

      /* Compute the function */
      for ( i = 0; i < iM; i++ )
      {
         dE = pRho[i];
         dA = pWrk1[i];

         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         pFunction[i] = dE * ( pCoef0[i] + dE * ( pCoef1[i] + dE * ( pCoef2[i] + dE * ( pCoef3[i] + dE * ( pCoef14[i] + dE * ( pCoef4[i] + dE * ( pCoef5[i] + dE * ( pCoef6[i] + dE * pCoef7[i] ) ) ) ) ) ) ) ) + dE * pWork[i] * ( dA * ( pCoef8[i] + dA * ( pCoef9[i] + dA * ( pCoef10[i] + dA * ( pCoef11[i] + dA * ( pCoef12[i] + dA * pCoef13[i] ) ) ) ) ) ) - pPressure[i] * pCoef16[i];
      }
   }
}


/*
// Class EosPvtModel
*/


/* 
// EosPvtModel
//
// Default construction of pvt data 
*/
EosPvtModel::EosPvtModel( void )
{
   Initialize( 0, NULL, NULL );
}


/* 
// EosPvtModel
//
// Construction of pvt data 
//
// iVersion (input):  Version of flasher (set to zero)
// piFlasher (input): Pointer to array of integer input variables
//                    as described above
// pdFlasher (input): Pointer to array of double input variables
//                    as described above
*/
EosPvtModel::EosPvtModel( int iVersion, int *piFlasher, double *pdFlasher )
{
   /* Initialize */
   Initialize( iVersion, piFlasher, pdFlasher );
}


/* 
// EosPvtModel
//
// Copy EOS Pvt Model 
*/
EosPvtModel::EosPvtModel( const EosPvtModel &self )
{
   m_dEnorm = self.m_dEnorm;
   m_dLnEnorm = self.m_dLnEnorm;
   m_dTiny = self.m_dTiny;
   m_dConverge = self.m_dConverge;
   m_dThermalDiffusion = self.m_dThermalDiffusion;
   m_dBubbleReduce = self.m_dBubbleReduce;
   m_iMaxIterations = self.m_iMaxIterations;
   m_iFlashLength = self.m_iFlashLength;
   m_iMichelson = self.m_iMichelson;
   m_iSubstitutions = self.m_iSubstitutions;
   m_iDebug = self.m_iDebug;
}


/* 
// Initialize
//
// Construction of pvt data 
//
// iVersion (input):  Version of flasher (set to zero)
// piFlasher (input): Pointer to array of integer input variables
//                    as described above
// pdFlasher (input): Pointer to array of double input variables
//                    as described above
//
// 1) Set defaults
// 2) Call ReadAllData to read data if supplied
*/
void EosPvtModel::Initialize( int iVersion, int *piFlasher, double *pdFlasher )
{
   /* Set general pointers */
   m_pApplication = (EosApplication *)0;
   m_pEosPvtTable = (EosPvtTable *)0;
   m_pWorkArray = (double *)0;
   m_iDrv = EOS_DRV_N;
   m_iHeat = EOS_OPTION_OFF;
   m_iVolume = EOS_OPTION_OFF;
   m_iMolarFlash = EOS_OPTION_OFF;
   m_iWaterComp = -1;

   /* Set default terms */
   if ( piFlasher == (int *)0 )
   {
      m_dEnorm = FLT_MAX;
      m_dLnEnorm = log( m_dEnorm );
      m_dTiny = DBL_EPSILON;
      m_dThermalDiffusion = 0.0;
      m_dBubbleReduce = 0.5;
      m_dConverge = 0.0001;
      m_iMichelson = EOS_OPTION_ON;
      m_iBubbleDew = EOS_OPTION_OFF;
      m_iPseudoProperties = EOS_OPTION_OFF;
      m_iDebug = EOS_OPTION_OFF;
      m_iMaxIterations = 20;
      m_iFlashLength = 64;
      m_iSubstitutions = 0;
   }

   /* Read data */
   else
   {
      ReadAllData( iVersion, piFlasher, pdFlasher );
   }
}


/* 
// DoFlash
// 
// Subroutine to perform the flash calculations  
//
// pTApplication
//    Pointer to the application class
// pTEosPvtTable
//    Pointer to the PVT table class
//
// 1) Get the needed control data from the EosApplication class
// 2) Set temporary memory.  The first pass is to get the
//    needed amount of memory.  The second is to actually
//    set the pointers after the memory is allocated
// 3) Standard flash calculation
//    a) First do debug printing
//    b) Then call the routine ModifyPhaseIdentification
//       in EosApplication to do some calculations involving
//       saved or new K values, phase splits, phase
//       identification, etc.
//    c) Then do bubble point calculations if required,
//       and get water properties
//    d) Else, if only one object, do flash and get
//       properties for a single object
//    e) Else, call the flash routines and then get
//       properties for multiple objects
//    f) Do some debug printing of the output
//    g) Flashes are only performed, of course, on
//       systems that are allowed to be two phase
// 4) Compositional grading calculations are separate
// 5) Separator calculations are separate
// 6) Finally deallocate the memory
*/
void EosPvtModel::DoFlash( EosApplication *pTApplication, EosPvtTable **pTEosPvtTable )
{
   int iType;
   int iFlashIt;
   int iSaved;
   int iNc;
   int iNObj;
   int iSize;
   int iProps;
   int iFlash;
   int iProperties;
   int iWater;
   int iInitialize;
   int iThermal;

   /* Set the application and the pvt table */
   m_pApplication = pTApplication;
   m_pEosPvtTable = pTEosPvtTable[0];

   /* Set the application type */
   iNc = ( m_pEosPvtTable )->GetNumberHydrocarbons();

   /* TODO: The next line is too long. Please rewrite to make it shorter. */
   ( m_pApplication )->WriteControlData( &iType, &iSaved, &iNObj, &iFlash, &iProperties, &m_iBubbleDewPoint, &iWater, &iInitialize, &m_iBubbleDew, &m_iPseudoProperties );

   /* Reset the maximum flash length */
   if ( iInitialize != EOS_FLASH_CALCULATIONS )
   {
      m_iFlashLength = 1;
   }

   /* Set the memory */
   m_pWorkArray = (double *)0;
   iSize = SetPointers();
   m_pWorkArray = CNEW ( double, iSize );
   iSize = SetPointers();

   /* Standard flash calculations */
   if ( iInitialize == EOS_FLASH_CALCULATIONS )
   {
      /* Set the cache to the proper value */
      m_pEosPvtTable = pTEosPvtTable[0];

      /* Set minimum pressure */
      WriteMinimumPressure();

      /* Debug print */
      if ( m_iDebug )
      {
         PrintInputData( iSize );
      }

      /* Need to flash anyway if nothing restored */
      iFlashIt = ( ( iFlash || iSaved == EOS_OPTION_OFF ) && iNc > 0 ) ? 1 : 0;
      m_iBubbleDewPoint = m_iBubbleDewPoint && ( iNc > 0 );

      /* Check grid blocks for maximum changes, etc. */
      if ( iFlashIt || m_iBubbleDewPoint )
      {
         ( m_pApplication )->ModifyPhaseIdentification( m_dEnorm );
      }

      /* Reset the flags */
      iFlashIt = ( iFlashIt && iType == EOS_TOF_2P ) ? 1 : 0;
      iProps = ( iProperties && ( iType != EOS_TOF_0P ) && ( iNc > 0 ) ) ? 1 : 0;

      /* Bubble and dew point calculations */
      if ( m_iBubbleDewPoint )
      {
         BubblePointCalculations( iNc, iType );

         /* Check to see if we need to get water properties */
         if ( iWater && iProperties )
         {
            WaterProperties( EOS_OPTION_OFF, EOS_OPTION_ON );
         }
      }

      /* Single object */
      else if ( iNObj == 1 )
      {
         /* Load data */
         if ( iNc > 0 )
         {
            ( m_pApplication )->SetTrivialSlice();
            ReadData( 1, 1, iNc, EOS_OPTION_OFF, EOS_GETK, &iThermal );
         }

         /* Perform the flash calculations and store values */
         if ( iFlashIt )
         {
            FlashOneObject( iNc );
         }

         /*
          * Calculate and store properties and get derivatives if
          * requested
          */
         if ( iProps )
         {
            PropertiesOneObject( iNc, iType );
         }

         /* Check to see if we need to get water properties */
         if ( iWater && iProperties )
         {
            WaterProperties( EOS_OPTION_ON, EOS_OPTION_ON );
         }
      }

      /* Multiple objects */
      else
      {
         /* Perform the flash calculations */
         if ( iFlashIt )
         {
            FlashMultipleObjects( iNc );
         }

         /* Calculate and store properties and get derivatives */
         if ( iProps )
         {
            PropertiesMultipleObjects( iNc, iType );
         }

         /* Check to see if we need to get water properties */
         if ( iWater && iProperties )
         {
            WaterProperties( EOS_OPTION_OFF, EOS_OPTION_ON );
         }
      }

      /* Debug printing */
      if ( m_iDebug )
      {
         ( m_pApplication )->PrintOutputData();
      }
   }

   /* Compositional grading */
   else if ( iInitialize == EOS_COMPOSITIONAL_GRADING )
   {
      /* Set the cache to the proper value */
      m_pEosPvtTable = pTEosPvtTable[0];

      /* Set minimum pressure */
      WriteMinimumPressure();

      /* If hydrocarbons present perform the grading */
      if ( iType != EOS_TOF_0P )
      {
         CompositionalGrading( iNc, iType );
      }
   }

   /* Separator flash */
   else if ( iInitialize == EOS_SEPARATOR_FLASH )
   {
      SeparatorFlash( iNc, iType, iWater, iFlash, pTEosPvtTable );
   }

   /* Free up the work array */
   CDELETE( m_pWorkArray );
}


/* 
// BubblePointCalculations
//
// Subroutine to calculate bubble point 
//
// iNc
//    Number of components
// iType
//    Indicator for type of system.  EOS_TOF_2P means that
//    two hydrocarbon phases are allowed
//
// 1) One will note that all the loops tend to be of
//    the following form:
//       a) Reset the slice pointer
//       b) Set slice
//       c) Read data from Application
//       d) Do the deed
//       e) Write data to Application
// 2) Get some control variables from the EosApplication class
// 3) Generate an initial guess for the bubble point when
//    two phases are allowed
// 4) Iterate until convergence for objects that have
//    a valid initial guess
// 5) Generate properties when a bubble point exists
// 6) Generate properties for single phase objects
// 7) Set dummy properties when there are no hydrocarbons 
*/
void EosPvtModel::BubblePointCalculations( int iNc, int iType )
{
   int iMolarDensity;
   int iGetViscosity;
   int iGetTension;
   int iGetMW;
   int iM;
   int iTherm;

   /* See which simulator data needed */
   /* TODO: The next line is too long. Please rewrite to make it shorter. */
   ( m_pApplication )->WriteOutputData( &m_iDrv, &iGetViscosity, &iGetTension, &m_iHeat, &iGetMW, &m_iVolume, &m_iMolarFlash, &iMolarDensity );

   /* Bubble point initial guess */
   if ( iType == EOS_TOF_2P )
   {
      ( m_pApplication )->ResetSlice();
      iM = ( m_pApplication )->SetSlice( EOS_FL_1P_NCV, m_iFlashLength );
      while ( iM )
      {
         ReadData( iM, iM, iNc, EOS_OPTION_OFF, EOS_NOGETK, &iTherm );
         BubblePointInit( iM, iNc, EOS_NORETROGRADE );
         ( m_pApplication )->ReadFlashResults( iM, iM, EOS_FL_BP_NCV, EOS_NORESTORE, m_pSplit, m_pPhase, m_pKValue );
         iM = ( m_pApplication )->SetSlice( EOS_FL_1P_NCV, m_iFlashLength );
      }

      /* Bubble point iterations */
      ( m_pApplication )->ResetSlice();
      iM = ( m_pApplication )->SetSlice( EOS_FL_BP_NCV, m_iFlashLength );
      while ( iM )
      {
         ReadData( iM, iM, iNc, EOS_OPTION_OFF, EOS_GETK, &iTherm );
         BubblePointNewton( iM, iNc, EOS_FL_BP_CV );

         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         ( m_pApplication )->ReadFlashResults( iM, iM, ( m_iDrv ? EOS_FL_BP_CV : EOS_FL_BP_NCV ), EOS_NORESTORE, m_pSplit, m_pPhase, m_pKValue );
         iM = ( m_pApplication )->SetSlice( EOS_FL_BP_NCV, m_iFlashLength );
      }
   }

   /* Reset phase indicator */
   ( m_pApplication )->ModifyPhaseIndicator( EOS_OPTION_ON );

   /* Set bubble point properties */
   if ( iType == EOS_TOF_2P )
   {
      ( m_pApplication )->ResetSlice();
      iM = ( m_pApplication )->SetSlice( EOS_FL_BP, m_iFlashLength );
      while ( iM )
      {
         AdjustPointers( iM, iNc, EOS_OPTION_OFF );
         ReadData( iM, iM, iNc, EOS_OPTION_OFF, EOS_GETK, &iTherm );
         BubblePoint( iM, iNc );
         WriteData( iM, iNc, iGetViscosity, iGetTension, iMolarDensity, iType, EOS_FL_2P );
         iM = ( m_pApplication )->SetSlice( EOS_FL_BP, m_iFlashLength );
      }
   }

   /* Set single phase properties */
   ( m_pApplication )->ResetSlice();
   iM = ( m_pApplication )->SetSlice( EOS_FL_1P, m_iFlashLength );
   while ( iM )
   {
      AdjustPointers( iM, iNc, EOS_OPTION_OFF );
      ReadData( iM, iM, iNc, EOS_OPTION_OFF, EOS_NOGETK, &iTherm );
      OnePhase( iM, iNc );
      BubblePointZero( iM, iNc );
      WriteData( iM, iNc, iGetViscosity, iGetTension, iMolarDensity, iType, EOS_FL_1P );
      iM = ( m_pApplication )->SetSlice( EOS_FL_1P, m_iFlashLength );
   }

   /* Set no hydrocarbon properties */
   ( m_pApplication )->ResetSlice();
   iM = ( m_pApplication )->SetSlice( EOS_FL_0P, m_iFlashLength );
   while ( iM )
   {
      WriteData( iM, iNc, iGetViscosity, iGetTension, iMolarDensity, iType, EOS_FL_0P );
      iM = ( m_pApplication )->SetSlice( EOS_FL_0P, m_iFlashLength );
   }
}


/* 
// AdjustPointers
//
// Adjust pointers 
//
// iM
//    Number of objects in current slice
// iNc
//    Number of components
// isWater
//    Indicator for when this is being called for water phase
//    calculations
//
// 1) The purpose of this routine is to adjust the pointers
//    for the current slice so that the pressure, accumulation,
//    and temperature derivatives will be contiguous in
//    memory.  This will allow for easier loading into the
//    Application data structures
*/
void EosPvtModel::AdjustPointers( int iM, int iNc, int isWater )
{
   int iNcm;
   int iNc2;

   /* Constants */
   iNcm = iNc * iM;
   iNc2 = iNc * iNcm;

   /* For water */
   if ( isWater )
   {
      m_pDWda = m_pDWdp + iNcm;
      m_pDWdt = m_pDWda + iNc2;
      m_pDMWda = m_pDMWdp + iM;
      m_pDMWdt = m_pDMWda + iNcm;
      m_pDRhoWda = m_pDRhoWdp + iM;
      m_pDRhoWdt = m_pDRhoWda + iNcm;
      m_pDMuWda = m_pDMuWdp + iM;
      m_pDMuWdt = m_pDMuWda + iNcm;
      m_pDIftWda = m_pDIftWdp + iM;
      m_pDIftWdt = m_pDIftWda + iNcm;
      m_pDHeatWda = m_pDHeatWdp + iM;
      m_pDHeatWdt = m_pDHeatWda + iNcm;
   }

   /* Make adjustments for hydrocarbons */
   else
   {
      m_pDXda = m_pDXdp + iNcm;
      m_pDXdt = m_pDXda + iNc2;
      m_pDYda = m_pDYdp + iNcm;
      m_pDYdt = m_pDYda + iNc2;
      m_pDMxda = m_pDMxdp + iM;
      m_pDMxdt = m_pDMxda + iNcm;
      m_pDMyda = m_pDMydp + iM;
      m_pDMydt = m_pDMyda + iNcm;
      m_pDZxda = m_pDZxdp + iM;
      m_pDZxdt = m_pDZxda + iNcm;
      m_pDZyda = m_pDZydp + iM;
      m_pDZydt = m_pDZyda + iNcm;
      m_pDHxda = m_pDHxdp + iM;
      m_pDHxdt = m_pDHxda + iNcm;
      m_pDHyda = m_pDHydp + iM;
      m_pDHydt = m_pDHyda + iNcm;
      m_pDMuxda = m_pDMuxdp + iM;
      m_pDMuxdt = m_pDMuxda + iNcm;
      m_pDMuyda = m_pDMuydp + iM;
      m_pDMuydt = m_pDMuyda + iNcm;
      m_pDIfxda = m_pDIfxdp + iM;
      m_pDIfxdt = m_pDIfxda + iNcm;
      m_pDIfyda = m_pDIfydp + iM;
      m_pDIfydt = m_pDIfyda + iNcm;
      m_pDBpda = m_pDBpdp + iM;
      m_pDBpdt = m_pDBpda + iNcm;
   }
}


/* 
// PrintInputData
//
// Debug print routine 
//
// iSize
//    Size of work array
//
// 1) Print out data for this class
// 2) Also, print out data for EosApplication and EosPvtTable classes
*/
void EosPvtModel::PrintInputData( int iSize )
{
   /* Print out the data */
   printf( "\nPrinting input data..." );
   printf( "\nEosPvtModel.iFlashLength: " );
   printf( "%i", m_iFlashLength );
   printf( "\nEosPvtModel.iMichelson: " );
   printf( "%i", m_iMichelson );
   printf( "\nEosPvtModel.iMaxIterations: " );
   printf( "%i", m_iMaxIterations );
   printf( "\nEosPvtModel.iSubstitutions: " );
   printf( "%i", m_iSubstitutions );
   printf( "\nEosPvtModel.dEnorm: " );
   printf( "%e", m_dEnorm );
   printf( "\nEosPvtModel.dLnEnorm: " );
   printf( "%e", m_dLnEnorm );
   printf( "\nEosPvtModel.dTiny: " );
   printf( "%e", m_dTiny );
   printf( "\nEosPvtModel.dConverge: " );
   printf( "%e", m_dConverge );
   printf( "\nEosPvtModel.dThermalDiffusion: " );
   printf( "%e", m_dThermalDiffusion );
   printf( "\nEosPvtModel.dBubbleReduce: " );
   printf( "%e", m_dBubbleReduce );
   printf( "\nEosPvtModel.iDebug: " );
   printf( "%i", m_iDebug );
   printf( "\nEosPvtModel.iDrv: " );
   printf( "%i", m_iDrv );
   printf( "\nEosPvtModel.iHeat: " );
   printf( "%i", m_iHeat );
   printf( "\nEosPvtModel.iVolume: " );
   printf( "%i", m_iVolume );
   printf( "\nEosPvtModel.iMolarFlash: " );
   printf( "%i", m_iMolarFlash );
   printf( "\nEosPvtModel.iBubbleDewPoint: " );
   printf( "%i", m_iBubbleDewPoint );
   printf( "\nEosPvtModel.iSize: " );
   printf( "%i", iSize );
   printf( "\nEosPvtModel.*m_pMolecularWeight: " );
   printf( "%p", (void *)( m_pMolecularWeight ) );
   printf( "\nEosPvtModel.*m_pH: " );
   printf( "%p", (void *)( m_pH ) );
   printf( "\nEosPvtModel.*m_pFx: " );
   printf( "%p", (void *)( m_pFx ) );
   printf( "\nEosPvtModel.*m_pFy: " );
   printf( "%p", (void *)( m_pFy ) );
   printf( "\nEosPvtModel.*m_pWork: " );
   printf( "%p", (void *)( m_pWork ) );
   printf( "\nEosPvtModel.*pTerm1: " );
   printf( "%p", (void *)( m_pTerm1 ) );
   printf( "\nEosPvtModel.*pTerm2: " );
   printf( "%p", (void *)( m_pTerm2 ) );
   printf( "\nEosPvtModel.*pTermx: " );
   printf( "%p", (void *)( m_pTermx ) );
   printf( "\nEosPvtModel.*pTermy: " );
   printf( "%p", (void *)( m_pTermy ) );
   printf( "\nEosPvtModel.*pOSplit: " );
   printf( "%p", (void *)( m_pOSplit ) );
   printf( "\nEnd of work area: " );
   printf( "%p", (void *)( m_pMolecularWeight + iSize ) );
   printf( "\nEosPvtModel.*m_pApplication: " );
   printf( "%p", (void *)( m_pApplication ) );
   printf( "\nEosPvtModel.*pEosPvtTable: " );
   printf( "%p", (void *)( m_pEosPvtTable ) );
   printf( "\n" );
   ( m_pEosPvtTable )->PrintInputData();
   ( m_pApplication )->PrintInputData();
}


/* 
// ReadAllData
//
// Set all flasher related data 
//
// iVersion
//    Version of the flasher; set to zero
// piFlasher
//    Pointer to integer terms for the flasher
// pdFlasher
//    Pointer to double precision terms for the flasher
//
// 1) Set the data in the flasher from the outside world
*/
void EosPvtModel::ReadAllData( int iVersion, int *piFlasher, double *pdFlasher )
{
   USE_PARAM_EOSPVT( iVersion );

   /* Set flasher terms */
   m_dEnorm = pdFlasher[EOS_ENORM];
   m_dLnEnorm = log( m_dEnorm );
   m_dTiny = pdFlasher[EOS_TINY];
   m_dConverge = pdFlasher[EOS_CONVERGENCE];
   m_dThermalDiffusion = pdFlasher[EOS_THERMALDIFFUSION];
   m_dBubbleReduce = pdFlasher[EOS_BUBBLEREDUCE];
   m_iMaxIterations = piFlasher[EOS_MAXITN];
   m_iFlashLength = piFlasher[EOS_MAXFLASH];
   m_iMichelson = piFlasher[EOS_MICHELSON];
   m_iSubstitutions = piFlasher[EOS_SUBSTITUTIONS];
   m_iDebug = piFlasher[EOS_DEBUG];
}


/* 
// ReadData
//
// Load grid block properties into temporary vectors 
//
// iM
//    Number of objects in slice
// iNc
//    Number of components
// isSalt
//    Indicator if water phase (1) with salts, or hydrocarbons (0)
// iGetK
//    Indicator if whether to read saved K values.                 
//       EOS_NOGETK  
//          Do not get K values             
//       EOS_GETK   
//          Get K values
// pThermal
//    Indicator as to whether the system is thermal or isothermal
//
// 1) Load the properties from the EosApplication class.
//    Like pressure, temperature, and accumulation
// 2) Need to determine where to store molecular weights
//    since water calculations differ from hydrocarbon
// 3) Load some additional properties from the EosPvtTable
//    class, such as molecular weights and temperatures
//    for isothermal simulations  
// 4) Calculate mole fractions and total moles.  Note that
//    depending upon what type of data comes from the EosApplication
//    class that the molecular weight is involved. 
// 5) If doing the water phase, make sure that the total
//    amount of water is greater than zero   
*/
void EosPvtModel::ReadData( int iS, int iM, int iNc, int isSalt, int iGetK, int *pThermal )
{
   double  dA;
   double  dB;
   double  dC;
   double *pTa;
   double *pforMW;
   int     iMolar;
   int     i;
   int     i1;
   int     i2;
   int     iNi;

   /* Loop bounds */
   if ( iS == iM )
   {
      i1 = 0;
      i2 = iM;
   }
   else
   {
      i1 = iS;
      i2 = iS + 1;
   }

   /* Load the properties */
   /* TODO: The next line is too long. Please rewrite to make it shorter. */
   ( m_pApplication )->WriteData( iM, i1, i2, iGetK, isSalt, m_pPressure, m_pTemperature, m_pComposition, m_pSplit, m_pKValue, m_pPhase, pThermal, &iMolar );

   /* Use correct molecular weight */
   pforMW = isSalt ? m_pSaltMW : m_pMolecularWeight;

   /* Set the pvt term assignments */
   /* TODO: The next line is too long. Please rewrite to make it shorter. */
   ( m_pEosPvtTable )->WritePvtInfo( iM, i1, i2, *pThermal, isSalt, m_iWaterComp, m_pAbcOffset, &m_iMultipleAbc, m_pTemperature, pforMW );

   /* Reset thermal indicator */
   *pThermal = ( *pThermal == EOS_OPTION_OFF ) ? 1 : 0;

   /* Calculate mole fraction and moles for multiple blocks */
   if ( iM > 1 )
   {
      if ( iNc == 0 )
      {
         for ( i = i1; i < i2; i++ )
         {
            m_pMoles[i] = 0.0;
         }
      }
      else if ( iMolar )
      {
         pTa = m_pComposition;
         for ( i = i1; i < i2; i++ )
         {
            m_pMoles[i] = pTa[i];
         }

         for ( iNi = 1; iNi < iNc; iNi++ )
         {
            pTa += iM;
            for ( i = i1; i < i2; i++ )
            {
               m_pMoles[i] += pTa[i];
            }
         }
      }
      else
      {
         pTa = m_pComposition;
         dA = pforMW[0];
         for ( i = i1; i < i2; i++ )
         {
            pTa[i] = pTa[i] / dA;
            m_pMoles[i] = pTa[i];
         }

         for ( iNi = 1; iNi < iNc; iNi++ )
         {
            pTa += iM;
            dA = pforMW[iNi];
            for ( i = i1; i < i2; i++ )
            {
               pTa[i] = pTa[i] / dA;
               m_pMoles[i] += pTa[i];
            }
         }
      }

      /* Normalize */
      for ( i = i1; i < i2; i++ )
      {
         dA = m_pMoles[i];
         m_pWork[i] = 1.0 / ( dA > 0.0 ? dA : 1.0 );
      }

      pTa = m_pComposition;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         for ( i = i1; i < i2; i++ )
         {
            dA = pTa[i] * m_pWork[i];
            pTa[i] = ( dA >= m_dTiny ) ? dA : 0.0;
         }

         pTa += iM;
      }
   }

   /* Calculate mole fraction and moles for a single block */
   else
   {
      if ( iNc == 0 )
      {
         m_pMoles[0] = 0.0;
      }
      else if ( iMolar )
      {
         m_pMoles[0] = m_pComposition[0];
         for ( iNi = 1; iNi < iNc; iNi++ )
         {
            m_pMoles[0] += m_pComposition[iNi];
         }
      }
      else
      {
         m_pComposition[0] = m_pComposition[0] / pforMW[0];
         m_pMoles[0] = m_pComposition[0];
         for ( iNi = 1; iNi < iNc; iNi++ )
         {
            m_pComposition[iNi] = m_pComposition[iNi] / pforMW[iNi];
            m_pMoles[0] += m_pComposition[iNi];
         }
      }

      /* Normalize */
      dA = m_pMoles[0];
      dB = 1.0 / ( dA > 0.0 ? dA : 1.0 );
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         dC = m_pComposition[iNi] * dB;
         m_pComposition[iNi] = ( dC >= m_dTiny ) ? dC : 0.0;
      }
   }

   /* If water check to see if anything there and take action */
   if ( isSalt && m_iWaterComp >= 0 )
   {
      iNi = m_iWaterComp * iM;
      for ( i = i1; i < i2; i++ )
      {
         if ( m_pMoles[i] == 0.0 )
         {
            m_pComposition[iNi] = 1.0;
         }

         iNi++;
      }
   }
}


/* 
// WriteMinimumPressure
// 
// Write out the minimum pressure.  
//
// 1) Get the minimum allowed temperature in the flashes
// 2) The minimum pressure is then the vapor pressure of
//    water at that temperature  
// 3) Send the results to Application
*/
void EosPvtModel::WriteMinimumPressure( void )
{
   /* TODO: The next line is too long. Please rewrite to make it shorter. */
   ( m_pApplication )->ReadMinimumPressure( ( m_pEosPvtTable )->WaterVaporPressure( (double)( ( m_pApplication )->WriteIsothermal() ? ( m_pEosPvtTable )->WriteTemperature() : ( m_pApplication )->WriteMinimumTemperature() ) ) );
}


/* 
// SetPointers
// 
// Set pointers in temporary memory
//
// Returns the size of the double precision memory that
// is required.  Thus the first call to this routine
// can be used to get the amount of memory used, and the
// second call to properly set the pointers
//
// 1) Memory usage is mapped out in the EosPvtModelP.h file.
//    Be very careful when touching anything in this routine!
// 2) Determine the slice length (either that which
//    is input, or the number of objects)
// 3) Determine the maximum number of components that will appear
// 4) Set aside the memory for the local copy of the
//    molecular weights and the PVT input
// 5) Set memory for hydrocarbon properties.  Note that
//    care must be used to ensure double precision alignment
// 6) Then set temporary pointers for the EosPvtTable class
//    and ensure that the proper highwatermark is set
// 7) After setting additional property derivative pointers
//    by calling adjust pointers, set the temporary pointers
//    These points are generally, but not always, appearing
//    only in the flash calculations and not in the
//    property calculations
// 8) Set water pointers, and call AdjustPointers to 
//    intially set those which will vary.  Set the
//    highwatermark
// 9) Set temporary memory for EosApplication class, which is
//    mostly for K values and some indirection terms,
//    and save the new highwatermark.
// 10) Finally, do something very nasty, namely, set the
//     pointers to the entry in the ABC tables for each
//     object in each slice as the last area in memory.
//     Thus, if anything overwrites this, the flash is
//     ensured to fail.  Then get the new highwatermark
*/
int EosPvtModel::SetPointers( void )
{
   int     i;
   int     i1;
   int     iJ;
   int     iNcm;
   int     iNc2;
   int     iN;
   int     iNc;
   int     iSalts;
   int     iFlashes;
   double *pFinal;

   /* Get the maximum flash length */
   iFlashes = ( m_pApplication )->WriteNumberFlashes();
   iN = iFlashes > m_iFlashLength ? m_iFlashLength : iFlashes;

   /* Terms to set up locations */
   iNc = ( m_pEosPvtTable )->GetNumberHydrocarbons();
   iSalts = ( m_pEosPvtTable )->GetNumberSalts();
   iNc = ( iNc > iSalts ) ? iNc : ( iSalts + 1 );
   iNcm = iNc * iN;
   iNc2 = iNc * iNcm;

   /* Keep a local copy of the molecular weight */
   m_pMolecularWeight = m_pWorkArray;

   /* Set up the array for the phase indicator */
   m_pPhase = (int *)( m_pMolecularWeight + iNc );

   /* Set up locations for saved properties in work array */
   m_pPressure = ( (double *)m_pPhase ) + iN;
   m_pTemperature = m_pPressure + iN;
   m_pMoles = m_pTemperature + iN;
   m_pSplit = m_pMoles + iN;
   m_pKValue = m_pSplit + iN;
   m_pComposition = m_pKValue + iNcm;

   /* Bubble point locations */
   i = iNcm + iNcm + iNc2;
   iJ = iN + iN + iNcm;
   m_pBp = m_pSplit;
   if ( m_iBubbleDewPoint )
   {
      m_pDBpdp = m_pComposition + iNcm;
      m_pPhaseId = (int *)( m_pDBpdp + iJ );
   }
   else
   {
      m_pPhaseId = (int *)( m_pComposition + iNcm );
   }

   /* Set up locations in the work array */
   m_pX = ( (double *)m_pPhaseId ) + iN;
   m_pDXdp = m_pX + iNcm;
   m_pY = m_pDXdp + i;
   m_pDYdp = m_pY + iNcm;
   m_pMx = m_pDYdp + i;
   m_pDMxdp = m_pMx + iN;
   m_pMy = m_pDMxdp + iJ;
   m_pDMydp = m_pMy + iN;
   m_pZx = m_pDMydp + iJ;
   m_pDZxdp = m_pZx + iN;
   m_pZy = m_pDZxdp + iJ;
   m_pDZydp = m_pZy + iN;
   m_pHx = m_pDZydp + iJ;
   m_pDHxdp = m_pHx + iN;
   m_pHy = m_pDHxdp + iJ;
   m_pDHydp = m_pHy + iN;
   m_pMWx = m_pDHydp + iJ;
   m_pMWy = m_pMWx + iN;
   m_pMux = m_pMWy + iN;
   m_pDMuxdp = m_pMux + iN;
   m_pMuy = m_pDMuxdp + iJ;
   m_pDMuydp = m_pMuy + iN;
   m_pIfx = m_pDMuydp + iJ;
   m_pDIfxdp = m_pIfx + iN;
   m_pIfy = m_pDIfxdp + iJ;
   m_pDIfydp = m_pIfy + iN;
   m_pWork = m_pDIfydp + iJ;

   /* Save bubble point locations */
   if ( m_iBubbleDewPoint == EOS_OPTION_OFF )
   {
      m_pDBpdp = m_pDMydp;
   }

   /* Set pvt table pointers */
   ( m_pEosPvtTable )->SetPointers( iN, m_pMWy, m_pIfx, m_pWork, &m_pATable, &m_pSumTable, &pFinal );
   i = (int)( pFinal - m_pWorkArray );

   /* Give values to other pointers */
   if ( m_pWorkArray != ( (double *)0 ) )
   {
      AdjustPointers( iN, iNc, EOS_OPTION_OFF );
      m_pOSplit = m_pWork;
      m_pFx = m_pWork + iN;
      m_pG = m_pMWx;
      m_pH = m_pDXdp;
      m_pFy = m_pDXdt;
      m_pGx = m_pDHxdp;
      m_pGy = m_pDHxdt;
      m_pBeta = m_pDHydp;
      m_pGold = m_pDHydt;
      m_pLow = m_pDMydp;
      m_pHigh = m_pDMydt;
      m_pXRhs = m_pMx;
      m_pXMat = m_pDMxdt;
      m_pTerm1 = m_pMux;
      m_pTerm2 = m_pMuy;
      m_pTerm3 = m_pDMuydp;
      m_pTermx = m_pDMuxda;
      m_pTermy = m_pDMuyda;
      m_pPotentialx = m_pDMxda;
      m_pPotentialy = m_pDMyda;
      m_pLastx = pFinal - iNcm;
      m_pLasty = m_pDHyda;
      m_pPoint = m_pDMxdp;
   }

   /* Water terms */
   m_pSaltMW = m_pMolecularWeight;
   m_pMW = m_pX;
   m_pDMWdp = m_pMW + iN;
   m_pW = m_pY;
   m_pDWdp = m_pW + iNcm;
   m_pRhoW = m_pMx;
   m_pDRhoWdp = m_pRhoW + iN;
   m_pMuW = m_pMy;
   m_pDMuWdp = m_pMuW + iN;
   m_pIftW = m_pZx;
   m_pDIftWdp = m_pIftW + iN;
   m_pHeatW = m_pZy;
   m_pDHeatWdp = m_pHeatW + iN;
   m_pDRhoWds = m_pHx;
   m_pDMuWds = m_pDRhoWds + iN;
   m_pDHeatWds = m_pDMuWds + iN;
   m_pWorkW = m_pDHeatWds + iN;
   m_pSalinity = m_pWorkW + iN * 20;
   m_pDSda = m_pSalinity + iN;
   m_pMWW = m_pDSda + iNcm;
   pFinal = m_pMWW + iN;

   /* Set other water pointers */
   AdjustPointers( iN, iNc, EOS_OPTION_ON );

   /* Calculate the total length */
   i1 = (int)( pFinal - m_pWorkArray );
   i1 = ( i > i1 ? i : i1 );

   /* One more test */
   i = ( (int)( m_pWork - m_pWorkArray ) ) + iNcm - iN;
   i1 = ( i > i1 ? i : i1 );

   /* Set application pointers */
   pFinal = m_pWorkArray + i1 - iN - iN;
   ( m_pApplication )->SetPointers( iN, &pFinal );

   /* Set pvt pointers */
   ( m_pEosPvtTable )->SetAbcPointers( iN, &m_pAbcOffset, &pFinal );

   /* Calculate the total length */
   i = pFinal - m_pWorkArray;
   i = ( i > i1 ? i : i1 );

   /* End of the routine */
   return ( i );
}


/* 
// BackSolve
// 
// Back substitution routine  
//
// iM
//    Number of objects
// iNc
//    Number of components
// pMatrix
//    Pointer to matrix.  The array order is
//       Objects (first)
//       Columns
//       Rows (last)
//    Only the diagonal and bottom subdiagonal elements are used
// pRhs
//    Pointer to the right hand side.  The array order is
//       Objects (first)
//       Columns (last) 
//
// 1) This routine assumes that Cholesky has first been called
// 2) This routine solves an equation of the form
//    
//          T
//       L L  S = R
// 
//    by solving, in order
//
//       ^    -1               -T ^
//       R = L   R        R = L   R
//
//    The right hand side is destroyed in the process.  See
//    J. E. Dennis and R. Schnabel, Numerical Methods for
//    Unconstrained Optimization and Nonlinear Equations, 
//    Prentice Hall, Englewood Cliffs, N.J., 1983.
*/
void EosPvtModel::BackSolve( int iM, int iNc, double *pMatrix, double *pRhs )
{
   int     i;
   int     iNc1;
   int     iNcm;
   int     iNi;
   int     iNj;
   int     iNk;
   int     iNn;
   double *pTa;
   double *pTb;
   double *pTc;
   double *pTd;

   /* Code for more than one grid block */
   if ( iM > 1 )
   {
      iNcm = iNc * iM;
      iNc1 = iNcm + iM;

      /* Forward loop */
      pTa = pRhs;
      pTb = pMatrix;
      for ( iNi = 1; iNi < iNc; iNi++ )
      {
         pTa += iM;
         pTb += iM;
         pTc = pRhs;
         pTd = pTb;
         for ( iNj = 0; iNj < iNi; iNj++ )
         {
#ifdef IPFtune
            int cnt = iM;
#pragma ivdep
            for ( i = 0; i < cnt; i++ )
            {
               pTa[i] += pTd[i] * pTc[i];
            }
#else
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
            for ( i = 0; i < iM; i++ )
            {
               pTa[i] += pTd[i] * pTc[i];
            }
#endif

            pTc += iM;
            pTd += iNcm;
         }
      }

      /* Hit by the diagonal matrix */
      pTa = pRhs;
      pTb = pMatrix;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
#ifdef IPFtune
         int cnt = iM;
#pragma ivdep
         for ( i = 0; i < cnt; i++ )
         {
            pTa[i] *= pTb[i];
         }
#else
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
         for ( i = 0; i < iM; i++ )
         {
            pTa[i] *= pTb[i];
         }
#endif

         pTa += iM;
         pTb += iM + iNcm;
      }

      /* Backsolve loop */
      pTa -= iM;
      pTb -= iNcm;
      for ( iNi = 1; iNi < iNc; iNi++ )
      {
         pTc = pTa;
         pTa -= iM;
         pTb -= iNc1;
         pTd = pTb;
         for ( iNj = 0; iNj < iNi; iNj++ )
         {
#ifdef IPFtune
            int cnt = iM;
#pragma ivdep
            for ( i = 0; i < cnt; i++ )
            {
               pTa[i] += pTd[i] * pTc[i];
            }
#else    
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
            for ( i = 0; i < iM; i++ )
            {
               pTa[i] += pTd[i] * pTc[i];
            }
#endif

            pTc += iM;
            pTd += iM;
         }
      }
   }

   /* Code for one grid block */
   else
   {
      iNc1 = iNc + 1;

      /* Forward loop */
      for ( iNi = 1; iNi < iNc; iNi++ )
      {
         iNk = iNi;
#ifdef IPFtune
         double tRhs = pRhs[iNi];
#pragma ivdep
         for ( iNj = 0; iNj < iNi; iNj++ )
         {
            tRhs += pMatrix[iNk] * pRhs[iNj];
            iNk += iNc;
         }
         pRhs[iNi] = tRhs;
#else
         for ( iNj = 0; iNj < iNi; iNj++ )
         {
            pRhs[iNi] += pMatrix[iNk] * pRhs[iNj];
            iNk += iNc;
         }
#endif 
      }

      /* Hit by the diagonal matrix */
      iNk = 0;
#ifdef IPFtun
#pragma ivdep
#endif      
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         pRhs[iNi] *= pMatrix[iNk];
         iNk += iNc1;
      }

      /* Backsolve loop */
      iNk -= iNc1;
      for ( iNi = iNc - 2; iNi >= 0; iNi-- )
      {
         iNk -= iNc1;
         iNn = iNk;
#ifdef IPFtune
         double tRhs = pRhs[iNi];
#pragma ivdep 
         for ( iNj = iNi + 1; iNj < iNc; iNj++ )
         {
            tRhs += pMatrix[++iNn] * pRhs[iNj];
         }
         pRhs[iNi] = tRhs;
#else 
         for ( iNj = iNi + 1; iNj < iNc; iNj++ )
         {
            pRhs[iNi] += pMatrix[++iNn] * pRhs[iNj];
         }
#endif
      }
   }
}


/* 
// Cholesky
//
// Cholesky factorization routine  
//
// iM
//    Number of objects
// iNc
//    Number of components
// pMatrix
//    Pointer to matrix.  The array order is
//       Objects (first)
//       Columns
//       Rows (last)
//    Only the diagonal and bottom subdiagonal elements are used.
//    The matrix is destroyed in the process, and only the
//    factored matrix remains
//
// 1) Generates a modified cholesky decomposition
//    of pMatrix, which is destroyed in the process.
//    this modified cholesky method is needed due
//    to the fact that negative curvature is often
//    encountered during the flash calculations
//    The first step is to compute the square root
//    of the maximum diagonal element of the matrix
//
//    AMAX = SQRT ( MAX ( ABS ( A    ) , ABS ( A    ) / NC ) )
//                               I,I            I,J
//
// 2) Now for each column, i = 1 to n, do
//
//                  I-1  2
//    A    = A    - SUM A
//     I,I    I,I   J=1  J,I
//
// 3) For each row j = i+1 to n in column i, do
//
//                  I-1
//    A    = A    + SUM A    * A
//     J,I    J,I   K=1  J,K    I,K
//
// 4) Compute perturbation parameter
//
//             N
//    MAXA =  MAX  ( A    ) / AMAX
//           J=I+1    J,I
//
// 5) Compute the diagonal term
//
//                                   2
//    A    = SQRT ( MAX ( A    , MAXA  , EPSMAC ) )
//     I,I                 I,I
//
// 6) For each row j = i+1 to n in column i, do
//
//    A    = A    / A
//     J,I    J,I    I,I
//
// 7) If the matrix is safely positive definite, we
//    have then formed a lower traingular matrix l
//    stored in "a" such that
//
//         T
//    A = L L - D
//
//    where D is a zero matrix.  If the original
//    matrix was not positive definite, the matrix
//    D is a positive diagonal matrix which is can
//    be computed from the maxa perturbation.  See
//    J. E. Dennis and R. Schnabel, Numerical Methods for
//    Unconstrained Optimization and Nonlinear Equations, 
//    Prentice Hall, Englewood Cliffs, N.J., 1983.
*/
void EosPvtModel::Cholesky( int iM, int iNc, double *pMatrix )
{
   int     iNc1;
   int     iNcm;
   int     iNi;
   int     iNj;
   int     iNk;
   int     iNm;
   int     iNn;
   int     iNo;
   int     iNp;
   int     iNq;
   int     i;
   double  dA;
   double  dB;
   double  dC;
   double  dD;
   double  dTerm;
   double  dNc;
   double  dTerm1;
   double  dTerm2;
   double *pTa;
   double *pTb;
   double *pTc;
   double *pTd;
   double *pTe;
   double *pTf;

   /* Temporary term */
   dTerm = sqrt( m_dTiny );
   dNc = (double)iNc;

   /* Code for more than one grid block */
   if ( iM > 1 )
   {
      iNcm = iNc * iM;
      iNc1 = iNcm + iM;

      /* Set scaling factor */
      pTa = pMatrix;
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
      for ( i = 0; i < iM; i++ )
      {
         dA = pTa[i];
         m_pTerm3[i] = dA > 0.0 ? dA : -dA;
      }

      for ( iNi = 1; iNi < iNc; iNi++ )
      {
         pTa += iNcm + iM;
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
         for ( i = 0; i < iM; i++ )
         {
            dB = pTa[i];
            dA = dB > 0.0 ? dB : -dB;
            dB = m_pTerm3[i];
            m_pTerm3[i] = ( dA > dB ) ? dA : dB;
         }
      }

      dA = 1.0 / dNc;
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
      for ( i = 0; i < iM; i++ )
      {
         m_pTerm3[i] = m_pTerm3[i] * dA;
      }

      iNk = 0;
      pTa = pMatrix;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         pTa += iNk;
         iNk += iM;
         for ( iNj = iNi + 1; iNj < iNc; iNj++ )
         {
#ifdef IPFtune
            int     cnt = iM;
            double *pT3 = m_pTerm3;
#pragma ivdep
            for ( i = 0; i < cnt; i++ )
            {
               dB = pTa[i];
               dA = fabs(dB);
               dB = pT3[i];
               pT3[i] = ( dA > dB ) ? dA : dB;
            }
#else
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
            for ( i = 0; i < iM; i++ )
            {
               dB = pTa[i];
               dA = dB > 0.0 ? dB : -dB;
               dB = m_pTerm3[i];
               m_pTerm3[i] = ( dA > dB ) ? dA : dB;
            }
#endif
         }

         pTa += iM;
      }

#ifdef IPFtune
      double *pT3 = m_pTerm3;
      double tiny = m_dTiny;
#pragma ivdep
      for ( i = 0; i < iM; i++ )
      {
         dA = dNc * pT3[i];
         pT3[i] = 1.0 / sqrt( ( dA > tiny ) ? dA : tiny );
      }
#else
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
      for ( i = 0; i < iM; i++ )
      {
         dA = dNc * m_pTerm3[i];
         m_pTerm3[i] = 1.0 / sqrt( ( dA > m_dTiny ) ? dA : m_dTiny );
      }
#endif

      /* For every row */
      pTa = pMatrix;
      pTb = pTa;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         /* Modify the diagonal element */
         pTd = pTb;
         pTe = pMatrix;
         for ( iNj = 0; iNj < iNi; iNj++ )
         {
#ifdef IPFtune
            int cnt = iM;
#pragma ivdep
            for ( i = 0; i < cnt; i++ )
            {
               dB = pTd[i];
               dA = 0-pTe[i] * dB;
               pTa[i] += dA * dB;
               pTd[i] = dA;
            }
#else
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
            for ( i = 0; i < iM; i++ )
            {
               dB = pTd[i];
               dA = -pTe[i] * dB;
               pTa[i] += dA * dB;
               pTd[i] = dA;
            }
#endif

            pTd += iNcm;
            pTe += iNc1;
         }

         /* Prepare to patch column */
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
         for ( i = 0; i < iM; i++ )
         {
            m_pTerm2[i] = 0.0;
         }

         /* Go down column */
         pTd = pTb;
         pTc = pTa + iM;
         for ( iNj = iNi + 1; iNj < iNc; iNj++ )
         {
            pTd += iM;
            pTe = pTb;
            pTf = pTd;
            for ( iNk = 0; iNk < iNi; iNk++ )
            {
#ifdef IPFtune
               int cnt = iM;
#pragma ivdep
               for ( i = 0; i < cnt; i++ )
               {
                  pTc[i] += pTe[i] * pTf[i];
               }
#else
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
               for ( i = 0; i < iM; i++ )
               {
                  pTc[i] += pTe[i] * pTf[i];
               }
#endif

               pTe += iNcm;
               pTf += iNcm;
            }

#ifdef IPFtune
            double *pT2 = m_pTerm2;
            int     cnt = iM;
#pragma ivdep
            for ( i = 0; i < cnt; i++ )
            {
               dB = pTc[i];
               dA = fabs(dB);
               dB = pT2[i];
               pT2[i] = ( dA > dB ) ? dA : dB;
            }
#else
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
            for ( i = 0; i < iM; i++ )
            {
               dB = pTc[i];
               dA = dB > 0.0 ? dB : -dB;
               dB = m_pTerm2[i];
               m_pTerm2[i] = ( dA > dB ) ? dA : dB;
            }
#endif

            pTc += iM;
         }

         /* Patch term for column and take inverse */
#ifdef IPFtune
                 pT3 = m_pTerm3;
         double *pT2 = m_pTerm2;
#pragma ivdep
         for ( i = 0; i < iM; i++ )
         {
            dB = pTa[i];
            dA = fabs(dB);
            dB = pT3[i] * pT2[i];
            dC = dB * dB;
            dD = dTerm > dC ? dTerm : dC;
            pTa[i] = 1.0 / ( dD > dA ? dD : dA );
         }
#else
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
         for ( i = 0; i < iM; i++ )
         {
            dB = pTa[i];
            dA = dB > 0.0 ? dB : -dB;
            dB = m_pTerm3[i] * m_pTerm2[i];
            dC = dB * dB;
            dD = dTerm > dC ? dTerm : dC;
            pTa[i] = 1.0 / ( dD > dA ? dD : dA );
         }
#endif

         pTa += iNcm + iM;
         pTb += iM;
      }
   }

   /* Code for one grid block */
   else
   {
      iNc1 = iNc + 1;

      /* Set scaling factor */
      iNk = 0;
      dB = pMatrix[0];
      dTerm1 = dB > 0.0 ? dB : -dB;
      for ( iNi = 1; iNi < iNc; iNi++ )
      {
         iNk += iNc1;
         dB = pMatrix[iNk];
         dA = dB > 0.0 ? dB : -dB;
         dTerm1 = ( dA > dTerm1 ) ? dA : dTerm1;
      }

      iNk = 0;
      dTerm1 = dTerm1 / dNc;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         iNk += iNi;
         for ( iNj = iNi + 1; iNj < iNc; iNj++ )
         {
            dB = pMatrix[iNk++];
            dA = dB > 0.0 ? dB : -dB;
            dTerm1 = ( dA > dTerm1 ) ? dA : dTerm1;
         }
      }

      dA = dNc * dTerm1;
      dTerm1 = 1.0 / sqrt( ( dA > m_dTiny ) ? dA : m_dTiny );

      /* For every row */
      iNm = 0;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         /* Modify the diagonal element */
         iNn = iNi;
         iNo = 0;
         dC = pMatrix[iNm];
         for ( iNj = 0; iNj < iNi; iNj++ )
         {
            dB = pMatrix[iNn];
            dA = -pMatrix[iNo] * dB;
            dC += dA * dB;
            pMatrix[iNn] = dA;
            iNn += iNc;
            iNo += iNc1;
         }

         pMatrix[iNm] = dC;

         /* Go down column */
         iNo = iNm;
         dTerm2 = 0;
         for ( iNj = iNi + 1; iNj < iNc; iNj++ )
         {
            iNo++;
            iNp = iNi;
            iNq = iNj;
            for ( iNk = 0; iNk < iNi; iNk++ )
            {
               pMatrix[iNo] += pMatrix[iNp] * pMatrix[iNq];
               iNp += iNc;
               iNq += iNc;
            }

            dA = fabs( pMatrix[iNo] );
            dTerm2 = ( dA > dTerm2 ) ? dA : dTerm2;
         }

         /* Patch term for column and take inverse */
         dA = fabs( pMatrix[iNm] );
         dB = dTerm1 * dTerm2;
         dC = dB * dB;
         dD = dTerm > dC ? dTerm : dC;
         pMatrix[iNm] = 1.0 / ( dD > dA ? dD : dA );
         iNm += iNc1;
      }
   }
}


/* 
// FlashMultipleObjects
//
// Subroutine to perform two phase flash calculations 
//
// iNc
//    Number of components
//
// 1) Note that every flash calculation is basically arranged
//    in the following way
//    a) Reset the slice pointer to beginning
//    b) For each slice...
//    c) Read Application data
//    d) Perform calculations
//    e) Write Application data
// 2) First of all there is a section for when restoring
//    old information.  If anything fails during this
//    section, the type of phase identification is set
//    so that a stability test will be performed later on
// 3) First need to update bubble point information for
//    any blocks with a bubble point
// 4) The bubble point information must then be checked.
//    Any grid block with a pressure less than the bubble
//    point is probably two phase
// 5) Then do some successive substitution iterations.
//    These might converge very quickly
// 6) Then do the full Newton iterations.  If the Newton
//    iterations do not converge, we assume there is a 
//    problem with negative curvature in the old K value
//    initial guess and prepare to redo the stability test
// 7) The remainder of this routine does not require the
//    saved K values.  In these cases, if
//    the test fails, we save the state of the system.
// 8) We then do either a full Michelson stability test
//    or a simplified stability test.  
// 9) Successive substitution is then applied to blocks
//    which can possibly be two phase.  The "possibly"
//    applies since the simplified test is only there
//    to get a reasonable initial guess that will converge
//    if the system is two phase
// 10) Then apply Newton iterations.  This time if there
//     is no convergence, we accept the solution
// 11) If we are tracking the bubble point, then generate
//     an initial guess
// 12) For objects with a valid initial guess for the bubble
//     point use Newton's method to obtain a solution.
*/
void EosPvtModel::FlashMultipleObjects( int iNc )
{
   int iM;
   int iRestore;
   int iThermal;

   /* Section for restored data */
   iRestore = ( m_pApplication )->WriteOldValues();
   if ( iRestore )
   {
      /* Reset bubble point terms */
      if ( m_iBubbleDew )
      {
         ( m_pApplication )->ModifyBubble( EOS_OPTION_ON, EOS_FL_1P_NCV, EOS_FL_BP_NCV );

         /* Bubble point iterations */
         ( m_pApplication )->ResetSlice();
         iM = ( m_pApplication )->SetSlice( EOS_FL_BP_NCV, m_iFlashLength );
         while ( iM )
         {
            /* Set up locations in the work array */
            ReadData( iM, iM, iNc, EOS_OPTION_OFF, EOS_GETK, &iThermal );

            /* Calculate the bubble point */
            BubblePointNewton( iM, iNc, EOS_FL_BP_CHK );

            /* Store grid block values */
            ( m_pApplication )->ReadFlashResults( iM, iM, EOS_FL_BP_CV, EOS_NORESTORE, m_pSplit, m_pPhase, m_pKValue );
            iM = ( m_pApplication )->SetSlice( EOS_FL_BP_NCV, m_iFlashLength );
         }

         /* Check bubble point */
         ( m_pApplication )->ResetSlice();
         iM = ( m_pApplication )->SetSlice( EOS_FL_BP_CHK, m_iFlashLength );
         while ( iM )
         {
            /* Set up locations in the work array */
            ReadData( iM, iM, iNc, EOS_OPTION_OFF, EOS_GETK, &iThermal );

            /* Test the bubble point */
            BubblePointTest( iM, iNc );

            /* Store grid block values */
            ( m_pApplication )->ReadFlashResults( iM, iM, EOS_FL_2P_NCV, EOS_NORESTORE, m_pSplit, m_pPhase, m_pKValue );
            iM = ( m_pApplication )->SetSlice( EOS_FL_BP_CHK, m_iFlashLength );
         }
      }

      /* Successive substitution */
      if ( m_iSubstitutions )
      {
         ( m_pApplication )->ResetSlice();
         iM = ( m_pApplication )->SetSlice( EOS_FL_2P_NCV, m_iFlashLength );
         while ( iM )
         {
            /* Set up locations in the work array */
            ReadData( iM, iM, iNc, EOS_OPTION_OFF, EOS_GETK, &iThermal );

            /* Do the successive substitutions */
            Substitution( iM, iNc );

            /* Store grid block values */
            ( m_pApplication )->ReadFlashResults( iM, iM, EOS_FL_2P_NCV, EOS_NORESTORE, m_pSplit, m_pPhase, m_pKValue );
            iM = ( m_pApplication )->SetSlice( EOS_FL_2P_NCV, m_iFlashLength );
         }
      }

      /* Newton iterations */
      ( m_pApplication )->ResetSlice();
      iM = ( m_pApplication )->SetSlice( EOS_FL_2P_NCV, m_iFlashLength );
      while ( iM )
      {
         /* Set up locations in the work array */
         ReadData( iM, iM, iNc, EOS_OPTION_OFF, EOS_GETK, &iThermal );

         /* Do the Newton iterations */
         NewtonFlash( iM, iNc, EOS_NORESTORE, EOS_FL_2P_CV );

         /* Store grid block values */
         ( m_pApplication )->ReadFlashResults( iM, iM, EOS_FL_2P_CV, EOS_NORESTORE, m_pSplit, m_pPhase, m_pKValue );
         iM = ( m_pApplication )->SetSlice( EOS_FL_2P_NCV, m_iFlashLength );
      }
   }

   /* Michelson stability analysis */
   if ( m_iMichelson )
   {
      ( m_pApplication )->ResetSlice();
      iM = ( m_pApplication )->SetSlice( EOS_FL_1P_NCV, m_iFlashLength );
      while ( iM )
      {
         /* Set up locations in the work array */
         ReadData( iM, iM, iNc, EOS_OPTION_OFF, EOS_NOGETK, &iThermal );

         /* Do the Michelson stability analysis */
         Michelson( iM, iNc );

         /* Store grid block values */
         ( m_pApplication )->ReadFlashResults( iM, iM, EOS_FL_2P_NCV, iRestore, m_pSplit, m_pPhase, m_pKValue );
         iM = ( m_pApplication )->SetSlice( EOS_FL_1P_NCV, m_iFlashLength );
      }
   }

   /* Fast stability analysis */
   if ( m_iMichelson == EOS_OPTION_OFF )
   {
      ( m_pApplication )->ResetSlice();
      iM = ( m_pApplication )->SetSlice( EOS_FL_1P_NCV, m_iFlashLength );
      while ( iM )
      {
         /* Set up locations in the work array */
         ReadData( iM, iM, iNc, EOS_OPTION_OFF, EOS_NOGETK, &iThermal );

         /* Do the fast stability analysis */
         FastInitialization( iM, iNc );

         /* Store grid block values */
         ( m_pApplication )->ReadFlashResults( iM, iM, EOS_FL_2P_NCV, iRestore, m_pSplit, m_pPhase, m_pKValue );
         iM = ( m_pApplication )->SetSlice( EOS_FL_1P_NCV, m_iFlashLength );
      }
   }

   /* Successive substitution */
   if ( m_iSubstitutions )
   {
      ( m_pApplication )->ResetSlice();
      iM = ( m_pApplication )->SetSlice( EOS_FL_2P_NCV, m_iFlashLength );
      while ( iM )
      {
         /* Set up locations in the work array */
         ReadData( iM, iM, iNc, EOS_OPTION_OFF, EOS_GETK, &iThermal );

         /* Do the successive substitutions */
         Substitution( iM, iNc );

         /* Store grid block values */
         ( m_pApplication )->ReadFlashResults( iM, iM, EOS_FL_2P_NCV, iRestore, m_pSplit, m_pPhase, m_pKValue );
         iM = ( m_pApplication )->SetSlice( EOS_FL_2P_NCV, m_iFlashLength );
      }
   }

   /* Newton iterations */
   ( m_pApplication )->ResetSlice();
   iM = ( m_pApplication )->SetSlice( EOS_FL_2P_NCV, m_iFlashLength );
   while ( iM )
   {
      /* Set up locations in the work array */
      ReadData( iM, iM, iNc, EOS_OPTION_OFF, EOS_GETK, &iThermal );

      /* Do the Newton iterations */
      NewtonFlash( iM, iNc, iRestore, EOS_FL_2P_NCV );

      /* Store grid block values */
      ( m_pApplication )->ReadFlashResults( iM, iM, EOS_FL_2P_NCV, iRestore, m_pSplit, m_pPhase, m_pKValue );
      iM = ( m_pApplication )->SetSlice( EOS_FL_2P_NCV, m_iFlashLength );
   }

   /* Bubble point initial guess */
   if ( m_iBubbleDew )
   {
      ( m_pApplication )->ResetSlice();
      iM = ( m_pApplication )->SetSlice( EOS_FL_1P_NCV, m_iFlashLength );
      while ( iM )
      {
         /* Set up locations in the work array */
         ReadData( iM, iM, iNc, EOS_OPTION_OFF, EOS_NOGETK, &iThermal );

         /* Generate the initial guess */
         BubblePointInit( iM, iNc, EOS_NORETROGRADE );

         /* Store grid block values */
         ( m_pApplication )->ReadFlashResults( iM, iM, EOS_FL_BP_NCV, iRestore, m_pSplit, m_pPhase, m_pKValue );
         iM = ( m_pApplication )->SetSlice( EOS_FL_1P_NCV, m_iFlashLength );
      }

      /* Bubble point iterations */
      ( m_pApplication )->ResetSlice();
      iM = ( m_pApplication )->SetSlice( EOS_FL_BP_NCV, m_iFlashLength );
      while ( iM )
      {
         /* Set up locations in the work array */
         ReadData( iM, iM, iNc, EOS_OPTION_OFF, EOS_GETK, &iThermal );

         /* Calculate the bubble point */
         BubblePointNewton( iM, iNc, EOS_FL_BP_CV );

         /* Store grid block values */
         ( m_pApplication )->ReadFlashResults( iM, iM, EOS_FL_BP_CV, EOS_NORESTORE, m_pSplit, m_pPhase, m_pKValue );
         iM = ( m_pApplication )->SetSlice( EOS_FL_BP_NCV, m_iFlashLength );
      }
   }

   /* Reset phase indicator */
   ( m_pApplication )->ModifyPhaseIndicator( EOS_OPTION_ON );

   /* Reset bubble point terms */
   if ( m_iBubbleDew )
   {
      ( m_pApplication )->ModifyBubble( EOS_TO_APPLICATION, EOS_FL_BP, EOS_FL_1P );
   }
}


/* 
// FlashOneObject
// 
// Subroutine to perform two phase flash calculations 
//
// iNc
//    Number of components
//
// 1) First of all there is a section for when restoring
//    old information.  If anything fails during this
//    section, the type of phase identification is set
//    so that a stability test will be performed later on
// 2) First need to update bubble point information for
//    any blocks with a bubble point
// 3) The bubble point information must then be checked.
//    Any grid block with a pressure less than the bubble
//    point is probably two phase
// 4) Then do some successive substitution iterations.
//    These might converge very quickly
// 5) Then do the full Newton iterations.  If the Newton
//    iterations do not converge, we assume there is a 
//    problem with negative curvature in the old K value
//    initial guess and prepare to redo the stability test
// 6) The remainder of this routine does not require the
//    saved K values.  In these cases, if
//    the test fails, we save the state of the system.
// 7) We then do either a full Michelson stability test
//    or a simplified stability test.  
// 8) Successive substitution is then applied to blocks
//    which can possibly be two phase.  The "possibly"
//    applies since the simplified test is only there
//    to get a reasonable initial guess that will converge
//    if the system is two phase
// 9) Then apply Newton iterations.  This time if there
//    is no convergence, we accept the solution
// 10) If we are tracking the bubble point, then generate
//     an initial guess
// 11) For objects with a valid initial guess for the bubble
//     point use Newton's method to obtain a solution.
// 12) Finally, we only save the state of the system at
//     the end of the routine
*/
void EosPvtModel::FlashOneObject( int iNc )
{
   int    iRestore;
   int    iTested;
   double dA;

   /* Section for restored data */
   iTested = EOS_OPTION_OFF;
   iRestore = ( m_pApplication )->WriteOldValues();
   if ( iRestore )
   {
      /* Reset bubble point terms */
      if ( m_iBubbleDew )
      {
         dA = *m_pSplit;
         if ( *m_pPhase == EOS_FL_1P_NCV && dA < 0.0 )
         {
            *m_pPhase = EOS_FL_BP_NCV;
            *m_pSplit = -dA;
         }

         /* Bubble point iterations */
         if ( *m_pPhase == EOS_FL_BP_NCV )
         {
            BubblePointNewton( 1, iNc, EOS_FL_BP_CHK );
         }

         /* Check bubble point */
         if ( *m_pPhase == EOS_FL_BP_CHK )
         {
            BubblePointTest( 1, iNc );
         }
      }

      /* Successive substitution */
      if ( m_iSubstitutions && *m_pPhase == EOS_FL_2P_NCV )
      {
         Substitution( 1, iNc );
      }

      /* Newton iterations */
      if ( *m_pPhase == EOS_FL_2P_NCV )
      {
         NewtonFlash( 1, iNc, EOS_NORESTORE, EOS_FL_2P_NCV );
      }
   }

   /* Michelson stability analysis */
   if ( m_iMichelson && *m_pPhase == EOS_FL_1P_NCV )
   {
      iTested = iRestore;
      Michelson( 1, iNc );
   }

   /* Fast stability analysis */
   else if ( *m_pPhase == EOS_FL_1P_NCV )
   {
      iTested = iRestore;
      FastInitialization( 1, iNc );
   }

   /* Successive substitution */
   if ( m_iSubstitutions && *m_pPhase == EOS_FL_2P_NCV )
   {
      Substitution( 1, iNc );
   }

   /* Newton iterations */
   if ( *m_pPhase == EOS_FL_2P_NCV )
   {
      NewtonFlash( 1, iNc, iRestore, EOS_FL_2P_NCV );
   }

   /* Bubble point initial guess */
   if ( m_iBubbleDew )
   {
      if ( *m_pPhase == EOS_FL_1P_NCV )
      {
         BubblePointInit( 1, iNc, EOS_NORETROGRADE );
      }

      /* Bubble point iterations */
      if ( *m_pPhase == EOS_FL_BP_NCV )
      {
         BubblePointNewton( 1, iNc, EOS_FL_BP_CV );
      }

      /* Reset bubble point indicator */
      if ( *m_pPhase == EOS_FL_BP_NCV || *m_pPhase == EOS_FL_BP_CV )
      {
         *m_pPhase = EOS_FL_1P_CV;
         *m_pSplit = -*m_pSplit;
      }
   }

   /* Store grid block values and reset phase indicator */
   if ( *m_pPhase != EOS_FL_0P )
   {
      ( m_pApplication )->ReadFlashResults( 1, 1, EOS_FL_1P_CV, iTested, m_pSplit, m_pPhase, m_pKValue );
      ( m_pApplication )->ModifyPhaseIndicator( EOS_OPTION_OFF );
      *m_pPhase = ( *m_pPhase + 1 ) / 2;
   }
}


/* 
// FlashEquations
//
// Solve the flash equations 
//
// iM
//    Number of objects in slice
// i1
//    Starting point for flash in slice
// i2
//    Ending point for flash in slice
// iNc
//    Number of components
// iUpdate
//    Indicator whether to update the mole numbers after all
//    is complete
//
// 
//    1) Define the following
//
//       CHIX  = X  * ( ONE - SPLIT ) , CHIY  = Y  * SPLIT
//           I    I                         I    I
//
//    2) We have the following material balances
//
//       Z  = CHIX  + CHIY   ,  K  = Y  / X  
//        I       I       I      I    I    I
//
//    3) Based upon the material balances, we can
//       obtain a standard flash equation
//
//       F(SPLIT) = SUM Z  * ( K  - 1 ) / ( 1 - SPLIT + SPLIT * K  ) 
//                   I   I      I                                I
//                       
//       This equation must be zero at the solution.  The Multisim 
//       experience is that this equation has overflow trouble if the
//       denominator is evaluated as
//
//       DENOM = 1 + SPLIT * ( K  - 1 )
//                              I
//
//       The following form, which is used in the
//       code below, has proved reliable in MULTISIM
//
//       DENOM = ( 1 - SPLIT ) + SPLIT * K
//                                        I
//    4) The first order of business is to see if we
//       have a solution in the interval [0,1].  As
//       this function is decreasing on this interval,
//       this will only happen if f(0) and f(1) have
//       different signs.  Normally the flash equation
//       should have a solution.  
//
//    5) If there is no solution then the K values
//       are reset to one and the phase indicator
//       reset
//
//    6) The starting guess is a split of 0.5, set in
//       another routine.  This has proved as good as anything.  
//       On subsequent calls we use a restored value.  When the
//       K values get very large or small, the big
//       slopes near the origin have a bad affect upon
//       saved starting guesses.  The split is then
//       updated using Newton's method.  Bisection (with a 
//       maximum step of 0.9 towards the end of the world) is 
//       employed to keep the solution in the interval [0,1]
//
//    7) At the end of the routine, if needed, the compositions
//       are determined from
//
//       CHIX  = ( 1 - SPLIT ) * Z  / ( 1 - SPLIT + SPLIT * K  ) ,
//           I                    I                          I
//
//       CHIY  = K  * SPLIT * Z  / ( 1 - SPLIT + SPLIT * K  )
//           I    I            I                          I
//
//    8) Note that convergence for this routine is set much
//       tighter than usual to avoid rounding problems in
//       other routines.  S is NOT well scaled
*/
void EosPvtModel::FlashEquations( int iM, int i1, int i2, int iNc, int iUpdate )
{
   double  dTerm1;
   double  dTerm2;
   double  dA;
   double  dB;
   double  dC;
   double  dD;
   double  dE;
   double  dF;
   double  dCnv;
   double  dSplit;
   double  dOSplit;
   double *pTa;
   double *pTb;
   double *pTc;
   double *pTd;
   int     i;
   int     iNi;
   int     iter;
   int     iTemp;
   int     iConvrg;

   /* Constant */
   dA = 100.0 * m_dTiny;
   dB = m_dConverge * m_dConverge;
   dCnv = ( dA > dB ) ? dA : dB;

   /* Code for more than one grid block */
   if ( iM > 1 )
   {
      /* Set up the help vector and test for a solution */
      pTa = m_pKValue;
      pTb = m_pComposition;
      for ( i = i1; i < i2; i++ )
      {
         dA = pTa[i];
         dB = ( dA - 1.0 ) * pTb[i];
         m_pTerm1[i] = dB;
         m_pTerm2[i] = dB / dA;
      }

#ifdef IPFtune
      double *pT1 = m_pTerm1;
      double *pT2 = m_pTerm2;
#endif
      for ( iNi = 1; iNi < iNc; iNi++ )
      {
         pTa += iM;
         pTb += iM;
#ifdef IPFtune
#pragma ivdep
#endif
         for ( i = i1; i < i2; i++ )
         {
            dA = pTa[i];
            dB = ( dA - 1.0 ) * pTb[i];
#ifdef IPFtune
            pT1[i] += dB;
            pT2[i] += dB / dA;
#else    
            m_pTerm1[i] += dB;
            m_pTerm2[i] += dB / dA;
#endif
         }
      }

      /* Check if root does not exist */
      for ( i = i1; i < i2; i++ )
      {
         dA = m_pTerm1[i];
         dB = m_pTerm2[i];
         iTemp = ( ( dA > 0.0 && dB < 0.0 ) || ( dA < 0.0 && dB > 0.0 ) ) ? 1 : 0;
         dC = iTemp ? 1.0 : 0.0;
         m_pPhase[i] = iTemp ? m_pPhase[i] : EOS_FL_1P_NCV;
         m_pSplit[i] = iTemp ? m_pSplit[i] : 0.5;
         m_pOSplit[i] = 1.0 - m_pSplit[i];
         m_pTerm2[i] = dC;
         m_pTerm1[i] = 1.0 - dC;
      }

      /* Reset K values if root does not exist */
      pTa = m_pKValue;
      pTb = m_pComposition;
      pTc = m_pTermx;
      pTd = m_pTermy;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
#ifdef IPFtune
#pragma ivdep
#endif
         for ( i = i1; i < i2; i++ )
         {
#ifdef IPFtune
            pTa[i] = pT2[i] * pTa[i] + pT1[i];
#else 
            pTa[i] = m_pTerm2[i] * pTa[i] + m_pTerm1[i];
#endif
            pTc[i] = pTa[i] - 1.0;
            pTd[i] = pTc[i] * pTb[i];
         }

         pTa += iM;
         pTb += iM;
         pTc += iM;
         pTd += iM;
      }

      /* Newton's method until convergence */
      iConvrg = EOS_NOCONVERGE;
      for ( iter = 0; iter < m_iMaxIterations && iConvrg == EOS_NOCONVERGE; iter++ )
      {
         /* Form the function */
         pTa = m_pKValue;
         pTb = m_pTermx;
         pTc = m_pTermy;
#ifdef IPFtune
         double *pOS  = m_pOSplit;
         double *pS   = m_pSplit;
         double mtiny = -m_dTiny;
#pragma ivdep
         for ( i = i1; i < i2; i++ )
         {
            dA = 1.0 / ( pOS[i] + pS[i] * pTa[i] );
            dB = pTc[i] * dA;
            pT1[i] = dB;
            pT2[i] = mtiny - pTb[i] * dB * dA;
         }
#else
         for ( i = i1; i < i2; i++ )
         {
            dA = 1.0 / ( m_pOSplit[i] + m_pSplit[i] * pTa[i] );
            dB = pTc[i] * dA;
            m_pTerm1[i] = dB;
            m_pTerm2[i] = -m_dTiny - pTb[i] * dB * dA;
         }
#endif

         for ( iNi = 1; iNi < iNc; iNi++ )
         {
            pTa += iM;
            pTb += iM;
            pTc += iM;
#ifdef IPFtune
#pragma ivdep
            for ( i = i1; i < i2; i++ )
            {
               dA = 1.0 / ( pOS[i] + pS[i] * pTa[i] );
               dB = pTc[i] * dA;
               pT1[i] += dB;
               pT2[i] -= pTb[i] * dB * dA;
            }
#else
            for ( i = i1; i < i2; i++ )
            {
               dA = 1.0 / ( m_pOSplit[i] + m_pSplit[i] * pTa[i] );
               dB = pTc[i] * dA;
               m_pTerm1[i] += dB;
               m_pTerm2[i] -= pTb[i] * dB * dA;
            }
#endif
         }

         /* New split and test for convergence */
         dF = 0.0;
#ifdef IPFtune
#pragma ivdep
#endif
         for ( i = i1; i < i2; i++ )
         {
            dA = 0.99 * m_pSplit[i];
            dB = dA - 0.99;
            dC = m_pTerm1[i] / m_pTerm2[i];
            dD = ( dC > dA ? dA : ( dC < dB ? dB : dC ) );
            m_pSplit[i] -= dD;
            m_pOSplit[i] = 1.0 - m_pSplit[i];
            dE = fabs( dD );
            dF = ( dE > dF ) ? dE : dF;
         }

         iConvrg = ( dF < dCnv ) ? EOS_CONVERGE : EOS_NOCONVERGE;
      }

      /* Get mole numbers */
      if ( iUpdate )
      {
         pTa = m_pKValue;
         pTb = m_pComposition;
         pTc = m_pX;
         pTd = m_pY;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
#ifdef IPFtune
#pragma ivdep
#endif
            for ( i = i1; i < i2; i++ )
            {
               dA = pTa[i];
               pTc[i] = pTb[i] / ( m_pOSplit[i] + m_pSplit[i] * dA );
               pTd[i] = pTc[i] * dA;
            }

            pTa += iM;
            pTb += iM;
            pTc += iM;
            pTd += iM;
         }
      }
   }

   /* Code for one grid block */
   else
   {
      /* Set up the help vector and test for a solution */
      dA = m_pKValue[0];
      dB = ( dA - 1.0 ) * m_pComposition[0];
      dTerm1 = dB;
      dTerm2 = dB / dA;
      for ( iNi = 1; iNi < iNc; iNi++ )
      {
         dA = m_pKValue[iNi];
         dB = ( dA - 1.0 ) * m_pComposition[iNi];
         dTerm1 += dB;
         dTerm2 += dB / dA;
      }

      /* Check if root does not exist */
      iTemp = ( ( dTerm1 > 0.0 && dTerm2 < 0.0 ) || ( dTerm1 < 0.0 && dTerm2 > 0.0 ) ) ? 1 : 0;
      dTerm2 = iTemp ? 1.0 : 0.0;
      dTerm1 = 1.0 - dTerm2;
      *m_pPhase = iTemp ? *m_pPhase : EOS_FL_1P_NCV;
      dSplit = iTemp ? *m_pSplit : 0.5;
      dOSplit = 1.0 - dSplit;

      /* Reset K values if root does not exist */
#ifdef IPFtune
      double *pKV = m_pKValue;
      double *pTx = m_pTermx;
      double *pTy = m_pTermy;
      double *pCm = m_pComposition;
#pragma ivdep
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         pKV[iNi] = dTerm2 * pKV[iNi] + dTerm1;
         pTx[iNi] = pKV[iNi] - 1.0;
         pTy[iNi] = pTx[iNi] * pCm[iNi];
      }
#else      
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         m_pKValue[iNi] = dTerm2 * m_pKValue[iNi] + dTerm1;
         m_pTermx[iNi] = m_pKValue[iNi] - 1.0;
         m_pTermy[iNi] = m_pTermx[iNi] * m_pComposition[iNi];
      }
#endif      

      /* Newton's method until convergence */
      iConvrg = EOS_NOCONVERGE;
      for ( iter = 0; iter < m_iMaxIterations && iConvrg == EOS_NOCONVERGE; iter++ )
      {
         /* Form the function */
         dA = 1.0 / ( dOSplit + dSplit * m_pKValue[0] );
         dB = m_pTermy[0] * dA;
         dTerm1 = dB;
         dTerm2 = -m_dTiny - m_pTermx[0] * dB * dA;
         for ( iNi = 1; iNi < iNc; iNi++ )
         {
            dA = 1.0 / ( dOSplit + dSplit * m_pKValue[iNi] );
            dB = m_pTermy[iNi] * dA;
            dTerm1 += dB;
            dTerm2 -= m_pTermx[iNi] * dB * dA;
         }

         /* New split and test for convergence */
         dA = 0.99 * dSplit;
         dB = dA - 0.99;
         dC = dTerm1 / dTerm2;
         dD = ( dC > dA ? dA : ( dC < dB ? dB : dC ) );
         dSplit -= dD;
         dOSplit = 1.0 - dSplit;
         iConvrg = ( fabs( dD ) < dCnv ) ? EOS_CONVERGE : EOS_NOCONVERGE;
      }

      /* Set splits */
      *m_pSplit = dSplit;
      *m_pOSplit = dOSplit;

      /* Get mole numbers */
      if ( iUpdate )
      {
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            dA = m_pKValue[iNi];
            m_pX[iNi] = m_pComposition[iNi] / ( dOSplit + dSplit * dA );
            m_pY[iNi] = m_pX[iNi] * dA;
         }
      }
   }
}


/* 
// OneSideStability
// 
// Function to perform one side of the stability test analysis 
//
// iM
//    Number of objects
// iNc
//    Number of components
// pP
//    Pressure
// pT 
//    Temperature
// pState
//    Term corresponding to the excess energy.  Is negative
//    if there is another stable phase
// pFz
//    The fugacity
//
// 1) Update mole number (y) values during stability
//    analysis using newtonian iteration and test
//    for convergence.  AS shown by michelson, if
//    a phase with composition z is unstable, there
//    must be a set of mole numbers y corresponding
//    to a composition x such that the
//    following conditions hold:
//
//    X  * { LN ( Y  ) + LN ( F (X) ) - H  } = R  = 0 , and
//     I           I           I         I      I
//
//    SUM Y  > 1
//     I   I
//
//    where h is a precalculated function of the composition
//
//    H  = LN ( Z  ) + LN ( F (Z) )
//     I         I           I
//
//    See M. L. Michelson, The isothermal flash problem
//    Part i - Stability, Fluid Phase Equilibria, 1982.
//
// 2) If the first two conditions hold, the free energy of
//    the system can be reduced by forming some
//    amount of the phase with a composition x,
//    indicating that flash calculations are required.
//    although the composition appearing in above
//    appears to be superfluous, it is needed to
//    preserve symmetry in the matrix.  After
//    testing for convergence, the value of pSTATE
//    is reset as follows
//
//    pSTATE = -dEnorm we have not converged
//    pSTATE = -dEnorm/2 we have converged to trivial solution
//    pSTATE = anything else corresponds to energy of the system
//
//  3) Given the current mole numbers y, form
//     normalized mole numbers x.
//
//  4) Using the normalized mole numbers x, calculate
//     the logarithm of the fugacity f and the mole number
//     derivatives dfdx using SolveCubic.
//
//  5) We calculate the new residual using 1, and
//     also calculate the quantities
//
//     G* = 1 + SUM Y  * { LN ( Y  ) + LN ( F (Y) ) - H  - 1 } , AND
//               I   I           I           I         I
//
//     B = SUM ( Y  - Z  ) * { LN ( Y  ) + LN ( F (Y) ) - H  }
//          I     I    I             I           I         I
//
//  6) We then test for convergence and update iphase
//     as follows in a loop which is not vectorized:
//
//     a) If pState is not -dEnorm, we have already
//        determined answer, so do not update pState again.
//
//     b) If the following conditions hold, we are
//        converging to the trivial solution, i.e.,
//        the original mole fractions z, and hence
//        set pState to -dEnorm/2:
//
//        G* < 0.001 AND | 2 * G* / B - 1 | < 0.2
//
//     c) If the square of the l2 norm of the
//        residuals, 1), is less than condition
//        epsmac, we have a second phase.  To see
//        if the energy is decreased, note that
//        g* must be negative at the solution.
//        Hence we pState to G*
//
//  7) If any grid block have not converged, we now form
//     the jacobian matrix to obtain a new estimate of
//     the mole numbers y.  The Jacobian can be shown to 
//     be equal to, when scaled by the square root of X,
//     
//             0.5    0.5
//     J    = X    * X    * DFDX    + DELTA    
//      I,J    I      J         I,J        I,J 
//
//     Note that this is really done by forming the
//     system
//
//            -              -1   -            -
//     ( DX * J * DX ) * ( DX   * S ) = ( DX * R )
//
//     where DX is the diagonal matrix of the square roots
//     of the composition.  The braces indicate how the
//     equations are solved, namely
//
//     J S = R
//
//     Note that in the case of an ideal gas this would be
//     the identity matrix
//
//  8) The Newton step is solved for using Choleski decomposition
//
//  9) The real Newton step for the original system is
//
//     -
//     S = DX * S
//
//     and since we will use the log of the mole number as
//     primary unknown to ensure we stay above zero, since
//
//     dF/dLn(X) = X dF/dX   
//
//     we use
//
//     ^
//     S = DX * S / Y
//
// 10) The mole numbers are updated as follows:
//
//                       ^
//     Y  = Y  * EXP ( - S  )
//      I    I            I
//    
//     Note that proper care must be taken in this expression
//     to avoid rounding trouble
*/
void EosPvtModel::OneSideStability( int iM, int iNc, double *pP, double *pT, double *pState, double *pFz )
{
   double *pTa;
   double *pTb;
   double *pTc;
   double *pTd;
   double *pTe;
   double *pTf;
   double *pTg;
   double  dA;
   double  dB;
   double  dC;
   double  dD;
   double  dE;
   double  dF;
   double  dG;
   double  dH;
   int     iNi;
   int     iNj;
   int     iNcm;
   int     i;
   int     iter;
   int     iConvrg;
   double  dO;
   double  dP;
   double  dPTwo;
   double  dSmall;
   double  dQ;

   /* Set terms */
   dH = 0.0;
   dO = 1.0 / m_dEnorm;
   dP = -m_dEnorm;
   dSmall = -0.001;
   dPTwo = 0.2;
   iNcm = iNc * iM;

   /* Turn convergence flag off */
   for ( i = 0; i < iM; i++ )
   {
      pState[i] = dP;
   }

   /* Iteration loop */
   iConvrg = EOS_NOCONVERGE;
   for ( iter = 0; iter < m_iMaxIterations && iConvrg == EOS_NOCONVERGE; iter++ )
   {
      /* Normalization factor for more than one grid block */
      if ( iM > 1 )
      {
         dQ = 0.0;
         pTa = m_pY;
         for ( i = 0; i < iM; i++ )
         {
            m_pXMat[i] = *pTa++;
         }

         for ( iNi = 1; iNi < iNc; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               m_pXMat[i] += *pTa++;
            }
         }

         /* Normalize component */
         pTa = m_pX;
         pTb = m_pY;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               *pTa++ = *pTb++ / m_pXMat[i];
            }
         }
      }

      /* Normalization factor for single grid block */
      else
      {
         dQ = m_pY[0];
         for ( iNi = 1; iNi < iNc; iNi++ )
         {
            dQ += m_pY[iNi];
         }

         /* Normalize component */
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            m_pX[iNi] = m_pY[iNi] / dQ;
         }
      }

      /* Calculate fugacity */
      /* TODO: The next line is too long. Please rewrite to make it shorter. */
      ( m_pEosPvtTable )->SolveCubic( iM, EOS_FUGACITY, EOS_NOHEAT, EOS_NOPOTENTIAL, EOS_DRV_N, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset, pP, pT, m_pX, m_pZy, m_pDZydp, m_pDZydt, m_pDZyda, pFz, m_pDYdp, m_pDYdt, m_pDYda, m_pHy, m_pDHydp, m_pDHydt, m_pDHyda, m_pATable, m_pPhaseId );

      /* Evaluate function for more than one block */
      if ( iM > 1 )
      {
         for ( i = 0; i < iM; i++ )
         {
            m_pG[i] = m_pXMat[i] - 1.0;
            m_pBeta[i] = 0.0;
         }

         /* Newton's method */
         pTa = m_pX;
         pTb = m_pY;
         pTc = m_pH;
         pTd = pFz;
         pTe = m_pComposition;
         pTf = m_pDYdt;
         pTg = m_pDYdp;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               dA = *pTb++;
               dB = ( log( dA > dO ? dA : dO ) +*pTd++ ) -*pTc++;
               dC = sqrt( dA );
               m_pG[i] -= dB * dA;
               m_pBeta[i] += dB * ( dA -*pTe++ );
               *pTa = dC;
               *pTf++ = dC / m_pXMat[i];
               *pTg++ = -dB **pTa++;
            }
         }

         /* Trivial convergence criteria */
         for ( i = 0; i < iM; i++ )
         {
            dA = pState[i];
            dB = m_pBeta[i];
            dC = m_pG[i];
            pState[i] = ( dA == dP && dC > dSmall && fabs( dB + dC + dC ) < dPTwo * fabs( dB ) ) ? dA / 2.0 : dA;
         }

         /* Set up the fugacity matrix */
         pTa = m_pX;
         pTc = m_pDYdt;
         pTd = m_pDYda;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            pTb = pTa;
            pTe = pTd;
            for ( iNj = iNi; iNj < iNc; iNj++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  *pTe *= *pTb++ *pTc[i];
                  pTe++;
               }
            }

            for ( i = 0; i < iM; i++ )
            {
               *pTd += 1.0;
               pTd++;
            }

            pTd += iNcm;
            pTa += iM;
            pTc += iM;
         }
      }

      /* Evaluate function for single block */
      else
      {
         dG = dQ - 1.0;
         dF = 0.0;

         /* Newton's method */
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            dA = m_pY[iNi];
            dB = ( log( dA > dO ? dA : dO ) + pFz[iNi] ) - m_pH[iNi];
            dC = sqrt( dA );
            dG -= dB * dA;
            dF += dB * ( dA - m_pComposition[iNi] );
            m_pX[iNi] = dC;
            m_pDYdt[iNi] = dC / dQ;
            m_pDYdp[iNi] = -dB * m_pX[iNi];
         }

         /* Trivial convergence criteria */
         *m_pG = dG;
         dA = *pState;
         *pState = ( dA == dP && dG > dSmall && fabs( dF + dG + dG ) < dPTwo * fabs( dF ) ) ? dA / 2.0 : dA;

         /* Set up the fugacity matrix */
         pTd = m_pDYda;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            pTe = pTd;
            for ( iNj = iNi; iNj < iNc; iNj++ )
            {
               *pTe *= m_pX[iNj] * m_pDYdt[iNi];
               pTe++;
            }

            *pTd += 1.0;
            pTd += iNc + 1;
         }
      }

      /* Newton step */
      Cholesky( iM, iNc, m_pDYda );
      BackSolve( iM, iNc, m_pDYda, m_pDYdp );

      /* Redo the scaling for many blocks */
      if ( iM > 1 )
      {
         pTa = m_pX;
         pTb = m_pDYdp;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               dA = *pTa++;
               *pTb = *pTb / ( dA > dO ? dA : dO );
               pTb++;
            }
         }

         /* Update y values */
         pTa = m_pY;
         pTb = m_pDYdp;
         dB = -m_dLnEnorm;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               dC = *pTa;
               dA = *pTb++;
               dD = log( dC > dO ? dC : dO );
               dE = m_dLnEnorm - ( dD > 0.0 ? dD : 0.0 );
               *pTa *= exp( dA < dB ? dB : ( dA > dE ? dE : dA ) );
               pTa++;
            }
         }

         /* Test for convergence */
         if ( iter > 0 )
         {
            for ( i = 0; i < iM; i++ )
            {
               dB = pState[i];
               pState[i] = ( dB == dP && fabs( m_pGold[i] - m_pG[i] ) < m_dConverge ) ? m_pG[i] : dB;
            }
         }

         /* See if the whole thing converged */
         iConvrg = ( pState[0] == dP ) ? EOS_NOCONVERGE : EOS_CONVERGE;
         for ( i = 1; ( i < iM ) && iConvrg; i++ )
         {
            iConvrg = ( pState[i] == dP ) ? EOS_NOCONVERGE : iConvrg;
         }

         /* Save the energy */
         if ( iConvrg == EOS_NOCONVERGE )
         {
            for ( i = 0; i < iM; i++ )
            {
               m_pGold[i] = m_pG[i];
            }
         }
      }

      /* Redo the scaling for single block */
      else
      {
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            dA = m_pX[iNi];
            m_pDYdp[iNi] = m_pDYdp[iNi] / ( dA > dO ? dA : dO );
         }

         /* Update y values */
         dB = -m_dLnEnorm;
         dG = *m_pG;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            dC = m_pY[iNi];
            dA = m_pDYdp[iNi];
            dD = log( dC > dO ? dC : dO );
            dE = m_dLnEnorm - ( dD > 0.0 ? dD : 0.0 );
            m_pY[iNi] *= exp( dA < dB ? dB : ( dA > dE ? dE : dA ) );
         }

         /* Test for convergence */
         if ( iter > 0 )
         {
            dB = *pState;
            *pState = ( dB == dP && fabs( dH - dG ) < m_dConverge ) ? dG : dB;
         }

         /* See if the whole thing converged */
         iConvrg = ( *pState == dP ) ? EOS_NOCONVERGE : EOS_CONVERGE;
         dH = dG;
      }
   }

   /* Set to the current estimate if not converged */
   if ( iConvrg == EOS_NOCONVERGE )
   {
      for ( i = 0; i < iM; i++ )
      {
         dB = pState[i];
         pState[i] = ( dB == dP ) ? m_pG[i] : dB;
      }
   }

   /* Eliminate shallow minimum */
   dA = 100.0 * m_dTiny;
   for ( i = 0; i < iM; i++ )
   {
      dB = pState[i];
      pState[i] = fabs( dB ) < dA ? dP : dB;
   }
}


/* 
// Michelson
//
// Function to perform the Michelson stability analysis 
//
// iM
//    Number of objects
// iNc
//    Number of components
//
// 1) See M. L. Michelson, The isothermal flash problem
//    Part i - Stability, Fluid Phase Equilibria, 1982.
//
// 2) First form
//
//    H  = F  + LN ( Z  )
//     i    i         i
//
// 3) Given an estimate to the K values, form
//
//    Y  = Z  * K
//     I    I    I
//
//    Note that in an initial estimate to the K values
//    that we call the WilsonKValue routine in a way
//    which ensures that they are not too close together
//
// 4) Call routine OneSideStability to obtain stationary
//    points wrt original composition and tangent
//    criterion corresponding to y of 1).  This routine
//    will also return the logarithm of the
//    fugacity, as well as the energy wrt current
//
// 5) Reset the mole numbers as follows:
//
//    Y  = Z  / K
//     I    I    I
//
// 6) Call routine OneSideStability again to obtain stationary
//    points wrt original composition and tangent
//    criterion corresponding to y of 1).  This routine
//    will also return the logarithm of the
//    fugacity, as well as the energy wrt current
//
// 7) If the energy obtained on either pass is negative,
//    then there are two phases.  The initial estimate of the
//    two phase K value is then
//
//                   ^1         ^2
//    K  = exp ( F ( Y  ) - F ( Y  ) )
//     i          i          i    
//         
//    where Y-hat-1 is the fugacity of the Y phase if it
//    is stable wrt the original composition and the original
//    composition if it is not.
//
// 8) We must be careful about rounding
*/
void EosPvtModel::Michelson( int iM, int iNc )
{
   /* Declaractions */
   double *pTa;

   /* Declaractions */
   double *pTb;

   /* Declaractions */
   double *pTc;

   /* Declaractions */
   double *pTd;

   /* Declaractions */
   double *pTe;
   int     iNi;
   int     id;
   int     iTemp;
   int     i;
   double  dA;
   double  dB;
   double  dC;

   /* Scaling factor for overflow */
   dC = 1.0 / m_dEnorm;

   /* Store ideal K values */
   ( m_pEosPvtTable )->WilsonKValues( iM, EOS_SCALEK, m_pTemperature, m_pKValue, m_pWork );

   /* Invarient part of function */
   /* TODO: The next line is too long. Please rewrite to make it shorter. */
   ( m_pEosPvtTable )->SolveCubic( iM, EOS_FUGACITY, EOS_NOHEAT, EOS_NOPOTENTIAL, EOS_DRV_N, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset, m_pPressure, m_pTemperature, m_pComposition, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pFx, m_pDXdp, m_pDXdt, m_pDXda, m_pHx, m_pDHxdp, m_pDHxdt, m_pDHxda, m_pATable, m_pPhaseId );

   /* Store invarient part of function for multiple blocks */
   if ( iM > 1 )
   {
      pTa = m_pComposition;
      pTb = m_pH;
      pTc = m_pFx;
      pTd = m_pY;
      pTe = m_pKValue;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         for ( i = 0; i < iM; i++ )
         {
            *pTe = *pTe / m_pPressure[i];
            dA = *pTa++;
            *pTb++ = *pTc++ +log( dA < dC ? dC : dA );
            *pTd++ = dA **pTe++;
         }
      }
   }

   /* Store invarient part of function for single block */
   else
   {
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         m_pKValue[iNi] = m_pKValue[iNi] / *m_pPressure;
         dA = m_pComposition[iNi];
         m_pH[iNi] = m_pFx[iNi] + log( dA < dC ? dC : dA );
         m_pY[iNi] = dA * m_pKValue[iNi];
      }
   }

   /* For each direction */
   for ( id = 0; id < 2; id++ )
   {
      /* One sided analysis */
      OneSideStability( iM, iNc, m_pPressure, m_pTemperature, m_pGx, m_pFy );

      /* Set new fractions and K values on first pass multiple blocks */
      if ( iM > 1 )
      {
         if ( id == 0 )
         {
            for ( i = 0; i < iM; i++ )
            {
               iTemp = ( m_pGx[i] > 0.0 ) ? 1 : 0;
               m_pPhase[i] = ( iTemp ? EOS_FL_2P_NCV : m_pPhase[i] );
               m_pPoint[i] = ( iTemp ? 1.0 : 0.0 );
            }

            pTa = m_pY;
            pTb = m_pComposition;
            pTc = m_pKValue;
            pTd = m_pFy;
            pTe = m_pFx;
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  dA = pTe[i];
                  *pTa++ = *pTb++ / *pTc;
                  *pTc++ = m_pPoint[i] * ( dA - pTd[i] ) - dA;
               }

               pTd += iM;
               pTe += iM;
            }
         }

         /* Second pass so keep sign */
         else
         {
            for ( i = 0; i < iM; i++ )
            {
               iTemp = ( m_pGx[i] > 0.0 ) ? 1 : 0;
               m_pPhase[i] = ( iTemp ? EOS_FL_2P_NCV : m_pPhase[i] );
               m_pPoint[i] = ( iTemp ? 1.0 : 0.0 );
            }

            pTa = m_pKValue;
            pTd = m_pFy;
            pTe = m_pFx;
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  dA = pTe[i];
                  *pTa += ( dA + m_pPoint[i] * ( pTd[i] - dA ) );
                  pTa++;
               }

               pTd += iM;
               pTe += iM;
            }
         }
      }

      /* Set new fractions and K values on first pass single block */
      else
      {
         if ( id == 0 )
         {
            iTemp = ( *m_pGx > 0.0 ) ? 1 : 0;
            *m_pPhase = ( iTemp ? EOS_FL_2P_NCV : *m_pPhase );
            pTa = ( iTemp ? m_pFy : m_pFx );
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               m_pY[iNi] = m_pComposition[iNi] / m_pKValue[iNi];
               m_pKValue[iNi] = -*pTa++;
            }
         }

         /* Second pass so keep sign */
         else
         {
            iTemp = ( *m_pGx > 0.0 ) ? 1 : 0;
            *m_pPhase = ( iTemp ? EOS_FL_2P_NCV : *m_pPhase );
            pTa = ( iTemp ? m_pFy : m_pFx );
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               m_pY[iNi] = m_pComposition[iNi] / m_pKValue[iNi];
               m_pKValue[iNi] += *pTa++;
            }
         }
      }
   }

   /* Set the split for multiple blocks */
   if ( iM > 1 )
   {
      for ( i = 0; i < iM; i++ )
      {
         m_pSplit[i] = 0.5;
      }

      /* Reset K values */
      pTa = m_pKValue;
      dA = -m_dLnEnorm;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         for ( i = 0; i < iM; i++ )
         {
            dB = *pTa;
            *pTa++ = exp( ( dB > m_dLnEnorm ? m_dLnEnorm : ( dB < dA ? dA : dB ) ) );
         }
      }
   }

   /* Set the split for single block */
   else
   {
      *m_pSplit = 0.5;

      /* Reset K values */
      dA = -m_dLnEnorm;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         dB = m_pKValue[iNi];
         m_pKValue[iNi] = exp( ( dB > m_dLnEnorm ? m_dLnEnorm : ( dB < dA ? dA : dB ) ) );
      }
   }
}


/* 
// FastInitialization
// 
// Function to perform a fast two sided stability analysis 
//
// iM 
//    Number of objects
// iNc
//    Number of components
//
// 1) Call the Wilson K value routine to obtain an
//    initial estimate for the K values.  Ensure that
//    they are not too close together.
//
// 2) Form, following Michelson,
//
//    X  = Z  * K
//     I    I    I
//
//    and
//
//    Y  = Z  / K
//     I    I    I
//
// 3) Use this to form an initial guess for the flasher,
//    namely, 
//
//    K  = exp [ F ( X ) - F ( Y ) ]
//     i          i         i    
//
// 4) Solve the flash equations to ensure there is at
//    least a possible solution at the initial guess
//    to ensure that too much time will not be spent
//    later on in the Newton routines.
//
// 5) This algorithm performed very well in MULTISIM for
//    wellbore flashes
*/
void EosPvtModel::FastInitialization( int iM, int iNc )
{
   double  dA;
   double  dB;
   double  dC;
   double  dD;
   double *pTa;
   double *pTb;
   double *pTc;
   double *pTd;
   int     i;
   int     iNi;

   /* Store ideal K values */
   ( m_pEosPvtTable )->WilsonKValues( iM, EOS_SCALEK, m_pTemperature, m_pKValue, m_pWork );

   /* Set and normalize compositions multiple grid block case */
   if ( iM > 1 )
   {
      pTa = m_pComposition;
      pTb = m_pKValue;
      pTc = m_pX;
      pTd = m_pY;
      for ( i = 0; i < iM; i++ )
      {
         dA = *pTa++;
         dB = *pTb++;
         *pTc = dA * dB;
         *pTd = dA / dB;
         m_pFx[i] = *pTc++;
         m_pFy[i] = *pTd++;
      }

      for ( iNi = 1; iNi < iNc; iNi++ )
      {
         for ( i = 0; i < iM; i++ )
         {
            dA = *pTa++;
            dB = *pTb++;
            *pTc = dA * dB;
            *pTd = dA / dB;
            m_pFx[i] += *pTc++;
            m_pFy[i] += *pTd++;
         }
      }

      pTa = m_pX;
      pTb = m_pY;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         for ( i = 0; i < iM; i++ )
         {
            *pTa = *pTa / m_pFx[i];
            *pTb = *pTb / m_pFy[i];
            pTa++;
            pTb++;
         }
      }
   }

   /* Set and normalize compositions single grid block case */
   else
   {
      dA = m_pComposition[0];
      dB = m_pKValue[0];
      m_pX[0] = dA * dB;
      m_pY[0] = dA / dB;
      dC = m_pX[0];
      dD = m_pY[0];
      for ( iNi = 1; iNi < iNc; iNi++ )
      {
         dA = m_pComposition[iNi];
         dB = m_pKValue[iNi];
         m_pX[iNi] = dA * dB;
         m_pY[iNi] = dA / dB;
         dC += m_pX[iNi];
         dD += m_pY[iNi];
      }

      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         m_pX[iNi] = m_pX[iNi] / dC;
         m_pY[iNi] = m_pY[iNi] / dD;
      }
   }

   /* Get the fugacities */
   /* TODO: The next line is too long. Please rewrite to make it shorter. */
   ( m_pEosPvtTable )->SolveCubic( iM, EOS_FUGACITY, EOS_NOHEAT, EOS_NOPOTENTIAL, EOS_NODRV, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset, m_pPressure, m_pTemperature, m_pX, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pFx, m_pDXdp, m_pDXdt, m_pDXda, m_pHx, m_pDHxdp, m_pDHxdt, m_pDHxda, m_pATable, m_pPhaseId );

   /* TODO: The next line is too long. Please rewrite to make it shorter. */
   ( m_pEosPvtTable )->SolveCubic( iM, EOS_FUGACITY, EOS_NOHEAT, EOS_NOPOTENTIAL, EOS_NODRV, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset, m_pPressure, m_pTemperature, m_pY, m_pZy, m_pDZydp, m_pDZydt, m_pDZyda, m_pFy, m_pDYdp, m_pDYdt, m_pDYda, m_pHy, m_pDHydp, m_pDHydt, m_pDHyda, m_pATable, m_pPhaseId );

   /* Overflow terms */
   dB = m_dLnEnorm;
   dA = -dB;

   /* Reset K values for multiple grid block case */
   if ( iM > 1 )
   {
      pTa = m_pFx;
      pTb = m_pFy;
      pTc = m_pKValue;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         for ( i = 0; i < iM; i++ )
         {
            dC = *pTa++ -*pTb++;
            *pTc++ = dC > dB ? dB : ( dC < dA ? dA : dC );
         }
      }

      EosUtil::VectorExp( iNc * iM, m_pKValue, m_pKValue );

      for ( i = 0; i < iM; i++ )
      {
         m_pSplit[i] = 0.5;
         m_pPhase[i] = EOS_FL_2P_NCV;
      }
   }

   /* Reset K values for single grid block case */
   else
   {
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         dC = m_pFx[iNi] - m_pFy[iNi];
         m_pKValue[iNi] = dC > dB ? dB : ( dC < dA ? dA : dC );
      }

      EosUtil::VectorExp( iNc, m_pKValue, m_pKValue );

      *m_pSplit = 0.5;
      *m_pPhase = EOS_FL_2P_NCV;
   }

   /* Perform a single flash */
   FlashEquations( iM, 0, iM, iNc, EOS_NONORMALIZE );
}


/* 
// Substitution
//
// Successive substitution routine 
//
// iM
//    Number of objects
// iNc
//    Number of components
//
// 1) Call the flasher to get X and Y
// 2) Call SolveCubic for fugacities
// 3) Update the K values 
//
//    K  = exp [ F ( X ) - F ( Y ) ]
//     i          i         i    
//
// 4) Test for convergence
// 5) At the end of the routine ensure that there
//    is still a possible two phase solution to avoid
//    loosing time later
*/
void EosPvtModel::Substitution( int iM, int iNc )
{
   double  dCnv;
   double  dA;
   double  dB;
   double  dC;
   double  dD;
   double  dE;
   double *pTa;
   double *pTb;
   double *pTc;
   int     i;
   int     iNi;
   int     iter;
   int     iConvrg;
   int     iTemp;

   /* Terms */
   dCnv = m_dConverge * m_dConverge;

   /* Loop over terms */
   iConvrg = EOS_NOCONVERGE;
   for ( iter = 0; iter < m_iSubstitutions && iConvrg == EOS_NOCONVERGE; iter++ )
   {
      /* Solve the flash equations */
      FlashEquations( iM, 0, iM, iNc, EOS_NORMALIZE );

      /* Get the x phase properties */
      /* TODO: The next line is too long. Please rewrite to make it shorter. */
      ( m_pEosPvtTable )->SolveCubic( iM, EOS_FUGACITY, EOS_NOHEAT, EOS_NOPOTENTIAL, EOS_NODRV, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset, m_pPressure, m_pTemperature, m_pX, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pFx, m_pDXdp, m_pDXdt, m_pDXda, m_pHx, m_pDHxdp, m_pDHxdt, m_pDHxda, m_pATable, m_pPhaseId );

      /* Get the y phase properties */
      /* TODO: The next line is too long. Please rewrite to make it shorter. */
      ( m_pEosPvtTable )->SolveCubic( iM, EOS_FUGACITY, EOS_NOHEAT, EOS_NOPOTENTIAL, EOS_NODRV, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset, m_pPressure, m_pTemperature, m_pY, m_pZy, m_pDZydp, m_pDZydt, m_pDZyda, m_pFy, m_pDYdp, m_pDYdt, m_pDYda, m_pHy, m_pDHydp, m_pDHydt, m_pDHyda, m_pATable, m_pPhaseId );

      /* Final update and compute norm */
      if ( iM > 1 )
      {
         pTa = m_pFx;
         pTb = m_pFy;
         pTc = m_pKValue;
         dA = -m_dLnEnorm;
         for ( i = 0; i < iM; i++ )
         {
            dB = *pTa++ -*pTb++;
            dC = ( dB > m_dLnEnorm ? m_dLnEnorm : ( dB < dA ? dA : dB ) );
            dD = dC - log( *pTc );
            m_pG[i] = dD * dD;
            *pTc++ = exp( dC );
         }

         for ( iNi = 1; iNi < iNc; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               dB = *pTa++ -*pTb++;
               dC = ( dB > m_dLnEnorm ? m_dLnEnorm : ( dB < dA ? dA : dB ) );
               dD = dC - log( *pTc );
               m_pG[i] += dD * dD;
               *pTc++ = exp( dC );
            }
         }

         /* Update the phase indicator */
         for ( i = 0; i < iM; i++ )
         {
            iTemp = m_pPhase[i];
            m_pPhase[i] = ( m_pG[i] < dCnv && iTemp == EOS_FL_2P_NCV ) ? EOS_FL_2P_CV : iTemp;
         }

         /* Test for convergence */
         iConvrg = ( m_pPhase[0] == EOS_FL_2P_NCV ) ? EOS_NOCONVERGE : EOS_CONVERGE;
         for ( i = 1; ( i < iM ) && iConvrg; i++ )
         {
            iConvrg = ( m_pPhase[i] == EOS_FL_2P_NCV ) ? EOS_NOCONVERGE : iConvrg;
         }
      }

      /* Final update and compute norm */
      else
      {
         dA = -m_dLnEnorm;
         dB = m_pFx[0] - m_pFy[0];
         dC = ( dB > m_dLnEnorm ? m_dLnEnorm : ( dB < dA ? dA : dB ) );
         dD = dC - log( m_pKValue[0] );
         dE = dD * dD;
         m_pKValue[0] = exp( dC );
         for ( iNi = 1; iNi < iNc; iNi++ )
         {
            dB = m_pFx[iNi] - m_pFy[iNi];
            dC = ( dB > m_dLnEnorm ? m_dLnEnorm : ( dB < dA ? dA : dB ) );
            dD = dC - log( m_pKValue[iNi] );
            dE += dD * dD;
            m_pKValue[iNi] = exp( dC );
         }

         /* Update the phase indicator */
         iTemp = *m_pPhase;
         *m_pPhase = ( dE < dCnv && iTemp == EOS_FL_2P_NCV ) ? EOS_FL_2P_CV : iTemp;
         iConvrg = ( *m_pPhase == EOS_FL_2P_NCV ) ? EOS_NOCONVERGE : EOS_CONVERGE;
      }
   }

   /* Perform a single flash */
   FlashEquations( iM, 0, iM, iNc, EOS_NONORMALIZE );
}


/* 
// NewtonFlash
// 
// Routine to perform Newton's method for a two phase flash 
//
// iM
//    Number of objects
// iNc
//    Number of components
//
// 1) See M. L. Michelson, "The Isothermal Flash Problem
//    Part II - Phase Split Calculations," Fluid
//    Phase Equilibria, 1982.
// 2) The input to this routine are the K values; the
//    object of this routine is to obtain converged
//    K values
// 3) From the K values, obtain the composition and splits
// 4) Obtain the fugacity terms from the compositions.
// 5) The equation to be solved is
//
//    FX  - FY  - LN ( K  ) = F = 0 , I = 1 ... NC
//      I     I         I
//
// 6) Since we wish to use the logarithm of the k values as our 
//    primary unknown, the jacobian matrix is formally (delta is 
//    the Kronecker function)
//
//             D FX          D FY
//                 I             I
//   J    = ----------- - ----------- - DELTA
//    I,J   D LN ( K  )   D LN ( K  )        I,J
//                  J             J
//
// 7) A major difficulty of using this jacobian is
//    that it is not symmetric.  If we use as the
//    primary unknowns the mole numbers of
//    components in the "x" phase we can write formally
//
//        D F     D LN ( K )  -1         -1
//    J = ----  ( ---------- )   = A * D
//        D NX      D NX
//
// 8) Since, as calculated in routine SolveCubic, the
//    derivatives of f, dfdx and dfdy, are wrt to mole
//    numbers, and using the factor that nx + ny is
//    constant throughout the flash for each component
//    which changes the sign on the term dfdy,
//    we can write the matrix a as follows:
//
//    A = DFDX - DFDY + DIAGONAL
//
//    which is really
//
//    A = DFXDX / ( 1 - SPLIT ) + DFYDY / SPLIT - DLN(K)DX
//
//    and multiplying through by (1-SPLIT) * SPLIT
//
//    A = SPLIT * dFXdX + ( 1 - SPLIT ) dFYdY - 
//                        ( 1 - SPLIT ) * SPLIT * dLN(k)/DX
//
//    and the last term is equal to
//
//    dLN(k)/DX = Z  / ( X  * Y  )
//                 I      I    I
//
// 9) Now, from flash equations
//
//            Ki * ( 1 - SPLIT ) * SPLIT * Zi
//    Xi Yi = -------------------------------
//               ( 1 - SPLIT + SPLIT * Ki )^2
//
//    Thus
//  
//    A = SPLIT * dFXdX + ( 1 - SPLIT ) dFYdY - 
//
//       ( 1 - SPLIT + SPLIT * Ki )^2 / (Ki * Zi)
//         
// 10) Define a scale term
//
//     Si = sqrt ( Ki * Zi ) / ( 1 - SPLIT + SPLIT * Ki )
//
//     Then
//
//     S A S = I + other terms
//
//     The matrix S hence scales the equations to the identity
//     matrix in the case of the ideal gas.  This is done to
//     ensure proper scaling of the system.  Our system to
//     solve then becomes
//
//            -1  -1
//     S A S S   D   dK = S F
//
// 11) We now need to calculate the matrix D. Note that
//
//     D LN ( K  )          D LN ( K  )  D K       1   D K
//             I                    I       K             I
//     -----------  =  SUM  -----------  -----  =  --  -----
//        D NX                 D K       D NX      K   D NX
//           J                   K          J      I      J
//
// 12) Expanding we can write the k values as a function of the
//     x phase mole numbers
//
//          X         X
//           I         I
//     K  = -- = -----------
//      I   Y    ( Z  - X  )
//           I      I    I
//
// 13) Differentiating this expression wrt X, we obtain
//
//     D K                         Z                 1
//        I                         I
//     ----- = K  * ( DELTA    --------  -  --------------------- )
//     D NX     I          I,J  X  * Y      SPLIT * ( 1 - SPLIT )
//         J                     I    I
//
// 14) Multiplying by SPLIT * ( 1 - SPLIT ), we obtain
//
//     D LN ( K  )            SPLIT * ( 1 - SPLIT ) * Z
//             I                                       I
//     ----------- = DELTA    -------------------------- -  1
//       D NX            I,J         X  * Y
//           J                        I    I
//
// 15) Define the vector diag as
//
//             SPLIT * ( 1 - SPLIT ) * Z
//                                      I
//     DIAG  = --------------------------
//         I             X  * Y
//                        I    I
//
// 16) We have then that the matrix d can be
//     expressed in terms of known quantities as
//
//     D    = DELTA    * DIAG  - 1
//      I,J        I,J       I
//
// 17) After having set up all of this, we first solve,
//     using Choleski, 
//
//                                -1  -1
//     S A S y = S F   where Y = S   D   dLNK
//
// 18) Thus
//
//     dLNK = D S y
//
//     and the equations can be updated until convergence
//
//     One really nice feature of the above formulation is
//     that the diagonal term of D S is unity.
//
// 19) Before exiting, solve the flash equations one more
//     time to ensure there really is a solution
//
// 20) A few other comments on the methods.  First, if
//     an object ever goes single phase, this is handled
//     within the solution method by setting the scaling
//     term to zero, which has the effect of giving a
//     zero update.  Secondly, only the bottom half of
//     the matrices are used during formation.
*/
void EosPvtModel::NewtonFlash( int iM, int iNc, int iRestore, int iLevel )
{
   int     iter;
   int     iNi;
   int     iNj;
   int     iNk;
   int     iConvrg;
   int     iThermal;
   int     iNcm;
   int     iAnyConvrg;
   int     iNextExists;
   int     i;
   double  dCnv;
   double  dA;
   double  dB;
   double  dC;
   double  dD;
   double  dE;
   double  dVeryTiny;
   double *pTa;
   double *pTb;
   double *pTc;
   double *pTd;
   double *pTe;
#ifdef IPFtune   
   double *pS;
   double *pOS;
   double *pT;
#endif

   /* Prepare for iterations for more than one block */
   if ( iM > 1 )
   {
      iNextExists = ( m_pApplication )->Aandebeurt( -1, EOS_FL_2P_NCV );
      iNi = -m_iMaxIterations;
      for ( i = 0; i < iM; i++ )
      {
         m_pPhase[i] = iNi;
      }
   }

   /* With a single element slice don't bother with next one */
   else
   {
      iNextExists = 0;
   }

   /* Set terms */
   dCnv = m_dConverge * m_dConverge;
   dVeryTiny = 1.0 / m_dEnorm;
   iNcm = iNc * iM;
   dE = 0.0;

   /* Newton Loop */
   iConvrg = EOS_NOCONVERGE;
   for ( iter = 0; iter < m_iMaxIterations && iConvrg == EOS_NOCONVERGE; iter++ )
   {
      /* Solve the flash equations */
      FlashEquations( iM, 0, iM, iNc, EOS_NORMALIZE );

      /* Chemical potential for the x phase */
      /* TODO: The next line is too long. Please rewrite to make it shorter. */
      ( m_pEosPvtTable )->SolveCubic( iM, EOS_FUGACITY, EOS_NOHEAT, EOS_NOPOTENTIAL, EOS_DRV_N, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset, m_pPressure, m_pTemperature, m_pX, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pFx, m_pDXdp, m_pDXdt, m_pDXda, m_pHx, m_pDHxdp, m_pDHxdt, m_pDHxda, m_pATable, m_pPhaseId );

      /* Chemical potential for the y phase */
      /* TODO: The next line is too long. Please rewrite to make it shorter. */
      ( m_pEosPvtTable )->SolveCubic( iM, EOS_FUGACITY, EOS_NOHEAT, EOS_NOPOTENTIAL, EOS_DRV_N, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset, m_pPressure, m_pTemperature, m_pY, m_pZy, m_pDZydp, m_pDZydt, m_pDZyda, m_pFy, m_pDYdp, m_pDYdt, m_pDYda, m_pHy, m_pDHydp, m_pDHydt, m_pDHyda, m_pATable, m_pPhaseId );

      /* Scale terms for multiple grid block case */
      if ( iM > 1 )
      {
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
         for ( i = 0; i < iM; i++ )
         {
            m_pG[i] = ( m_pPhase[i] != EOS_FL_1P_NCV ) ? 1.0 : 0.0;
         }

         pTa = m_pKValue;
         pTb = m_pComposition;
         pTc = m_pDYdp;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
#ifdef IPFtune
            pT  = m_pG;
            pS  = m_pSplit;
            pOS = m_pOSplit;
#pragma ivdep
            for ( i = 0; i < iM; i++ )
            {
               dA = pTa[i];
               pTc[i] = sqrt( pTb[i] * dA ) * pT[i] / ( pOS[i] + pS[i] * dA );
            }
#else
            for ( i = 0; i < iM; i++ )
            {
               dA = pTa[i];
               pTc[i] = sqrt( pTb[i] * dA ) * m_pG[i] / ( m_pOSplit[i] + m_pSplit[i] * dA );
            }
#endif

            pTa += iM;
            pTb += iM;
            pTc += iM;
         }

         /* Assemble the matrix */
         pTa = m_pDXda;
         pTb = m_pDYda;
         pTc = m_pDYdp;
         iNk = 0;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            pTd = pTc;
            pTa += iNk;
            pTb += iNk;
            for ( iNj = iNi; iNj < iNc; iNj++ )
            {
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
#ifdef IPFtune
               pS  = m_pSplit;
               pOS = m_pOSplit;
#pragma ivdep
               for ( i = 0; i < iM; i++ )
               {
                  pTa[i] = pTd[i] * pTc[i] * ( pTa[i] * pS[i] + pTb[i] * pOS[i] - 1.0 );
               }
#else
               for ( i = 0; i < iM; i++ )
               {
                  pTa[i] = pTd[i] * pTc[i] * ( pTa[i] * m_pSplit[i] + pTb[i] * m_pOSplit[i] - 1.0 );
               }
#endif

               pTa += iM;
               pTb += iM;
               pTd += iM;
            }

            iNk += iM;
            pTc += iM;
         }

         /* Diagonal terms and right hand side */
         pTa = m_pKValue;
         pTb = m_pFx;
         pTc = m_pFy;
         pTd = m_pDYdp;
         pTe = m_pDXda;
         EosUtil::VectorLog( iNc * iM, m_pKValue, m_pKValue );
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
#ifdef IPFtune
            pT  = m_pG;
#pragma ivdep
            for ( i = 0; i < iM; i++ )
            {
               dA = pT[i] * ( ( pTb[i] - pTc[i] ) - pTa[i] );
               pTc[i] = dA;
               pTb[i] = pTd[i] * dA;
               pTe[i] += 1.0;
            }
#else
            for ( i = 0; i < iM; i++ )
            {
               dA = m_pG[i] * ( ( pTb[i] - pTc[i] ) - pTa[i] );
               pTc[i] = dA;
               pTb[i] = pTd[i] * dA;
               pTe[i] += 1.0;
            }
#endif

            pTa += iM;
            pTb += iM;
            pTc += iM;
            pTd += iM;
            pTe += iM + iNcm;
         }
      }

      /* Scale terms for single grid block case */
      else
      {
         dE = ( *m_pPhase != EOS_FL_1P_NCV ) ? 1.0 : 0.0;
         dB = *m_pOSplit;
         dC = *m_pSplit;
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            dA = m_pKValue[iNi];
            m_pDYdp[iNi] = sqrt( m_pComposition[iNi] * dA ) * dE / ( dB + dC * dA );
         }

         /* Assemble the matrix */
         pTa = m_pDXda;
         pTb = m_pDYda;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
            for ( iNj = iNi; iNj < iNc; iNj++ )
            {
               pTa[iNj] = m_pDYdp[iNj] * m_pDYdp[iNi] * ( pTa[iNj] * dC + pTb[iNj] * dB - 1.0 );
            }

            pTa += iNc;
            pTb += iNc;
         }

         /* Diagonal terms and right hand side */
         pTa = m_pDXda;
         EosUtil::VectorLog( iNc, m_pKValue, m_pKValue );
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            *pTa += 1.0;
            m_pFy[iNi] = dE * ( ( m_pFx[iNi] - m_pFy[iNi] ) - m_pKValue[iNi] );
            m_pFx[iNi] = m_pDYdp[iNi] * m_pFy[iNi];
            pTa += iNc + 1;
         }
      }

      /* Factor the matrix */
      Cholesky( iM, iNc, m_pDXda );

      /* Perform back substitution */
      BackSolve( iM, iNc, m_pDXda, m_pFx );

      /* Transform solution with the matrix and get maximum residual */
      if ( iM > 1 )
      {
         pTa = m_pDYdp;
         pTb = m_pFx;
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
         for ( i = 0; i < iM; i++ )
         {
            m_pOSplit[i] = pTb[i] * pTa[i];
         }

         for ( iNi = 1; iNi < iNc; iNi++ )
         {
            pTa += iM;
            pTb += iM;
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
            for ( i = 0; i < iM; i++ )
            {
               m_pOSplit[i] += pTb[i] * pTa[i];
            }
         }

#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
         for ( i = 0; i < iM; i++ )
         {
            m_pOSplit[i] *= m_pG[i];
         }

         pTa = m_pFx;
         pTb = m_pFy;
         pTc = m_pDYdp;
         pTd = m_pComposition;
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
#ifdef IPFtune
#pragma ivdep
#endif
         for ( i = 0; i < iM; i++ )
         {
            dB = pTc[i];
            dA = ( pTa[i] - dB * m_pOSplit[i] ) / ( dB > dVeryTiny ? dB : dVeryTiny );
            dC = ( pTd[i] > 0.0 ? dA : pTb[i] );
            m_pDYdt[i] = dC * dC;
            pTa[i] = dC;
         }

         for ( iNi = 1; iNi < iNc; iNi++ )
         {
            pTa += iM;
            pTb += iM;
            pTc += iM;
            pTd += iM;
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
#ifdef IPFtune
            pOS = m_pOSplit;
            pT  = m_pDYdt;
#pragma ivdep
            for ( i = 0; i < iM; i++ )
            {
               dB = pTc[i];
               dA = ( pTa[i] - dB * pOS[i] ) / ( dB > dVeryTiny ? dB : dVeryTiny );
               dC = ( pTd[i] > 0.0 ? dA : pTb[i] );
               pT[i] += dC * dC;
               pTa[i] = dC;
            }
#else
            for ( i = 0; i < iM; i++ )
            {
               dB = pTc[i];
               dA = ( pTa[i] - dB * m_pOSplit[i] ) / ( dB > dVeryTiny ? dB : dVeryTiny );
               dC = ( pTd[i] > 0.0 ? dA : pTb[i] );
               m_pDYdt[i] += dC * dC;
               pTa[i] = dC;
            }
#endif
         }

         /* Test for convergence */
         iConvrg = EOS_CONVERGE;
         iAnyConvrg = EOS_NOCONVERGE;
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
         for ( i = 0; i < iM; i++ )
         {
            iNk = m_pPhase[i];
            iNj = ( iNk < 0 ) ? 1 : 0;
            iNk += iNj;
            iNi = ( m_pDYdt[i] > dCnv ) ? iNj : 0;
            iConvrg = iNi ? EOS_NOCONVERGE : iConvrg;
            iNj = ( iNj && ( !iNi ) ) ? EOS_FL_2P_CV : ( iNk ? iNk : EOS_FL_2P_NCV );
            iAnyConvrg = iAnyConvrg || ( iNj > 0 );
            m_pPhase[i] = iNj;
         }

         /* Update K values */
         pTa = m_pKValue;
         pTb = m_pFx;
         dB = -m_dLnEnorm;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
#ifdef IPFtune
#pragma ivdep
#endif
            for ( i = 0; i < iM; i++ )
            {
               dC = pTa[i] + pTb[i];
               pTa[i] = dC > m_dLnEnorm ? m_dLnEnorm : ( dC < dB ? dB : dC );
            }

            pTa += iM;
            pTb += iM;
         }

         EosUtil::VectorExp( iM * iNc, m_pKValue, m_pKValue );

         /* If anything "aan de beurt" */
         iAnyConvrg = ( iConvrg ? 0 : iAnyConvrg ) ? iM : 0;
         for ( i = 0; i < iNextExists * iAnyConvrg; i++ )
         {
            /* For converged blocks */
            if ( m_pPhase[i] > 0 )
            {
               /* One more check with flash equations */
               FlashEquations( iM, i, i + 1, iNc, EOS_NONORMALIZE );

               /* Store grid block values */
               ( m_pApplication )->ReadFlashResults( i, iM, iLevel, iRestore, m_pSplit, m_pPhase, m_pKValue );

               /* Set the indirection location */
               iNextExists = ( m_pApplication )->Aandebeurt( i, EOS_FL_2P_NCV );

               /* Load the next location */
               ReadData( i, iM, iNc, EOS_OPTION_OFF, EOS_GETK, &iThermal );

               /* Reset iteration counters */
               m_pPhase[i] = -m_iMaxIterations;
               iter = -1;
            }
         }
      }

      /* Transform solution with the matrix and get maximum residual */
      else
      {
         dC = m_pFx[0] * m_pDYdp[0];
         for ( iNi = 1; iNi < iNc; iNi++ )
         {
            dC += m_pFx[iNi] * m_pDYdp[iNi];
         }

         dC *= dE;
         dB = m_pDYdp[0];
         dA = ( m_pFx[0] - dB * dC ) / ( dB > dVeryTiny ? dB : dVeryTiny );
         dB = ( m_pComposition[0] > 0.0 ? dA : m_pFy[0] );
         dD = dB * dB;
         m_pFx[0] = dB;
         for ( iNi = 1; iNi < iNc; iNi++ )
         {
            dB = m_pDYdp[iNi];
            dA = ( m_pFx[iNi] - dB * dC ) / ( dB > dVeryTiny ? dB : dVeryTiny );
            dB = ( m_pComposition[iNi] > 0.0 ? dA : m_pFy[iNi] );
            dD += dB * dB;
            m_pFx[iNi] = dB;
         }

         /* Test for convergence */
         iNk = *m_pPhase;
         iNi = ( iNk == EOS_FL_2P_NCV ) ? 1 : 0;
         iNj = ( dD > dCnv ) ? 1 : 0;
         iConvrg = ( iNi && iNj ) ? EOS_NOCONVERGE : EOS_CONVERGE;
         *m_pPhase = ( iNi && ( !iNj ) ) ? EOS_FL_2P_NCV : iNk;

         /* Update K values */
         dB = -m_dLnEnorm;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            dC = m_pKValue[iNi] + m_pFx[iNi];
            m_pKValue[iNi] = dC > m_dLnEnorm ? m_dLnEnorm : ( dC < dB ? dB : dC );
         }

         EosUtil::VectorExp( iNc, m_pKValue, m_pKValue );
      }
   }

   /* Solve the flash equations once more before exiting */
   FlashEquations( iM, 0, iM, iNc, EOS_NONORMALIZE );
}


/* 
// BubblePointInit
// 
// Function to perform Michelson analysis for bubble points 
//
// iM 
//    Number of objects
// iNc
//    Number of components
// iRetrograde
//    Indicator if we are to undertake retrograde computations,
//    where the bubble/dew point has two values for each
//    composition.  Generally set to off.
//
// 1) First of all need some initial guesses.
//    a) For a standard system with a known pressure that is
//       known to be single phase, set the upper pressure
//       to be twice the current pressure.  Set
//       the lower pressure to zero
//    b) For a standard system without any knowledge of a
//       current pressure, set the current estimate
//       to be a rough estimate of the bubble point pressure.  
//       Set the lower and upper pressures to zero
//    c) For retrograde calculations, we know the current
//       bubble point pressure is an upper bound, so set the 
//       upper bound to the current bubble point pressure, the lower
//       bound to zero
// 2) Save some terms which are used throughout the calculations,
//    namely the Wilson K values and the log of the 
//    mole fraction term
// 3) Then take the next step.  If an upper bound has not been
//    found yet double the step.  If one has been found, then
//    use a modified bisection technique in which the reduction
//    factor, as it moves towards one, is weighted to the higher
//    pressure.  In other words,
//
//    pPnew = pPlow + factor * ( pPhigh - pPlow )
//
//    This is because the algorithm needs to error towards
//    the high side to ensure catching thin envelopes.
//    If the solution has converged do not perform any
//    updates, as rounding error can catch us!
// 4) Call SolveCubic and save the invarient part of the
//    function.  See routine Michelson for a further description
// 5) Perform a two sided stability analysis using the ideal
//    K values as starting guesses.  See routine Michelson
//    for a further description.
// 6) Then we need to look at the return value of the energies
//    to see what to do.  If both energies are positive in
//    the standard case, we are above the solution; if both
//    negative, we are below the solution in pressure.  Thus
//    reset plow and phigh accordingly.  The logic is reversed
//    in the retrograde case.
// 7) If the energy in one direction is negative and in the
//    other direction is positive the current pressure will
//    be a fine pressure to start the Newton iterations.  Use
//    the fugacities of the composition and the negative 
//    energy solution to generate an initial guess for
//    K values for Newton's method.  
*/
void EosPvtModel::BubblePointInit( int iM, int iNc, int iRetrograde )
{
   double  dA;
   double  dB;
   double  dC;
   double  dD;
   double  dE;
   double  dF;
   double  dVeryTiny;
   double  dL;
   double  dH;
   double *pTa;
   double *pTb;
   double *pTc;
   double *pTd;
   double *pTe;
   double *pTf;
   int     iNi;
   int     i;
   int     iter;
   int     iConvrg;

   /* Scaling factor for overflow */
   dVeryTiny = 1.0 / m_dEnorm;
   dF = -1.0 / m_dTiny;
   dB = ( m_pEosPvtTable )->InitBubblePoint();
   dL = 0.0;
   dH = 0.0;

   /* Store plow and phigh and set pointer to current value */
   if ( iM > 1 )
   {
      pTf = m_pFx;
      pTa = m_pPotentialy;
      if ( iRetrograde )
      {
         for ( i = 0; i < iM; i++ )
         {
            m_pPoint[i] = -1.0;
            dA = m_pPressure[i];
            m_pLow[i] = 0.0;
            m_pHigh[i] = m_pBp[i];
         }
      }
      else
      {
         for ( i = 0; i < iM; i++ )
         {
            m_pPoint[i] = -1.0;
            dA = m_pPressure[i];
            m_pLow[i] = 0.0;
            m_pHigh[i] = m_iBubbleDewPoint ? 0.0 : dA + dA;
            m_pBp[i] = m_iBubbleDewPoint ? dB : dA;
         }
      }
   }
   else
   {
      pTf = m_pPotentialy;
      dA = *m_pPressure;
      if ( iRetrograde )
      {
         dL = 0.0;
         dH = *m_pBp;
      }
      else
      {
         dL = 0.0;
         dH = m_iBubbleDewPoint ? 0.0 : dA + dA;
      }

      *m_pBp = m_iBubbleDewPoint ? dB : dA;
   }

   /* Get wilson k values */
   ( m_pEosPvtTable )->WilsonKValues( iM, EOS_SCALEK, m_pTemperature, m_pKValue, m_pWork );

   /* Store log term */
   if ( iM > 1 )
   {
      pTa = m_pComposition;
      pTb = m_pPotentialx;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         for ( i = 0; i < iM; i++ )
         {
            dA = *pTa++;
            *pTb++ = log( dA < dVeryTiny ? dVeryTiny : dA );
         }
      }
   }
   else
   {
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         dA = m_pComposition[iNi];
         m_pPotentialx[iNi] = log( dA < dVeryTiny ? dVeryTiny : dA );
      }
   }

   /* Iterations */
   iConvrg = EOS_NOCONVERGE;
   for ( iter = 0; iter < m_iMaxIterations && iConvrg == EOS_NOCONVERGE; iter++ )
   {
      /* Current guess */
      if ( iM > 1 )
      {
         for ( i = 0; i < iM; i++ )
         {
            dD = m_pBp[i];
            dC = m_pLow[i];
            dA = m_pHigh[i];
            dB = ( dA > 0.0 ) ? 0.5 * ( dA + dC ) : dD + dD;
            dE = ( dC > 0.0 ) ? dB : dA + m_dBubbleReduce * ( dC - dA );
            m_pBp[i] = ( dC == dA ) ? dD : dE;
         }
      }
      else
      {
         dA = *m_pBp;
         dB = ( dH > 0.0 ) ? 0.5 * ( dH + dL ) : dA + dA;
         dC = ( dL > 0.0 ) ? dB : dH + m_dBubbleReduce * ( dL - dH );
         *m_pBp = ( dL == dH ) ? dA : dC;
      }

      /* Invarient part of function */
      /* TODO: The next line is too long. Please rewrite to make it shorter. */
      ( m_pEosPvtTable )->SolveCubic( iM, EOS_FUGACITY, EOS_NOHEAT, EOS_NOPOTENTIAL, EOS_NODRV, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset, m_pBp, m_pTemperature, m_pComposition, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pPotentialy, m_pDXdp, m_pDXdt, m_pDXda, m_pHx, m_pDHxdp, m_pDHxdt, m_pDHxda, m_pATable, m_pPhaseId );

      /* Store invarient part of function */
      if ( iM > 1 )
      {
         pTa = m_pH;
         pTb = m_pPotentialy;
         pTc = m_pPotentialx;
         pTd = m_pY;
         pTe = m_pKValue;
         pTf = m_pComposition;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               *pTa++ = *pTb++ +*pTc++;
               *pTd++ = *pTe++ **pTf++;
            }
         }
      }
      else
      {
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            m_pH[iNi] = m_pPotentialy[iNi] + m_pPotentialx[iNi];
            m_pY[iNi] = m_pKValue[iNi] * m_pComposition[iNi];
         }
      }

      /* Test for stable phase */
      OneSideStability( iM, iNc, m_pBp, m_pTemperature, m_pGx, m_pFx );

      /* Set new mole fractions */
      if ( iM > 1 )
      {
         pTa = m_pY;
         pTb = m_pComposition;
         pTc = m_pKValue;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               *pTa++ = *pTb++ / *pTc++;
            }
         }
      }
      else
      {
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            m_pY[iNi] = m_pComposition[iNi] / m_pKValue[iNi];
         }
      }

      /* Test for stable phase in other direction */
      OneSideStability( iM, iNc, m_pBp, m_pTemperature, m_pGy, m_pFy );

      /* Test for convergence... only one stable phase */
      if ( iRetrograde && iM > 1 )
      {
         iConvrg = EOS_CONVERGE;
         for ( i = 0; i < iM; i++ )
         {
            dC = m_pGx[i];
            dA = m_pGy[i];
            dB = dC < dF ? dF : dC;
            dC = dA < dF ? dF : dA;
            dA = m_pBp[i];
            if ( m_pLow[i] == m_pHigh[i] )
            {
               m_pPoint[i] = ( dB < 0.0 ) ? 1.0 : 0.0;
            }
            else if ( dB > 0.0 || dC > 0.0 )
            {
               m_pHigh[i] = dA;
               iConvrg = EOS_NOCONVERGE;
            }
            else
            {
               m_pLow[i] = m_pHigh[i];
               m_pBp[i] = m_pHigh[i];
               iConvrg = EOS_NOCONVERGE;
            }
         }
      }
      else if ( iM > 1 )
      {
         iConvrg = EOS_CONVERGE;
         for ( i = 0; i < iM; i++ )
         {
            dC = m_pGx[i];
            dA = m_pGy[i];
            dB = dC < dF ? dF : dC;
            dC = dA < dF ? dF : dA;
            dA = m_pBp[i];
            if ( dB * dC > 0.0 )
            {
               m_pLow[i] = ( dB > 0.0 ) ? dA : m_pLow[i];
               m_pHigh[i] = ( dB < 0.0 ) ? dA : m_pHigh[i];
               iConvrg = EOS_NOCONVERGE;
            }
            else
            {
               m_pLow[i] = dA;
               m_pHigh[i] = dA;
               m_pPoint[i] = ( dB < 0.0 ) ? 1.0 : 0.0;
            }
         }
      }
      else
      {
         dC = *m_pGx;
         dA = *m_pGy;
         dB = dC < dF ? dF : dC;
         dC = dA < dF ? dF : dA;
         dA = *m_pBp;
         if ( iRetrograde )
         {
            if ( dL == dH )
            {
               iConvrg = EOS_CONVERGE;
               pTf = ( dB < 0.0 ) ? m_pFy : m_pFx;
            }
            else if ( dB > 0.0 || dC > 0.0 )
            {
               dH = dA;
               iConvrg = EOS_NOCONVERGE;
            }
            else
            {
               dL = dH;
               *m_pBp = dH;
               iConvrg = EOS_NOCONVERGE;
            }
         }
         else if ( dB * dC > 0.0 )
         {
            dL = ( dB > 0.0 ) ? dA : dL;
            dH = ( dB < 0.0 ) ? dA : dH;
            iConvrg = EOS_NOCONVERGE;
         }
         else
         {
            dL = dA;
            dH = dA;
            pTf = ( dB < 0.0 ) ? m_pFy : m_pFx;
            iConvrg = EOS_CONVERGE;
         }
      }
   }

   /* Set K values for multiple blocks */
   if ( iM > 1 )
   {
      pTa = m_pPotentialy;
      pTb = m_pKValue;
      pTc = m_pFy;
      pTd = m_pFx;
      dA = -m_dLnEnorm;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         for ( i = 0; i < iM; i++ )
         {
            dC = m_pPoint[i];
            dD = pTd[i];
            dB = ( dC < 0.0 ) ? 0.0 : ( pTa[i] + dC * ( dD - pTc[i] ) - dD );
            *pTb++ = exp( dB > m_dLnEnorm ? m_dLnEnorm : ( dB < dA ? dA : dB ) );
         }

         pTa += iM;
         pTc += iM;
         pTd += iM;
      }

      /* Check if there is a solution */
      for ( i = 0; i < iM; i++ )
      {
         m_pPhase[i] = ( m_pLow[i] == m_pHigh[i] ) ? EOS_FL_BP_NCV : EOS_FL_1P_NCV;
      }
   }

   /* Set K values for single block */
   else
   {
      dA = -m_dLnEnorm;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         dB = m_pPotentialy[iNi] -*pTf++;
         m_pKValue[iNi] = exp( dB > m_dLnEnorm ? m_dLnEnorm : ( dB < dA ? dA : dB ) );
      }

      /* Check if there is a solution */
      *m_pPhase = ( dL == dH ) ? EOS_FL_BP_NCV : EOS_FL_1P_NCV;
   }
}


/* 
// BubblePointNewton
//
// Function to obtain the saturation pressure of a fluid 
//
// iM
//    Number of objects
// iNc
//    Number of components
// iCheck
//    Indicator whether bubble point pressure is close
//    enough to current pressure to see if mixture is two
//    phase.  If set to EOS_FLP_BP_CHK, additional testing
//    will be done later on to see if the mixture is really
//    two phase
//
// 1) This routine assumes that a valid initial guess will
//    have been generated by the BubblePointInit routine such
//    that the current pressure has only another possible
//    phase in only one direction.  Negative curvature can
//    cause havoc.
// 2) The input to this routine are the K values; the
//    object of this routine is to obtain converged
//    K values and a bubble point pressure.  The primary unknowns 
//    will be the composition and the log of the pressure
// 3) From the initial estimate of K values, obtain the 
//    initial estimate of the composition 
// 4) For each step in Newton's method, obtain the fugacity terms 
//    from the compositions and pressures
// 5) The equations to be solved are
//
//    FX  + LN ( X  ) - FY  - LN ( Y  ) = F = 0 , I = 1 ... NC
//      I         I       I         I
//
//    Note that since the "Y" compositions are fixed that
//    the only derivatives which appear owing to the Y
//    contribution are the pressure derivatives.  The other
//    condition is that the sum of the mole fractions in the
//    X phase is 1.
//
// 6) The equations can be written in the form
//
//    A P dX = G
//    1 0 dP   H
//
//    The pressure derivatives appearing in the matrix
//    P are obtained by multiplying the standard
//    pressure derivatives by the current pressure,
//    which produces the derivative wrt the log of the
//    pressure
//
// 7) By choosing a scaling factor of S = sqrt ( X ) the
//    above equation is rewritten as
//
//            -1
//    SAS SP S  dX = SG
//     S  0   dP     H
//
//    In this case the matrix SAS will have the property
//    that it will be the identity matrix for an ideal
//    gas
//
// 8) Let 
//
//         -1
//    y = S  dX
//
//    Then solve
//
//    SAS z = SG and SAS w = SP
//
//    The equations are then of the form
//     
//    I  w  y   = z
//    S  0  dP    H
//
// 9) Now solve for dP.  We have
//
//    dP = - ( H - Sz ) / Sw
//
//    The solution is not allowed to grow more than one log
//    cycle.  The equations are now of the form
//
//    I  w  y   = z
//    0  1  dP    dP
//
//    and thus
// 
//    y = z - w dP
//
// 10) dX is not used; rather dlnX is used.  Thus
//
//     dlnX = S y / X = y / S
//
//     A new composition and pressure can then be set.
//
// 11) At the end of the routine new K values are
//     generated from the fugacities
//
// 12) Only the bottom half of
//     the matrices are used during formation.
//
*/
void EosPvtModel::BubblePointNewton( int iM, int iNc, int iCheck )
{
   double *pTa;
   double *pTb;
   double *pTc;
   double *pTd;
   double *pTe;
   double *pTf;
   double *pTg;
   double  dA;
   double  dB;
   double  dC;
   double  dD;
   double  dE;
   double  dF;
   double  dG;
   double  dVeryTiny;
   int     iNi;
   int     iNj;
   int     iNcm;
   int     i;
   int     iter;
   int     iTemp;
   int     iConvrg;

   /* Set terms */
   dVeryTiny = 1.0 / m_dEnorm;

   /* Pointers for storage */
   iNcm = iNc * iM;

   /* Restore y */
   if ( iM > 1 )
   {
      pTa = m_pY;
      pTb = m_pComposition;
      pTc = m_pKValue;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         for ( i = 0; i < iM; i++ )
         {
            *pTa++ = *pTb++ **pTc++;
         }
      }
   }
   else
   {
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         m_pY[iNi] = m_pComposition[iNi] * m_pKValue[iNi];
      }
   }

   /* Newton's method */
   iConvrg = EOS_NOCONVERGE;
   for ( iter = 0; iter < m_iMaxIterations && iConvrg == EOS_NOCONVERGE; iter++ )
   {
      /* Normalize the current estimate for many blocks */
      if ( iM > 1 )
      {
         dA = 0.0;
         pTa = m_pY;
         for ( i = 0; i < iM; i++ )
         {
            m_pMy[i] = *pTa++;
         }

         for ( iNi = 1; iNi < iNc; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               m_pMy[i] += *pTa++;
            }
         }

         pTa = m_pY;
         pTb = m_pX;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               *pTb++ = *pTa++ / m_pMy[i];
            }
         }
      }

      /* Normalize the current estimate for single block */
      else
      {
         dA = m_pY[0];
         for ( iNi = 1; iNi < iNc; iNi++ )
         {
            dA += m_pY[iNi];
         }

         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            m_pX[iNi] = m_pY[iNi] / dA;
         }
      }

      /* Calculate x phase fugacity */
      /* TODO: The next line is too long. Please rewrite to make it shorter. */
      ( m_pEosPvtTable )->SolveCubic( iM, EOS_FUGACITY, EOS_NOHEAT, EOS_NOPOTENTIAL, EOS_DRV_P, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset, m_pBp, m_pTemperature, m_pComposition, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pFx, m_pDXdp, m_pDXdt, m_pDXda, m_pHx, m_pDHxdp, m_pDHxdt, m_pDHxda, m_pATable, m_pPhaseId );

      /* Calculate y phase fugacity */
      /* TODO: The next line is too long. Please rewrite to make it shorter. */
      ( m_pEosPvtTable )->SolveCubic( iM, EOS_FUGACITY, EOS_NOHEAT, EOS_NOPOTENTIAL, EOS_DRV_P, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset, m_pBp, m_pTemperature, m_pX, m_pZy, m_pDZydp, m_pDZydt, m_pDZyda, m_pFy, m_pDYdp, m_pDYdt, m_pDYda, m_pHy, m_pDHydp, m_pDHydt, m_pDHyda, m_pATable, m_pPhaseId );

      /* Evaluate function and help matrix for multiple blocks */
      if ( iM > 1 )
      {
         dE = 0.0;
         dF = 0.0;
         for ( i = 0; i < iM; i++ )
         {
            m_pXMat[i] = 0.0;
            m_pXRhs[i] = m_pMy[i] - 1.0;
         }

         pTa = m_pY;
         pTb = m_pComposition;
         pTc = m_pFx;
         pTd = m_pFy;
         pTe = m_pDYdp;
         pTf = m_pDXdp;
         pTg = m_pX;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               dA = *pTa++;
               dB = *pTb++;
               dG = sqrt( dA );
               dC = dG * ( ( *pTc++ +log( dB > dVeryTiny ? dB : dVeryTiny ) ) - ( *pTd + log( dA > dVeryTiny ? dA : dVeryTiny ) ) );
               dD = dG * m_pBp[i] * ( *pTe -*pTf );
               *pTf++ = dC + dG;
               m_pXMat[i] += dG * dD;
               m_pXRhs[i] += dG * dC;
               *pTe++ = dD;
               *pTd++ = dC;
               *pTg++ = dG;
            }
         }

         /* Set up the fugacity matrix */
         pTb = m_pX;
         pTc = m_pDYda;
         pTd = m_pDXdp;
         pTe = m_pDYda;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            pTa = m_pX;
            for ( iNj = 0; iNj < iNc; iNj++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  dA = *pTa++;
                  dB = *pTc * pTb[i] * dA;
                  *pTc++ = dB;
                  pTd[i] -= dA * dB;
               }
            }

            for ( i = 0; i < iM; i++ )
            {
               pTd[i] -= pTb[i];
               *pTe += 1.0;
               pTe++;
            }

            pTe += iNcm;
            pTb += iM;
            pTd += iM;
         }
      }

      /* Evaluate function and help matrix for single block */
      else
      {
         dE = 0.0;
         dF = dA - 1.0;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            dA = m_pY[iNi];
            dB = m_pComposition[iNi];
            dG = sqrt( dA );

            /* TODO: The next line is too long. Please rewrite to make it shorter. */
            dC = dG * ( ( m_pFx[iNi] + log( dB > dVeryTiny ? dB : dVeryTiny ) ) - ( m_pFy[iNi] + log( dA > dVeryTiny ? dA : dVeryTiny ) ) );
            dD = dG **m_pBp * ( m_pDYdp[iNi] - m_pDXdp[iNi] );
            m_pDXdp[iNi] = dC + dG;
            dE += dG * dD;
            dF += dG * dC;
            m_pDYdp[iNi] = dD;
            m_pFy[iNi] = dC;
            m_pX[iNi] = dG;
         }

         /* Set up the fugacity matrix */
         pTa = m_pDYda;
         pTb = m_pDYda;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            for ( iNj = 0; iNj < iNc; iNj++ )
            {
               dA = m_pX[iNj];
               dB = *pTa * m_pX[iNi] * dA;
               *pTa++ = dB;
               m_pDXdp[iNi] -= dA * dB;
            }

            m_pDXdp[iNi] -= m_pX[iNi];
            *pTb += 1.0;
            pTb += iNc + 1;
         }
      }

      /* Newton step */
      Cholesky( iM, iNc, m_pDYda );
      BackSolve( iM, iNc, m_pDYda, m_pDYdp );
      BackSolve( iM, iNc, m_pDYda, m_pFy );

      /* Update pressure for multiple blocks */
      if ( iM > 1 )
      {
         pTa = m_pDXdp;
         pTb = m_pDYdp;
         pTc = m_pFy;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               dA = *pTa++;
               m_pXMat[i] += dA **pTb++;
               m_pXRhs[i] += dA **pTc++;
            }
         }

         /* Diagonal for pressure step */
         dG = 2.0 / m_dConverge;
         for ( i = 0; i < iM; i++ )
         {
            dA = m_pXRhs[i];
            dB = m_pXMat[i];
            dC = ( dG * fabs( dB ) <= fabs( dA ) ) ? dG : ( dA / dB );
            dD = fabs( dC );
            m_pXRhs[i] = dC * ( ( dD < 1.0 ) ? 1.0 : ( 1.0 / dD ) );
         }

         /* Update y values */
         pTa = m_pY;
         pTb = m_pX;
         pTc = m_pFy;
         pTd = m_pDYdp;
         dB = -m_dLnEnorm;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               dC = *pTa;
               dG = log( dC > dVeryTiny ? dC : dVeryTiny );
               dC = m_dLnEnorm - ( ( dG > 0.0 ) ? dG : 0.0 );
               dG = *pTb++;
               dD = ( *pTc++ -m_pXRhs[i] **pTd++ ) / ( dG > dVeryTiny ? dG : dVeryTiny );
               *pTa *= exp( dD < dB ? dB : ( dD > dC ? dC : dD ) );
               pTa++;
            }
         }

         /* Update pressure */
         iConvrg = EOS_CONVERGE;
         for ( i = 0; i < iM; i++ )
         {
            dA = m_pXRhs[i];
            m_pBp[i] *= exp( dA );
            iTemp = fabs( dA ) < m_dConverge;
            iConvrg *= iTemp;
            m_pPhase[i] = iTemp ? EOS_FL_BP_CV : m_pPhase[i];
         }
      }

      /* Update pressure for single block */
      else
      {
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            dA = m_pDXdp[iNi];
            dE += dA * m_pDYdp[iNi];
            dF += dA * m_pFy[iNi];
         }

         /* Diagonal for pressure step */
         dG = 2.0 / m_dTiny;
         dC = ( dG * fabs( dE ) <= fabs( dF ) ) ? dG : ( dF / dE );
         dD = fabs( dC );
         dF = dC * ( ( dD < 1.0 ) ? 1.0 : ( 1.0 / dD ) );

         /* Update y values */
         dB = -m_dLnEnorm;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            dC = m_pY[iNi];
            dG = log( dC > dVeryTiny ? dC : dVeryTiny );
            dC = m_dLnEnorm - ( ( dG > 0.0 ) ? dG : 0.0 );
            dG = m_pX[iNi];
            dD = ( m_pFy[iNi] - dF * m_pDYdp[iNi] ) / ( dG > dVeryTiny ? dG : dVeryTiny );
            m_pY[iNi] *= exp( dD < dB ? dB : ( dD > dC ? dC : dD ) );
         }

         /* Update pressure */
         *m_pBp *= exp( dF );
         iConvrg = ( fabs( dF ) < m_dConverge ) ? EOS_CONVERGE : EOS_NOCONVERGE;
         *m_pPhase = iConvrg ? EOS_FL_BP_CV : *m_pPhase;
      }
   }

   /* Reset K values */
   if ( iM > 1 )
   {
      pTa = m_pComposition;
      pTb = m_pKValue;
      pTc = m_pY;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         for ( i = 0; i < iM; i++ )
         {
            dA = *pTa++;
            *pTb++ = *pTc++ / ( dA > dVeryTiny ? dA : dVeryTiny );
         }
      }

      if ( iCheck == EOS_FL_BP_CHK )
      {
         for ( i = 0; i < iM; i++ )
         {
            if ( m_pPhase[i] == EOS_FL_BP_CV )
            {
               m_pPhase[i] = ( m_pPressure[i] - 1.001 * m_pBp[i] < 0.0 ) ? EOS_FL_BP_CHK : EOS_FL_BP_CV;
            }
            else
            {
               m_pPhase[i] = EOS_FL_1P_NCV;
            }
         }
      }
   }
   else
   {
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         dA = m_pComposition[iNi];
         m_pKValue[iNi] = m_pY[iNi] / ( dA > dVeryTiny ? dA : dVeryTiny );
      }

      if ( iCheck == EOS_FL_BP_CHK )
      {
         if ( *m_pPhase == EOS_FL_BP_CV )
         {
            *m_pPhase = ( *m_pPressure - 1.001 **m_pBp < 0.0 ) ? EOS_FL_BP_CHK : EOS_FL_BP_CV;
         }
         else
         {
            *m_pPhase = EOS_FL_1P_NCV;
         }
      }
   }
}


/* 
// BubblePointTest
//
// Function to test to see if a two phase mixture based upon current 
// bubble point estimate 
//
// iM
//    Number of objects to test
// iNc
//    Number of components
//
// 1) If an object has a pressure slighty above the bubble
//    point pressure or below the bubble point pressure, it
//    might be two phase.  The purpose of this routine is
//    to perform the Michelson stability test on the 
//    bubble point composition and the overall composition
//    to see if it is indeed two phase.  
// 2) The computations follow the methods in the Michelson
//    routine, although only one side of the analysis need
//    be performed by default
*/
void EosPvtModel::BubblePointTest( int iM, int iNc )
{
   double *pTa;
   double *pTb;
   double *pTc;
   double *pTd;
   double *pTe;
   double  dA;
   double  dB;
   double  dTerm;
   int     iNi;
   int     iNk;
   int     i;

   /* Scaling factor for overflow */
   dTerm = 1.0 / m_dEnorm;

   /* Invarient part of function */
   /* TODO: The next line is too long. Please rewrite to make it shorter. */
   ( m_pEosPvtTable )->SolveCubic( iM, EOS_FUGACITY, EOS_NOHEAT, EOS_NOPOTENTIAL, EOS_NODRV, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset, m_pPressure, m_pTemperature, m_pComposition, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pFx, m_pDXdp, m_pDXdt, m_pDXda, m_pHx, m_pDHxdp, m_pDHxdt, m_pDHxda, m_pATable, m_pPhaseId );

   /* Store invarient part of function */
   if ( iM > 1 )
   {
      pTa = m_pComposition;
      pTb = m_pH;
      pTc = m_pFx;
      pTd = m_pY;
      pTe = m_pKValue;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         for ( i = 0; i < iM; i++ )
         {
            dA = *pTa++;
            *pTb++ = *pTc++ +log( dA < dTerm ? dTerm : dA );
            *pTd++ = *pTe++ *dA;
         }
      }
   }
   else
   {
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         dA = m_pComposition[iNi];
         m_pH[iNi] = m_pFx[iNi] + log( dA < dTerm ? dTerm : dA );
         m_pY[iNi] = m_pKValue[iNi] * dA;
      }
   }

   /* Test for stable phase */
   OneSideStability( iM, iNc, m_pPressure, m_pTemperature, m_pGy, m_pFy );

   /* Check if there is a solution */
   dA = -m_dLnEnorm;
   for ( i = 0; i < iM; i++ )
   {
      if ( m_pGy[i] < 0.0 )
      {
         m_pPhase[i] = EOS_FL_BP_CV;
      }
      else
      {
         iNk = i;
         m_pPhase[i] = EOS_FL_2P_NCV;
         m_pSplit[i] = 0.0;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            dB = m_pFx[iNk] - m_pFy[iNk];
            m_pKValue[iNk] = exp( dB > m_dLnEnorm ? m_dLnEnorm : ( dB < dA ? dA : dB ) );
            iNk += iM;
         }
      }
   }
}


/* 
// PropertiesMultipleObjects
//
// Routine to store properties 
//
// iNc
//    Number of components
// iType
//    Indicator for type of system.  If iType is EOS_TOF_2P
//    two hydrocarbon phases are possible; otherwise only
//    one is
//
// 1) Note that most of the organization of this routine
//    is of the form
//    a) Reset the pointers to the beginning of the stack
//    b) While slices are not empty...
//    c) Read in stored results from flash
//    d) Do calculations
//    e) Write out additional calculations
// 2) Begin the routine by getting information from the
//    application concerning things to output, e.g., viscosity
// 3) Then store two phase results
// 4) If using a "pseudo phase" and bubble points, store the
//    "pseudo phase" results based upon the bubble point
// 5) If using a "pseudo phase" without bubble points, store
//    a "pseudo phase" based upon the Wilson K values
// 6) If a single phase without a "pseudo phase" store
//    single phase results
// 7) Store results for objects without hydrocarbons present
*/
void EosPvtModel::PropertiesMultipleObjects( int iNc, int iType )
{
   int iBubble;
   int iMolarDensity;
   int iGetViscosity;
   int iGetTension;
   int iGetMW;
   int iM;
   int iThermal;

   /* See which simulator data needed */
   /* TODO: The next line is too long. Please rewrite to make it shorter. */
   ( m_pApplication )->WriteOutputData( &m_iDrv, &iGetViscosity, &iGetTension, &m_iHeat, &iGetMW, &m_iVolume, &m_iMolarFlash, &iMolarDensity );

   /* See if a bubble point possible */
   iBubble = m_iPseudoProperties && m_iBubbleDew && m_iVolume == EOS_DENSITY && iType == EOS_TOF_2P;

   /* Fix bubble point terms */
   if ( iBubble )
   {
      ( m_pApplication )->ModifyBubble( EOS_TO_INTERNAL, EOS_FL_1P, EOS_FL_BP );
   }

   /* Two phase calculations */
   if ( iType == EOS_TOF_2P )
   {
      ( m_pApplication )->ResetSlice();
      iM = ( m_pApplication )->SetSlice( EOS_FL_2P, m_iFlashLength );
      while ( iM )
      {
         /* Set up locations in the work array */
         AdjustPointers( iM, iNc, EOS_OPTION_OFF );

         /* Get object data for the model */
         ReadData( iM, iM, iNc, EOS_OPTION_OFF, EOS_GETK, &iThermal );

         /* Get properties */
         TwoPhase( iM, iNc );

         /* Store properties */
         WriteData( iM, iNc, iGetViscosity, iGetTension, iMolarDensity, iType, EOS_FL_2P );
         iM = ( m_pApplication )->SetSlice( EOS_FL_2P, m_iFlashLength );
      }
   }

   /* Single phase calculations with known bubble point */
   if ( iBubble )
   {
      ( m_pApplication )->ResetSlice();
      iM = ( m_pApplication )->SetSlice( EOS_FL_BP, m_iFlashLength );
      while ( iM )
      {
         /* Set up locations in the work array */
         AdjustPointers( iM, iNc, EOS_OPTION_OFF );

         /* Redo with proper terms */
         ReadData( iM, iM, iNc, EOS_OPTION_OFF, EOS_GETK, &iThermal );

         /* Get properties */
         BubblePoint( iM, iNc );

         /* Store properties */
         WriteData( iM, iNc, iGetViscosity, iGetTension, iMolarDensity, iType, EOS_FL_2P );
         iM = ( m_pApplication )->SetSlice( EOS_FL_BP, m_iFlashLength );
      }
   }

   /* Single phase calculations with a pseudo phase */
   if ( m_iPseudoProperties && m_iVolume == EOS_DENSITY && iType == EOS_TOF_2P )
   {
      ( m_pApplication )->ResetSlice();
      iM = ( m_pApplication )->SetSlice( EOS_FL_1P, m_iFlashLength );
      while ( iM )
      {
         /* Set up locations in the work array */
         AdjustPointers( iM, iNc, EOS_OPTION_OFF );

         /* Get grid block properties for the flash */
         ReadData( iM, iM, iNc, EOS_OPTION_OFF, EOS_NOGETK, &iThermal );

         /* Get properties */
         PseudoPhase( iM, iNc );

         /* Store properties */
         WriteData( iM, iNc, iGetViscosity, iGetTension, iMolarDensity, iType, EOS_FL_2P );
         iM = ( m_pApplication )->SetSlice( EOS_FL_1P, m_iFlashLength );
      }
   }

   /* Single phase calculations without a pseudo phase */
   if ( m_iVolume != EOS_DENSITY || m_iPseudoProperties == EOS_OPTION_OFF || iType != EOS_TOF_2P )
   {
      ( m_pApplication )->ResetSlice();
      iM = ( m_pApplication )->SetSlice( EOS_FL_1P, m_iFlashLength );
      while ( iM )
      {
         /* Set up locations in the work array */
         AdjustPointers( iM, iNc, EOS_OPTION_OFF );

         /* Get grid block properties for the flash */
         ReadData( iM, iM, iNc, EOS_OPTION_OFF, EOS_NOGETK, &iThermal );

         /* Get properties */
         OnePhase( iM, iNc );

         /* Store properties */
         WriteData( iM, iNc, iGetViscosity, iGetTension, iMolarDensity, iType, EOS_FL_1P );
         iM = ( m_pApplication )->SetSlice( EOS_FL_1P, m_iFlashLength );
      }
   }

   /* Nothing present */
   ( m_pApplication )->ResetSlice();
   iM = ( m_pApplication )->SetSlice( EOS_FL_0P, m_iFlashLength );
   while ( iM )
   {
      WriteData( iM, iNc, iGetViscosity, iGetTension, iMolarDensity, iType, EOS_FL_0P );
      iM = ( m_pApplication )->SetSlice( EOS_FL_0P, m_iFlashLength );
   }

   /* Fix bubble point terms */
   if ( iBubble )
   {
      ( m_pApplication )->ModifyBubble( EOS_TO_APPLICATION, EOS_FL_BP, EOS_FL_1P );
   }
}


/* 
// PropertiesOneObject
//
// Routine to store properties for one object
//
// iNc
//    Number of components
// iType
//    Indicator for type of system.  If iType is EOS_TOF_2P
//    two hydrocarbon phases are possible; otherwise only
//    one is
//
// 1) Note that this routine assumes that the K values are
//    current for the system
// 2) Begin the routine by getting information from the
//    application concerning things to output, e.g., viscosity
// 3) Then store two phase results
// 4) If using a "pseudo phase" and bubble points, store the
//    "pseudo phase" results based upon the bubble point
// 5) If using a "pseudo phase" without bubble points, store
//    a "pseudo phase" based upon the Wilson K values
// 6) If a single phase without a "pseudo phase" store
//    single phase results
// 7) Get results for objects without hydrocarbons present
//    Finally, store the results
*/
void EosPvtModel::PropertiesOneObject( int iNc, int iType )
{
   int    iBubble;
   int    iMolarDensity;
   int    iGetViscosity;
   int    iGetTension;
   int    iGetMW;
   int    iPhase;
   double dA;

   /* Get simulator data */
   /* TODO: The next line is too long. Please rewrite to make it shorter. */
   ( m_pApplication )->WriteOutputData( &m_iDrv, &iGetViscosity, &iGetTension, &m_iHeat, &iGetMW, &m_iVolume, &m_iMolarFlash, &iMolarDensity );

   /* See if a bubble point possible */
   iBubble = m_iPseudoProperties && m_iBubbleDew && m_iVolume == EOS_DENSITY && iType == EOS_TOF_2P;

   /* Adjust the pointers */
   AdjustPointers( 1, iNc, EOS_OPTION_OFF );

   /* Two phase calculations */
   dA = *m_pSplit;
   if ( *m_pPhase == EOS_FL_2P )
   {
      TwoPhase( 1, iNc );
      iPhase = EOS_FL_2P;
   }

   /* Single phase calculations with known bubble point */
   else if ( iBubble && *m_pPhase == EOS_FL_1P && dA < 0.0 )
   {
      *m_pBp = -dA;
      BubblePoint( 1, iNc );
      iPhase = EOS_FL_2P;
      *m_pBp = dA;
   }

   /* Single phase calculations with a pseudo phase */
   else if ( m_iPseudoProperties && m_iVolume == EOS_DENSITY && iType == EOS_TOF_2P && *m_pPhase == EOS_FL_1P )
   {
      PseudoPhase( 1, iNc );
      iPhase = EOS_FL_2P;
   }

   /* Single phase calculations without a pseudo phase */
   else if ( *m_pPhase == EOS_FL_1P )
   {
      OnePhase( 1, iNc );
      iPhase = EOS_FL_1P;
   }

   /* Nothing */
   else
   {
      iPhase = EOS_FL_0P;
   }

   /* Output data */
   WriteData( 1, iNc, iGetViscosity, iGetTension, iMolarDensity, iType, iPhase );
}


/*
// PhaseId
//
// Determine molecular weight and whether a phase is a liquid or a 
// vapour
//
// iM
//    Number of objects
// iNc
//    Number of components
// iMolar
//    Indicator whether molar (1) or mass (2) quantities
// iGetPhase
//    Indicator whether to get phase identification for two phase
//    mixture... any non-zero number indicates that we need
//    to get the phase index
// 
// 1) Compute the molecular weight
// 2) Determine phase identification.  If the "x" phase is
//    more dense than the "y" phase, set the phase indicator
//    to EOS_SINGLE_PHASE_OIL, else set to EOS_SINGLE_PHASE_GAS.
//    The phase indicator is that corresponding to the x phase
*/
void EosPvtModel::PhaseId( int iM, int iNc, int iMolar, int iGetPhase )
{
   double  dA;
   double  dB;
   double  dC;
   double *pTa;
   double *pTb;
   int     i;
   int     iNi;

   /* Scale terms by the molecular weight */
   if ( iM > 1 )
   {
      /* Compute the molecular weights for mole fractions */
      if ( iMolar )
      {
         pTa = m_pX;
         pTb = m_pY;
         dA = m_pMolecularWeight[0];
         for ( i = 0; i < iM; i++ )
         {
            m_pMWx[i] = dA **pTa++;
            m_pMWy[i] = dA **pTb++;
         }

         for ( iNi = 1; iNi < iNc; iNi++ )
         {
            dA = m_pMolecularWeight[iNi];
            for ( i = 0; i < iM; i++ )
            {
               m_pMWx[i] += dA **pTa++;
               m_pMWy[i] += dA **pTb++;
            }
         }
      }

      /* Compute molecular weight for mass fractions */
      else
      {
         pTa = m_pX;
         pTb = m_pY;
         dA = m_pMolecularWeight[0];
         for ( i = 0; i < iM; i++ )
         {
            *pTa *= dA;
            *pTb *= dA;
            m_pMWx[i] = *pTa++;
            m_pMWy[i] = *pTb++;
         }

         for ( iNi = 1; iNi < iNc; iNi++ )
         {
            dA = m_pMolecularWeight[iNi];
            for ( i = 0; i < iM; i++ )
            {
               *pTa *= dA;
               *pTb *= dA;
               m_pMWx[i] += *pTa++;
               m_pMWy[i] += *pTb++;
            }
         }

         /* Normalize */
         pTa = m_pX;
         pTb = m_pY;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               *pTa = *pTa / m_pMWx[i];
               *pTb = *pTb / m_pMWy[i];
               pTa++;
               pTb++;
            }
         }
      }

      /* Set phase id */
      if ( iGetPhase )
      {
         for ( i = 0; i < iM; i++ )
         {
            m_pPhaseId[i] = m_pMWx[i] * m_pZy[i] > m_pMWy[i] * m_pZx[i] ? EOS_SINGLE_PHASE_OIL : EOS_SINGLE_PHASE_GAS;
         }
      }
   }

   /* No need to form mass fractions */
   else
   {
      if ( iMolar )
      {
         dC = m_pMolecularWeight[0];
         dA = dC * m_pX[0];
         dB = dC * m_pY[0];
         for ( iNi = 1; iNi < iNc; iNi++ )
         {
            dC = m_pMolecularWeight[iNi];
            dA += dC * m_pX[iNi];
            dB += dC * m_pY[iNi];
         }

         *m_pMWx = dA;
         *m_pMWy = dB;
      }

      /* Compute the molecular weights for mass fractions */
      else
      {
         dC = m_pMolecularWeight[0];
         m_pX[0] *= dC;
         m_pY[0] *= dC;
         dA = m_pX[0];
         dB = m_pY[0];
         for ( iNi = 1; iNi < iNc; iNi++ )
         {
            dC = m_pMolecularWeight[iNi];
            m_pX[iNi] *= dC;
            m_pY[iNi] *= dC;
            dA += m_pX[iNi];
            dB += m_pY[iNi];
         }

         *m_pMWx = dA;
         *m_pMWy = dB;

         /* Normalize */
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            m_pX[iNi] = m_pX[iNi] / dA;
            m_pY[iNi] = m_pY[iNi] / dB;
         }
      }

      /* Set phase id */
      if ( iGetPhase )
      {
         *m_pPhaseId = dA **m_pZy > dB **m_pZx ? EOS_SINGLE_PHASE_OIL : EOS_SINGLE_PHASE_GAS;
      }
   }
}


/* 
// WriteData
// 
// Routine to write properties to outside world 
//
// iM
//    Number of objects
// iNc
//    Number of components
// iGetViscosity
//    Indicator as to whether to compute viscosity
// iGetTension
//    Indicator as to whether to compute interfacial tension
// iMolarDensity
//    Indicator as to whether density or volume to be returned
//    as a molar quantity
// iType
//    Indicator for the number of hydrocarbon phases possible.
//    EOS_TOF_2P indicates that two phases are possible
// iPhase
//    Indicator for type of output.  The possibilities are
//       EOS_FL_2P for two phase output
//       EOS_FL_1P for one phase output
//       EOS_FL_0P for zero phase output
//
// 1) There are basically several variations on the same theme
//    a) write out results when only one hydrocarbon phase is possible
//    b) write out results for two hydrocarbon phases
//    c) write out results for one hydrocarbon phase, the other not
//       present
//    d) write out results when no hydrocarbons present
// 2) We first compute the viscosity, if required, in this routine
// 3) We then compute the interfacial tension, if required.
//    Note that for a two phase mixture two steps are involved,
//    namely, first get the parachor terms and then set the tension.
//    When only one phase is present the tension is set to zero.
// 4) Then convert molar to mass density if required
// 5) Then write out data.  The call is to ReadData in the
//    application class for a phase which is present, and
//    ReadNull in the application class for a phase which
//    is not present.
*/
void EosPvtModel::WriteData( int iM, int iNc, int iGetViscosity, int iGetTension, int iMolarDensity, int iType,
                             int iPhase )
{
   int i;

   /* Single phase mixture */
   if ( iType != EOS_TOF_2P )
   {
      if ( iPhase == EOS_FL_1P )
      {
         /* Get the viscosity */
         if ( iGetViscosity )
         {
            /* TODO: The next line is too long. Please rewrite to make it shorter. */
            ( m_pEosPvtTable )->Viscosity( iM, m_iDrv, m_iMolarFlash, m_dTiny, m_pPressure, m_pTemperature, m_pX, m_pDXdp, m_pDXdt, m_pDXda, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pMux, m_pDMuxdp, m_pDMuxdt, m_pDMuxda, m_pSumTable );
         }

         /* Get the parachor terms */
         if ( iGetTension )
         {
            /* TODO: The next line is too long. Please rewrite to make it shorter. */
            ( m_pEosPvtTable )->Tension( iM, m_iDrv, m_iMolarFlash, EOS_OPTION_OFF, m_pX, m_pDXdp, m_pDXdt, m_pDXda, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pIfx, m_pDIfxdp, m_pDIfxdt, m_pDIfxda, m_pWork );
         }

         /* Get the mass density */
         if ( m_iMolarFlash && iMolarDensity == EOS_OPTION_OFF )
         {
            /* TODO: The next line is too long. Please rewrite to make it shorter. */
            ( m_pEosPvtTable )->MassDensity( iM, m_iDrv, m_pDXdp, m_pDXdt, m_pDXda, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pMWx, m_pWork );
         }

         /* Set results */
         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         ( m_pApplication )->ReadData( iType, iM, iNc, m_pPhaseId, m_pX, m_pDXdp, m_pMx, m_pDMxdp, m_pZx, m_pDZxdp, m_pMux, m_pDMuxdp, m_pIfx, m_pDIfxdp, m_pHx, m_pDHxdp, m_pBp, m_pDBpdp, m_pMWx );
      }
      else
      {
         ( m_pApplication )->ReadNull( iType, iM, m_pPhaseId );
      }
   }

   /* Two phase flash results */
   else if ( iPhase == EOS_FL_2P )
   {
      /* Get the viscosity */
      if ( iGetViscosity )
      {
         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         ( m_pEosPvtTable )->Viscosity( iM, m_iDrv, m_iMolarFlash, m_dTiny, m_pPressure, m_pTemperature, m_pX, m_pDXdp, m_pDXdt, m_pDXda, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pMux, m_pDMuxdp, m_pDMuxdt, m_pDMuxda, m_pSumTable );

         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         ( m_pEosPvtTable )->Viscosity( iM, m_iDrv, m_iMolarFlash, m_dTiny, m_pPressure, m_pTemperature, m_pY, m_pDYdp, m_pDYdt, m_pDYda, m_pZy, m_pDZydp, m_pDZydt, m_pDZyda, m_pMuy, m_pDMuydp, m_pDMuydt, m_pDMuyda, m_pSumTable );
      }

      /* Get the parachor terms */
      if ( iGetTension )
      {
         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         ( m_pEosPvtTable )->Tension( iM, m_iDrv, m_iMolarFlash, EOS_OPTION_ON, m_pX, m_pDXdp, m_pDXdt, m_pDXda, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pIfx, m_pDIfxdp, m_pDIfxdt, m_pDIfxda, m_pWork );

         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         ( m_pEosPvtTable )->Tension( iM, m_iDrv, m_iMolarFlash, EOS_OPTION_ON, m_pY, m_pDYdp, m_pDYdt, m_pDYda, m_pZy, m_pDZydp, m_pDZydt, m_pDZyda, m_pIfy, m_pDIfydp, m_pDIfydt, m_pDIfyda, m_pWork );

         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         ( m_pEosPvtTable )->SetTension( iM, m_iDrv, m_pIfx, m_pDIfxdp, m_pDIfxdt, m_pDIfxda, m_pIfy, m_pDIfydp, m_pDIfydt, m_pDIfyda, m_pWork );
      }

      /* Get the mass density */
      if ( m_iMolarFlash && iMolarDensity == EOS_OPTION_OFF )
      {
         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         ( m_pEosPvtTable )->MassDensity( iM, m_iDrv, m_pDXdp, m_pDXdt, m_pDXda, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pMWx, m_pWork );

         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         ( m_pEosPvtTable )->MassDensity( iM, m_iDrv, m_pDYdp, m_pDYdt, m_pDYda, m_pZy, m_pDZydp, m_pDZydt, m_pDZyda, m_pMWy, m_pWork );
      }

      /* Set results */
      /* TODO: The next line is too long. Please rewrite to make it shorter. */
      ( m_pApplication )->ReadData( EOS_SINGLE_PHASE_OIL, iM, iNc, m_pPhaseId, m_pX, m_pDXdp, m_pMx, m_pDMxdp, m_pZx, m_pDZxdp, m_pMux, m_pDMuxdp, m_pIfx, m_pDIfxdp, m_pHx, m_pDHxdp, m_pBp, m_pDBpdp, m_pMWx );

      /* TODO: The next line is too long. Please rewrite to make it shorter. */
      ( m_pApplication )->ReadData( EOS_SINGLE_PHASE_GAS, iM, iNc, m_pPhaseId, m_pY, m_pDYdp, m_pMy, m_pDMydp, m_pZy, m_pDZydp, m_pMuy, m_pDMuydp, m_pIfy, m_pDIfydp, m_pHy, m_pDHydp, m_pBp, m_pDBpdp, m_pMWy );
   }

   /* Single phase */
   else if ( iPhase == EOS_FL_1P )
   {
      /* Get the viscosity */
      if ( iGetViscosity )
      {
         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         ( m_pEosPvtTable )->Viscosity( iM, m_iDrv, m_iMolarFlash, m_dTiny, m_pPressure, m_pTemperature, m_pX, m_pDXdp, m_pDXdt, m_pDXda, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pMux, m_pDMuxdp, m_pDMuxdt, m_pDMuxda, m_pSumTable );
      }

      /* Get the parachor terms */
      if ( iGetTension )
      {
         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         ( m_pEosPvtTable )->Tension( iM, m_iDrv, m_iMolarFlash, EOS_OPTION_OFF, m_pX, m_pDXdp, m_pDXdt, m_pDXda, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pIfx, m_pDIfxdp, m_pDIfxdt, m_pDIfxda, m_pWork );
      }

      /* Get the mass density */
      if ( m_iMolarFlash && iMolarDensity == EOS_OPTION_OFF )
      {
         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         ( m_pEosPvtTable )->MassDensity( iM, m_iDrv, m_pDXdp, m_pDXdt, m_pDXda, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pMWx, m_pWork );
      }

      /* Set results */
      /* TODO: The next line is too long. Please rewrite to make it shorter. */
      ( m_pApplication )->ReadData( EOS_SINGLE_PHASE_OIL, iM, iNc, m_pPhaseId, m_pX, m_pDXdp, m_pMx, m_pDMxdp, m_pZx, m_pDZxdp, m_pMux, m_pDMuxdp, m_pIfx, m_pDIfxdp, m_pHx, m_pDHxdp, m_pBp, m_pDBpdp, m_pMWx );
      ( m_pApplication )->ReadNull( EOS_SINGLE_PHASE_GAS, iM, m_pPhaseId );
   }

   /* No hydrocarbons */
   else
   {
      for ( i = 0; i < iM; i++ )
      {
         m_pPhaseId[i] = EOS_SINGLE_PHASE_GAS;
      }

      ( m_pApplication )->ReadNull( EOS_SINGLE_PHASE_OIL, iM, m_pPhaseId );
      ( m_pApplication )->ReadNull( EOS_SINGLE_PHASE_GAS, iM, m_pPhaseId );
   }
}


/* 
//
// BubblePoint
//
// Subroutine to get bubble point properties 
//
// iM
//    Number of objects
// iNc
//    Number of components
//
// 1) Compute all the terms involving the cubic equation of state
// 2) Set the derivative flags; since this routine is called
//    for both regular bubble point calculations and calculations
//    involving a pseudo phase, then we need to differentiate.
//    When this routine used for a pseudo-phase, the actual
//    pressure is used; when used for a bubble point calculation,
//    the bubble point pressure is used.
// 3) Compute which phase is the liquid phase and which is the
//    vapor phase
// 4) If computing enthalpy, convert to a enthalpy per unit
//    mass
// 5) Convert volumes to a density
// 6) Compute total mass or moles per phase
// 7) Compute all derivatives
*/
void EosPvtModel::BubblePoint( int iM, int iNc )
{
   int i;
   int iDrvp;

   /* Set the cubic terms */
   BubblePointCubicTerms( iM, iNc );

   /* Set correct derivative flag */
   iDrvp = ( m_iDrv == EOS_DRV_N ) ? EOS_DRV_P : m_iDrv;
   iDrvp = m_iBubbleDewPoint ? iDrvp : m_iDrv;

   /* Set the phase id and molecular weight */
   PhaseId( iM, iNc, m_iMolarFlash, EOS_PHASEID );

   /* Modify heats */
   if ( m_iHeat && m_iMolarFlash == 0 )
   {
      ( m_pEosPvtTable )->MassHeat( iM, iDrvp, m_pMWx, m_pHx, m_pDHxdp, m_pDHxdt, m_pDHxda );
      ( m_pEosPvtTable )->MassHeat( iM, iDrvp, m_pMWy, m_pHy, m_pDHydp, m_pDHydt, m_pDHyda );
   }

   /* Convert to a density */
   ( m_pEosPvtTable )->Density( iM, iDrvp, m_iMolarFlash, m_pMWx, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda );
   ( m_pEosPvtTable )->Density( iM, iDrvp, m_iMolarFlash, m_pMWy, m_pZy, m_pDZydp, m_pDZydt, m_pDZyda );

   /* Scale terms by the molecular weight */
   if ( m_iMolarFlash )
   {
      for ( i = 0; i < iM; i++ )
      {
         m_pMx[i] = m_pMoles[i];
         m_pMy[i] = 0.0;
      }
   }
   else
   {
      for ( i = 0; i < iM; i++ )
      {
         m_pMx[i] = m_pMoles[i] * m_pMWx[i];
         m_pMy[i] = 0.0;
      }
   }

   /* Derivatives */
   if ( m_iDrv )
   {
      BubblePointDerivatives( iM, iNc );
   }
}


/* 
// BubblePointCubicTerms
//
// Subroutine to get cubic terms for bubble point properties
//
// iM
//    Number of objects
// iNc
//    Number of components
//
// 1) We always need the pressure derivatives to compute
//    accumulation returns for bubble points
// 2) Set the mole fractions from the K values
// 3) Compute the Z factor for both phases.  Only compute
//    enthalpy terms if at bubble point pressure
// 4) If being called as a pseudo-phase, then we need
//    to evaluate properties at actual pressure, not the
//    bubble point pressure
// 5) Compute volumes from the z factors
*/
void EosPvtModel::BubblePointCubicTerms( int iM, int iNc )
{
   double *pTa;
   double *pTb;
   double *pTc;
   double *pTd;
   double *pP;
   double  dA;
   int     iNi;
   int     i;
   int     iDrvp;
   int     iFugacity;
   int     iGetH;

   /* Needed terms */
   iDrvp = ( m_iDrv == EOS_DRV_N ) ? EOS_DRV_P : m_iDrv;

   /* Set the mole fractions */
   if ( iM > 1 )
   {
      pTa = m_pComposition;
      pTb = m_pX;
      pTc = m_pY;
      pTd = m_pKValue;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         for ( i = 0; i < iM; i++ )
         {
            dA = *pTa++;
            *pTb++ = dA;
            *pTc++ = dA **pTd++;
         }
      }
   }
   else
   {
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         dA = m_pComposition[iNi];
         m_pX[iNi] = dA;
         m_pY[iNi] = dA * m_pKValue[iNi];
      }
   }

   /* If derivatives or using the bubble point pressure */
   if ( m_iDrv || m_iBubbleDewPoint )
   {
      iFugacity = iDrvp ? EOS_FUGACITY : EOS_NOFUGACITY;
      iGetH = m_iHeat && m_iBubbleDewPoint;

      /* Z factor for the x phase */
      /* TODO: The next line is too long. Please rewrite to make it shorter. */
      ( m_pEosPvtTable )->SolveCubic( iM, iFugacity, iGetH, EOS_NOPOTENTIAL, iDrvp, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset, m_pBp, m_pTemperature, m_pX, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pWork, m_pDXdp, m_pDXdt, m_pDXda, m_pHx, m_pDHxdp, m_pDHxdt, m_pDHxda, m_pATable, m_pPhaseId );

      /* Z factor for the y phase */
      /* TODO: The next line is too long. Please rewrite to make it shorter. */
      ( m_pEosPvtTable )->SolveCubic( iM, iFugacity, iGetH, EOS_NOPOTENTIAL, iDrvp, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset, m_pBp, m_pTemperature, m_pY, m_pZy, m_pDZydp, m_pDZydt, m_pDZyda, m_pWork, m_pDYdp, m_pDYdt, m_pDYda, m_pHy, m_pDHydp, m_pDHydt, m_pDHyda, m_pATable, m_pPhaseId );
   }

   /* Solve the matrix */
   if ( m_iDrv )
   {
      BubblePointMatrix( iM, iNc );
   }

   /* Set correct pressure pointer */
   if ( m_iBubbleDewPoint )
   {
      pP = m_pBp;
   }

   /* Redo at current pressure */
   else
   {
      pP = m_pPressure;
      iDrvp = m_iDrv;

      /* TODO: The next line is too long. Please rewrite to make it shorter. */
      ( m_pEosPvtTable )->SolveCubic( iM, EOS_NOFUGACITY, m_iHeat, EOS_NOPOTENTIAL, m_iDrv, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset, m_pPressure, m_pTemperature, m_pX, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pWork, m_pDYdp, m_pDXdt, m_pDYda, m_pHx, m_pDHxdp, m_pDHxdt, m_pDHxda, m_pATable, m_pPhaseId );

      /* TODO: The next line is too long. Please rewrite to make it shorter. */
      ( m_pEosPvtTable )->SolveCubic( iM, EOS_NOFUGACITY, m_iHeat, EOS_NOPOTENTIAL, m_iDrv, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset, m_pPressure, m_pTemperature, m_pY, m_pZy, m_pDZydp, m_pDZydt, m_pDZyda, m_pWork, m_pDYdp, m_pDXdt, m_pDYda, m_pHy, m_pDHydp, m_pDHydt, m_pDHyda, m_pATable, m_pPhaseId );
   }

   /* Convert to a volume */
   /* TODO: The next line is too long. Please rewrite to make it shorter. */
   ( m_pEosPvtTable )->Volume( iM, iDrvp, m_iMultipleAbc, m_pAbcOffset, pP, m_pTemperature, m_pX, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pWork );

   /* TODO: The next line is too long. Please rewrite to make it shorter. */
   ( m_pEosPvtTable )->Volume( iM, iDrvp, m_iMultipleAbc, m_pAbcOffset, pP, m_pTemperature, m_pY, m_pZy, m_pDZydp, m_pDZydt, m_pDZyda, m_pWork );
}


/* 
// BubblePointMatrix
//
// Subroutine to form matrix for bubble point derivatives
// and solve for terms
//
// iM
//    Number of objects
// iNc
//    Number of components 
//
// 1) The matrix is formed, scaled, factored and solved in
//    precisely the same manner as in BubblePointNewton, so
//    see description there.
// 2) To compute the accumulation derivatives, solve
//
//    M dYdA  = dRdA 
//        j i       i
//
//    where R is the right hand side.  A similar equation
//    holds for the temperature derivatives.  Pressure
//    derivatives need not be calculated, but derivatives
//    with respect to the bubble point pressure are 
//    computed in the solution process as the last entry
//    in each row.
*/
void EosPvtModel::BubblePointMatrix( int iM, int iNc )
{
   double *pTa;
   double *pTb;
   double *pTc;
   double *pTd;
   double *pTe;
   double *pTf;
   double *pW;
   double  dVeryTiny;
   double  dA;
   double  dB;
   double  dC;
   double  dD;
   int     iNi;
   int     iNj;
   int     iNcm;
   int     i;

   /* Set terms */
   pW = m_pWork + iM;
   iNcm = iNc * iM;
   dVeryTiny = 1.0 / m_dEnorm;

   /* Set up scaling terms for multiple blocks */
   if ( iM > 1 )
   {
      pTa = pW;
      pTb = m_pY;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         for ( i = 0; i < iM; i++ )
         {
            *pTa++ = sqrt( *pTb++ );
         }
      }

      /* Scale term */
      pTa = m_pDXda;
      pTb = m_pDYda;
      pTc = pW;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         pTd = pW;
         for ( iNj = 0; iNj < iNi; iNj++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               *pTa = *pTd++ *( *pTa - 1.0 );
               pTa++;
            }
         }

         pTb += iM * iNi;
         for ( iNj = iNi; iNj < iNc; iNj++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               dA = *pTd++;
               *pTa = dA * ( *pTa - 1.0 );
               *pTb = dA * ( *pTb - 1.0 ) * pTc[i];
               pTa++;
               pTb++;
            }
         }

         pTc += iM;
      }

      /* Set up matrix */
      pTa = m_pX;
      pTb = pW;
      pTc = m_pDXdp;
      pTd = m_pDYdp;
      pTe = m_pDXda;
      pTf = m_pDYda;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         for ( i = 0; i < iM; i++ )
         {
            dB = *pTa++;
            dC = *pTb++;
            *pTe += dC / ( dB < dVeryTiny ? dVeryTiny : dB );
            *pTf += 1.0;
            *pTd = dC * ( *pTd -*pTc++ );
            pTd++;
            pTe++;
            pTf++;
         }

         pTe += iNcm;
         pTf += iNcm;
      }

      /* Factor matrix */
      Cholesky( iM, iNc, m_pDYda );

      /* Solve matrix for pressure derivatives */
      BackSolve( iM, iNc, m_pDYda, m_pDYdp );

      /* Set up diagonals */
      pTa = m_pDYdp;
      pTb = pW;
      for ( i = 0; i < iM; i++ )
      {
         *pTa *= *pTb++;
         m_pDBpdp[i] = *pTa++;
      }

      for ( iNi = 1; iNi < iNc; iNi++ )
      {
         for ( i = 0; i < iM; i++ )
         {
            *pTa *= *pTb++;
            m_pDBpdp[i] += *pTa++;
         }
      }

      /* Composition derivatives backsolve first */
      pTa = m_pDXda;
      pTb = m_pDBpda;
      for ( iNj = 0; iNj < iNc; iNj++ )
      {
         /* Solve matrix for composition derivatives */
         BackSolve( iM, iNc, m_pDYda, pTa );

         /* Update composition step */
         pTd = pW;
         pTe = pTa;
         if ( m_iMolarFlash )
         {
            for ( i = 0; i < iM; i++ )
            {
               m_pWork[i] = m_pMoles[i];
               *pTe *= *pTd++ / m_pWork[i];
               pTb[i] = *pTe++;
            }
         }
         else
         {
            dA = m_pMolecularWeight[iNj];
            for ( i = 0; i < iM; i++ )
            {
               m_pWork[i] = m_pMoles[i] * dA;
               *pTe *= *pTd++ / m_pWork[i];
               pTb[i] = *pTe++;
            }
         }

         for ( iNi = 1; iNi < iNc; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               *pTe *= *pTd++ / m_pWork[i];
               pTb[i] += *pTe++;
            }
         }

         /* Diagonal for pressure step */
         for ( i = 0; i < iM; i++ )
         {
            dA = m_pDBpdp[i];
            pTb[i] = ( dA == 0.0 ) ? 0.0 : pTb[i] / dA;
         }

         /* Update composition step */
         pTd = m_pDYdp;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               *pTa -= pTb[i] **pTd++;
               pTa++;
            }
         }

         pTb += iM;
      }

      /* Temperature derivatives */
      if ( m_iDrv >= EOS_DRV_P )
      {
         if ( m_iDrv >= EOS_DRV_T )
         {
            pTa = m_pDYdt;
            pTb = m_pDXdt;
            pTc = pW;
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  *pTa = *pTc++ *( *pTa -*pTb++ );
                  pTa++;
               }
            }

            /* Solve matrix for temperature derivatives */
            BackSolve( iM, iNc, m_pDYda, m_pDYdt );

            /* Update pressure */
            pTa = m_pDYdt;
            pTb = pW;
            for ( i = 0; i < iM; i++ )
            {
               *pTa *= *pTb++;
               m_pDBpdt[i] = -*pTa++;
            }

            for ( iNi = 1; iNi < iNc; iNi++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  *pTa *= *pTb++;
                  m_pDBpdt[i] -= *pTa++;
               }
            }

            /* Diagonal for pressure step */
            for ( i = 0; i < iM; i++ )
            {
               dA = m_pDBpdp[i];
               m_pDBpdt[i] = ( dA == 0.0 ) ? 0.0 : m_pDBpdt[i] / dA;
            }

            /* Update composition step */
            pTa = m_pDYdt;
            pTb = m_pDYdp;
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  *pTa += m_pDBpdt[i] **pTb++;
                  pTa++;
               }
            }
         }

         /* Derivative of bubble point wrt pressure */
         for ( i = 0; i < iM; i++ )
         {
            m_pDBpdp[i] = 0.0;
         }
      }
   }

   /* Set up scaling terms for single block */
   else
   {
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         pW[iNi] = sqrt( m_pY[iNi] );
      }

      /* Scale term */
      pTa = m_pDXda;
      pTb = m_pDYda;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         for ( iNj = 0; iNj < iNi; iNj++ )
         {
            *pTa = pW[iNj] * ( *pTa - 1.0 );
            pTa++;
         }

         pTb += iNi;
         for ( iNj = iNi; iNj < iNc; iNj++ )
         {
            dA = pW[iNj];
            *pTa = dA * ( *pTa - 1.0 );
            *pTb = dA * ( *pTb - 1.0 ) * pW[iNi];
            pTa++;
            pTb++;
         }
      }

      /* Set up matrix */
      pTa = m_pDXda;
      pTb = m_pDYda;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         dB = m_pX[iNi];
         dC = pW[iNi];
         *pTa += dC / ( dB < dVeryTiny ? dVeryTiny : dB );
         *pTb += 1.0;
         m_pDYdp[iNi] = dC * ( m_pDYdp[iNi] - m_pDXdp[iNi] );
         pTa += iNc + 1;
         pTb += iNc + 1;
      }

      /* Factor matrix */
      Cholesky( iM, iNc, m_pDYda );

      /* Solve matrix for pressure derivatives */
      BackSolve( iM, iNc, m_pDYda, m_pDYdp );

      /* Set up diagonals */
      m_pDYdp[0] *= pW[0];
      dA = m_pDYdp[0];
      for ( iNi = 1; iNi < iNc; iNi++ )
      {
         m_pDYdp[iNi] *= pW[iNi];
         dA += m_pDYdp[iNi];
      }

      *m_pDBpdp = dA;

      /* Composition derivatives backsolve first */
      pTa = m_pDXda;
      dD = *m_pMoles;
      for ( iNj = 0; iNj < iNc; iNj++ )
      {
         /* Solve matrix for composition derivatives */
         BackSolve( iM, iNc, m_pDYda, pTa );

         /* Update composition step */
         if ( m_iMolarFlash )
         {
            dC = dD;
         }
         else
         {
            dC = dD * m_pMolecularWeight[iNj];
         }

         pTa[0] *= pW[0] / dC;
         dB = pTa[0];
         for ( iNi = 1; iNi < iNc; iNi++ )
         {
            pTa[iNi] *= pW[iNi] / dC;
            dB += pTa[iNi];
         }

         /* Diagonal for pressure step */
         dB = ( dA == 0.0 ) ? 0.0 : dB / dA;
         m_pDBpda[iNj] = dB;

         /* Update composition step */
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            *pTa -= dB * m_pDYdp[iNi];
            pTa++;
         }
      }

      /* Temperature derivatives */
      if ( m_iDrv >= EOS_DRV_P )
      {
         if ( m_iDrv >= EOS_DRV_T )
         {
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               m_pDYdt[iNi] = pW[iNi] * ( m_pDYdt[iNi] - m_pDXdt[iNi] );
            }

            /* Solve matrix for temperature derivatives */
            BackSolve( iM, iNc, m_pDYda, m_pDYdt );

            /* Update pressure */
            pTa = m_pDYdt;
            pTb = pW;
            m_pDYdt[0] *= pW[0];
            dB = -m_pDYdt[0];
            for ( iNi = 1; iNi < iNc; iNi++ )
            {
               m_pDYdt[iNi] *= pW[iNi];
               dB -= m_pDYdt[iNi];
            }

            /* Diagonal for pressure step */
            dB = ( dA == 0.0 ) ? 0.0 : dB / dA;
            *m_pDBpdt = dB;

            /* Update composition step */
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               m_pDYdt[iNi] += dB * m_pDYdp[iNi];
            }

            /* Derivative of bubble point wrt pressure */
            *m_pDBpdp = 0.0;
         }
      }
   }
}


/* 
// BubblePointZero
//
// Zero out the bubble point terms
//
// iM
//    Number of objects
// iNc
//    Number of components
//
// 1) Do the deed; zero out the bubble point and derivatives
*/
void EosPvtModel::BubblePointZero( int iM, int iNc )
{
   int iNcm;
   int i;
   iNcm = iNc * iM;

   /* Reset function */
   for ( i = 0; i < iM; i++ )
   {
      m_pBp[i] = 0.0;
   }

   /* Reset derivatives */
   if ( m_iDrv )
   {
      for ( i = 0; i < iNcm; i++ )
      {
         m_pDBpda[i] = 0.0;
      }

      if ( m_iDrv >= EOS_DRV_P )
      {
         for ( i = 0; i < iM; i++ )
         {
            m_pDBpdp[i] = 0.0;
         }

         if ( m_iDrv >= EOS_DRV_T )
         {
            for ( i = 0; i < iM; i++ )
            {
               m_pDBpdt[i] = 0.0;
            }
         }
      }
   }
}


/* 
// BubblePointDerivatives
// 
// Subroutine to compute derivatives of bubble point properties 
//
// iM
//    Number of objects
// iNc
//    Number of components
//
// 1) If this routine is being called to compute a pseudo-phase
//    property, reset the bubble point pressure derivatives to
//    zero for the chain rules.
// 2) Compute density and enthalpy temperature derivatives.
//    Note that 
//
//    dDdT = dDdT + dDdBP * dBPdT
//
//    Also, since the y phase composition depends upon the
//    derivatives
//
//    dDydT = dDydT + sum dDydN  * dNdT
//                     i       i     i
//
//    The sign is changed in the code since we have not done
//    the final calculations
// 3) Then set the total mole or mass temperature derivatives
// 4) Then have to set the composition temperature derivatives.
//    Since
//    
//    Y = N  / SUM N     
//     i   i    j   j        
//
//    or
//
//    Y  = M N  / sum M N
//     i    i i        j j
//
//    the derivatives are
//
//    dYdT = ( dNdT - Y  sum dNdT )   
//      i        i     i  j    j        
//
//    or 
//
//    dYdT  = ( M * dNdT- Y  sum M dNdT ) / MW
//      i        i    i    i  j   j  j        
//
//    The signs are different in the code since the computation
//    of dNdT in BubblePointMatrix did not use the negative
//    sign.
// 5) Now we need the density derivatives with respect to
//    accumulation.  For the X phase, the derivatives
//    only depend upon the bubble point pressure, thus
//
//    dDxdA  = dDxdA  / moles + dBPdA * dDxdBP
//         i        i                i        
//
//    The above expression is for molar quantities.  For
//    mass quantities, 
//
//    dDxdA  = dDxdA  / moles / M  + dBPdA * dDxdBP [ / M  ]
//         i        i            i        i              i
//
//    The molecular weight term is omitted from computations
//    since the dBPdA calculation already included it.
//    Note that the division by moles is needed since the
//    derivatives earlier were normalized to a mole fraction
//    of one.  If needed, the computation of enthalpy
//    derivatives is similar
// 6) For the y phase, the X phase composition derivatives have been
//    defined so that
//
//    dDydA  = ( dBPdA * dDydBP + sum dDydN  * dNdA  ) [ / M  ]
//         i          i            j       j     j i        i
//
//    The molecular weight appearing in the brackets only
//    appears when mass quantities are used.  Note, however,
//    that BubblePointMatrix already took into account the
//    molecular weight in the provisional composition derivatives
// 7) For the Y phase composition derivatives, since
//    
//    Y = N  / SUM N     
//     i   i    j   j        
//
//    or
//
//    Y  = M N  / sum M N
//     i    i i        j j
//
//    the derivatives are
//
//    dYdA  = ( dNdA  - Y  sum dNdA  )   
//      i j       i j    i  k    k j       
//
//    or 
//
//    dYdA  = ( M * dNdA  - Y  sum M dNdA  ) / MW [ / M  ]
//      i j      i    i j    i  k   k  k j             j
//
//    The signs are reversed since the derivatives were
//    not solved with the negative sign in BubblePointMatrix
//    Also, the composition derivatives were already divided
//    by the molecular weight in BubblePointMatrix, so the
//    term in brackets does not appear
// 8) Since
//    
//    X = A  / SUM A     
//     i   i    j   j        
//
//    the derivatives are
//
//    dXdA  = ( d   - X  ) / total   
//      i j      ij    i  
//
// 9) The total mass/mole derivative of the x phase is 
//    one; that of the y phase is zero.
// 10) Finally, set the appropriate pressure derivatives
//     to zero since they have been used as intermediate
//     storage
*/
void EosPvtModel::BubblePointDerivatives( int iM, int iNc )
{
   double *pTa;
   double *pTb;
   double *pTc;
   double  dA;
   double  dB;
   int     iNi;
   int     iNj;
   int     iNcm;
   int     i;

   /* Code for multiple blocks */
   if ( iM > 1 )
   {
      iNcm = iNc * iM;

      /* Reset derivatives */
      if ( ( m_iBubbleDewPoint == 0 ) && m_iDrv )
      {
         pTa = m_pDBpda;
         for ( i = 0; i < iNcm; i++ )
         {
            *pTa++ = 0.0;
         }

         if ( m_iDrv >= EOS_DRV_P )
         {
            for ( i = 0; i < iM; i++ )
            {
               m_pDBpdp[i] = 1.0;
            }

            if ( m_iDrv >= EOS_DRV_T )
            {
               for ( i = 0; i < iM; i++ )
               {
                  m_pDBpdt[i] = 0.0;
               }
            }
         }
      }

      /* Temperature derivatives */
      if ( m_iDrv >= EOS_DRV_T )
      {
         /* Derivative of density wrt temperature */
         for ( i = 0; i < iM; i++ )
         {
            m_pDZxdt[i] += m_pDBpdt[i] * m_pDZxdp[i];
            m_pDZydt[i] += m_pDBpdt[i] * m_pDZydp[i];
         }

         pTa = m_pDZyda;
         pTb = m_pDYdt;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               m_pDZydt[i] -= *pTa++ **pTb++;
            }
         }

         /* Derivative of enthalpy wrt temperature */
         if ( m_iHeat )
         {
            for ( i = 0; i < iM; i++ )
            {
               m_pDHxdt[i] += m_pDBpdt[i] * m_pDHxdp[i];
               m_pDHydt[i] += m_pDBpdt[i] * m_pDHydp[i];
            }

            pTa = m_pDHyda;
            pTb = m_pDYdt;
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  m_pDHydt[i] -= *pTa++ **pTb++;
               }
            }
         }

         /* Get mole number derivative for mole fractions */
         if ( m_iMolarFlash )
         {
            pTa = m_pDYdt;
            for ( i = 0; i < iM; i++ )
            {
               m_pDMydt[i] = *pTa++;
            }

            for ( iNi = 1; iNi < iNc; iNi++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  m_pDMydt[i] += *pTa++;
               }
            }

            /* Finally set the composition temperature derivative */
            pTa = m_pDYdt;
            pTb = m_pDXdt;
            pTc = m_pY;
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  *pTa = *pTc++ *m_pDMydt[i] -*pTa;
                  *pTb++ = 0.0;
                  pTa++;
               }
            }
         }

         /* Take into account the molecular weights mass fractions */
         else
         {
            pTb = m_pDYdt;
            dA = m_pMolecularWeight[0];
            for ( i = 0; i < iM; i++ )
            {
               *pTb *= dA;
               m_pDMydt[i] = *pTb++;
            }

            for ( iNi = 1; iNi < iNc; iNi++ )
            {
               dA = m_pMolecularWeight[iNi];
               for ( i = 0; i < iM; i++ )
               {
                  *pTb *= dA;
                  m_pDMydt[i] += *pTb++;
               }
            }

            /* Finally set the composition temperature derivative */
            pTa = m_pDYdt;
            pTb = m_pDXdt;
            pTc = m_pY;
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  *pTa = ( *pTc++ *m_pDMydt[i] -*pTa ) / m_pMWy[i];
                  *pTb++ = 0.0;
                  pTa++;
               }
            }
         }

         /* Accumulation derivatives */
         for ( i = 0; i < iM; i++ )
         {
            m_pDMxdt[i] = 0.0;
            m_pDMydt[i] = 0.0;
         }
      }

      /* Density derivatives for x phase */
      if ( m_iMolarFlash )
      {
         pTa = m_pDZxda;
         pTb = m_pDBpda;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               *pTa = *pTb++ *m_pDZxdp[i] +*pTa / m_pMoles[i];
               pTa++;
            }
         }
      }
      else
      {
         pTa = m_pDZxda;
         pTb = m_pDBpda;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            dA = m_pMolecularWeight[iNi];
            for ( i = 0; i < iM; i++ )
            {
               *pTa = *pTb++ *m_pDZxdp[i] +*pTa / m_pMoles[i] / dA;
               pTa++;
            }
         }
      }

      /* Density composition derivatives from chain rule for y phase */
      pTa = m_pDZyda;
      pTb = m_pDBpda;
      pTc = m_pWork;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         for ( i = 0; i < iM; i++ )
         {
            *pTc++ = *pTa;
            *pTa++ = *pTb++ *m_pDZydp[i];
         }
      }

      pTa = m_pDZyda;
      pTb = m_pDXda;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         pTc = m_pWork;
         for ( iNj = 0; iNj < iNc; iNj++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               pTa[i] += *pTc++ **pTb++;
            }
         }

         pTa += iM;
      }

      /* Enthalpy derivatives for x phase */
      if ( m_iHeat )
      {
         if ( m_iMolarFlash )
         {
            pTa = m_pDHxda;
            pTb = m_pDBpda;
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  *pTa = *pTb++ *m_pDHxdp[i] +*pTa / m_pMoles[i];
                  pTa++;
               }
            }
         }
         else
         {
            pTa = m_pDHxda;
            pTb = m_pDBpda;
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               dA = m_pMolecularWeight[iNi];
               for ( i = 0; i < iM; i++ )
               {
                  *pTa = *pTb++ *m_pDHxdp[i] +*pTa / m_pMoles[i] / dA;
                  pTa++;
               }
            }
         }

         /* Enthalpy composition derivatives from chain rule */
         pTa = m_pDHyda;
         pTb = m_pDBpda;
         pTc = m_pWork;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               *pTc++ = *pTa;
               *pTa++ = *pTb++ *m_pDHydp[i];
            }
         }

         pTa = m_pDHyda;
         pTb = m_pDXda;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            pTc = m_pWork;
            for ( iNj = 0; iNj < iNc; iNj++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  pTa[i] += *pTc++ **pTb++;
               }
            }

            pTa += iM;
         }
      }

      /* Mole fraction derivative code */
      if ( m_iMolarFlash )
      {
         pTa = m_pDXda;
         pTb = m_pDYda;
         for ( iNj = 0; iNj < iNc; iNj++ )
         {
            pTc = pTa;
            for ( i = 0; i < iM; i++ )
            {
               m_pWork[i] = *pTc++;
            }

            for ( iNi = 1; iNi < iNc; iNi++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  m_pWork[i] += *pTc++;
               }
            }

            pTc = m_pY;
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  *pTb++ = *pTa++ -*pTc++ *m_pWork[i];
               }
            }
         }
      }

      /* Mass fractions so hit by the molecular weights */
      else
      {
         pTa = m_pDXda;
         pTb = m_pDYda;
         for ( iNj = 0; iNj < iNc; iNj++ )
         {
            pTc = pTb;
            dA = m_pMolecularWeight[0];
            for ( i = 0; i < iM; i++ )
            {
               *pTc = dA **pTa++;
               m_pWork[i] = *pTc++;
            }

            for ( iNi = 1; iNi < iNc; iNi++ )
            {
               dA = m_pMolecularWeight[iNi];
               for ( i = 0; i < iM; i++ )
               {
                  *pTc = dA **pTa++;
                  m_pWork[i] += *pTc++;
               }
            }

            pTc = m_pY;
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  *pTb = ( *pTb -*pTc++ *m_pWork[i] ) / m_pMWy[i];
                  pTb++;
               }
            }
         }
      }

      /* Total mass derivatives */
      pTa = m_pDMxda;
      pTb = m_pDMyda;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         for ( i = 0; i < iM; i++ )
         {
            *pTa++ = 1.0;
            *pTb++ = 0.0;
         }
      }

      /* Composition derivatives */
      for ( i = 0; i < iM; i++ )
      {
         m_pWork[i] = -1.0 / m_pMx[i];
      }

      pTa = m_pDXda;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         pTb = m_pX;
         for ( iNj = 0; iNj < iNc; iNj++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               *pTa++ = *pTb++ *m_pWork[i];
            }
         }
      }

      pTa = m_pDXda;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         for ( i = 0; i < iM; i++ )
         {
            *pTa -= m_pWork[i];
            pTa++;
         }

         pTa += iNcm;
      }

      /* Pressure derivatives which need modification */
      if ( m_iDrv >= EOS_DRV_P )
      {
         if ( m_iHeat )
         {
            for ( i = 0; i < iM; i++ )
            {
               dA = m_pDBpdp[i];
               m_pDHxdp[i] *= dA;
               m_pDHydp[i] *= dA;
            }
         }

         for ( i = 0; i < iM; i++ )
         {
            dA = m_pDBpdp[i];
            m_pDZxdp[i] *= dA;
            m_pDZydp[i] *= dA;
            m_pDMxdp[i] = 0.0;
            m_pDMydp[i] = 0.0;
         }

         pTa = m_pDXdp;
         pTb = m_pDYdp;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               *pTa++ = 0.0;
               *pTb++ = 0.0;
            }
         }
      }
   }

   /* Code for single block */
   else
   {
      /* Reset derivatives */
      if ( ( m_iBubbleDewPoint == 0 ) && m_iDrv )
      {
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            m_pDBpda[iNi] = 0.0;
         }

         if ( m_iDrv >= EOS_DRV_P )
         {
            *m_pDBpdp = 1.0;
            if ( m_iDrv >= EOS_DRV_T )
            {
               *m_pDBpdt = 0.0;
            }
         }
      }

      /* Temperature derivatives */
      if ( m_iDrv >= EOS_DRV_T )
      {
         /* Derivative of density wrt temperature */
         dA = *m_pDBpdt;
         *m_pDZxdt += dA **m_pDZxdp;
         dB = *m_pDZydt + dA **m_pDZydp;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            dB -= m_pDZyda[iNi] * m_pDYdt[iNi];
         }

         *m_pDZydt = dB;

         /* Derivative of enthalpy wrt temperature */
         if ( m_iHeat )
         {
            *m_pDHxdt += dA **m_pDHxdp;
            dB = *m_pDHydt + dA **m_pDHydp;
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               dB -= m_pDHyda[iNi] * m_pDYdt[iNi];
            }

            *m_pDHydt = dB;
         }

         /* Get mole number derivative for mole fractions */
         if ( m_iMolarFlash )
         {
            dA = m_pDYdt[0];
            for ( iNi = 1; iNi < iNc; iNi++ )
            {
               dA += m_pDYdt[iNi];
            }

            *m_pDMydt = dA;

            /* Finally set the composition temperature derivative */
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               m_pDYdt[iNi] = m_pY[iNi] * dA - m_pDYdt[iNi];
               m_pDXdt[iNi] = 0.0;
            }
         }

         /* Take into account the molecular weights for fractions */
         else
         {
            m_pDYdt[0] *= m_pMolecularWeight[0];
            dA = m_pDYdt[0];
            for ( iNi = 1; iNi < iNc; iNi++ )
            {
               m_pDYdt[iNi] *= m_pMolecularWeight[iNi];
               dA += m_pDYdt[iNi];
            }

            /* Finally set the composition temperature derivative */
            dB = *m_pMWy;
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               m_pDYdt[iNi] = ( m_pY[iNi] * dA - m_pDYdt[iNi] ) / dB;
               m_pDXdt[iNi] = 0.0;
            }
         }

         /* Accumulation derivatives */
         *m_pDMxdt = 0.0;
         *m_pDMydt = 0.0;
      }

      /* Density derivatives for x phase */
      if ( m_iMolarFlash )
      {
         dA = *m_pDZxdp;
         dB = *m_pMoles;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            m_pDZxda[iNi] = m_pDBpda[iNi] * dA + m_pDZxda[iNi] / dB;
         }
      }
      else
      {
         dA = *m_pDZxdp;
         dB = *m_pMoles;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            m_pDZxda[iNi] = m_pDBpda[iNi] * dA + m_pDZxda[iNi] / dB / m_pMolecularWeight[iNi];
         }
      }

      /* Density composition derivatives from chain rule */
      dA = *m_pDZydp;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         m_pWork[iNi] = m_pDZyda[iNi];
         m_pDZyda[iNi] = m_pDBpda[iNi] * dA;
      }

      pTb = m_pDXda;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         for ( iNj = 0; iNj < iNc; iNj++ )
         {
            m_pDZyda[iNi] += m_pWork[iNj] **pTb++;
         }
      }

      /* Enthalpy derivatives for x phase */
      if ( m_iHeat )
      {
         if ( m_iMolarFlash )
         {
            dA = *m_pDHxdp;
            dB = *m_pMoles;
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               m_pDHxda[iNi] = m_pDBpda[iNi] * dA + m_pDHxda[iNi] / dB;
            }
         }
         else
         {
            dA = *m_pDHxdp;
            dB = *m_pMoles;
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               m_pDHxda[iNi] = m_pDBpda[iNi] * dA + m_pDHxda[iNi] / dB / m_pMolecularWeight[iNi];
            }
         }

         /* Enthalpy composition derivatives from chain rule */
         dA = *m_pDHydp;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            m_pWork[iNi] = m_pDHyda[iNi];
            m_pDHyda[iNi] = m_pDBpda[iNi] * dA;
         }

         pTb = m_pDXda;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            for ( iNj = 0; iNj < iNc; iNj++ )
            {
               m_pDHyda[iNi] += m_pWork[iNj] **pTb++;
            }
         }
      }

      /* Mole fraction derivative code */
      if ( m_iMolarFlash )
      {
         pTa = m_pDXda;
         pTb = m_pDYda;
         for ( iNj = 0; iNj < iNc; iNj++ )
         {
            dA = pTa[0];
            for ( iNi = 1; iNi < iNc; iNi++ )
            {
               dA += pTa[iNi];
            }

            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               *pTb++ = *pTa++ -m_pY[iNi] * dA;
            }
         }
      }

      /* Mass fractions so hit by the molecular weights */
      else
      {
         pTa = m_pDXda;
         pTb = m_pDYda;
         dB = *m_pMWy;
         for ( iNj = 0; iNj < iNc; iNj++ )
         {
            pTb[0] = m_pMolecularWeight[0] **pTa++;
            dA = pTb[0];
            for ( iNi = 1; iNi < iNc; iNi++ )
            {
               pTb[iNi] = m_pMolecularWeight[iNi] **pTa++;
               dA += pTb[iNi];
            }

            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               *pTb = ( *pTb - m_pY[iNi] * dA ) / dB;
               pTb++;
            }
         }
      }

      /* Total mass derivatives */
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         m_pDMxda[iNi] = 1.0;
         m_pDMyda[iNi] = 0.0;
      }

      /* Composition derivatives */
      dA = -1.0 / *m_pMx;
      pTa = m_pDXda;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         for ( iNj = 0; iNj < iNc; iNj++ )
         {
            *pTa++ = m_pX[iNj] * dA;
         }
      }

      pTa = m_pDXda;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         *pTa++ -= dA;
         pTa += iNc;
      }

      /* Pressure derivatives which need modification */
      if ( m_iDrv >= EOS_DRV_P )
      {
         dA = *m_pDBpdp;
         *m_pDZxdp *= dA;
         *m_pDZydp *= dA;
         *m_pDMxdp = 0.0;
         *m_pDMydp = 0.0;
         if ( m_iHeat )
         {
            *m_pDHxdp *= dA;
            *m_pDHydp *= dA;
         }

         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            m_pDXdp[iNi] = 0.0;
            m_pDYdp[iNi] = 0.0;
         }
      }
   }
}


/* 
// OnePhase
// 
// Routine to get properties of one phase object 
//
// iM
//    Number of objects
// iNc
//    Number of components
//
// 1) SolveCubic
// 2) Translate to volumes
// 3) If normal call for densities, get properties
// 4) Otherwise compute phase volume 
*/
void EosPvtModel::OnePhase( int iM, int iNc )
{
   /* Chemical potential and z factor for the x phase */
   /* TODO: The next line is too long. Please rewrite to make it shorter. */
   ( m_pEosPvtTable )->SolveCubic( iM, EOS_NOFUGACITY, m_iHeat, EOS_NOPOTENTIAL, m_iDrv, EOS_PHASEID, m_iMultipleAbc, m_pAbcOffset, m_pPressure, m_pTemperature, m_pComposition, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pWork, m_pDXdp, m_pDXdt, m_pDXda, m_pHx, m_pDHxdp, m_pDHxdt, m_pDHxda, m_pATable, m_pPhaseId );

   /* Perform the volume translations */
   /* TODO: The next line is too long. Please rewrite to make it shorter. */
   ( m_pEosPvtTable )->Volume( iM, m_iDrv, m_iMultipleAbc, m_pAbcOffset, m_pPressure, m_pTemperature, m_pComposition, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pWork );

   /* Finish getting the properties */
   if ( m_iVolume == EOS_DENSITY )
   {
      OnePhaseProperties( iM, iNc );
   }
   else
   {
      OnePhaseVolume( iM, iNc );
   }
}


/* 
// OnePhaseProperties
//
// Routine to get properties of one phase object
//
// iM
//    Number of objects
// iNc
//    Number of components
//
// 1) Compute molecular weights and mole/mass fractions
// 2) If enthalpy needed and mass quantities, convert
//    enthalpy
// 3) Convert volume to density
// 4) The temperature and pressure derivatives of 
//    fractions and total mass/moles are zero
// 5) The temperature and pressure derivatives of
//    the density and enthalpy are as calculated
// 6) The density and enthalpy composition derivatives
//    must be modified by
//
//    dDxdA  = dDxdN  / moles
//         i        i
//    
//    or
//
//    dDxdA  = dDxdN  / moles / M
//         i        i            i
//    
//    since the call to SolveCubic is based upon a normalized
//    mole/mass fraction
// 7) The accumulation derivatives of composition are
//
//    dXdA  = ( d   - X  ) / total   
//      i j      ij    i  
// 
// 8) The total mass or mole derivatives with respect to
//    accumulation are unity
*/
void EosPvtModel::OnePhaseProperties( int iM, int iNc )
{
   double *pTa;
   double *pTb;
   double  dA;
   int     iNi;
   int     iNj;
   int     i;

   /* Set the mole fractions */
   if ( iM > 1 )
   {
      /* Compute the molecular weights for mole fractions */
      if ( m_iMolarFlash )
      {
         pTa = m_pX;
         pTb = m_pComposition;
         dA = m_pMolecularWeight[0];
         for ( i = 0; i < iM; i++ )
         {
            *pTa = *pTb++;
            m_pMWx[i] = dA **pTa++;
         }

         for ( iNi = 1; iNi < iNc; iNi++ )
         {
            dA = m_pMolecularWeight[iNi];
            for ( i = 0; i < iM; i++ )
            {
               *pTa = *pTb++;
               m_pMWx[i] += dA **pTa++;
            }
         }

         for ( i = 0; i < iM; i++ )
         {
            m_pMx[i] = m_pMoles[i];
         }
      }

      /* Compute molecular weight for mass fractions */
      else
      {
         pTa = m_pX;
         pTb = m_pComposition;
         dA = m_pMolecularWeight[0];
         for ( i = 0; i < iM; i++ )
         {
            *pTa = dA **pTb++;
            m_pMWx[i] = *pTa++;
         }

         for ( iNi = 1; iNi < iNc; iNi++ )
         {
            dA = m_pMolecularWeight[iNi];
            for ( i = 0; i < iM; i++ )
            {
               *pTa = dA **pTb++;
               m_pMWx[i] += *pTa++;
            }
         }

         for ( i = 0; i < iM; i++ )
         {
            m_pMx[i] = m_pMWx[i] * m_pMoles[i];
         }

         /* Normalize */
         pTa = m_pX;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               *pTa = *pTa / m_pMWx[i];
               pTa++;
            }
         }
      }
   }

   /* Mole fraction molecular weights */
   else
   {
      if ( m_iMolarFlash )
      {
         m_pX[0] = m_pComposition[0];
         dA = m_pMolecularWeight[0] * m_pX[0];
         for ( iNi = 1; iNi < iNc; iNi++ )
         {
            m_pX[iNi] = m_pComposition[iNi];
            dA += m_pMolecularWeight[iNi] * m_pX[iNi];
         }

         *m_pMWx = dA;
         *m_pMx = *m_pMoles;
      }

      /* Compute the molecular weights for mass fractions */
      else
      {
         m_pX[0] = m_pComposition[0] * m_pMolecularWeight[0];
         dA = m_pX[0];
         for ( iNi = 1; iNi < iNc; iNi++ )
         {
            m_pX[iNi] = m_pComposition[iNi] * m_pMolecularWeight[iNi];
            dA += m_pX[iNi];
         }

         *m_pMWx = dA;
         *m_pMx = dA **m_pMoles;

         /* Normalize */
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            m_pX[iNi] = m_pX[iNi] / dA;
         }
      }
   }

   /* Modify heats */
   if ( m_iHeat && m_iMolarFlash == 0 )
   {
      ( m_pEosPvtTable )->MassHeat( iM, m_iDrv, m_pMWx, m_pHx, m_pDHxdp, m_pDHxdt, m_pDHxda );
   }

   /* Convert to a density */
   ( m_pEosPvtTable )->Density( iM, m_iDrv, m_iMolarFlash, m_pMWx, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda );

   /* Multiple blocks: derivatives */
   if ( iM > 1 )
   {
      if ( m_iDrv )
      {
         /* Pressure derivatives */
         if ( m_iDrv >= EOS_DRV_P )
         {
            /* Composition derivatives */
            pTa = m_pDXdp;
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  *pTa++ = 0.0;
               }
            }

            /* Accumulation derivatives */
            for ( i = 0; i < iM; i++ )
            {
               m_pDMxdp[i] = 0.0;
            }

            /* Now for temperature derivatives */
            if ( m_iDrv >= EOS_DRV_T )
            {
               /* Composition derivatives */
               pTa = m_pDXdt;
               for ( iNi = 0; iNi < iNc; iNi++ )
               {
                  for ( i = 0; i < iM; i++ )
                  {
                     *pTa++ = 0.0;
                  }
               }

               /* Accumulation derivatives */
               for ( i = 0; i < iM; i++ )
               {
                  m_pDMxdt[i] = 0.0;
               }
            }
         }

         /* Density derivatives */
         if ( m_iMolarFlash )
         {
            pTa = m_pDZxda;
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  *pTa = *pTa / m_pMoles[i];
                  pTa++;
               }
            }
         }
         else
         {
            pTa = m_pDZxda;
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               dA = m_pMolecularWeight[iNi];
               for ( i = 0; i < iM; i++ )
               {
                  *pTa = *pTa / dA / m_pMoles[i];
                  pTa++;
               }
            }
         }

         /* Enthalpy derivatives */
         if ( m_iHeat )
         {
            if ( m_iMolarFlash )
            {
               pTa = m_pDHxda;
               for ( iNi = 0; iNi < iNc; iNi++ )
               {
                  for ( i = 0; i < iM; i++ )
                  {
                     *pTa = *pTa / m_pMoles[i];
                     pTa++;
                  }
               }
            }
            else
            {
               pTa = m_pDHxda;
               for ( iNi = 0; iNi < iNc; iNi++ )
               {
                  dA = m_pMolecularWeight[iNi];
                  for ( i = 0; i < iM; i++ )
                  {
                     *pTa = *pTa / dA / m_pMoles[i];
                     pTa++;
                  }
               }
            }
         }

         /* Total mass derivatives */
         pTa = m_pDMxda;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               *pTa++ = 1.0;
            }
         }

         /* Composition derivatives */
         for ( i = 0; i < iM; i++ )
         {
            m_pWork[i] = 1.0 / m_pMx[i];
         }

         pTa = m_pDXda;
         pTb = m_pX;
         for ( iNj = 0; iNj < iNc; iNj++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               *pTa++ = -*pTb++ *m_pWork[i];
            }
         }

         for ( iNi = 1; iNi < iNc; iNi++ )
         {
            pTb = m_pDXda;
            for ( iNj = 0; iNj < iNc; iNj++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  *pTa++ = *pTb++;
               }
            }
         }

         pTa = m_pDXda;
         iNj = iNc * iM;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               *pTa += m_pWork[i];
               pTa++;
            }

            pTa += iNj;
         }
      }
   }

   /* Single block: derivatives */
   else
   {
      if ( m_iDrv )
      {
         /* Pressure derivatives */
         if ( m_iDrv >= EOS_DRV_P )
         {
            /* Composition derivatives */
            *m_pDMxdp = 0.0;
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               m_pDXdp[iNi] = 0.0;
            }

            /* Now for temperature derivatives */
            if ( m_iDrv >= EOS_DRV_T )
            {
               /* Composition derivatives */
               *m_pDMxdt = 0.0;
               for ( iNi = 0; iNi < iNc; iNi++ )
               {
                  m_pDXdt[iNi] = 0.0;
               }
            }
         }

         /* Density derivatives */
         dA = *m_pMoles;
         if ( m_iMolarFlash )
         {
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               m_pDZxda[iNi] = m_pDZxda[iNi] / dA;
            }
         }
         else
         {
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               m_pDZxda[iNi] = m_pDZxda[iNi] / m_pMolecularWeight[iNi] / dA;
            }
         }

         /* Enthalpy derivatives */
         if ( m_iHeat )
         {
            if ( m_iMolarFlash )
            {
               for ( iNi = 0; iNi < iNc; iNi++ )
               {
                  m_pDHxda[iNi] = m_pDHxda[iNi] / dA;
               }
            }
            else
            {
               for ( iNi = 0; iNi < iNc; iNi++ )
               {
                  m_pDHxda[iNi] = m_pDHxda[iNi] / m_pMolecularWeight[iNi] / dA;
               }
            }
         }

         /* Total mass derivatives */
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            m_pDMxda[iNi] = 1.0;
         }

         /* Composition derivatives */
         dA = 1.0 / *m_pMx;
         pTa = m_pDXda;
         for ( iNj = 0; iNj < iNc; iNj++ )
         {
            *pTa++ = -m_pX[iNj] * dA;
         }

         for ( iNi = 1; iNi < iNc; iNi++ )
         {
            for ( iNj = 0; iNj < iNc; iNj++ )
            {
               *pTa++ = m_pDXda[iNj];
            }
         }

         pTa = m_pDXda;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            *pTa += dA;
            pTa += iNc + 1;
         }
      }
   }
}


/* 
// OnePhaseVolume
//
// Subroutine to do computations for volume of a one phase mixture 
//
// iM
//    Number of objects
// iNc
//    Number of components
//
// 1) The purpose of this routine is to provide very fast
//    volumetric or compositional calculations for a volume
//    balance or separator calculations.  The terms
//    returned are
//
//        -
//    V = V * moles
//
//    and/or
//
//    N
//     i
//
//    where N  is the total mass/mole of each component
//           i
//
//    Thus, when iVolume is either EOS_VOLUME or EOS_STOCK_TANK
//    the total volume is computed, and when iVolume is
//    either EOS_SEPARATOR or EOS_STOCK_TANK the total mass/mole
//    is computed
// 2) The pressure and temperature derivatives of the total
//    volume can be obtained by multiplying the results
//    from the SolveCubic routine by the moles
// 3) Pressure and temperature derivatives of the moles/mass
//    of each component are zero.
// 4) The accumulation derivative of the volume is trivial.
//    SolveCubic has not yet scaled the accumulation 
//    derivative by the total moles, thus
//
//            -    -
//    dVdA  = V + dVdA  [ * moles / moles ]
//        i           i
//
// 5) The composition derivative of the mass/mole accumulation
//    is likewise trivial, namely, 1
// 6) Finally need to set the mass/moles of each component
//    and the total volume
*/
void EosPvtModel::OnePhaseVolume( int iM, int iNc )
{
   int     iNi;
   int     iNj;
   int     iNcv;
   int     iNcs;
   int     i;
   double *pTa;
   double *pTb;
   double  dA;
   double  dB;

   /* Number of components for loop */
   iNcs = iNc * ( m_iVolume != EOS_SEPARATOR ? 1 : 0 );
   iNcv = iNc * ( m_iVolume != EOS_VOLUME ? 1 : 0 );

   /* Derivatives for more than one grid block */
   if ( iM > 1 )
   {
      if ( m_iDrv )
      {
         /* Pressure derivatives */
         if ( m_iDrv >= EOS_DRV_P )
         {
            for ( i = 0; i < iM; i++ )
            {
               m_pDZxdp[i] *= m_pMoles[i];
            }

            pTa = m_pDXdp;
            for ( iNi = 0; iNi < iNcv; iNi++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  *pTa++ = 0.0;
               }
            }

            /* Now for temperature derivatives */
            if ( m_iDrv >= EOS_DRV_T )
            {
               for ( i = 0; i < iM; i++ )
               {
                  m_pDZxdt[i] *= m_pMoles[i];
               }

               pTa = m_pDXdt;
               for ( iNi = 0; iNi < iNcv; iNi++ )
               {
                  for ( i = 0; i < iM; i++ )
                  {
                     *pTa++ = 0.0;
                  }
               }
            }
         }

         /* Density derivatives */
         if ( m_iMolarFlash )
         {
            pTa = m_pDZxda;
            for ( iNi = 0; iNi < iNcs; iNi++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  *pTa = m_pZx[i] +*pTa;
                  pTa++;
               }
            }
         }
         else
         {
            pTa = m_pDZxda;
            for ( iNi = 0; iNi < iNcs; iNi++ )
            {
               dA = m_pMolecularWeight[iNi];
               for ( i = 0; i < iM; i++ )
               {
                  *pTa = ( m_pZx[i] +*pTa ) / dA;
                  pTa++;
               }
            }
         }

         /* Composition derivatives */
         pTa = m_pDXda;
         for ( iNi = 0; iNi < iNcv; iNi++ )
         {
            for ( iNj = 0; iNj < iNc; iNj++ )
            {
               dA = ( iNi == iNj ) ? 1.0 : 0.0;
               for ( i = 0; i < iM; i++ )
               {
                  *pTa++ = dA;
               }
            }
         }
      }

      /* Adjust compositions */
      if ( m_iMolarFlash )
      {
         pTa = m_pX;
         pTb = m_pComposition;
         for ( iNi = 0; iNi < iNcv; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               *pTa++ = *pTb++ *m_pMoles[i];
            }
         }
      }
      else
      {
         pTa = m_pX;
         pTb = m_pComposition;
         for ( iNi = 0; iNi < iNcv; iNi++ )
         {
            dA = m_pMolecularWeight[iNi];
            for ( i = 0; i < iM; i++ )
            {
               *pTa++ = *pTb++ *dA * m_pMoles[i];
            }
         }
      }

      /* Finally adjust the volume */
      for ( i = 0; i < iM; i++ )
      {
         m_pZx[i] *= m_pMoles[i];
      }
   }

   /* Derivatives for single grid block */
   else
   {
      dA = *m_pZx;
      dB = *m_pMoles;
      if ( m_iDrv )
      {
         /* Pressure derivatives */
         if ( m_iDrv >= EOS_DRV_P )
         {
            *m_pDZxdp *= dB;
            for ( iNi = 0; iNi < iNcv; iNi++ )
            {
               m_pDXdp[iNi] = 0.0;
            }

            /* Now for temperature derivatives */
            if ( m_iDrv >= EOS_DRV_T )
            {
               *m_pDZxdt *= dB;
               for ( iNi = 0; iNi < iNcv; iNi++ )
               {
                  m_pDXdt[iNi] = 0.0;
               }
            }
         }

         /* Density derivatives */
         if ( m_iMolarFlash )
         {
            for ( iNi = 0; iNi < iNcs; iNi++ )
            {
               m_pDZxda[iNi] = dA + m_pDZxda[iNi];
            }
         }
         else
         {
            for ( iNi = 0; iNi < iNcs; iNi++ )
            {
               m_pDZxda[iNi] = ( dA + m_pDZxda[iNi] ) / m_pMolecularWeight[iNi];
            }
         }

         /* Composition derivatives */
         pTa = m_pDXda;
         for ( iNi = 0; iNi < iNcv; iNi++ )
         {
            for ( iNj = 0; iNj < iNc; iNj++ )
            {
               *pTa++ = ( iNi == iNj ) ? 1.0 : 0.0;
            }
         }
      }

      /* Adjust compositions */
      if ( m_iMolarFlash )
      {
         for ( iNi = 0; iNi < iNcv; iNi++ )
         {
            m_pX[iNi] = m_pComposition[iNi] * dB;
         }
      }
      else
      {
         for ( iNi = 0; iNi < iNcv; iNi++ )
         {
            m_pX[iNi] = m_pComposition[iNi] * dB * m_pMolecularWeight[iNi];
         }
      }

      /* Finally adjust the volume */
      *m_pZx = dA * dB;
   }
}


/* 
// PseudoPhase
//
// Routine to get results of one phase flash with a pseudo phase 
//
// iM
//    Number of objects
// iNc
//    Number of components
// 
// 1) Normalize and get compositions
// 2) Get the z factor and phase identification by calling SolveCubic
// 3) Get the Wilson K values.
// 4) Form a "pseudophase" by
//
//    Y  = K  X   / ( sum K  X  )
//     i    i   i      j   j  j 
//
//    if the single phase mixture is an oil, else, for a gas,
//
//    Y  = X  / K   / ( sum X  / K  )
//     i    i    i       j   j    j 
//
//    This is implemented by inverting the K values for
//    the gas case
// 5) Get the z factor for the pseudophase by calling 
//    SolveCubic
// 6) Convert the z factor to a volume for both phases
// 7) Compute the actual properties
*/
void EosPvtModel::PseudoPhase( int iM, int iNc )
{
   double *pTa;
   double *pTb;
   double *pTc;
   double  dA;
   double  dB;
   int     iNi;
   int     iNj;
   int     i;

   /* Set the mole fractions */
   if ( iM > 1 )
   {
      pTa = m_pX;
      pTb = m_pComposition;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         for ( i = 0; i < iM; i++ )
         {
            *pTa++ = *pTb++;
         }
      }
   }
   else
   {
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         m_pX[iNi] = m_pComposition[iNi];
      }
   }

   /* Chemical potential and z factor for the x phase */
   /* TODO: The next line is too long. Please rewrite to make it shorter. */
   ( m_pEosPvtTable )->SolveCubic( iM, EOS_NOFUGACITY, m_iHeat, EOS_NOPOTENTIAL, m_iDrv, EOS_PHASEID, m_iMultipleAbc, m_pAbcOffset, m_pPressure, m_pTemperature, m_pX, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pWork, m_pDXdp, m_pDXdt, m_pDXda, m_pHx, m_pDHxdp, m_pDHxdt, m_pDHxda, m_pATable, m_pPhaseId );

   /* Get wilson k values */
   ( m_pEosPvtTable )->WilsonKValues( iM, EOS_NOSCALEK, m_pTemperature, m_pKValue, m_pWork );

   /* Set the mole fractions and normalize for multiple grid blocks */
   if ( iM > 1 )
   {
      pTa = m_pKValue;
      pTb = m_pComposition;
      pTc = m_pY;
      for ( i = 0; i < iM; i++ )
      {
         dA = *pTa;
         *pTa = m_pPhaseId[i] ? dA : ( 1.0 / dA );
         *pTc = *pTb++ **pTa++;
         m_pSplit[i] = *pTc++;
      }

      for ( iNi = 1; iNi < iNc; iNi++ )
      {
         for ( i = 0; i < iM; i++ )
         {
            dA = *pTa;
            *pTa = m_pPhaseId[i] ? dA : ( 1.0 / dA );
            *pTc = *pTb++ **pTa++;
            m_pSplit[i] += *pTc++;
         }
      }

      pTa = m_pY;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         for ( i = 0; i < iM; i++ )
         {
            *pTa = *pTa / m_pSplit[i];
            pTa++;
         }
      }

      for ( i = 0; i < iM; i++ )
      {
         m_pSplit[i] *= m_pMoles[i];
      }
   }

   /* Set the mole fractions and normalize for a single grid block */
   else
   {
      iNj = ( *m_pPhaseId == EOS_SINGLE_PHASE_OIL ) ? 1 : 0;
      dA = m_pKValue[0];
      m_pKValue[0] = iNj ? dA : ( 1.0 / dA );
      m_pY[0] = m_pComposition[0] * m_pKValue[0];
      dB = m_pY[0];
      for ( iNi = 1; iNi < iNc; iNi++ )
      {
         dA = m_pKValue[iNi];
         m_pKValue[iNi] = iNj ? dA : ( 1.0 / dA );
         m_pY[iNi] = m_pComposition[iNi] * m_pKValue[iNi];
         dB += m_pY[iNi];
      }

      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         m_pY[iNi] = m_pY[iNi] / dB;
      }

      dB *= *m_pMoles;
      *m_pSplit = dB;
   }

   /* Chemical potential and z factor for the y phase */
   /* TODO: The next line is too long. Please rewrite to make it shorter. */
   ( m_pEosPvtTable )->SolveCubic( iM, EOS_NOFUGACITY, m_iHeat, EOS_NOPOTENTIAL, m_iDrv, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset, m_pPressure, m_pTemperature, m_pY, m_pZy, m_pDZydp, m_pDZydt, m_pDZyda, m_pWork, m_pDYdp, m_pDYdt, m_pDYda, m_pHy, m_pDHydp, m_pDHydt, m_pDHyda, m_pATable, m_pPhaseId );

   /* Perform the volume translations */
   /* TODO: The next line is too long. Please rewrite to make it shorter. */
   ( m_pEosPvtTable )->Volume( iM, m_iDrv, m_iMultipleAbc, m_pAbcOffset, m_pPressure, m_pTemperature, m_pX, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pWork );

   /* TODO: The next line is too long. Please rewrite to make it shorter. */
   ( m_pEosPvtTable )->Volume( iM, m_iDrv, m_iMultipleAbc, m_pAbcOffset, m_pPressure, m_pTemperature, m_pY, m_pZy, m_pDZydp, m_pDZydt, m_pDZyda, m_pWork );

   /* Get pseudo phase properties */
   PseudoPhaseProperties( iM, iNc );
}


/* 
// PseudoPhaseProperties
//
// Routine to get properties of one phase object with pseudo
// properties set for the other phase
//
// iM
//    Number of objects
// iNc
//    Number of components
//
// 1) Compute molecular weights and mole/mass fractions
// 2) If enthalpy needed and mass quantities, convert
//    enthalpy
// 3) Convert volume to density
// 4) The temperature and pressure derivatives of 
//    fractions and total mass/moles are zero.  Well, not
//    quite... there really should be a temperature
//    dependence in the temperature derivatives of the
//    pseudo-phase since the Wilson K values depend
//    non-linearly on temperature, but... STUPID
// 5) The temperature and pressure derivatives of
//    the density and enthalpy are as calculatedell, not
//    quite... there really should be a temperature
//    dependence in the temperature derivatives of the
//    pseudo-phase since the Wilson K values depend
//    non-linearly on temperature, but... STUPID
// 6) The density and enthalpy composition derivatives
//    must be modified by
//
//    dDxdA  = dDxdN  / moles
//         i        i
//    
//    or
//
//    dDxdA  = dDxdN  / moles / M
//         i        i            i
//    
//    since the call to SolveCubic is based upon a normalized
//    mole/mass fraction.  For the pseudo phase
//
//    dDydA  = dDydN  * K  / ( sum A  K  )
//         i        i    i      j   j  j
//    
//    or
//
//    dDxdA  = dDxdN  / ( sum A  K  ) / M
//         i        i          j  j      i
//
// 7) The accumulation derivatives of composition are
//
//    dXdA  = ( d   - X  ) / total   
//      i j      ij    i  
//
//    For the pseudo phase, 
//
//    Y = A  K   / ( sum A  K  )
//     i   i   i      j   j  j 
//   
//    or
//
//    Y = A  K   M  / ( sum A  K  M  )
//     i   i   i  i      j   j  j  j
//
//    Thus
//
//    dYdA  = ( d   K  - K  Y  ) / ( sum A  K  )
//      i j      ij  i    j  i        k   k  k  
//
//    or
//
//    dYdA  = ( d   K  M  - M  K  Y  ) / ( sum A  K  M  ) / M
//      i j      ij  i  i    j  j  i        k   k  k  k      j
// 
// 8) The total mass or mole derivatives with respect to
//    accumulation are unity
*/
void EosPvtModel::PseudoPhaseProperties( int iM, int iNc )
{
   double *pTa;
   double *pTb;
   double *pTc;
   double *pTd;
   double *pTe;
   double  dA;
   double  dB;
   double  dC;
   int     iNi;
   int     iNj;
   int     i;

   /* Set the molecular weights but not the phase id */
   PhaseId( iM, iNc, m_iMolarFlash, EOS_NOPHASEID );

   /* Modify heats */
   if ( m_iHeat && m_iMolarFlash == 0 )
   {
      ( m_pEosPvtTable )->MassHeat( iM, m_iDrv, m_pMWx, m_pHx, m_pDHxdp, m_pDHxdt, m_pDHxda );
      ( m_pEosPvtTable )->MassHeat( iM, m_iDrv, m_pMWy, m_pHy, m_pDHydp, m_pDHydt, m_pDHyda );
   }

   /* Convert to a density */
   ( m_pEosPvtTable )->Density( iM, m_iDrv, m_iMolarFlash, m_pMWx, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda );
   ( m_pEosPvtTable )->Density( iM, m_iDrv, m_iMolarFlash, m_pMWy, m_pZy, m_pDZydp, m_pDZydt, m_pDZyda );

   /* Multiple blocks */
   if ( iM > 1 )
   {
      if ( m_iMolarFlash == EOS_OPTION_OFF )
      {
         for ( i = 0; i < iM; i++ )
         {
            m_pMx[i] = m_pMWx[i] * m_pMoles[i];
            m_pMy[i] = 0.0;
         }
      }
      else
      {
         for ( i = 0; i < iM; i++ )
         {
            m_pMx[i] = m_pMoles[i];
            m_pMy[i] = 0.0;
         }
      }

      /* Derivatives */
      if ( m_iDrv )
      {
         /* Pressure derivatives */
         if ( m_iDrv >= EOS_DRV_P )
         {
            /* Composition derivatives */
            pTa = m_pDXdp;
            pTb = m_pDYdp;
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  *pTa++ = 0.0;
                  *pTb++ = 0.0;
               }
            }

            /* Accumulation derivatives */
            for ( i = 0; i < iM; i++ )
            {
               m_pDMxdp[i] = 0.0;
               m_pDMydp[i] = 0.0;
            }

            /* Now for temperature derivatives */
            if ( m_iDrv >= EOS_DRV_T )
            {
               /* Composition derivatives */
               pTa = m_pDXdt;
               pTb = m_pDYdt;
               for ( iNi = 0; iNi < iNc; iNi++ )
               {
                  for ( i = 0; i < iM; i++ )
                  {
                     *pTa++ = 0.0;
                     *pTb++ = 0.0;
                  }
               }

               /* Accumulation derivatives */
               for ( i = 0; i < iM; i++ )
               {
                  m_pDMxdt[i] = 0.0;
                  m_pDMydt[i] = 0.0;
               }
            }
         }

         /* Density derivatives */
         if ( m_iMolarFlash )
         {
            pTa = m_pDZxda;
            pTb = m_pDZyda;
            pTc = m_pKValue;
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  *pTa = *pTa / m_pMoles[i];
                  *pTb = *pTb **pTc++ / m_pSplit[i];
                  pTa++;
                  pTb++;
               }
            }
         }
         else
         {
            pTa = m_pDZxda;
            pTc = m_pDZyda;
            pTd = m_pKValue;
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               dA = m_pMolecularWeight[iNi];
               for ( i = 0; i < iM; i++ )
               {
                  *pTa = *pTa / dA / m_pMoles[i];
                  *pTc = *pTc **pTd++ / dA / m_pSplit[i];
                  pTa++;
                  pTc++;
               }
            }
         }

         /* Enthalpy derivatives */
         if ( m_iHeat )
         {
            if ( m_iMolarFlash )
            {
               pTa = m_pDHxda;
               pTb = m_pDHyda;
               pTc = m_pKValue;
               for ( iNi = 0; iNi < iNc; iNi++ )
               {
                  for ( i = 0; i < iM; i++ )
                  {
                     *pTa = *pTa / m_pMoles[i];
                     *pTb = *pTb **pTc++ / m_pSplit[i];
                     pTa++;
                     pTb++;
                  }
               }
            }
            else
            {
               pTa = m_pDHxda;
               pTc = m_pDHyda;
               pTd = m_pKValue;
               for ( iNi = 0; iNi < iNc; iNi++ )
               {
                  dA = m_pMolecularWeight[iNi];
                  for ( i = 0; i < iM; i++ )
                  {
                     *pTa = *pTa / dA / m_pMoles[i];
                     *pTc = *pTc **pTd++ / dA / m_pSplit[i];
                     pTa++;
                     pTc++;
                  }
               }
            }
         }

         /* Total mass derivatives */
         pTa = m_pDMxda;
         pTb = m_pDMyda;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               *pTa++ = 1.0;
               *pTb++ = 0.0;
            }
         }

         /* Composition derivatives */
         if ( m_iMolarFlash == EOS_OPTION_OFF )
         {
            for ( i = 0; i < iM; i++ )
            {
               m_pSplit[i] *= m_pMWy[i];
            }
         }

         pTa = m_pDXda;
         pTc = m_pDYda;
         pTb = m_pX;
         pTd = m_pY;
         pTe = m_pKValue;
         for ( iNj = 0; iNj < iNc; iNj++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               *pTa++ = -*pTb++ / m_pMx[i];
               *pTc++ = -*pTd++ / m_pSplit[i];
            }
         }

         for ( iNi = 1; iNi < iNc; iNi++ )
         {
            pTb = m_pDXda;
            pTd = m_pDYda;
            pTe += iM;
            for ( iNj = 0; iNj < iNc; iNj++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  *pTa++ = *pTb++;
                  *pTc++ = *pTd++ *pTe[i];
               }
            }
         }

         pTc = m_pDYda;
         for ( iNj = 0; iNj < iNc; iNj++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               *pTc *= m_pKValue[i];
               pTc++;
            }
         }

         pTa = m_pDXda;
         pTb = m_pDYda;
         pTc = m_pKValue;
         iNj = iNc * iM;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               *pTa += 1.0 / m_pMx[i];
               *pTb += *pTc++ / m_pSplit[i];
               pTa++;
               pTb++;
            }

            pTa += iNj;
            pTb += iNj;
         }
      }

      /* Set the total mass to zero */
      for ( i = 0; i < iM; i++ )
      {
         m_pSplit[i] = 0.0;
      }
   }

   /* Single block */
   else
   {
      if ( m_iMolarFlash == EOS_OPTION_OFF )
      {
         *m_pMx = *m_pMWx **m_pMoles;
         *m_pMy = 0.0;
      }
      else
      {
         *m_pMx = *m_pMoles;
         *m_pMy = 0.0;
      }

      /* Derivatives */
      if ( m_iDrv )
      {
         /* Pressure derivatives */
         if ( m_iDrv >= EOS_DRV_P )
         {
            /* Composition derivatives */
            *m_pDMxdp = 0.0;
            *m_pDMydp = 0.0;
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               m_pDXdp[iNi] = 0.0;
               m_pDYdp[iNi] = 0.0;
            }

            /* Now for temperature derivatives */
            if ( m_iDrv >= EOS_DRV_T )
            {
               /* Composition derivatives */
               *m_pDMxdt = 0.0;
               *m_pDMydt = 0.0;
               for ( iNi = 0; iNi < iNc; iNi++ )
               {
                  m_pDXdt[iNi] = 0.0;
                  m_pDYdt[iNi] = 0.0;
               }
            }
         }

         /* Density derivatives */
         dA = *m_pMoles;
         dB = *m_pSplit;
         if ( m_iMolarFlash )
         {
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               m_pDZxda[iNi] = m_pDZxda[iNi] / dA;
               m_pDZyda[iNi] = m_pDZyda[iNi] * m_pKValue[iNi] / dB;
            }
         }
         else
         {
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               dC = m_pMolecularWeight[iNi];
               m_pDZxda[iNi] = m_pDZxda[iNi] / dA / dC;
               m_pDZyda[iNi] = m_pDZyda[iNi] * m_pKValue[iNi] / dB / dC;
            }
         }

         /* Enthalpy derivatives */
         if ( m_iHeat )
         {
            if ( m_iMolarFlash )
            {
               for ( iNi = 0; iNi < iNc; iNi++ )
               {
                  m_pDHxda[iNi] = m_pDHxda[iNi] / dA;
                  m_pDHyda[iNi] = m_pDHyda[iNi] * m_pKValue[iNi] / dB;
               }
            }
            else
            {
               for ( iNi = 0; iNi < iNc; iNi++ )
               {
                  dC = m_pMolecularWeight[iNi];
                  m_pDHxda[iNi] = m_pDHxda[iNi] / dA / dC;
                  m_pDHyda[iNi] = m_pDHyda[iNi] * m_pKValue[iNi] / dB / dC;
               }
            }
         }

         /* Total mass derivatives */
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            m_pDMxda[iNi] = 1.0;
            m_pDMyda[iNi] = 0.0;
         }

         /* Composition derivatives */
         if ( m_iMolarFlash == EOS_OPTION_OFF )
         {
            *m_pSplit *= *m_pMWy;
         }

         dA = 1.0 / *m_pMx;
         dB = 1.0 / *m_pSplit;
         pTa = m_pDXda;
         pTb = m_pDYda;
         for ( iNj = 0; iNj < iNc; iNj++ )
         {
            *pTa++ = -m_pX[iNj] * dA;
            *pTb++ = -m_pY[iNj] * dB;
         }

         for ( iNi = 1; iNi < iNc; iNi++ )
         {
            for ( iNj = 0; iNj < iNc; iNj++ )
            {
               *pTa++ = m_pDXda[iNj];
               *pTb++ = m_pDYda[iNj] * m_pKValue[iNi];
            }
         }

         for ( iNj = 0; iNj < iNc; iNj++ )
         {
            m_pDYda[iNj] *= m_pKValue[0];
         }

         pTa = m_pDXda;
         pTb = m_pDYda;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            *pTa += dA;
            *pTb += m_pKValue[iNi] * dB;
            pTa += iNc + 1;
            pTb += iNc + 1;
         }
      }

      /* Set the total mass to zero */
      *m_pSplit = 0.0;
   }
}


/* 
// TwoPhase
//
// Routine to get results of two phase flash 
//
// iM
//    Number of objects
// iNc
//    Number of components
//
// 1) Call TwoPhaseMatrix to get z factors and to do all
//    matrix related calculations for derivatives
// 2) Convert z factors to volumes
// 3) Call TwoPhaseProperties for normal mode, i.e., for
//    densities, compositions, etc.
// 4) Call TwoPhaseVolume for a volume balance or
//    a separator calculation
*/
void EosPvtModel::TwoPhase( int iM, int iNc )
{
   /* Get mole fraction derivatives */
   TwoPhaseMatrix( iM, iNc );

   /* Convert to a volume */
   /* TODO: The next line is too long. Please rewrite to make it shorter. */
   ( m_pEosPvtTable )->Volume( iM, m_iDrv, m_iMultipleAbc, m_pAbcOffset, m_pPressure, m_pTemperature, m_pX, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pWork );

   /* TODO: The next line is too long. Please rewrite to make it shorter. */
   ( m_pEosPvtTable )->Volume( iM, m_iDrv, m_iMultipleAbc, m_pAbcOffset, m_pPressure, m_pTemperature, m_pY, m_pZy, m_pDZydp, m_pDZydt, m_pDZyda, m_pWork );

   /* Set properties */
   if ( m_iVolume == EOS_DENSITY )
   {
      TwoPhaseProperties( iM, iNc );
   }
   else
   {
      TwoPhaseVolume( iM, iNc );
   }
}


/* 
// TwoPhaseMatrix
//
// Subroutine to compute derivatives of phase mole numbers 
//
// iM
//    Number of objects
// iNc
//    Number of components
//
// 1) To avoid problems with rounding, the first thing that
//    is done is to ensure that the "Y" phase corresponds
//    to the phase with the largest phase split.  Towards
//    this end, the K values are inverted if the split
//    is less than 0.5.  The y mole fraction is set to
//    the split, and the x mole fraction to 1 minus the split
// 2) Then the compositions are calculated from
//
//    X = Z / ( 1 - S + K S ), Y = K X
//
// 3) Then get the z factors.  Fugacities are required if
//    there are derivatives.  Enthalpies may also be needed
// 4) Matrix formation and scaling is similar to that 
//    done in NewtonFlash.  There are a few differences,
//    however, for reasons which will be clear in other 
//    derivative routines.  Recall in NewtonFlash that
//    both sides were multiplied by Split * ( 1 - Split ).
//    In this routine, we only multiply the X term right
//    hand side by Split; the 1 - Split term is missing.
//    Hence:
//
//    a) Temperature and pressure derivatives will be scaled
//       so that what is returned will be divided by the split,
//       in other words, what is in brackets is returned
//
//       dNxdP = [ dNxdP / ( 1 - S ) ]
//
//       dNydP = [ dNydP / S ] 
//
//       The reasons for this will be clear later on.
//    b) Considerable care is taken for components that
//       are less than rounding that dNxdA is set to the K value
//
//  5) We solve the matrix systems, for example,
//
//     S * ( 1 - S ) * M dNxdP = S * dRdP 
//
//     where M represents the Jacobian and R represents the right
//     hand side
//  6) Compute Y derivative from the X derivatives.  This
//     is accomplished by, for pressure
//
//     [ dNydP / S ] = [ dNxdP / ( 1 - S ) ] * ( 1 - S ) / S
//
//     Note that since [ dNxdP / ( 1 - S ) ] is returned with the 
//     wronf sign that [ dNydP / S ] will have the correct sign 
//     For accumulations
//
//     [ dNydA / S ] = I / S - [ dNxdA / ( 1 - S ) ] * ( 1 - S ) / S
//
//     Note that the division by the split is safe since the
//     convention for the X and Y phase was chosen so that
//     the split is greater than 0.5
*/
void EosPvtModel::TwoPhaseMatrix( int iM, int iNc )
{
   double *pTa;
   double *pTb;
   double *pTc;
   double *pTd;
   double *pTf;
   int     iNi;
   int     iNj;
   int     iNcm;
   int     i;
   int     iFugacity;
   double  dA;
   double  dB;
   double  dC;
   double  dD;
   double  dE;

   /* Set terms */
   iNcm = iNc * iM;

   /* Change K values and set mole numbers for multiple grid blocks */
   if ( iM > 1 )
   {
      pTc = m_pComposition;
      pTa = m_pKValue;
      pTd = m_pX;
      pTf = m_pY;
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
#ifdef IPFtune
#pragma ivdep
#endif
      for ( i = 0; i < iM; i++ )
      {
         dD = m_pSplit[i];
         dA = ( dD > 0.5 ) ? 1.0 : 0.0;
         dB = 1.0 - dA;
         dC = pTa[i];
         dD = dB + ( dA - dB ) * dD;
         dE = 1.0 - dD;
         dC = dA * dC + dB / dC;
         pTa[i] = dC;
         m_pWork[i] = dA;
         m_pPotentialx[i] = dB;
         m_pSplit[i] = dD;
         m_pMy[i] = dD;
         m_pMx[i] = dE;
         pTd[i] = pTc[i] / ( dE + dD * dC );
         pTf[i] = pTd[i] * dC;
      }

      for ( iNi = 1; iNi < iNc; iNi++ )
      {
         pTc += iM;
         pTa += iM;
         pTd += iM;
         pTf += iM;
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
#ifdef IPFtune
#pragma ivdep
#endif
         for ( i = 0; i < iM; i++ )
         {
            dD = pTa[i];
            dC = m_pWork[i] * dD + m_pPotentialx[i] / dD;
            pTa[i] = dC;
            pTd[i] = pTc[i] / ( m_pMx[i] + m_pMy[i] * dC );
            pTf[i] = pTd[i] * dC;
         }
      }
   }

   /* Change K values and set mole numbers for single grid block */
   else
   {
      dD = *m_pSplit;
      dA = ( dD > 0.5 ) ? 1.0 : 0.0;
      dB = 1.0 - dA;
      dC = m_pKValue[0];
      dD = dB + ( dA - dB ) * dD;
      dE = 1.0 - dD;
      dC = dA * dC + dB / dC;
      m_pKValue[0] = dC;
      *m_pSplit = dD;
      *m_pMy = dD;
      *m_pMx = dE;
      m_pX[0] = m_pComposition[0] / ( dE + dD * dC );
      m_pY[0] = m_pX[0] * dC;
      for ( iNi = 1; iNi < iNc; iNi++ )
      {
         dC = m_pKValue[iNi];
         dC = dA * dC + dB / dC;
         m_pKValue[iNi] = dC;
         m_pX[iNi] = m_pComposition[iNi] / ( dE + dD * dC );
         m_pY[iNi] = m_pX[iNi] * dC;
      }
   }

   /* Chemical potential for the x phase */
   iFugacity = m_iDrv ? EOS_FUGACITY : EOS_NOFUGACITY;

   /* TODO: The next line is too long. Please rewrite to make it shorter. */
   ( m_pEosPvtTable )->SolveCubic( iM, iFugacity, m_iHeat, EOS_NOPOTENTIAL, m_iDrv, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset, m_pPressure, m_pTemperature, m_pX, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pWork, m_pDXdp, m_pDXdt, m_pDXda, m_pHx, m_pDHxdp, m_pDHxdt, m_pDHxda, m_pATable, m_pPhaseId );

   /* Chemical potential for the y phase */
   /* TODO: The next line is too long. Please rewrite to make it shorter. */
   ( m_pEosPvtTable )->SolveCubic( iM, iFugacity, m_iHeat, EOS_NOPOTENTIAL, m_iDrv, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset, m_pPressure, m_pTemperature, m_pY, m_pZy, m_pDZydp, m_pDZydt, m_pDZyda, m_pWork, m_pDYdp, m_pDYdt, m_pDYda, m_pHy, m_pDHydp, m_pDHydt, m_pDHyda, m_pATable, m_pPhaseId );

   /* Set scaling terms */
   if ( m_iDrv )
   {
      /* Multiple grid block */
      if ( iM > 1 )
      {
         pTb = m_pPotentialx;
         pTd = m_pWork;
         pTa = m_pKValue;
         pTc = m_pComposition;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
#ifdef IPFtune
#pragma ivdep
#endif
            for ( i = 0; i < iM; i++ )
            {
               dA = pTa[i];
               dB = pTc[i];
               pTd[i] = 1.0 / ( m_pMx[i] + m_pMy[i] * dA );
               pTb[i] = sqrt( ( ( dB > m_dTiny ) ? dB : m_dTiny ) * dA ) * pTd[i];
            }

            pTa += iM;
            pTb += iM;
            pTc += iM;
            pTd += iM;
         }

         /* Set the pressure and temperature derivative terms first */
         if ( m_iDrv >= EOS_DRV_P )
         {
            pTd = m_pDXdp;
            pTf = m_pDYdp;
            pTb = m_pPotentialx;
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
#ifdef IPFtune
#pragma ivdep
#endif
               for ( i = 0; i < iM; i++ )
               {
                  pTd[i] = pTb[i] * m_pMy[i] * ( pTd[i] - pTf[i] );
               }

               pTd += iM;
               pTb += iM;
               pTf += iM;
            }

            if ( m_iDrv >= EOS_DRV_T )
            {
               pTd = m_pDXdt;
               pTf = m_pDYdt;
               pTb = m_pPotentialx;
               for ( iNi = 0; iNi < iNc; iNi++ )
               {
                  for ( i = 0; i < iM; i++ )
                  {
                     *pTd = *pTb++ *m_pMy[i] * ( *pTd -*pTf++ );
                     pTd++;
                  }
               }
            }
         }

         /* Assemble the matrix */
         pTc = m_pPotentialx;
         pTd = m_pDXda;
         pTf = m_pDYda;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            pTb = m_pPotentialx;
            for ( iNj = 0; iNj < iNi; iNj++ )
            {
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
#ifdef IPFtune
#pragma ivdep
#endif
               for ( i = 0; i < iM; i++ )
               {
                  pTf[i] = pTb[i] * ( pTf[i] - 1.0 );
               }

               pTb += iM;
               pTf += iM;
            }

            pTd += iNi * iM;
            for ( iNj = iNi; iNj < iNc; iNj++ )
            {
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
#ifdef IPFtune
#pragma ivdep
#endif
               for ( i = 0; i < iM; i++ )
               {
                  dA = pTb[i];
                  dB = pTf[i];
                  pTd[i] = dA * pTc[i] * ( m_pMy[i] * pTd[i] + m_pMx[i] * dB - 1.0 );
                  pTf[i] = dA * ( dB - 1.0 );
               }

               pTb += iM;
               pTd += iM;
               pTf += iM;
            }

            pTc += iM;
         }

         /* Diagonal terms */
         pTc = m_pComposition;
         pTd = m_pDXda;
         pTf = m_pDYda;
         pTb = m_pWork;
         pTa = m_pPotentialx;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
#ifdef IPFtune
#pragma ivdep
#endif
            for ( i = 0; i < iM; i++ )
            {
               dB = pTa[i];
               pTd[i] += 1.0;
               pTf[i] += pTb[i] / dB;
               pTa[i] = ( pTc[i] > m_dTiny ) ? dB : 0.0;
            }

            pTa += iM;
            pTb += iM;
            pTc += iM;
            pTd += iNcm + iM;
            pTf += iNcm + iM;
         }
      }

      /* Single grid block */
      else
      {
         dB = *m_pMx;
         dC = *m_pMy;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            dA = m_pKValue[iNi];
            dD = m_pComposition[iNi];
            m_pWork[iNi] = 1.0 / ( dB + dC * dA );
            m_pPotentialx[iNi] = sqrt( ( ( dD > m_dTiny ) ? dD : m_dTiny ) * dA ) * m_pWork[iNi];
         }

         /* Set the pressure and temperature derivative terms first */
         if ( m_iDrv >= EOS_DRV_P )
         {
#ifdef IPFtune
#pragma ivdep
#endif
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               m_pDXdp[iNi] = m_pPotentialx[iNi] * dC * ( m_pDXdp[iNi] - m_pDYdp[iNi] );
            }

            if ( m_iDrv >= EOS_DRV_T )
            {
#ifdef IPFtune
#pragma ivdep
#endif
               for ( iNi = 0; iNi < iNc; iNi++ )
               {
                  m_pDXdt[iNi] = m_pPotentialx[iNi] * dC * ( m_pDXdt[iNi] - m_pDYdt[iNi] );
               }
            }
         }

         /* Assemble the matrix */
         pTd = m_pDXda;
         pTf = m_pDYda;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            dD = m_pPotentialx[iNi];
            for ( iNj = 0; iNj < iNi; iNj++ )
            {
               pTf[iNj] = m_pPotentialx[iNj] * ( pTf[iNj] - 1.0 );
            }

            for ( iNj = iNi; iNj < iNc; iNj++ )
            {
               dA = m_pPotentialx[iNj];
               dE = pTf[iNj];
               pTd[iNj] = dA * dD * ( dC * pTd[iNj] + dB * dE - 1.0 );
               pTf[iNj] = dA * ( dE - 1.0 );
            }

            pTd += iNc;
            pTf += iNc;
         }

         /* Diagonal terms */
         pTd = m_pDXda;
         pTf = m_pDYda;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            dB = m_pPotentialx[iNi];
            *pTd += 1.0;
            *pTf += m_pWork[iNi] / dB;
            m_pPotentialx[iNi] = ( m_pComposition[iNi] > m_dTiny ) ? dB : 0.0;
            pTd += iNc + 1;
            pTf += iNc + 1;
         }
      }

      /* Factor the matrix */
      Cholesky( iM, iNc, m_pDXda );

      /* Backsolve matrices multiple grid block */
      if ( iM > 1 )
      {
         pTf = m_pDYda;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            BackSolve( iM, iNc, m_pDXda, pTf );
            pTc = m_pComposition;
            pTd = pTf;
            pTb = m_pWork;
            pTa = m_pPotentialx;
            for ( iNj = 0; iNj < iNc; iNj++ )
            {
               if ( iNi == iNj )
               {
                  for ( i = 0; i < iM; i++ )
                  {
                     pTd[i] = ( pTc[i] > m_dTiny ) ? ( pTd[i] * pTa[i] ) : pTb[i];
                  }
               }
               else
               {
#ifdef IPFtune
#pragma ivdep
#endif       
                  for ( i = 0; i < iM; i++ )
                  {
                     pTd[i] *= pTa[i];
                  }
               }

               pTa += iM;
               pTb += iM;
               pTc += iM;
               pTd += iM;
            }

            pTf += iNcm;
         }

         /* Pressure derivatives */
         if ( m_iDrv >= EOS_DRV_P )
         {
            BackSolve( iM, iNc, m_pDXda, m_pDXdp );
            pTd = m_pDXdp;
            pTb = m_pPotentialx;
            for ( iNj = 0; iNj < iNc; iNj++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  *pTd *= *pTb++;
                  pTd++;
               }
            }

            /* Temperature derivatives */
            if ( m_iDrv >= EOS_DRV_T )
            {
               BackSolve( iM, iNc, m_pDXda, m_pDXdt );
               pTd = m_pDXdt;
               pTb = m_pPotentialx;
               for ( iNj = 0; iNj < iNc; iNj++ )
               {
                  for ( i = 0; i < iM; i++ )
                  {
                     *pTd *= *pTb++;
                     pTd++;
                  }
               }
            }
         }

         /* Set terms properly for multiple blocks */
         for ( i = 0; i < iM; i++ )
         {
            m_pPotentialx[i] = 1.0 / m_pSplit[i] - 1.0;
         }

         if ( m_iDrv >= EOS_DRV_P )
         {
            pTd = m_pDXdp;
            pTf = m_pDYdp;
            for ( iNj = 0; iNj < iNc; iNj++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  *pTf++ = m_pPotentialx[i] **pTd++;
               }
            }

            if ( m_iDrv >= EOS_DRV_T )
            {
               pTd = m_pDXdt;
               pTf = m_pDYdt;
               for ( iNj = 0; iNj < iNc; iNj++ )
               {
                  for ( i = 0; i < iM; i++ )
                  {
                     *pTf++ = m_pPotentialx[i] **pTd++;
                  }
               }
            }
         }

         pTd = m_pDXda;
         pTf = m_pDYda;
         iNj = iNc * iNc;
         for ( iNi = 0; iNi < iNj; iNi++ )
         {
#ifdef IPFtune
            double *pPx = m_pPotentialx;
#pragma ivdep
            for ( i = 0; i < iM; i++ )
            {
               dA = *pTf;
               *pTd++ = dA;
               *pTf++ = 0-pPx[i] * dA;
            }
#else
            for ( i = 0; i < iM; i++ )
            {
               dA = *pTf;
               *pTd++ = dA;
               *pTf++ = -m_pPotentialx[i] * dA;
            }
#endif
         }

         for ( i = 0; i < iM; i++ )
         {
            m_pPotentialx[i] += 1.0;
         }

         iNj = iNc * iM;
         pTf = m_pDYda;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               *pTf += m_pPotentialx[i];
               pTf++;
            }

            pTf += iNj;
         }
      }

      /* Backsolve matrices single grid block */
      else
      {
         pTf = m_pDYda;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            BackSolve( 1, iNc, m_pDXda, pTf );
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
#ifdef IPFtune
#pragma ivdep
#endif
            for ( iNj = 0; iNj < iNc; iNj++ )
            {
               pTf[iNj] *= m_pPotentialx[iNj];
            }

            pTf[iNi] = ( m_pComposition[iNi] > m_dTiny ) ? pTf[iNi] : m_pWork[iNi];
            pTf += iNc;
         }

         /* Pressure derivatives */
         if ( m_iDrv >= EOS_DRV_P )
         {
            BackSolve( 1, iNc, m_pDXda, m_pDXdp );
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
#ifdef IPFtune
#pragma ivdep
#endif
            for ( iNj = 0; iNj < iNc; iNj++ )
            {
               m_pDXdp[iNj] *= m_pPotentialx[iNj];
            }

            /* Temperature derivatives */
            if ( m_iDrv >= EOS_DRV_T )
            {
               BackSolve( 1, iNc, m_pDXda, m_pDXdt );
               for ( iNj = 0; iNj < iNc; iNj++ )
               {
                  m_pDXdt[iNj] *= m_pPotentialx[iNj];
               }
            }
         }

         /* Set terms properly */
         dB = 1.0 / *m_pSplit - 1.0;
         if ( m_iDrv >= EOS_DRV_P )
         {
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
#ifdef IPFtune
#pragma ivdep
#endif
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               m_pDYdp[iNi] = m_pDXdp[iNi] * dB;
            }

            if ( m_iDrv >= EOS_DRV_T )
            {
               for ( iNi = 0; iNi < iNc; iNi++ )
               {
                  m_pDYdt[iNi] = m_pDXdt[iNi] * dB;
               }
            }
         }

         iNj = iNc * iNc;
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
#ifdef IPFtune
#pragma ivdep
#endif
         for ( iNi = 0; iNi < iNj; iNi++ )
         {
            dA = m_pDYda[iNi];
            m_pDXda[iNi] = dA;
            m_pDYda[iNi] = -dB * dA;
         }

         dB += 1.0;
         pTf = m_pDYda;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            *pTf += dB;
            pTf += iNc + 1;
         }
      }
   }
}


/* 
// TwoPhaseProperties
//
// Routine to set results of two phase flash 
//
// iM
//    Number of objects
// iNc
//    Number of components
//
// 1) First set the phase identifications and the molecular
//    weights
// 2) Then, if needed, convert enthalpies to a mass basis
// 3) Then convert volumes to densities
// 4) Then set the total mass or mole number for each phase
// 5) Then get the pressure and temperature derivatives
// 6) Then get the composition derivatives of the density,
//    and the enthalpy, if required
// 7) Finally, set the composition derivatives
*/
void EosPvtModel::TwoPhaseProperties( int iM, int iNc )
{
   double dA;
   int    i;

   /* Set the phase id */
   PhaseId( iM, iNc, m_iMolarFlash, EOS_PHASEID );

   /* Modify heats */
   if ( m_iHeat && m_iMolarFlash == 0 )
   {
      ( m_pEosPvtTable )->MassHeat( iM, m_iDrv, m_pMWx, m_pHx, m_pDHxdp, m_pDHxdt, m_pDHxda );
      ( m_pEosPvtTable )->MassHeat( iM, m_iDrv, m_pMWy, m_pHy, m_pDHydp, m_pDHydt, m_pDHyda );
   }

   /* Convert to a density */
   ( m_pEosPvtTable )->Density( iM, m_iDrv, m_iMolarFlash, m_pMWx, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda );
   ( m_pEosPvtTable )->Density( iM, m_iDrv, m_iMolarFlash, m_pMWy, m_pZy, m_pDZydp, m_pDZydt, m_pDZyda );

   /* Scale terms */
   if ( m_iMolarFlash )
   {
      for ( i = 0; i < iM; i++ )
      {
         dA = m_pMoles[i];
         m_pMx[i] *= dA;
         m_pMy[i] *= dA;
      }
   }
   else
   {
      for ( i = 0; i < iM; i++ )
      {
         dA = m_pMoles[i];
         m_pMx[i] *= dA * m_pMWx[i];
         m_pMy[i] *= dA * m_pMWy[i];
      }
   }

   /* Derivatives */
   if ( m_iDrv )
   {
      /* Pressure derivatives */
      if ( m_iDrv >= EOS_DRV_P )
      {
         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         TwoPhaseDerivatives( iM, iNc, m_pDXdp, m_pDYdp, m_pDZxdp, m_pDZydp, m_pDHxdp, m_pDHydp, m_pDMxdp, m_pDMydp );

         /* Now for temperature derivatives */
         if ( m_iDrv >= EOS_DRV_T )
         {
            /* TODO: The next line is too long. Please rewrite to make it shorter. */
            TwoPhaseDerivatives( iM, iNc, m_pDXdt, m_pDYdt, m_pDZxdt, m_pDZydt, m_pDHxdt, m_pDHydt, m_pDMxdt, m_pDMydt );
         }
      }

      /* Density derivatives from chain rule */
      TwoPhaseDensity( iM, iNc, m_pDZxda, m_pDXda );
      TwoPhaseDensity( iM, iNc, m_pDZyda, m_pDYda );

      /* Enthalpy derivatives from chain rule */
      if ( m_iHeat )
      {
         TwoPhaseDensity( iM, iNc, m_pDHxda, m_pDXda );
         TwoPhaseDensity( iM, iNc, m_pDHyda, m_pDYda );
      }

      /* Composition derivatives from chain rule */
      TwoPhaseComposition( iM, iNc );
   }
}


/* 
// TwoPhaseDerivatives
//
// Routine to compute two phase pressure and temperature derivatives 
//
// iM
//    Number of objects
// iNc
//    Number of components
// pDX
//    Derivative of composition of x phase
// pDY
//    Derivative of composition of y phase
// pDZx
//    Derivative of density of x phase
// pDZy
//    Derivative of density of y phase
// pDHx
//    Derivative of enthalpy of x phase
// pDHy
//    Derivative of enthalpy of y phase
// pDMx
//    Derivative of total mass or mole of x phase
// pDMy
//    Derivative of total mass or mole of y phase
//
// 1) The routine will assume that it is the pressure derivative;
//    temperature derivatives are similar
// 2) If enthalpies, then
//
//    dHxdP = dHxdP + ( sum dHxdNx  dNxdP ) / ( 1 - S )
//                       i        i    i
//    
//    Note here we discover why dNxdP was scaled by ( 1 - S )!
//    The equation for dHydP is similar.  There are some
//    differences in sign in the x derivatives.  The
//    molecular weight terms were accounted for in MassHeat
//    thus there is no difference between mass and moles
// 3) Then need to handle density and total mass terms, 
//    as well as compositions.  For mole fractions and mass
//    fractions
//
//    dZxdP = dZxdP + ( sum dZxdNx  dNxdP ) / ( 1 - S )
//                       i        i    i
//
//    For mole fractions, 
//
//    dMxdP = sum dNxdP 
//             i     i
//
//    For mass fractions
//
//    dMxdP = sum M dNxdP
//             i   i   i
//
// 4) Then need the composition derivatives.  For mole fractions
//
//    X  = Nx  / ( Sum Nx  )
//     i     i      j    j
//
//    Thus
//
//    dXdP = ( dNxdP - X  Sum dNxdP ) / ( 1 - S )
//      i         i     i  j               
//
//    Once again we see why the derivative terms were scaled
//    as they were.  For mass fractions, 
//
//    dXdP = ( M dNxdP - X  Sum M dNxdP ) / ( 1 - S ) / MWx
//      i       i   i     i  j   j
//
//    The signs must be watched to conform to those found in
//    TwoPhaseMatrix
// 5) Finally, we must take into account everything for
//    the total mass and moles
//
//    dMydP = dMydP * Split * moles
//
//    dMxdP = - dMydP            
*/
void EosPvtModel::TwoPhaseDerivatives( int iM, int iNc, double *pDX, double *pDY, double *pDZx, double *pDZy,
                                       double *pDHx, double *pDHy, double *pDMx, double *pDMy )
{
   double *pTa;
   double *pTb;
   double *pTc;
   double *pTd;
   double  dA;
   double  dB;
   double  dC;
   double  dDZxt;
   double  dDZyt;
   double  dDMxt;
   double  dDMyt;
   int     i;
   int     iNi;

   /* Code for multiple grid blocks */
   if ( iM > 1 )
   {
      /* Heat derivatives */
      if ( m_iHeat )
      {
         pTa = m_pDHxda;
         pTb = m_pDHyda;
         pTc = pDX;
         pTd = pDY;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               pDHx[i] -= *pTc++ **pTa++;
               pDHy[i] += *pTd++ **pTb++;
            }
         }
      }

      /* Scale terms and take derivatives for mole fractions */
      if ( m_iMolarFlash )
      {
         pTc = pDX;
         pTd = pDY;
         pTa = m_pDZxda;
         pTb = m_pDZyda;
         for ( i = 0; i < iM; i++ )
         {
            dC = *pTc++;
            dB = *pTd++;
            pDZx[i] -= dC **pTa++;
            pDZy[i] += dB **pTb++;
            pDMx[i] = dC;
            pDMy[i] = dB;
         }

         for ( iNi = 1; iNi < iNc; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               dC = *pTc++;
               dB = *pTd++;
               pDZx[i] -= dC **pTa++;
               pDZy[i] += dB **pTb++;
               pDMx[i] += dC;
               pDMy[i] += dB;
            }
         }

         /* Mole fraction derivatives */
         pTc = pDX;
         pTd = pDY;
         pTa = m_pX;
         pTb = m_pY;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               *pTd = *pTd -*pTb++ *pDMy[i];
               *pTc = *pTa++ *pDMx[i] -*pTc;
               pTc++;
               pTd++;
            }
         }
      }

      /* Scale terms and take derivatives for mass fractions */
      else
      {
         pTc = pDX;
         pTd = pDY;
         pTa = m_pDZxda;
         pTb = m_pDZyda;
         dA = m_pMolecularWeight[0];
         for ( i = 0; i < iM; i++ )
         {
            dC = *pTc;
            dB = *pTd;
            pDZx[i] -= dC **pTa++;
            pDZy[i] += dB **pTb++;
            dC *= dA;
            dB *= dA;
            *pTc++ = dC;
            *pTd++ = dB;
            pDMx[i] = dC;
            pDMy[i] = dB;
         }

         for ( iNi = 1; iNi < iNc; iNi++ )
         {
            dA = m_pMolecularWeight[iNi];
            for ( i = 0; i < iM; i++ )
            {
               dC = *pTc;
               dB = *pTd;
               pDZx[i] -= dC **pTa++;
               pDZy[i] += dB **pTb++;
               dC *= dA;
               dB *= dA;
               *pTc++ = dC;
               *pTd++ = dB;
               pDMx[i] += dC;
               pDMy[i] += dB;
            }
         }

         /* Mole fraction derivatives */
         pTc = pDX;
         pTd = pDY;
         pTa = m_pX;
         pTb = m_pY;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               *pTd = ( *pTd -*pTb++ *pDMy[i] ) / m_pMWy[i];
               *pTc = ( *pTa++ *pDMx[i] -*pTc ) / m_pMWx[i];
               pTc++;
               pTd++;
            }
         }
      }

      /* Scale by moles */
      for ( i = 0; i < iM; i++ )
      {
         pDMy[i] *= m_pMoles[i] * m_pSplit[i];
         pDMx[i] = -pDMy[i];
      }
   }

   /* Code for single grid blocks */
   else
   {
      /* Heat terms */
      if ( m_iHeat )
      {
         dDZxt = *pDHx - pDX[0] * m_pDHxda[0];
         dDZyt = *pDHy + pDY[0] * m_pDHyda[0];
         for ( iNi = 1; iNi < iNc; iNi++ )
         {
            dDZxt -= pDX[iNi] * m_pDHxda[iNi];
            dDZyt += pDY[iNi] * m_pDHyda[iNi];
         }

         *pDHx = dDZxt;
         *pDHy = dDZyt;
      }

      /* Scale terms and take derivatives for mole fractions */
      if ( m_iMolarFlash )
      {
         dC = pDX[0];
         dB = pDY[0];
         dDZxt = *pDZx - dC * m_pDZxda[0];
         dDZyt = *pDZy + dB * m_pDZyda[0];
         dDMxt = dC;
         dDMyt = dB;
         for ( iNi = 1; iNi < iNc; iNi++ )
         {
            dC = pDX[iNi];
            dB = pDY[iNi];
            dDZxt -= dC * m_pDZxda[iNi];
            dDZyt += dB * m_pDZyda[iNi];
            dDMxt += dC;
            dDMyt += dB;
         }

         *pDZx = dDZxt;
         *pDZy = dDZyt;

         /* Mole fraction derivatives */
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            pDY[iNi] = pDY[iNi] - m_pY[iNi] * dDMyt;
            pDX[iNi] = m_pX[iNi] * dDMxt - pDX[iNi];
         }
      }

      /* Scale terms and take derivatives for mass fractions */
      else
      {
         dA = m_pMolecularWeight[0];
         dC = pDX[0];
         dB = pDY[0];
         dDZxt = *pDZx - dC * m_pDZxda[0];
         dDZyt = *pDZy + dB * m_pDZyda[0];
         dC *= dA;
         dB *= dA;
         pDX[0] = dC;
         pDY[0] = dB;
         dDMxt = dC;
         dDMyt = dB;
         for ( iNi = 1; iNi < iNc; iNi++ )
         {
            dA = m_pMolecularWeight[iNi];
            dC = pDX[iNi];
            dB = pDY[iNi];
            dDZxt -= dC * m_pDZxda[iNi];
            dDZyt += dB * m_pDZyda[iNi];
            dC *= dA;
            dB *= dA;
            pDX[iNi] = dC;
            pDY[iNi] = dB;
            dDMxt += dC;
            dDMyt += dB;
         }

         *pDZx = dDZxt;
         *pDZy = dDZyt;

         /* Mole fraction derivatives */
         dA = *m_pMWy;
         dB = *m_pMWx;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            pDY[iNi] = ( pDY[iNi] - m_pY[iNi] * dDMyt ) / dA;
            pDX[iNi] = ( m_pX[iNi] * dDMxt - pDX[iNi] ) / dB;
         }
      }

      /* Scale by moles */
      *pDMy = dDMyt * m_pMoles[0] * m_pSplit[0];
      *pDMx = -*pDMy;
   }
}


/* 
// TwoPhaseDensity
//
// Routine to compute density derivatives wrt composition
//
// iM
//    Number of objects
// iNc
//    Number of components
// pDZ
//    Density derivative to take
// pDX
//    Corresponding composition derivative
//
// 1) The density derivative for the x phase is computed from
//
//    dZxdA  = sum dZxdNx  * dNxdA  / moles / ( 1 - S )
//         i    j        j      j i
//
//    Once again the utility of what was done in TwoPhaseMatrix
//    with respect to scaling is clear.  The above
//    equation is for mole fractions.  For mass fractions
//
//    dZxdA  = sum dZxdNx  * dNxdA  / moles / ( 1 - S ) / M
//         i    j        j      j i                        i
*/
void EosPvtModel::TwoPhaseDensity( int iM, int iNc, double *pDZ, double *pDX )
{
   double  dA;
   double  dB;
   double  dC;
   double *pTa;
   double *pTb;
   double *pTc;
   int     iNi;
   int     iNj;
   int     i;

   /* Take the derivatives for multiple blocks */
   if ( iM > 1 )
   {
      /* Molar flash */
      if ( m_iMolarFlash )
      {
         pTb = m_pWork;
         pTa = pDZ;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               *pTb++ = *pTa++ / m_pMoles[i];
            }
         }

         pTc = pDX;
         pTa = pDZ;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            pTb = m_pWork;
            for ( i = 0; i < iM; i++ )
            {
               pTa[i] = *pTb++ **pTc++;
            }

            for ( iNj = 1; iNj < iNc; iNj++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  pTa[i] += *pTb++ **pTc++;
               }
            }

            pTa += iM;
         }
      }

      /* Mass flash */
      else
      {
         pTb = m_pWork;
         pTa = pDZ;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               *pTb++ = *pTa++ / m_pMoles[i];
            }
         }

         pTc = pDX;
         pTa = pDZ;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            pTb = m_pWork;
            for ( i = 0; i < iM; i++ )
            {
               pTa[i] = *pTb++ **pTc++;
            }

            for ( iNj = 1; iNj < iNc; iNj++ )
            {
#ifdef IPFtune
               int cnt = iM;
#pragma ivdep
               for ( i = 0; i < cnt; i++ )
               {
                  pTa[i] += *pTb++ * *pTc++;
               }
#else
               for ( i = 0; i < iM; i++ )
               {
                  pTa[i] += *pTb++ **pTc++;
               }
#endif
            }

            dC = m_pMolecularWeight[iNi];
            for ( i = 0; i < iM; i++ )
            {
               pTa[i] = pTa[i] / dC;
            }

            pTa += iM;
         }
      }
   }

   /* Take the derivatives for single block */
   else
   {
      /* Moles */
      if ( m_iMolarFlash )
      {
         dA = *m_pMoles;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            m_pWork[iNi] = pDZ[iNi] / dA;
         }

         pTc = pDX;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            dB = m_pWork[0] **pTc++;
            for ( iNj = 1; iNj < iNc; iNj++ )
            {
               dB += m_pWork[iNj] **pTc++;
            }

            pDZ[iNi] = dB;
         }
      }

      /* Mass */
      else
      {
         dA = *m_pMoles;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            m_pWork[iNi] = pDZ[iNi] / dA;
         }

         pTc = pDX;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            dB = m_pWork[0] **pTc++;
            for ( iNj = 1; iNj < iNc; iNj++ )
            {
               dB += m_pWork[iNj] **pTc++;
            }

            pDZ[iNi] = dB / m_pMolecularWeight[iNi];
         }
      }
   }
}


/* 
// TwoPhaseComposition
// 
// Routine to compute composition derivatives wrt accumulation
// 
// iM
//    Number of objects
// iNc
//    Number of components
//
// 1) The composition, for mole fractions, is
//
//    X  = Nx  / ( Sum Nx  )
//     i     i      k    k
//
//    Thus
//
//    dXdA  = ( dNxdA - X  Sum dNxdA ) / ( 1 - S ) / moles
//      i j        i j   i  k     k j        
//
//    Once again we see why the derivative terms were scaled
//    as they were.  For mass fractions, 
//
//    dXdA  = ( M dNxdA  - X  Sum M dNxdA  ) / (1-S) / MWx / M  / mole
//      i j      i   i j    i  k   k     j                    j
//
//    The signs must be watched to conform to those found in
//    TwoPhaseMatrix
// 2) The total mass or mole derivatives are also calculated
*/
void EosPvtModel::TwoPhaseComposition( int iM, int iNc )
{
   double *pTa;
   double *pTb;
   double *pTc;
   double *pTd;
   double *pTe;
   double *pTf;
   double  dA;
   double  dB;
   double  dC;
   double  dD;
   double  dE;
   double  dF;
   double  dG;
   int     iNi;
   int     iNj;
   int     i;

   /* Take the derivatives for multiple blocks */
   if ( iM > 1 )
   {
      /* Molar derivatives */
      if ( m_iMolarFlash )
      {
         pTc = m_pDMxda;
         pTd = m_pDMyda;
         pTe = m_pDXda;
         pTf = m_pDYda;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               dA = m_pMoles[i];
               *pTe = *pTe / dA;
               *pTf = *pTf / dA;
               pTc[i] = *pTe++;
               pTd[i] = *pTf++;
            }

            for ( iNj = 1; iNj < iNc; iNj++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  dA = m_pMoles[i];
                  *pTe = *pTe / dA;
                  *pTf = *pTf / dA;
                  pTc[i] += *pTe++;
                  pTd[i] += *pTf++;
               }
            }

            pTc += iM;
            pTd += iM;
         }

         /* Mole fraction derivatives */
         pTc = m_pDMxda;
         pTd = m_pDMyda;
         pTe = m_pDXda;
         pTf = m_pDYda;
         for ( i = 0; i < iM; i++ )
         {
            m_pWork[i] = m_pMoles[i] * m_pSplit[i];
         }

         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            pTa = m_pX;
            pTb = m_pY;
            for ( iNj = 0; iNj < iNc; iNj++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  *pTe -= *pTa++ *pTc[i];
                  *pTf -= *pTb++ *pTd[i];
                  pTe++;
                  pTf++;
               }
            }

            for ( i = 0; i < iM; i++ )
            {
               pTd[i] *= m_pWork[i];
               pTc[i] = 1.0 - pTd[i];
            }

            pTc += iM;
            pTd += iM;
         }
      }

      /* Scale by molecular weight and take total mass derivatives */
      else
      {
         pTc = m_pDMxda;
         pTd = m_pDMyda;
         pTe = m_pDXda;
         pTf = m_pDYda;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            dB = m_pMolecularWeight[iNi];
            dC = m_pMolecularWeight[0];
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
            for ( i = 0; i < iM; i++ )
            {
               m_pWork[i] = 1.0 / ( dB * m_pMoles[i] );
               dA = dC * m_pWork[i];
               dD = pTe[i] * dA;
               dE = pTf[i] * dA;
               pTc[i] = dD;
               pTd[i] = dE;
               pTe[i] = dD;
               pTf[i] = dE;
            }

            for ( iNj = 1; iNj < iNc; iNj++ )
            {
               pTe += iM;
               pTf += iM;
               dC = m_pMolecularWeight[iNj];
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
               for ( i = 0; i < iM; i++ )
               {
                  dA = dC * m_pWork[i];
                  dD = pTe[i] * dA;
                  dE = pTf[i] * dA;
                  pTc[i] += dD;
                  pTd[i] += dE;
                  pTe[i] = dD;
                  pTf[i] = dE;
               }
            }

            pTc += iM;
            pTd += iM;
            pTe += iM;
            pTf += iM;
         }

         /* Mass fraction derivatives */
         pTc = m_pDMxda;
         pTd = m_pDMyda;
         pTe = m_pDXda;
         pTf = m_pDYda;
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
         for ( i = 0; i < iM; i++ )
         {
            m_pWork[i] = m_pMoles[i] * m_pSplit[i];
         }

         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            pTa = m_pX;
            pTb = m_pY;
            for ( iNj = 0; iNj < iNc; iNj++ )
            {
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
#ifdef IPFtune
#pragma ivdep
#endif
               for ( i = 0; i < iM; i++ )
               {
                  pTe[i] -= ( pTa[i] * pTc[i] );
                  pTf[i] -= ( pTb[i] * pTd[i] );
               }

               pTa += iM;
               pTb += iM;
               pTe += iM;
               pTf += iM;
            }

            for ( i = 0; i < iM; i++ )
            {
               pTd[i] *= m_pWork[i];
               pTc[i] = 1.0 - pTd[i];
            }

            pTc += iM;
            pTd += iM;
         }

#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
         for ( i = 0; i < iM; i++ )
         {
            m_pWork[i] = 1.0 / m_pMWx[i];
         }

         iNj = iNc * iNc;
         pTe = m_pDXda;
         for ( iNi = 0; iNi < iNj; iNi++ )
         {
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
#ifdef IPFtune
#pragma ivdep
#endif
            for ( i = 0; i < iM; i++ )
            {
               pTe[i] *= m_pWork[i];
            }

            pTe += iM;
         }

#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
         for ( i = 0; i < iM; i++ )
         {
            m_pWork[i] = 1.0 / m_pMWy[i];
         }

         pTe = m_pDYda;
         for ( iNi = 0; iNi < iNj; iNi++ )
         {
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
#ifdef IPFtune
#pragma ivdep
#endif
            for ( i = 0; i < iM; i++ )
            {
               pTe[i] *= m_pWork[i];
            }

            pTe += iM;
         }
      }
   }

   /* Take the derivatives for single block */
   else
   {
      /* Molar derivatives */
      if ( m_iMolarFlash )
      {
         dA = *m_pMoles;
         pTe = m_pDXda;
         pTf = m_pDYda;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            *pTe = *pTe / dA;
            *pTf = *pTf / dA;
            dB = *pTe++;
            dC = *pTf++;
            for ( iNj = 1; iNj < iNc; iNj++ )
            {
               *pTe = *pTe / dA;
               *pTf = *pTf / dA;
               dB += *pTe++;
               dC += *pTf++;
            }

            m_pDMxda[iNi] = dB;
            m_pDMyda[iNi] = dC;
         }

         /* Mole fraction derivatives */
         pTe = m_pDXda;
         pTf = m_pDYda;
         dA *= *m_pSplit;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            dB = m_pDMxda[iNi];
            dC = m_pDMyda[iNi];
            for ( iNj = 0; iNj < iNc; iNj++ )
            {
               *pTe -= m_pX[iNj] * dB;
               *pTf -= m_pY[iNj] * dC;
               pTe++;
               pTf++;
            }

            m_pDMyda[iNi] = dC * dA;
            m_pDMxda[iNi] = 1.0 - m_pDMyda[iNi];
         }
      }

      /* Scale by molecular weight and take total mass derivatives */
      else
      {
         pTe = m_pDXda;
         pTf = m_pDYda;
         dC = *m_pMoles;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            dB = dC * m_pMolecularWeight[iNi];
            dA = m_pMolecularWeight[0] / dB;
            dD = pTe[0] * dA;
            dE = pTf[0] * dA;
            pTe[0] = dD;
            pTf[0] = dE;
#ifdef IPFtune
            double *pMW = m_pMolecularWeight;
#pragma ivdep
#endif
            for ( iNj = 1; iNj < iNc; iNj++ )
            {
#ifdef IPFtune
               dA = pMW[iNj] / dB;
#else       
               dA = m_pMolecularWeight[iNj] / dB;
#endif
               dF = pTe[iNj] * dA;
               dG = pTf[iNj] * dA;
               dD += dF;
               dE += dG;
               pTe[iNj] = dF;
               pTf[iNj] = dG;
            }

            pTe += iNc;
            pTf += iNc;
            m_pDMxda[iNi] = dD;
            m_pDMyda[iNi] = dE;
         }

         /* Mass fraction derivatives */
         pTe = m_pDXda;
         pTf = m_pDYda;
         dD = 1.0 / *m_pMWx;
         dE = 1.0 / *m_pMWy;
         dC *= *m_pSplit;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            dA = m_pDMxda[iNi];
            dB = m_pDMyda[iNi];
#ifdef IPFtune
            double *ptX = m_pX;
            double *ptY = m_pY;
#pragma ivdep
            for ( iNj = 0; iNj < iNc; iNj++ )
            {
               pTe[iNj] = ( pTe[iNj] - ptX[iNj] * dA ) * dD;
               pTf[iNj] = ( pTf[iNj] - ptY[iNj] * dB ) * dE;
            }
#else    
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
            for ( iNj = 0; iNj < iNc; iNj++ )
            {
               pTe[iNj] = ( pTe[iNj] - m_pX[iNj] * dA ) * dD;
               pTf[iNj] = ( pTf[iNj] - m_pY[iNj] * dB ) * dE;
            }
#endif    

            pTe += iNc;
            pTf += iNc;
            m_pDMyda[iNi] = dB * dC;
            m_pDMxda[iNi] = 1.0 - m_pDMyda[iNi];
         }
      }
   }
}


/* 
// TwoPhaseVolume
//
// Subroutine to do computations for total volume of two phases 
//
// iM
//    Number of objects
// iNc
//    Number of components
//
// 1) The purpose of this routine is to calculate
//        
//         -
//    Nx = Nx * moles * ( 1 - split )
//
//    and
//
//         -
//    Vx = Vx * moles * ( 1 - split )
//
//    Of course we need to calculate Ny, Vy, and the formulas
//    change slightly when on a mass basis, but here is the
//    broad outline for the x phase  Also, if EOS_VOLUME or
//    EOS_STOCK_TANK is the value of iVolume, we calculate
//    the total volume.  If EOS_SEPARATOR or EOS_STOCK_TANK
//    is the value of iVolume, we calculate the total mass/mole
//    of each component in each phase
// 2) First, of course, we must store the phase identification
//    and obtain the molecular weights
// 3) For the derivatives of the moles with respect to 
//    pressure or temperature.  Recall that dNxdP is the
//    really the derivative of the mole fraction times the
//    phase moles as defined in TwoPhaseMatrix.  Hence
//
//    dNxdP = [ dNxdP / ( 1 - S ) ] * moles * ( 1 - S )
// 
//    Note that the sign must be consistent with what is
//    returned in TwoPhaseMatrix
// 4) For the volume derivatives
//          
//    dVxdP = ((1-S)*(dVxdP + sum dVxdNx dNxdP) - Vx*dSdP ) * moles
//                             j        j   j
//
//    as dNxdP is the derivative of the ( 1 - S ) * X divided
//    by ( 1 - S ).  Of course the signs must be consistent
//    with those used in TwoPhaseMatrix. dSdP is
//    calculated from
//
//    dSdP = sum [ dNxdP / ( 1 - S ) ] * ( 1 - S )
//            j       j
//
// 5) For the derivatives of the moles with respect to 
//    accumulations.  Recall that dNxdA is the
//    really the derivative of the mole fraction times the
//    phase moles as defined in TwoPhaseMatrix.  Hence
//
//    dNxdA = [ dNxdA / ( 1 - S ) ] * ( 1 - S )
// 
//    Note that the sign must be consistent with what is
//    returned in TwoPhaseMatrix
// 6) For the volume derivatives
//          
//    dVxdA = ((1-S)*(dVxdA + sum dVxdNx dNxdA) - Vx*dSdA ) * moles
//                             j        j   j
//
//    as dNxdA is the derivative of the ( 1 - S ) * X divided
//    by ( 1 - S ).  Of course the signs must be consistent
//    with those used in TwoPhaseMatrix.  dSdA is
//    calculated from
//
//    dSdA = sum [ dNxdA / ( 1 - S ) ] * ( 1 - S )
//            j       j
*/
void EosPvtModel::TwoPhaseVolume( int iM, int iNc )
{
   double *pTa;
   double *pTb;
   double *pTc;
   double *pTd;
   double *pTe;
   double *pTf;
   double *pTg;
   double  dA;
   double  dB;
   double  dC;
   double  dD;
   double  dE;
   double  dF;
   double  dG;
   double  dH;
   double  dO;
   int     iNi;
   int     iNj;
   int     iNcv;
   int     iNcs;
   int     i;

   /* Set loop control parameter for volume */
   iNcv = iNc * ( m_iVolume != EOS_VOLUME ? 1 : 0 );
   iNcs = iNc * ( m_iVolume != EOS_SEPARATOR ? 1 : 0 );

   /* Set the molecular weights and phase id */
   PhaseId( iM, iNc, EOS_OPTION_ON, EOS_PHASEID );

   /* Set the phase indicator for multiple blocks */
   if ( iM > 1 )
   {
      for ( i = 0; i < iM; i++ )
      {
         dA = m_pMoles[i];
         m_pIfx[i] = dA * m_pMx[i];
         m_pIfy[i] = dA * m_pMy[i];
      }

      /* Derivatives */
      if ( m_iDrv )
      {
         /* Pressure derivatives */
         if ( m_iDrv >= EOS_DRV_P )
         {
            pTa = m_pDXdp;
            pTb = m_pDYdp;
            pTc = m_pDZxda;
            pTd = m_pDZyda;
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
            for ( i = 0; i < iM; i++ )
            {
               dB = pTb[i];
               m_pDZxdp[i] -= pTa[i] * pTc[i];
               m_pDZydp[i] += dB * pTd[i];
               m_pDMydp[i] = dB;
            }

            for ( iNi = 1; iNi < iNcs; iNi++ )
            {
               pTa += iM;
               pTb += iM;
               pTc += iM;
               pTd += iM;
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
               for ( i = 0; i < iM; i++ )
               {
                  dB = pTb[i];
                  m_pDZxdp[i] -= pTa[i] * pTc[i];
                  m_pDZydp[i] += dB * pTd[i];
                  m_pDMydp[i] += dB;
               }
            }

#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
            for ( i = 0; i < iM; i++ )
            {
               dB = m_pMoles[i];
               dA = m_pDMydp[i] * m_pSplit[i];
               m_pDZxdp[i] = ( m_pMx[i] * m_pDZxdp[i] - m_pZx[i] * dA ) * dB;
               m_pDZydp[i] = ( m_pMy[i] * m_pDZydp[i] + m_pZy[i] * dA ) * dB;
            }

            if ( m_iMolarFlash )
            {
               pTa = m_pDXdp;
               pTb = m_pDYdp;
               for ( iNi = 0; iNi < iNcv; iNi++ )
               {
                  for ( i = 0; i < iM; i++ )
                  {
                     *pTa = -m_pIfx[i] **pTa;
                     *pTb *= m_pIfy[i];
                     pTa++;
                     pTb++;
                  }
               }
            }
            else
            {
               pTa = m_pDXdp;
               pTb = m_pDYdp;
               for ( iNi = 0; iNi < iNcv; iNi++ )
               {
                  dA = m_pMolecularWeight[iNi];
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
                  for ( i = 0; i < iM; i++ )
                  {
                     pTa[i] *= ( -dA * m_pIfx[i] );
                     pTb[i] *= dA * m_pIfy[i];
                  }

                  pTa += iM;
                  pTb += iM;
               }
            }

            /* Temperature derivatives */
            if ( m_iDrv >= EOS_DRV_T )
            {
               pTa = m_pDXdt;
               pTb = m_pDYdt;
               pTc = m_pDZxda;
               pTd = m_pDZyda;
               for ( i = 0; i < iM; i++ )
               {
                  dB = *pTb++;
                  m_pDZxdt[i] -= *pTa++ **pTc++;
                  m_pDZydt[i] += dB **pTd++;
                  m_pDMydt[i] = dB;
               }

               for ( iNi = 1; iNi < iNcs; iNi++ )
               {
                  for ( i = 0; i < iM; i++ )
                  {
                     dB = *pTb++;
                     m_pDZxdt[i] -= *pTa++ **pTc++;
                     m_pDZydt[i] += dB **pTd++;
                     m_pDMydt[i] += dB;
                  }
               }

               for ( i = 0; i < iM; i++ )
               {
                  dB = m_pMoles[i];
                  dA = m_pDMydt[i] * m_pSplit[i];
                  m_pDZxdt[i] = ( m_pMx[i] * m_pDZxdt[i] - m_pZx[i] * dA ) * dB;
                  m_pDZydt[i] = ( m_pMy[i] * m_pDZydt[i] + m_pZy[i] * dA ) * dB;
               }

               if ( m_iMolarFlash )
               {
                  pTa = m_pDXdt;
                  pTb = m_pDYdt;
                  for ( iNi = 0; iNi < iNcv; iNi++ )
                  {
                     for ( i = 0; i < iM; i++ )
                     {
                        *pTa = -m_pIfx[i] **pTa;
                        *pTb *= m_pIfy[i];
                        pTa++;
                        pTb++;
                     }
                  }
               }
               else
               {
                  pTa = m_pDXdt;
                  pTb = m_pDYdt;
                  for ( iNi = 0; iNi < iNcv; iNi++ )
                  {
                     dA = m_pMolecularWeight[iNi];
                     for ( i = 0; i < iM; i++ )
                     {
                        *pTa = -dA * m_pIfx[i] **pTa;
                        *pTb *= dA * m_pIfy[i];
                        pTa++;
                        pTb++;
                     }
                  }
               }
            }
         }

         /* Derivatives from chain rule */
         pTc = m_pMux;
         pTb = m_pDZxda;
         pTd = m_pMuy;
         pTe = m_pDZyda;
         for ( iNi = 0; iNi < iNcs; iNi++ )
         {
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
            for ( i = 0; i < iM; i++ )
            {
               pTc[i] = pTb[i];
               pTd[i] = pTe[i];
            }

            pTb += iM;
            pTc += iM;
            pTd += iM;
            pTe += iM;
         }

         pTa = m_pDXda;
         pTb = m_pDZxda;
         pTc = m_pDYda;
         pTd = m_pDZyda;
         pTg = m_pDMyda;
         for ( iNi = 0; iNi < iNcs; iNi++ )
         {
            pTe = m_pMux;
            pTf = m_pMuy;
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
            for ( i = 0; i < iM; i++ )
            {
               dA = pTc[i];
               pTg[i] = dA;
               pTb[i] = pTe[i] * pTa[i];
               pTd[i] = pTf[i] * dA;
            }

            pTc += iM;
            pTe += iM;
            pTa += iM;
            pTf += iM;

            for ( iNj = 1; iNj < iNc; iNj++ )
            {
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
               for ( i = 0; i < iM; i++ )
               {
                  dA = pTc[i];
                  pTg[i] += dA;
                  pTb[i] += pTe[i] * pTa[i];
                  pTd[i] += pTf[i] * dA;
               }

               pTc += iM;
               pTe += iM;
               pTa += iM;
               pTf += iM;
            }

            pTb += iM;
            pTd += iM;
            pTg += iM;
         }

         /* Set the volume derivatives for mole fractions */
         if ( m_iMolarFlash )
         {
            pTa = m_pDZxda;
            pTb = m_pDZyda;
            pTc = m_pDMyda;
            for ( iNi = 0; iNi < iNcs; iNi++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  dA = m_pSplit[i] **pTc++;
                  dC = m_pZx[i];
                  *pTa = m_pMx[i] **pTa + dC - dC * dA;
                  *pTb = m_pMy[i] **pTb + m_pZy[i] * dA;
                  pTa++;
                  pTb++;
               }
            }

            pTa = m_pDXda;
            pTb = m_pDYda;
            for ( iNi = 0; iNi < iNcv; iNi++ )
            {
               for ( iNj = 0; iNj < iNc; iNj++ )
               {
                  for ( i = 0; i < iM; i++ )
                  {
                     *pTa *= m_pMx[i];
                     *pTb *= m_pMy[i];
                     pTa++;
                     pTb++;
                  }
               }
            }
         }

         /* Finally set the volume derivatives for mass fractions */
         else
         {
            pTa = m_pDZxda;
            pTb = m_pDZyda;
            pTc = m_pDMyda;
            for ( iNi = 0; iNi < iNcs; iNi++ )
            {
               dB = m_pMolecularWeight[iNi];
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
               for ( i = 0; i < iM; i++ )
               {
                  dA = m_pSplit[i] * pTc[i];
                  dC = m_pZx[i];
                  pTa[i] = ( m_pMx[i] * pTa[i] + dC - dC * dA ) / dB;
                  pTb[i] = ( m_pMy[i] * pTb[i] + m_pZy[i] * dA ) / dB;
               }

               pTa += iM;
               pTb += iM;
               pTc += iM;
            }

            pTa = m_pDXda;
            pTb = m_pDYda;
            for ( iNi = 0; iNi < iNcv; iNi++ )
            {
               dB = m_pMolecularWeight[iNi];
               for ( iNj = 0; iNj < iNc; iNj++ )
               {
                  dA = m_pMolecularWeight[iNj] / dB;
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
                  for ( i = 0; i < iM; i++ )
                  {
                     pTa[i] *= dA * m_pMx[i];
                     pTb[i] *= dA * m_pMy[i];
                  }

                  pTa += iM;
                  pTb += iM;
               }
            }
         }
      }

      /* Set the composition */
      if ( m_iMolarFlash )
      {
         pTa = m_pX;
         pTb = m_pY;
         for ( iNi = 0; iNi < iNcv; iNi++ )
         {
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
            for ( i = 0; i < iM; i++ )
            {
               pTa[i] *= m_pIfx[i];
               pTb[i] *= m_pIfy[i];
            }

            pTa += iM;
            pTb += iM;
         }
      }
      else
      {
         pTa = m_pX;
         pTb = m_pY;
         for ( iNi = 0; iNi < iNcv; iNi++ )
         {
#ifdef USE_VECTOR_ALWAYS
#pragma vector always
#endif
            for ( i = 0; i < iM; i++ )
            {
               dA = m_pMolecularWeight[iNi];
               pTa[i] *= dA * m_pIfx[i];
               pTb[i] *= dA * m_pIfy[i];
            }

            pTa += iM;
            pTb += iM;
         }
      }

      /* Finally set the volume */
      for ( i = 0; i < iM; i++ )
      {
         m_pZx[i] *= m_pIfx[i];
         m_pZy[i] *= m_pIfy[i];
      }
   }

   /* Set the phase indicator for aw single block */
   else
   {
      dA = *m_pMoles;
      dH = dA **m_pMx;
      dO = dA **m_pMy;

      /* Derivatives */
      if ( m_iDrv )
      {
         dA = *m_pSplit;

         /* Pressure derivatives */
         if ( m_iDrv >= EOS_DRV_P )
         {
            dE = m_pDYdp[0];
            dC = *m_pDZxdp - m_pDXdp[0] * m_pDZxda[0];
            dD = *m_pDZydp + dE * m_pDZyda[0];
            for ( iNi = 1; iNi < iNcs; iNi++ )
            {
               dB = m_pDYdp[iNi];
               dC -= m_pDXdp[iNi] * m_pDZxda[iNi];
               dD += dB * m_pDZyda[iNi];
               dE += dB;
            }

            dB = *m_pMoles;
            dE *= dA;
            *m_pDZxdp = ( *m_pMx * dC -*m_pZx * dE ) * dB;
            *m_pDZydp = ( *m_pMy * dD +*m_pZy * dE ) * dB;
            if ( m_iMolarFlash )
            {
               for ( iNi = 0; iNi < iNcv; iNi++ )
               {
                  m_pDXdp[iNi] = -dH * m_pDXdp[iNi];
                  m_pDYdp[iNi] = dO * m_pDYdp[iNi];
               }
            }
            else
            {
               for ( iNi = 0; iNi < iNcv; iNi++ )
               {
                  dE = m_pMolecularWeight[iNi];
                  m_pDXdp[iNi] = -dE * dH * m_pDXdp[iNi];
                  m_pDYdp[iNi] = dE * dO * m_pDYdp[iNi];
               }
            }

            /* Temperature derivatives */
            if ( m_iDrv >= EOS_DRV_T )
            {
               dE = m_pDYdt[0];
               dC = *m_pDZxdt - m_pDXdt[0] * m_pDZxda[0];
               dD = *m_pDZydt + dE * m_pDZyda[0];
               for ( iNi = 1; iNi < iNcs; iNi++ )
               {
                  dB = m_pDYdt[iNi];
                  dC -= m_pDXdt[iNi] * m_pDZxda[iNi];
                  dD += dB * m_pDZyda[iNi];
                  dE += dB;
               }

               dB = *m_pMoles;
               dE *= dA;
               *m_pDZxdt = ( *m_pMx * dC -*m_pZx * dE ) * dB;
               *m_pDZydt = ( *m_pMy * dD +*m_pZy * dE ) * dB;
               if ( m_iMolarFlash )
               {
                  for ( iNi = 0; iNi < iNcv; iNi++ )
                  {
                     m_pDXdt[iNi] = -dH * m_pDXdt[iNi];
                     m_pDYdt[iNi] = dO * m_pDYdt[iNi];
                  }
               }
               else
               {
                  for ( iNi = 0; iNi < iNcv; iNi++ )
                  {
                     dE = m_pMolecularWeight[iNi];
                     m_pDXdt[iNi] = -dE * dH * m_pDXdt[iNi];
                     m_pDYdt[iNi] = dE * dO * m_pDYdt[iNi];
                  }
               }
            }
         }

         /* Derivatives from chain rule */
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            m_pMux[iNi] = m_pDZxda[iNi];
            m_pMuy[iNi] = m_pDZyda[iNi];
         }

         pTa = m_pDXda;
         pTb = m_pDYda;
         for ( iNi = 0; iNi < iNcs; iNi++ )
         {
            dB = *pTb++;
            dC = m_pMux[0] **pTa++;
            dD = m_pMuy[0] * dB;
            for ( iNj = 1; iNj < iNc; iNj++ )
            {
               dE = *pTb++;
               dC += m_pMux[iNj] **pTa++;
               dD += m_pMuy[iNj] * dE;
               dB += dE;
            }

            m_pDMyda[iNi] = dB;
            m_pDZxda[iNi] = dC;
            m_pDZyda[iNi] = dD;
         }

         /* Set the volume derivatives for mole fractions */
         if ( m_iMolarFlash )
         {
            dB = *m_pMx;
            dC = *m_pMy;
            dD = *m_pZx;
            dE = *m_pZy;
            for ( iNi = 0; iNi < iNcs; iNi++ )
            {
               dF = dA * m_pDMyda[iNi];
               m_pDZxda[iNi] = dB * m_pDZxda[iNi] + dD - dD * dF;
               m_pDZyda[iNi] = dC * m_pDZyda[iNi] + dE * dF;
            }

            pTa = m_pDXda;
            pTb = m_pDYda;
            dF = *m_pMx;
            dG = *m_pMy;
            for ( iNi = 0; iNi < iNcv; iNi++ )
            {
               for ( iNj = 0; iNj < iNc; iNj++ )
               {
                  *pTa *= dF;
                  *pTb *= dG;
                  pTa++;
                  pTb++;
               }
            }
         }

         /* Finally set the volume derivatives for mass fractions */
         else
         {
            dB = *m_pMx;
            dC = *m_pMy;
            dD = *m_pZx;
            dE = *m_pZy;
            for ( iNi = 0; iNi < iNcs; iNi++ )
            {
               dF = dA * m_pDMyda[iNi];
               dG = m_pMolecularWeight[iNi];
               m_pDZxda[iNi] = ( dB * m_pDZxda[iNi] + dD - dD * dF ) / dG;
               m_pDZyda[iNi] = ( dC * m_pDZyda[iNi] + dE * dF ) / dG;
            }

            pTa = m_pDXda;
            pTb = m_pDYda;
            dF = *m_pMx;
            dG = *m_pMy;
            for ( iNi = 0; iNi < iNcv; iNi++ )
            {
               dE = m_pMolecularWeight[iNi];
               for ( iNj = 0; iNj < iNc; iNj++ )
               {
                  dD = m_pMolecularWeight[iNj] / dE;
                  pTa[iNj] *= dF * dD;
                  pTb[iNj] *= dG * dD;
               }

               pTa += iNc;
               pTb += iNc;
            }
         }
      }

      /* Set the composition */
      if ( m_iMolarFlash )
      {
         for ( iNi = 0; iNi < iNcv; iNi++ )
         {
            m_pX[iNi] *= dH;
            m_pY[iNi] *= dO;
         }
      }
      else
      {
         for ( iNi = 0; iNi < iNcv; iNi++ )
         {
            dE = m_pMolecularWeight[iNi];
            m_pX[iNi] *= dE * dH;
            m_pY[iNi] *= dE * dO;
         }
      }

      /* Finally set the volume */
      *m_pZx *= dH;
      *m_pZy *= dO;
   }
}


/* 
// WaterProperties
//
// Routine to store water properties 
//
// iUseCurrent
//    Indicator for type of slicing.  If iUseCurrent is
//    true, then use a trivial slice
// iStatus
//    Indicator for whether we need to get output control terms
//
// 1) Very straightforward to calculate the water properties;
//    start each slice by reading properties from Application
// 2) Then compute the water composition information
// 3) Then the water properties
// 7) Then convert the density and viscosity to molar values, if
//    required
// 8) Then convert the density to a total volume, if required
// 9) Finally store the results in the application class
*/
void EosPvtModel::WaterProperties( int iUseCurrent, int iStatus )
{
   int     iMolarDensity;
   int     iGetViscosity;
   int     iGetTension;
   int     iGetMW;
   int     iM;
   int     iSalts;
   int     iTherm;
   int     iNc;
   double *pDummy;

   /* Set null pointer */
   pDummy = (double *)0;

   /* See which simulator data needed */
   /* TODO: The next line is too long. Please rewrite to make it shorter. */
   ( m_pApplication )->WriteOutputData( &m_iDrv, &iGetViscosity, &iGetTension, &m_iHeat, &iGetMW, &m_iVolume, &m_iMolarFlash, &iMolarDensity );
   iMolarDensity = iMolarDensity && m_iMolarFlash;

   /* Get number of components */
   iSalts = ( m_pEosPvtTable )->GetNumberSalts();
   m_iWaterComp = ( m_pApplication )->WriteWaterIndex();
   iNc = iSalts + ( ( m_iWaterComp >= 0 ) ? 1 : 0 );

   /* Start anew if not using current */
   if ( iUseCurrent )
   {
      iM = 1;
   }
   else
   {
      ( m_pApplication )->ResetSlice();
      iM = ( m_pApplication )->SetSlice( EOS_FL_AP, m_iFlashLength );
   }

   /* Loop over all slices */
   while ( iM )
   {
      /* Read the water data if not using old solution */
      if ( iStatus )
      {
         ReadData( iM, iM, iNc, EOS_OPTION_ON, EOS_NOGETK, &iTherm );

         /* Adjust pointers */
         AdjustPointers( iM, iNc, EOS_OPTION_ON );

         /* Set the salinity */
         WaterPhase( iM, iNc );

         /* Calculate the water properties */
         /* TODO: The next line is too long. Please rewrite to make it shorter. */
         ( m_pEosPvtTable )->Water( iM, iNc, m_iDrv, m_iVolume, iGetViscosity, iGetTension, m_iHeat, iTherm, m_pPressure, m_pTemperature, m_pSalinity, m_pDSda, m_pRhoW, m_pDRhoWdp, m_pDRhoWdt, m_pDRhoWds, m_pDRhoWda, m_pMuW, m_pDMuWdp, m_pDMuWdt, m_pDMuWds, m_pDMuWda, m_pIftW, m_pDIftWdp, m_pDIftWdt, m_pDIftWda, m_pHeatW, m_pDHeatWdp, m_pDHeatWdt, m_pDHeatWds, m_pDHeatWda, m_pWorkW );

         /* Adjust water density and heat to molar values */
         if ( iMolarDensity )
         {
            if ( m_iVolume != EOS_SEPARATOR )
            {
               WaterMolarTerms( iM, iNc, m_iVolume, m_pRhoW, m_pDRhoWdp, m_pDRhoWdt, m_pDRhoWda );
            }

            if ( m_iHeat )
            {
               WaterMolarTerms( iM, iNc, EOS_VOLUME, m_pHeatW, m_pDHeatWdp, m_pDHeatWdt, m_pDHeatWda );
            }
         }

         /* Change to partial volumes */
         if ( m_iVolume == EOS_STOCK_TANK || m_iVolume == EOS_VOLUME )
         {
            WaterVolume( iM, iNc );
         }
      }

      /* Store the data */
      /* TODO: The next line is too long. Please rewrite to make it shorter. */
      ( m_pApplication )->ReadData( EOS_WATER, iM, iNc, m_pPhaseId, m_pW, m_pDWdp, m_pMW, m_pDMWdp, m_pRhoW, m_pDRhoWdp, m_pMuW, m_pDMuWdp, m_pIftW, m_pDIftWdp, m_pHeatW, m_pDHeatWdp, pDummy, pDummy, m_pMWW );

      /* Next slice */
      if ( iUseCurrent )
      {
         iM = 0;
      }
      else
      {
         iM = ( m_pApplication )->SetSlice( EOS_FL_AP, m_iFlashLength );
      }
   }
}


/* 
// WaterPhase
//
// Routine to set water phase compositions 
//
// iM
//    Number of objects
// iNc
//    Number of components
//
// 1) The first business of this routine is to set the 
//    molecular weights.
// 2) Then the total moles are set
// 3) Then the compositions are set.
// 4) Then the salinity is set.  The salinity is equal to 
//    the molecular weight divided by the weight of pure water
// 5) Derivative must be taken.  The derivatives of salinity
//    are taken with respect to the primary unknowns for 
//    later use in the property routines
// 6) If the total moles/mass of each component are to be
//    returned, need to form product of composition and
//    total moles/mass.
// 7) In the special case where only a salinity is specified,
//    need to set the salinity anyway, as this is used
//    in the property calculations
*/
void EosPvtModel::WaterPhase( int iM, int iNc )
{
   double *pTa;
   double *pTb;
   double  dA;
   double  dB;
   double  dC;
   int     iNi;
   int     iNj;
   int     i;

   /* Set the mole fractions */
   if ( iM > 1 )
   {
      /* If salt components present */
      if ( iNc > 1 )
      {
         /* Compute the molecular weights for mole fractions */
         if ( m_iMolarFlash )
         {
            pTa = m_pW;
            pTb = m_pComposition;
            dA = m_pSaltMW[0];
            for ( i = 0; i < iM; i++ )
            {
               *pTa = *pTb++;
               m_pMWW[i] = dA **pTa++;
            }

            for ( iNi = 1; iNi < iNc; iNi++ )
            {
               dA = m_pSaltMW[iNi];
               for ( i = 0; i < iM; i++ )
               {
                  *pTa = *pTb++;
                  m_pMWW[i] += dA **pTa++;
               }
            }

            for ( i = 0; i < iM; i++ )
            {
               m_pSalinity[i] = m_pMWW[i] / 18.01534;
               m_pMW[i] = m_pMoles[i];
            }

            /* Molecular weight derivatives */
            if ( m_iDrv )
            {
               pTa = m_pDSda;
               dC = m_pSaltMW[0];
               for ( i = 0; i < iM; i++ )
               {
                  dA = 18.01534 * m_pMoles[i];
                  dB = ( dA == 0.0 ) ? 0.0 : 1.0 / dA;
                  *pTa++ = dB * ( dC - m_pMWW[i] );
                  m_pWorkW[i] = dB;
               }

               for ( iNi = 1; iNi < iNc; iNi++ )
               {
                  dC = m_pSaltMW[iNi];
                  for ( i = 0; i < iM; i++ )
                  {
                     *pTa++ = m_pWorkW[i] * ( dC - m_pMWW[i] );
                  }
               }
            }
         }

         /* Compute molecular weight for mass fractions */
         else
         {
            pTa = m_pW;
            pTb = m_pComposition;
            dA = m_pSaltMW[0];
            for ( i = 0; i < iM; i++ )
            {
               *pTa = dA **pTb++;
               m_pMWW[i] = *pTa++;
            }

            for ( iNi = 1; iNi < iNc; iNi++ )
            {
               dA = m_pSaltMW[iNi];
               for ( i = 0; i < iM; i++ )
               {
                  *pTa = dA **pTb++;
                  m_pMWW[i] += *pTa++;
               }
            }

            for ( i = 0; i < iM; i++ )
            {
               m_pSalinity[i] = m_pMWW[i] / 18.01534;
               m_pMW[i] = m_pMWW[i] * m_pMoles[i];
            }

            /* Normalize */
            pTa = m_pW;
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  *pTa = *pTa / m_pMWW[i];
                  pTa++;
               }
            }

            /* Molecular weight derivatives */
            if ( m_iDrv )
            {
               pTa = m_pDSda;
               dC = m_pSaltMW[0];
               for ( i = 0; i < iM; i++ )
               {
                  dA = 18.01534 * m_pMoles[i];
                  dB = ( dA == 0.0 ) ? 0.0 : 1.0 / dA;
                  *pTa++ = dB * ( 1.0 - m_pMWW[i] / dC );
                  m_pWorkW[i] = dB;
               }

               for ( iNi = 1; iNi < iNc; iNi++ )
               {
                  dC = m_pSaltMW[iNi];
                  for ( i = 0; i < iM; i++ )
                  {
                     *pTa++ = m_pWorkW[i] * ( 1.0 - m_pMWW[i] / dC );
                  }
               }
            }
         }

         /* Derivatives for density */
         if ( m_iVolume == EOS_DENSITY )
         {
            if ( m_iDrv )
            {
               /* Total mass derivatives */
               pTa = m_pDMWda;
               for ( iNi = 0; iNi < iNc; iNi++ )
               {
                  for ( i = 0; i < iM; i++ )
                  {
                     *pTa++ = 1.0;
                  }
               }

               /* Composition derivatives */
               for ( i = 0; i < iM; i++ )
               {
                  dA = m_pMW[i];
                  m_pWorkW[i] = ( dA == 0.0 ) ? 0.0 : 1.0 / dA;
               }

               pTa = m_pDWda;
               pTb = m_pW;
               for ( iNj = 0; iNj < iNc; iNj++ )
               {
                  for ( i = 0; i < iM; i++ )
                  {
                     *pTa++ = -*pTb++ *m_pWorkW[i];
                  }
               }

               for ( iNi = 1; iNi < iNc; iNi++ )
               {
                  pTb = m_pDWda;
                  for ( iNj = 0; iNj < iNc; iNj++ )
                  {
                     for ( i = 0; i < iM; i++ )
                     {
                        *pTa++ = *pTb++;
                     }
                  }
               }

               pTa = m_pDWda;
               iNj = iNc * iM;
               for ( iNi = 0; iNi < iNc; iNi++ )
               {
                  for ( i = 0; i < iM; i++ )
                  {
                     *pTa += m_pWorkW[i];
                     pTa++;
                  }

                  pTa += iNj;
               }
            }
         }

         /* Volume */
         else
         {
            /* Total mass modified */
            pTa = m_pW;
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  *pTa *= m_pMW[i];
                  pTa++;
               }
            }

            /* Composition derivatives */
            if ( m_iDrv )
            {
               pTa = m_pDMWda;
               for ( iNi = 0; iNi < iNc; iNi++ )
               {
                  for ( i = 0; i < iM; i++ )
                  {
                     *pTa++ = 1.0;
                  }
               }

               pTa = m_pDWda;
               for ( iNi = 0; iNi < iNc; iNi++ )
               {
                  for ( iNj = 0; iNj < iNc; iNj++ )
                  {
                     dA = ( iNi == iNj ) ? 1.0 : 0.0;
                     for ( i = 0; i < iM; i++ )
                     {
                        *pTa++ = dA;
                     }
                  }
               }
            }
         }
      }

      /* Single component first set salinity */
      else
      {
         dA = m_pSaltMW[0];
         dB = dA / 18.01534;
         for ( i = 0; i < iM; i++ )
         {
            m_pMWW[i] = dA;
            m_pSalinity[i] = dB;
            m_pMW[i] = m_pMoles[i];
            m_pW[i] = 1.0;
         }

         if ( !m_iMolarFlash )
         {
            for ( i = 0; i < iM; i++ )
            {
               m_pMW[i] *= m_pMWW[i];
            }
         }

         if ( m_iVolume != EOS_DENSITY )
         {
            for ( i = 0; i < iM; i++ )
            {
               m_pW[i] *= m_pMW[i];
            }
         }

         /* Composition derivatives */
         if ( m_iDrv )
         {
            for ( i = 0; i < iM; i++ )
            {
               m_pDSda[i] = 0.0;
               m_pDMWda[i] = 1.0;
               m_pDWda[i] = 0.0;
            }
         }
      }

      /* Pressure derivatives */
      if ( m_iDrv >= EOS_DRV_P )
      {
         /* Composition derivatives */
         for ( i = 0; i < iM; i++ )
         {
            m_pDMWdp[i] = 0.0;
         }

         pTa = m_pDWdp;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               *pTa++ = 0.0;
            }
         }

         /* Now for temperature derivatives */
         if ( m_iDrv >= EOS_DRV_T )
         {
            /* Composition derivatives */
            for ( i = 0; i < iM; i++ )
            {
               m_pDMWdt[i] = 0.0;
            }

            pTa = m_pDWdt;
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  *pTa++ = 0.0;
               }
            }
         }
      }
   }

   /* One grid block */
   else
   {
      /* If more than one component */
      if ( iNc > 1 )
      {
         /* Mole fraction molecular weights */
         if ( m_iMolarFlash )
         {
            m_pW[0] = m_pComposition[0];
            dA = m_pSaltMW[0] * m_pW[0];
            for ( iNi = 1; iNi < iNc; iNi++ )
            {
               m_pW[iNi] = m_pComposition[iNi];
               dA += m_pSaltMW[iNi] * m_pW[iNi];
            }

            *m_pMWW = dA;
            *m_pSalinity = dA / 18.01534;
            *m_pMW = *m_pMoles;

            /* Molecular weight derivatives */
            if ( m_iDrv )
            {
               dC = *m_pMWW;
               dA = 18.01534 **m_pMoles;
               dB = ( dA == 0.0 ) ? 0.0 : 1.0 / dA;
               for ( iNi = 0; iNi < iNc; iNi++ )
               {
                  m_pDSda[iNi] = dB * ( m_pSaltMW[iNi] - dC );
               }
            }
         }

         /* Compute the molecular weights for mass fractions */
         else
         {
            m_pW[0] = m_pComposition[0] * m_pSaltMW[0];
            dA = m_pW[0];
            for ( iNi = 1; iNi < iNc; iNi++ )
            {
               m_pW[iNi] = m_pComposition[iNi] * m_pSaltMW[iNi];
               dA += m_pW[iNi];
            }

            *m_pMWW = dA;
            *m_pSalinity = dA / 18.01534;
            *m_pMW = dA **m_pMoles;

            /* Normalize */
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               m_pW[iNi] = m_pW[iNi] / dA;
            }

            /* Molecular weight derivatives */
            if ( m_iDrv )
            {
               dC = *m_pMWW;
               dA = 18.01534 **m_pMoles;
               dB = ( dA == 0.0 ) ? 0.0 : 1.0 / dA;
               for ( iNi = 0; iNi < iNc; iNi++ )
               {
                  m_pDSda[iNi] = dB * ( 1.0 - dC / m_pSaltMW[iNi] );
               }
            }
         }

         /* Derivatives for density */
         if ( m_iVolume == EOS_DENSITY )
         {
            if ( m_iDrv )
            {
               /* Total mass derivatives */
               for ( iNi = 0; iNi < iNc; iNi++ )
               {
                  m_pDMWda[iNi] = 1.0;
               }

               /* Composition derivatives */
               dA = *m_pMW;
               dB = ( dA == 0.0 ) ? 0.0 : 1.0 / dA;
               pTa = m_pDWda;
               for ( iNj = 0; iNj < iNc; iNj++ )
               {
                  *pTa++ = -dB * m_pW[iNj];
               }

               for ( iNi = 1; iNi < iNc; iNi++ )
               {
                  for ( iNj = 0; iNj < iNc; iNj++ )
                  {
                     *pTa++ = m_pDWda[iNj];
                  }
               }

               pTa = m_pDWda;
               for ( iNi = 0; iNi < iNc; iNi++ )
               {
                  *pTa += dB;
                  pTa += iNc + 1;
               }
            }
         }

         /* Convert to volume */
         else
         {
            /* Total mass modified */
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               m_pW[iNi] *= *m_pMW;
            }

            /* Composition derivatives */
            if ( m_iDrv )
            {
               for ( iNi = 0; iNi < iNc; iNi++ )
               {
                  m_pDMWda[iNi] = 1.0;
               }

               pTa = m_pDWda;
               for ( iNi = 0; iNi < iNc; iNi++ )
               {
                  for ( iNj = 0; iNj < iNc; iNj++ )
                  {
                     *pTa++ = ( iNi == iNj ) ? 1.0 : 0.0;
                  }
               }
            }
         }
      }

      /* Single component */
      else
      {
         dA = m_pSaltMW[0];
         *m_pMWW = dA;
         *m_pSalinity = dA / 18.01534;
         *m_pMW = *m_pMoles;
         *m_pW = 1.0;

         if ( !m_iMolarFlash )
         {
            *m_pMW *= dA;
         }

         if ( m_iVolume != EOS_DENSITY )
         {
            *m_pW *= *m_pMW;
         }

         /* Composition derivatives */
         if ( m_iDrv )
         {
            *m_pDSda = 0.0;
            *m_pDMWda = 1.0;
            *m_pDWda = 0.0;
         }
      }

      /* Pressure derivatives */
      if ( m_iDrv >= EOS_DRV_P )
      {
         /* Composition derivatives */
         *m_pDMWdp = 0.0;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            m_pDWdp[iNi] = 0.0;
         }

         /* Now for temperature derivatives */
         if ( m_iDrv >= EOS_DRV_T )
         {
            /* Composition derivatives */
            *m_pDMWdt = 0.0;
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               m_pDWdt[iNi] = 0.0;
            }
         }
      }
   }
}


/* 
// WaterMolarTerms
//
// Adjust terms to molar values 
//
// iM
//    Number of objects
// iNc
//    Number of components in aqueous phase
// iType
//    Type of conversion, either for densities or volumes.
// pRo
//    Density, volume, or enthalpy
// pDRodp
//    Pressure derivative of density, volume, or enthalpy
// pDRodt
//    Temperature derivative of density, volume, or enthalpy
// pDRoda
//    Accumulation derivatives of density, volume, or enthalpy
//       The object index is first
//       The component index is last
// 
// 1) This routine converts mass values to molar values.
//    For density, for example, this is achieved by
//    dividing by the molecular weight the mass density
*/
void EosPvtModel::WaterMolarTerms( int iM, int iNc, int iType, double *pRo, double *pDRodp, double *pDRodt,
                                   double *pDRoda )
{
   double *pTa;
   double  dA;
   int     i;
   int     iNi;

   /* Multiple grid blocks */
   if ( iM > 1 )
   {
      /* If density */
      if ( iType == EOS_DENSITY )
      {
         for ( i = 0; i < iM; i++ )
         {
            pRo[i] = pRo[i] / m_pMWW[i];
         }

         /* Derivatives */
         if ( m_iDrv )
         {
            pTa = pDRoda;
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  *pTa++ = 0.0;
               }
            }

            /* Pressure and temperature derivatives */
            if ( m_iDrv >= EOS_DRV_P )
            {
               for ( i = 0; i < iM; i++ )
               {
                  pDRodp[i] = pDRodp[i] / m_pMWW[i];
               }

               if ( m_iDrv >= EOS_DRV_T )
               {
                  for ( i = 0; i < iM; i++ )
                  {
                     pDRodt[i] = pDRodt[i] / m_pMWW[i];
                  }
               }
            }
         }
      }

      /* If volume */
      else
      {
         /* Derivatives */
         if ( m_iDrv )
         {
            pTa = pDRoda;
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               for ( i = 0; i < iM; i++ )
               {
                  *pTa++ = 0.0;
               }
            }

            /* Pressure and temperature derivatives */
            if ( m_iDrv >= EOS_DRV_P )
            {
               for ( i = 0; i < iM; i++ )
               {
                  pDRodp[i] = pDRodp[i] * m_pMWW[i];
               }

               if ( m_iDrv >= EOS_DRV_T )
               {
                  for ( i = 0; i < iM; i++ )
                  {
                     pDRodt[i] = pDRodt[i] * m_pMWW[i];
                  }
               }
            }
         }

         /* Set function */
         for ( i = 0; i < iM; i++ )
         {
            pRo[i] *= m_pMWW[i];
         }
      }
   }

   /* Single grid block */
   else
   {
      dA = *m_pMWW;

      /* If density */
      if ( iType == EOS_DENSITY )
      {
         *pRo = *pRo / dA;

         /* Derivatives */
         if ( m_iDrv )
         {
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               pDRoda[iNi] = 0.0;
            }

            /* Pressure and temperature derivatives */
            if ( m_iDrv >= EOS_DRV_P )
            {
               *pDRodp = *pDRodp / dA;
               if ( m_iDrv >= EOS_DRV_T )
               {
                  *pDRodt = *pDRodt / dA;
               }
            }
         }
      }

      /* If volume */
      else
      {
         /* Derivatives */
         if ( m_iDrv )
         {
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               pDRoda[iNi] = 0.0;
            }

            /* Pressure and temperature derivatives */
            if ( m_iDrv >= EOS_DRV_P )
            {
               *pDRodp = *pDRodp * dA;
               if ( m_iDrv >= EOS_DRV_T )
               {
                  *pDRodt = *pDRodt * dA;
               }
            }
         }

         /* Set function */
         *pRo *= dA;
      }
   }
}


/* 
// WaterVolume
//
// Adjust to water volume 
//
// iM
//    Number of objects
// iNc
//    Number of components
//
// 1) To obtain the total volume of the water phase, multiply
//    the total mass of the phase by the specific mass volume
*/
void EosPvtModel::WaterVolume( int iM, int iNc )
{
   double *pTa;
   double *pTb;
   double  dA;
   double  dB;
   int     i;
   int     iNi;

   /* Multiple grid blocks */
   if ( iM > 1 )
   {
      /* Derivatives */
      if ( m_iDrv )
      {
         pTa = m_pDRhoWda;
         pTb = m_pDMWda;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               *pTa = *pTa * m_pMW[i] +*pTb++ *m_pRhoW[i];
               pTa++;
            }
         }

         /* Pressure and temperature derivatives */
         if ( m_iDrv >= EOS_DRV_P )
         {
            for ( i = 0; i < iM; i++ )
            {
               m_pDRhoWdp[i] *= m_pMW[i];
            }

            if ( m_iDrv >= EOS_DRV_T )
            {
               for ( i = 0; i < iM; i++ )
               {
                  m_pDRhoWdt[i] *= m_pMW[i];
               }
            }
         }
      }

      /* Set function */
      for ( i = 0; i < iM; i++ )
      {
         m_pRhoW[i] *= m_pMW[i];
      }
   }

   /* Single grid block */
   else
   {
      /* Set function */
      dA = *m_pRhoW;
      dB = *m_pMW;
      *m_pRhoW = dA * dB;

      /* Derivatives */
      if ( m_iDrv )
      {
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            m_pDRhoWda[iNi] = m_pDRhoWda[iNi] * dB + m_pDMWda[iNi] * dA;
         }

         /* Pressure and temperature derivatives */
         if ( m_iDrv >= EOS_DRV_P )
         {
            *m_pDRhoWdp *= dB;
            if ( m_iDrv >= EOS_DRV_T )
            {
               *m_pDRhoWdt *= dB;
            }
         }
      }
   }
}


/* 
// CompositionalGrading
//
// Driving routine to perform compositional grading 
//
// iM
//    Number of objects
// iType
//    Type of hydrocarbon system.  When iType is EOS_TOF_2P
//    two hydrocarbon phases can exist
//
// 1) Get information regarding type of output, constants, etc.
// 2) There is one entry in the table with a known pressure
//    and composition.  Get the bubble point pressure for
//    this entry.
// 3) Integrate "up" from the known point.  "Up" means
//    monotone.
// 4) Recompute data at starting point
// 5) Integrate in the other direction
*/
void EosPvtModel::CompositionalGrading( int iNc, int iType )
{
   int iMolar;
   int iTemp;
   int iMove = 0;

   /* Get output values */
   ( m_pApplication )->WriteOutputData( &iTemp, &iTemp, &iTemp, &m_iHeat, &iTemp, &iTemp, &iMolar, &iTemp );

   /* Set the gravity constant in internal units */
   ( m_pEosPvtTable )->GetGravity( &m_dGravity, &m_dGascon );

   /* Set pointer to the starting table entry */
   ( m_pApplication )->SetTableEntry( EOS_INIT_START );

   /* Read the compositional grading data */
   ReadGradingData( iNc, EOS_INIT_COMP );

   /* Adjust to the bubble point */
   GradingBubblePoint( iNc, iType, EOS_NOFORCEBP, EOS_RETROGRADE );

   /* Tell Application that this is a GOC if two phases present */
   if ( *m_pPhase == EOS_FL_2P_CV )
   {
      ( m_pApplication )->ReadGOC( m_dDepthx );
   }

   /* Write out results */
   WriteGradingData( iNc, iMolar );

   /* Go one direction */
   while ( ( m_pApplication )->SetTableEntry( EOS_INIT_UP ) )
   {
      /* Iterate using Newton's method */
      CompositionalGradingDo( iNc, iType, &iMove );

      /* Write out results */
      WriteGradingData( iNc, iMolar );
   }

   /* Set pointer again to the starting table entry */
   ( m_pApplication )->SetTableEntry( EOS_INIT_START );

   /* Read the compositional grading data */
   ReadGradingData( iNc, EOS_INIT_COMP );

   /* Adjust to the bubble point */
   GradingBubblePoint( iNc, iType, EOS_NOFORCEBP, EOS_RETROGRADE );

   /* Go other direction */
   while ( ( m_pApplication )->SetTableEntry( EOS_INIT_DOWN ) )
   {
      /* Iterate using Newton's method */
      CompositionalGradingDo( iNc, iType, &iMove );

      /* Write out results */
      WriteGradingData( iNc, iMolar );
   }
}


/* 
// ReadGradingData
//
// Load object properties into temporary vectors for composition
// grading calculations
//
// iNc
//    Number of components
// iDirection
//    Direction to go from the current pointer
//       EOS_INIT_COMP - get composition from simulator
//       EOS_INIT_NONE - do not get composition
//
// 1) We need to load the properties such as temperature
//    and depth for each point.  We may optionally
//    need to load the composition if at the first
//    point, signalled by EOS_INIT_COMP
// 2) If at the starting point, normalize the compositions
*/
void EosPvtModel::ReadGradingData( int iNc, int iDirection )
{
   int    isothermal;
   int    iMolar;
   int    iNi;
   double dA;
   double dB;

   /* Load the properties */
   /* TODO: The next line is too long. Please rewrite to make it shorter. */
   ( m_pApplication )->WriteGradingData( iDirection, &m_dDepthx, m_pPressure, m_pTemperature, m_pComposition, &isothermal, &iMolar );

   /* Set the pvt term assignments */
   /* TODO: The next line is too long. Please rewrite to make it shorter. */
   ( m_pEosPvtTable )->WritePvtInfo( 1, 0, 1, isothermal, EOS_OPTION_OFF, 0, m_pAbcOffset, &m_iMultipleAbc, m_pTemperature, m_pMolecularWeight );

   /* Calculate mole fraction and moles for first block */
   if ( iDirection == EOS_INIT_COMP )
   {
      if ( iMolar )
      {
         m_pMoles[0] = m_pComposition[0];
         for ( iNi = 1; iNi < iNc; iNi++ )
         {
            m_pMoles[0] += m_pComposition[iNi];
         }
      }
      else
      {
         m_pComposition[0] = m_pComposition[0] / m_pMolecularWeight[0];
         m_pMoles[0] = m_pComposition[0];
         for ( iNi = 1; iNi < iNc; iNi++ )
         {
            m_pComposition[iNi] = m_pComposition[iNi] / m_pMolecularWeight[iNi];
            m_pMoles[0] += m_pComposition[iNi];
         }
      }

      /* Normalize and check pressure */
      dA = m_pMoles[0];
      dB = 1.0 / ( dA > 0.0 ? dA : 1.0 );
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         m_pComposition[iNi] *= dB;
      }
   }
}


/* 
// WriteGradingData
//
// Routine to write out to simulator compositions for compositional 
// grading 
//
// iNc
//    Number of components
// iMolar
//    Indicator for molar values
//
// 1) Call CompositionGradingDensity to calculate the 
//    needed phase properties
// 2) Need to store the values into more permenant arrays.
// 3) If not using molar values, compute to mass values
// 4) Call ReadGradingData to write out the results to
//    the outside world
*/
void EosPvtModel::WriteGradingData( int iNc, int iMolar )
{
   double dA;
   double dB;
   double dC;
   double dD;
   double dDensityx;
   double dDensityy;
   int    iNi;

   /* Compute the densities at bubble point */
   dA = *m_pPressure;
   *m_pPressure = m_dPressurex > m_dPressurey ? m_dPressurey : m_dPressurex;
   CompositionalGradingDensity( iNc, iMolar, m_pPressure, m_pX, m_pZx, m_pHx, m_pMWx );
   CompositionalGradingDensity( iNc, iMolar, m_pPressure, m_pY, m_pZy, m_pHy, m_pMWy );
   *m_pPressure = dA;

   /* Densities */
   if ( iMolar )
   {
      dDensityx = *m_pMWx **m_pZx;
      dDensityy = *m_pMWy **m_pZy;
      dB = 1.0;
      dC = 1.0;
   }
   else
   {
      dDensityx = *m_pZx;
      dDensityy = *m_pZy;
      dB = *m_pMWx;
      dC = *m_pMWy;
   }

   /* Compute the density at current conditions */
   if ( m_dPressurex > m_dPressurey )
   {
      dD = *m_pZx;
      *m_pPressure = m_dPressurex;
      CompositionalGradingDensity( iNc, iMolar, m_pPressure, m_pX, m_pZx, m_pHx, m_pMWx );
      *m_pZx = dD;
   }
   else
   {
      dD = *m_pZy;
      *m_pPressure = m_dPressurey;
      CompositionalGradingDensity( iNc, iMolar, m_pPressure, m_pY, m_pZy, m_pHy, m_pMWy );
      *m_pZy = dD;
   }

   *m_pPressure = dA;

   /* For molar, just save the values */
   if ( iMolar )
   {
      dA = m_pComposition[0];
      for ( iNi = 1; iNi < iNc; iNi++ )
      {
         dA += m_pComposition[iNi];
      }

      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         m_pComposition[iNi] = m_pComposition[iNi] / dA;
         m_pLastx[iNi] = m_pX[iNi];
         m_pLasty[iNi] = m_pY[iNi];
      }
   }

   /* Need to scale by molecular weights */
   else
   {
      m_pComposition[0] *= m_pMolecularWeight[0];
      dA = m_pComposition[0];
      for ( iNi = 1; iNi < iNc; iNi++ )
      {
         m_pComposition[iNi] *= m_pMolecularWeight[iNi];
         dA += m_pComposition[iNi];
      }

      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         dD = m_pMolecularWeight[iNi];
         m_pComposition[iNi] = m_pComposition[iNi] / dA;
         m_pLastx[iNi] = dD * m_pX[iNi] / dB;
         m_pLasty[iNi] = dD * m_pY[iNi] / dC;
      }
   }

   /* Store data */
   if ( dDensityx > dDensityy )
   {
      /* TODO: The next line is too long. Please rewrite to make it shorter. */
      ( m_pApplication )->ReadGradingData( m_dDepthx, *m_pTemperature, *m_pPressure, m_dPressurex, m_dPressurey, *m_pZx, *m_pZy, *m_pHx, *m_pHy, m_pComposition, m_pLastx, m_pLasty );
   }
   else
   {
      /* TODO: The next line is too long. Please rewrite to make it shorter. */
      ( m_pApplication )->ReadGradingData( m_dDepthx, *m_pTemperature, *m_pPressure, m_dPressurey, m_dPressurex, *m_pZy, *m_pZx, *m_pHy, *m_pHx, m_pComposition, m_pLasty, m_pLastx );
   }
}


/* 
// CompositionalGradingDensity
// 
// Routine to compute needed properties for compositional grading 
//
// iNc
//    Number of components
// iMolar
//    Indicator for mass or molar values
// pP
//    Phase pressure
// pComp
//    Phase composition
// pZfactor
//    Eventually the density
// pHt
//    The enthalpy
// pMWt
//    The molecular weight
//
// 1) First calculate the z factor and the enthalpy, if required
// 2) Then convert to a volume
// 3) Then obtain the molecular weight
// 4) Then convert to a density.  
// 5) The density and possibly the enthalpy are then
//    prepared to be output
*/
void EosPvtModel::CompositionalGradingDensity( int iNc, int iMolar, double *pP, double *pComp, double *pZfactor,
                                               double *pHt, double *pMWt )
{
   double dA;
   int    iNi;

   /* Get the z factor */
   /* TODO: The next line is too long. Please rewrite to make it shorter. */
   ( m_pEosPvtTable )->SolveCubic( 1, EOS_NOFUGACITY, m_iHeat, EOS_NOPOTENTIAL, EOS_NODRV, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset, pP, m_pTemperature, pComp, pZfactor, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pPotentialx, m_pDXdp, m_pDXdt, m_pDXda, pHt, m_pDHxdp, m_pDHxdt, m_pDHxda, m_pATable, m_pPhaseId );

   /* Perform the volume translations */
   /* TODO: The next line is too long. Please rewrite to make it shorter. */
   ( m_pEosPvtTable )->Volume( 1, EOS_NODRV, m_iMultipleAbc, m_pAbcOffset, pP, m_pTemperature, pComp, pZfactor, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pWork );

   /* Set the molecular weight */
   dA = m_pMolecularWeight[0] * pComp[0];
   for ( iNi = 1; iNi < iNc; iNi++ )
   {
      dA += m_pMolecularWeight[iNi] * pComp[iNi];
   }

   *pMWt = dA;

   /* Change to density */
   if ( iMolar )
   {
      *pZfactor = 1.0 / *pZfactor;
   }
   else if ( m_iHeat )
   {
      *pHt = *pHt / dA;
      *pZfactor = dA / *pZfactor;
   }
   else
   {
      *pZfactor = dA / *pZfactor;
   }
}


/* 
// GradingBubblePoint
//
// Routine to get bubble point for compositional grading 
//
// iNc
//    Number of components
// iType
//    Indicator for type of hydrocarbon phases that are possible;
//    EOS_TOF_2P means that two hydrocarbon phases can occur
// iForceBP
//    Indicator as to whether to force a bubble point calculation
//       EOS_FORCEBP: A bubble point calculation will always occur
//       EOS_NOFORCEBP: A bubble point calculation will only
//                      occur if the mixture is not two phase
//                      already
// iRetrograde
//    Indicator whether to check for retrograde regions
// 
// 1) First we must normalize the composition
// 2) If we are not intending to force a bubble point calculation,
//    determine if the mixture is two phase
// 3) If the mixture is not two phase, or if the 
//    mixture is forced to have a bubble point, calculate it
// 4) If the mixture has a bubble point but the bubble
//    point pressure is greater than the current pressure, then
//    check for a retrograde region.
// 5) Save results for current "X" and "Y" compositions from
//    current "Z" composition depending upon whether there
//    is a bubble point or not.
// 6) Comment: the current phase is the phase with the
//    ** highest ** pressure.  The phase at the lower pressure
//    is at lower pressure for two reasons, the first being
//    gravitational, the second owing to capillary pressure.
*/
void EosPvtModel::GradingBubblePoint( int iNc, int iType, int iForceBP, int iRetrograde )
{
   double dA;
   double dB;
   double dC;
   int    iNi;

   /* Normalize the composition */
   dA = m_pComposition[0];
   for ( iNi = 1; iNi < iNc; iNi++ )
   {
      dA += m_pComposition[iNi];
   }

   for ( iNi = 0; iNi < iNc; iNi++ )
   {
      m_pComposition[iNi] = m_pComposition[iNi] / dA;
   }

   /* Set phase indicator */
   *m_pPhase = EOS_FL_1P_NCV;

   /* Two phase flash */
   if ( iType == EOS_TOF_2P && iForceBP == EOS_NOFORCEBP )
   {
      /* Stability analysis */
      if ( m_iMichelson )
      {
         Michelson( 1, iNc );
      }
      else
      {
         FastInitialization( 1, iNc );
      }

      /* Successive substitution */
      if ( m_iSubstitutions && *m_pPhase == EOS_FL_2P_NCV )
      {
         Substitution( 1, iNc );
      }

      /* Newton iterations */
      if ( *m_pPhase == EOS_FL_2P_NCV )
      {
         NewtonFlash( 1, iNc, EOS_NORESTORE, EOS_FL_2P_NCV );
      }

      /* Reset phase indicator */
      if ( *m_pPhase == EOS_FL_2P_NCV )
      {
         *m_pPhase = EOS_FL_2P_CV;
      }
      else if ( *m_pPhase == EOS_FL_1P_CV )
      {
         *m_pPhase = EOS_FL_1P_NCV;
      }
   }

   /* See if a bubble point */
   if ( iType == EOS_TOF_2P && *m_pPhase == EOS_FL_1P_NCV )
   {
      BubblePointInit( 1, iNc, EOS_NORETROGRADE );
   }

   /* Compute the bubble point */
   if ( *m_pPhase == EOS_FL_BP_NCV )
   {
      BubblePointNewton( 1, iNc, EOS_FL_BP_CV );

      /* Reset phase indicator */
      if ( *m_pPhase == EOS_FL_BP_NCV )
      {
         *m_pPhase = EOS_FL_BP_CV;
      }
   }

   /* See if retrograde */
   if ( ( iRetrograde == EOS_RETROGRADE ) && ( *m_pPhase == EOS_FL_BP_CV ) )
   {
      *m_pPhase = *m_pBp > *m_pPressure ? EOS_FL_BP_NCV : EOS_FL_BP_CV;
   }

   /* See if a retrograde bubble point */
   if ( *m_pPhase == EOS_FL_BP_NCV )
   {
      *m_pPhase = EOS_FL_1P_NCV;
      BubblePointInit( 1, iNc, EOS_RETROGRADE );
   }

   /* Compute the bubble point */
   if ( *m_pPhase == EOS_FL_BP_NCV )
   {
      BubblePointNewton( 1, iNc, EOS_FL_BP_CV );

      /* Reset phase indicator */
      if ( *m_pPhase == EOS_FL_BP_NCV )
      {
         *m_pPhase = EOS_FL_BP_CV;
      }
   }

   /* Save results when things are two phase */
   if ( *m_pPhase == EOS_FL_2P_CV )
   {
      m_dBubblePoint = *m_pPressure;
      m_dPressurex = m_dBubblePoint;
      m_dPressurey = m_dBubblePoint;
      dB = *m_pSplit;
      dC = 1.0 - dB;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         dA = m_pKValue[iNi];
         m_pX[iNi] = m_pComposition[iNi] / ( dC + dB * dA );
         m_pY[iNi] = m_pX[iNi] * dA;
      }
   }

   /* Save results when there is a bubble point */
   else if ( *m_pPhase == EOS_FL_BP_CV )
   {
      m_dBubblePoint = *m_pSplit;
      m_dPressurex = *m_pPressure;
      m_dPressurey = m_dBubblePoint;

      /* Set output compositions */
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         m_pX[iNi] = m_pComposition[iNi];
         m_pY[iNi] = m_pComposition[iNi] * m_pKValue[iNi];
      }
   }

   /* Results without a bubble point */
   else
   {
      m_dBubblePoint = *m_pPressure;
      m_dPressurex = m_dBubblePoint;
      m_dPressurey = m_dBubblePoint;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         m_pX[iNi] = m_pComposition[iNi];
         m_pY[iNi] = m_pComposition[iNi];
      }
   }
}


/* 
// CompositionalGradingDo
//
// Routine to do one step of compositional grading 
//
// iNc
//    Number of components
// iType
//    Type of hydrocarbon phases which can be present.  If 
//    EOS_TOF_2P, two hydrocarbon phases may occur.
// iMove
//    Indicator whether we have moved things or not
//
// 1) Save the terms.  Use the "X" phase from the last
//    step as a starting point, and then take the compositional
//    grading step.
// 2) Check the bubble point.  If the system has went two
//    phase, then the reference phase has changed, as the
//    system went through a bubble point.
// 3) If the reference phase has changed, perform the calculations
//    anew using the "Y" phase from the last step as starting
//    point.  If the GOC is "close" to one of the current
//    depths nothing is changed; otherwise the depths
//    are shifted until the GOC is reached
// 4) Since internally the calculations always assume that the
//    "X" phase is the reference phase, the "X" and "Y"
//    phases must be switched when the reference phase has
//    changed
*/
void EosPvtModel::CompositionalGradingDo( int iNc, int iType, int *iMove )
{
   double dA;
   double dBubblePointSave;
   int    iNi;
   int    iRepeat;
   int    iError;

   /* Initialize repeat indicator */
   iRepeat = 0;

   /* Save the terms */
   m_dDepthy = m_dDepthx;
   m_dTemperature = *m_pTemperature;

   /* Normal case */
   if ( m_dPressurex >= m_dPressurey )
   {
      m_dPressure = m_dPressurex;
      dBubblePointSave = m_dPressurey;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         dA = m_pX[iNi];
         m_pComposition[iNi] = dA;
         m_pLastx[iNi] = dA;
         m_pLasty[iNi] = m_pY[iNi];
      }
   }

   /* Reversed case */
   else
   {
      m_dPressure = m_dPressurey;
      dBubblePointSave = m_dPressurex;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         dA = m_pY[iNi];
         m_pComposition[iNi] = dA;
         m_pLastx[iNi] = dA;
         m_pLasty[iNi] = m_pX[iNi];
      }
   }

   /* Read the compositional grading data */
   ReadGradingData( iNc, EOS_INIT_NONE );

   /* Perform the compositional grading */
   iError = CompositionalGradingNewton( iNc );

   /* Compute whether two phase */
   if ( !iError )
   {
      GradingBubblePoint( iNc, iType, EOS_NOFORCEBP, EOS_RETROGRADE );

      /* See if to repeat */
      iRepeat = ( *m_pPhase == EOS_FL_2P_CV ? 1 : 0 );
   }

   /* Depth okay */
   if ( iError || ( !iRepeat ) || *iMove )
   {
      *iMove = 0;
   }

   /* See if to move depth to GOC */
   else
   {
      double dTemperatureSave = *m_pTemperature;
      double dDepthSave = m_dDepthx;
      double dBottom = 0.0;
      double dTop = 1.0;
      double dFactor;

      /* Relative error of 0.01 */
      while ( dTop - dBottom > 0.01 )
      {
         dFactor = 0.5 * ( dTop + dBottom );

         /* Restore data */
         m_dDepthx = m_dDepthy + dFactor * ( dDepthSave - m_dDepthy );
         *m_pTemperature = m_dTemperature + dFactor * ( dTemperatureSave - m_dTemperature );
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            m_pComposition[iNi] = m_pLastx[iNi];
         }

         /* Perform the compositional grading */
         iError = CompositionalGradingNewton( iNc );

         /* Compute whether two phase */
         if ( !iError )
         {
            GradingBubblePoint( iNc, iType, EOS_NOFORCEBP, EOS_RETROGRADE );
         }

         /* Reset top or bottom */
         if ( iError || ( *m_pPhase == EOS_FL_2P_CV ) )
         {
            dTop = dFactor;
         }
         else
         {
            dBottom = dFactor;
         }
      }

      /* Just repeat it since close to where we are at now */
      if ( dBottom == 0.0 )
      {
         iRepeat = 1;
         m_dDepthx = dDepthSave;
         *m_pTemperature = dTemperatureSave;

         /* Tell Application that this is a GOC */
         ( m_pApplication )->ReadGOC( m_dDepthy );
      }

      /* Move contact */
      else
      {
         iRepeat = 0;
         *iMove = 1;

         /* Restore data to bottom */
         m_dDepthx = m_dDepthy + dBottom * ( dDepthSave - m_dDepthy );
         *m_pTemperature = m_dTemperature + dBottom * ( dTemperatureSave - m_dTemperature );
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            m_pComposition[iNi] = m_pLastx[iNi];
         }

         /* Perform the compositional grading; note bottom always valid */
         iError = CompositionalGradingNewton( iNc );

         /* Compute whether two phase */
         GradingBubblePoint( iNc, iType, EOS_NOFORCEBP, EOS_RETROGRADE );

         /* Tell Application that this is a GOC */
         ( m_pApplication )->ReadGOC( m_dDepthx );
      }
   }

   /* Start with other starting guess */
   if ( iRepeat )
   {
      /* Restore old values */
      m_dPressure = dBubblePointSave;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         m_pComposition[iNi] = m_pLasty[iNi];
      }

      /* Perform the compositional grading */
      iError = CompositionalGradingNewton( iNc );

      /* Compute the bubble point anyway */
      GradingBubblePoint( iNc, iType, EOS_FORCEBP, EOS_RETROGRADE );

      /* Reverse x and y phases */
      dA = m_dPressurex;
      m_dPressurex = m_dPressurey;
      m_dPressurey = dA;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         dA = m_pX[iNi];
         m_pX[iNi] = m_pY[iNi];
         m_pY[iNi] = dA;
      }
   }
}


/* 
// CompositionalGradingNewton
//
// Routine to take Newton iterations for compositional grading 
//
// iNc
//    Number of components
//
// 1) Load properties from the previous step.
// 2) Determine whether enthalpy calculations are
//    required.  The enthalpy is required if thermal
//    diffusion is used AND the temperature is not 
//    constant between steps.
// 3) Calculate the chemical potential at the initial guess
// 4) Calculate the thermal diffusion term if required
// 5) Restore current properties
// 6) Calculate the constant terms used in the iterations
//    At this point we will describe the system to be solved.
//    The equation to be solved is
//
//    dMu  = - M g dH - K  dlnT
//       i      i        i
//
//    where Mu is the chemical potential, M is the molecular
//    weight, and K is the thermal diffusion term.  The
//    thermal diffusion term is based upon the work of 
//    Kempers (L.J.T.M Kempers, "A thermodynamic theory of the Soret
//    effect in a multicomponent liquid", J. Chem. Phys, 90, 
//    June 1989.), while the basic form of the equation is
//    due to Schulte (A.M. Schulte, "Compositional Variations
//    within a Hydrocarbon Column Due to Gravity", SPE 9235,
//    1980)
// 7) This differential equation can be integrated to
//    yield
//
//      +     -          +    -              +    -
//    Mu  = Mu  - M g ( H  - H  ) - K  ln ( T  / T  )
//      i     i    i                 i
//
//    The thermal diffusion term will always be lagged
//    at the last level.  Also, the thermal diffusion
//    can be tuned by the same constant for all components
// 8) The term dMu depends upon pressure, composition,
//    and temperature.  The temperature field is known,
//    and thus the temperature dependence in the differential
//    of the chemical potential must be estimated.  It
//    is estimated in a second order correct way, namely,
//
//                    +   +      -   -           +    -
//    dMu/dT = 1/2 ( T dMu/dT + T dMu/dT ) ln ( T  / T  )
//       i                i          i
//
//    Note that in the iteration scheme these terms
//    are lagged by one iteration at the new level.  Actually,
//    since the temperature derivative of the chemical potential
//    is the non-ideal partial entropy, we are doing a midpoint
//    rule for estimating S for S dT
// 9) Upon expansion we get
//
//          +         +        -     +                  
//    Sum Mu  dX  + Mu  dP = Mu  - Mu  - M g dH - AdlnT
//     j    ij  j     ip       i     i    i
//
// 10) Note, however, that this condition means that there
//     are more unknowns than equations.  The appropriate
//     additional constraint is that the sum of the mole
//     fractions at the next level must be unity.  This
//     is ensured since
//
//     Sum dMu  = V dP = - MW g dH
//      i     i         
//
//     and thus
//
//     dP = - rho g dH
//
//     meaning that the system is in mechanical equilibrium.
//     Also, the thermal diffusion terms sum to zero when
//     performing the sum over all components
// 11) Use as an initial guess at the new level the current
//     composition and the pressure
// 12) When forming the Jacobian matrix we choose the
//     square root of the mole fraction divided by a
//     scaling term involving units to ensure that 
//     the matrix will be the identity matrix for the 
//     ideal gas case
// 13) The equations can be written in the form
//
//     A P dX = G
//     1 0 dP   H
//
//     The pressure derivatives appearing in the matrix
//     P are obtained by multiplying the standard
//     pressure derivatives by the current pressure,
//     which produces the derivative wrt the log of the
//     pressure.  But instead of this, we will use as
//     primary unknowns PX (the partial pressure) and 
//     the pressure.  Thus the equations will be
//
//     A  P dPX = G
//     1 -1 dP    H
//
// 14) By choosing a scaling factor of S = sqrt ( PX / RT )
//     above equation is rewritten as
//
//             -1
//     SAS SP S  dPX = SG
//      S  -1    dP     H
//
//     In this case the matrix SAS will have the property
//     that it will be a diagonal matrix for an ideal gas
// 15) Let 
//
//          -1
//     y = S  dPX
//
//     Then solve
//
//     SAS z = SG and SAS w = SP
//
//     The equations are then of the form
//     
//     I   w   y  = z
//     S  -1  dP    H
//
// 16) Now solve for dP.  We have
//
//     dP = ( Sz - H ) / ( 1 + Sw )
//
//     The solution is not allowed to grow more than one log
//     cycle.  The equations are now of the form
//
//     I  w  y   = z
//     0  1  dP    dP
//
//     and thus
// 
//     y = z - w dP
//
// 17) dPX is not used; rather dlnPX is used.  Thus
//
//     dlnPX = S y / PX = y / S
//
//     New partial pressures can then be set.
// 18) The iterations are stopped when the pressure
//     has converged.  As noted above the system is then
//     in gravitational equilibrium owing to the definition
//     of the chemical potential
*/
int EosPvtModel::CompositionalGradingNewton( int iNc )
{
   int     iConvrg;
   int     iter;
   int     iNi;
   int     iNj;
   int     iDrvt;
   int     iGetH;
   int     iError = 0;
   double  dDensity;
   double  dHead;
   double  dScale;
   double  dVeryTiny;
   double  dA;
   double  dB;
   double  dC;
   double  dD;
   double  dE;
   double  dF;
   double  dT;
   double *pTa;

   /* Load data from previous step */
   dA = m_dTemperature;
   m_dTemperature = *m_pTemperature;
   *m_pTemperature = dA;
   dA = m_dPressure;
   m_dPressure = *m_pPressure;
   *m_pPressure = dA;

   /* Settings for flasher */
   if ( *m_pTemperature == m_dTemperature )
   {
      iDrvt = EOS_NODRV;
      iGetH = EOS_NOHEAT;
      dT = 0.0;
   }
   else
   {
      iDrvt = EOS_DRV_T;
      iGetH = ( m_dThermalDiffusion > 0.0 ) ? EOS_HEAT : EOS_NOHEAT;
      dT = log( m_dTemperature / *m_pTemperature );
   }

   /* Calculate the chemical potentials from the last step */
   EosPvtModel::ChemicalPotentials( iNc, iDrvt, iGetH, EOS_OPTION_ON );

   /* Compute thermal diffusion contribution */
   if ( iGetH )
   {
      /* TODO: The next line is too long. Please rewrite to make it shorter. */
      ( m_pEosPvtTable )->ThermalDiffusion( 1, dT, m_dThermalDiffusion, m_pZx, m_pDZxda, m_pHx, m_pDHxda, m_pPotentialy );
   }

   /* Restore data for current step */
   dA = m_dTemperature;
   m_dTemperature = *m_pTemperature;
   *m_pTemperature = dA;
   dA = m_dPressure;
   m_dPressure = *m_pPressure;
   *m_pPressure = dA;
   dA = 0.0;
   for ( iNi = 0; iNi < iNc; iNi++ )
   {
      dA += m_pComposition[iNi] * m_pMolecularWeight[iNi];
   }

   /* Save the terms for integration */
   dDensity = *m_pZx / dA;
   dHead = m_dGravity * ( m_dDepthy - m_dDepthx );

   /* Integration */
   if ( iDrvt == EOS_NODRV )
   {
      iDrvt = EOS_DRV_P;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         m_pPotentialy[iNi] = m_pPotentialx[iNi] + m_pMolecularWeight[iNi] * dHead;
      }
   }
   else if ( iGetH )
   {
      dT *= 0.5;
      dD = dT * m_dTemperature;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         m_pPotentialy[iNi] += m_pPotentialx[iNi] + dD * m_pDXdt[iNi] + m_pMolecularWeight[iNi] * dHead;
      }

      dT *= *m_pTemperature;
   }
   else
   {
      dT *= 0.5;
      dD = dT * m_dTemperature;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         m_pPotentialy[iNi] = m_pPotentialx[iNi] + dD * m_pDXdt[iNi] + m_pMolecularWeight[iNi] * dHead;
      }

      dT *= *m_pTemperature;
   }

   /* Generate an initial guess */
   *m_pPressure = m_dPressure * exp( dHead / dDensity );

   /* Set partial pressures */
   for ( iNi = 0; iNi < iNc; iNi++ )
   {
      m_pComposition[iNi] *= *m_pPressure;
   }

   /* Save log of the y pressure */
   dScale = m_dGascon **m_pTemperature;
   dVeryTiny = 1.0 / m_dEnorm;

   /* Newton's method */
   iConvrg = EOS_NOCONVERGE;
   for ( iter = 0; iter < m_iMaxIterations && iConvrg == EOS_NOCONVERGE; iter++ )
   {
      /* Chemical potential and PV factor for the x phase */
      ChemicalPotentials( iNc, iDrvt, EOS_NOHEAT, EOS_OPTION_OFF );

      /* Logarithmic scaling for pressure */
      dC = *m_pPressure;

      /* Right hand side for potential calculations */
      if ( iDrvt < EOS_DRV_T )
      {
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            dD = sqrt( m_pComposition[iNi] / dScale );
            m_pDZxda[iNi] = dD;
            m_pDXdp[iNi] *= dD * dC;
            m_pDXdt[iNi] = ( m_pPotentialy[iNi] - m_pPotentialx[iNi] ) * dD;
            m_pY[iNi] = dD;
         }
      }
      else
      {
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            dD = sqrt( m_pComposition[iNi] / dScale );
            m_pDZxda[iNi] = dD;
            m_pDXdp[iNi] *= dD * dC;
            m_pDXdt[iNi] = ( m_pPotentialy[iNi] - m_pPotentialx[iNi] + m_pDXdt[iNi] * dT ) * dD;
            m_pY[iNi] = dD;
         }
      }

      /* Form Jacobian Matrix */
      pTa = m_pDXda;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         pTa += iNi;
         dD = m_pY[iNi];
         *pTa = *pTa * dD * dD + 1.0;
         pTa++;
         for ( iNj = iNi + 1; iNj < iNc; iNj++ )
         {
            *pTa *= dD * m_pY[iNj];
            pTa++;
         }
      }

      /* Newton step */
      Cholesky( 1, iNc, m_pDXda );
      BackSolve( 1, iNc, m_pDXda, m_pDXdp );
      BackSolve( 1, iNc, m_pDXda, m_pDXdt );

      /* Update pressure */
      dA = m_pDZxda[0];
      dB = m_pComposition[0] - dC;
      dE = dA * m_pDXdp[0] + dC;
      dF = dA * m_pDXdt[0];
      for ( iNi = 1; iNi < iNc; iNi++ )
      {
         dA = m_pDZxda[iNi];
         dB += m_pComposition[iNi];
         dE += dA * m_pDXdp[iNi];
         dF += dA * m_pDXdt[iNi];
      }

      dF += dB;

      /* Diagonal for pressure step */
      if ( fabs( dE ) < fabs( dF ) )
      {
         iError = 1;
         dF = 0.0;
      }
      else
      {
         dF = dF / dE;
         *m_pPressure *= exp( dF );

         /* Compute Newton step */
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            dA = dScale * m_pY[iNi];
            dB = ( dA > dVeryTiny ? dA : dVeryTiny );
            dC = m_pDXdt[iNi] - dF * m_pDXdp[iNi];
            m_pComposition[iNi] *= exp( dC / dB );
         }
      }

      /* Update pressure and check for convergence */
      iConvrg = ( fabs( dF ) < m_dConverge ) ? EOS_CONVERGE : EOS_NOCONVERGE;
   }

   /* Return the error code */
   return ( iError );
}


/* 
// ChemicalPotential
//
// Routine to get the chemical potentials 
//
// iNc
//    Number of components
// iDrvt
//    Indicator for derivative level.  Either
//       EOS_NODRV - isothermal case
//       EOS_DRV_T - thermal case
// iGetH
//    Indicator as to whether we need enthalpy for thermal
//    diffusion
//
// 1) Call SolveCubic to get the chemical potential
// 2) Add the term RT log PXi
*/
void EosPvtModel::ChemicalPotentials( int iNc, int iDrvt, int iGetH, int iNorm )
{
   double dA;
   double dB;
   double dC;
   double dD;
   double dE;
   int    iNi;
   int    iNj;

   /* Normalize composition */
   dA = m_pComposition[0];
   for ( iNi = 1; iNi < iNc; iNi++ )
   {
      dA += m_pComposition[iNi];
   }

   for ( iNi = 0; iNi < iNc; iNi++ )
   {
      m_pY[iNi] = m_pComposition[iNi] / dA;
   }

   /* Chemical potential and PV factor for the x phase */
   /* TODO: The next line is too long. Please rewrite to make it shorter. */
   ( m_pEosPvtTable )->SolveCubic( 1, EOS_FUGACITY, iGetH, EOS_POTENTIAL, iDrvt, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset, m_pPressure, m_pTemperature, m_pY, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pPotentialx, m_pDXdp, m_pDXdt, m_pDXda, m_pHx, m_pDHxdp, m_pDHxdt, m_pDHxda, m_pATable, m_pPhaseId );

   /* Scale derivatives */
   if ( iDrvt && ( !iNorm ) )
   {
      iNj = iNc * iNc;
      for ( iNi = 0; iNi < iNj; iNi++ )
      {
         m_pDXda[iNi] = m_pDXda[iNi] / dA;
      }
   }

   /* Scale factor and other terms */
   dB = 1.0 / m_dEnorm;
   dC = m_dGascon **m_pTemperature;
   if ( iNorm )
   {
      dE = *m_pPressure;
   }
   else
   {
      dE = 1.0;
   }

   /* Add the composition term to the chemical potential */
   if ( iDrvt < EOS_DRV_T )
   {
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         dA = dE * m_pComposition[iNi];
         m_pPotentialx[iNi] += ( dC * log( dA < dB ? dB : dA ) );
      }
   }
   else
   {
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         dA = dE * m_pComposition[iNi];
         dD = log( dA < dB ? dB : dA );
         m_pPotentialx[iNi] += ( dC * dD );
         m_pDXdt[iNi] += ( m_dGascon * dD );
      }
   }
}


/* 
// SeparatorFlash
//
// Routine to perform a surface flash through a separator train
//
// iNc
//    Number of hydrocarbon components
// iType
//    Type of hydrocarbon phases that can be present.  If
//    EOS_TOF_2P, two phases can be present
// iWater
//    Indicator if a water phase is present
// iFlash
//    Indicator whether we are to flash
// pPvtTable
//    Array of pointers to pvt tables for each separator
//
// 1) The following type of system is modeled
//
//          [stock tank oil]
//                  ^
//                  |
//    More feed--> [ ] -----------------+
//                  ^                   |
//              oil |                   |
//                  |     gas           |
//    More feed--> [ ] -----------------+
//                  ^                   |
//              oil |                   |
//                  |     gas           |
//    Feed ------> [ ] ------------------->[stock tank gas]
//
// 1) For each separator...
// 2) First read in the previous values present in the oil
//    phase, and add in the new feed.  Of course, for the
//    first separator, there is only feed
// 3) Then flash the mixture, obtaining only mole fractions,
//    except at the last stage
// 4) Move on to the next separator
// 5) Now for a pass in the other direction.  We must gather
//    the gas, and form the volumetric terms.  The computations
//    are different for a system that can have two hydrocarbon
//    phases and a single phase hydrocarbon system
// 6) Finally, add in the water properties
*/
void EosPvtModel::SeparatorFlash( int iNc, int iType, int iWater, int iFlash, EosPvtTable **pPvtTable )
{
   int iSep;
   int iCount;
   int iThermal;

   /* See which simulator data needed */
   /* TODO: The next line is too long. Please rewrite to make it shorter. */
   ( m_pApplication )->WriteOutputData( &m_iDrv, &iCount, &iCount, &m_iHeat, &iCount, &m_iVolume, &m_iMolarFlash, &iCount );

   /* Reset the slice */
   iSep = ( m_pApplication )->SetSeparator( EOS_INIT_START );

   /* If anything to do */
   iCount = 0;
   while ( iSep >= 0 )
   {
      iCount++;

      /* Set the pvt table */
      m_pEosPvtTable = pPvtTable[iSep];

      /* Load previous values */
      ( m_pApplication )->ModifySeparatorAccumulations();

      /* If hydrocarbon present */
      if ( iType != EOS_TOF_0P )
      {
         /* Set the phase identification flag */
         ( m_pApplication )->ModifyOnePhaseIdentification( m_dEnorm );

         /* Read Data */
         ReadData( 1, 1, iNc, EOS_OPTION_OFF, EOS_NOGETK, &iThermal );

         /* Perform the flash calculations and store values */
         if ( iType == EOS_TOF_2P && iFlash )
         {
            FlashOneObject( iNc );
         }

         /* Need to reset phase identification flags */
         else
         {
            ( m_pApplication )->ModifyPhaseIndicator( EOS_OPTION_OFF );
            *m_pPhase = ( *m_pPhase + 1 ) / 2;
         }

         /* Calculate and store properties if requested */
         PropertiesOneObject( iNc, iType );
      }

      /* Reset the slice */
      iSep = ( m_pApplication )->SetSeparator( EOS_INIT_UP );
   }

   /* Load separator gas */
   if ( iCount > 1 )
   {
      if ( iType == EOS_TOF_2P )
      {
         SeparatorGas( iNc, pPvtTable );
      }

      /* Load hydrocarbons */
      else if ( iType != EOS_TOF_0P )
      {
         SeparatorHydrocarbon( iNc );
      }
   }

   /* Get water properties */
   if ( iWater )
   {
      SeparatorWater( pPvtTable );
   }

   /* Restore volume control */
   ( m_pApplication )->ReadVolumeControl( m_iVolume );
}


/* 
// SeparatorGas
//
// Load separator gas 
//
// iNc
//    Number of components
// pPvtTable
//    Array of pointers to pvt tables for each separator
// 
// 1) Ensure that the routine is going to calculate
//    both volumes and total moles/mass of each component
// 2) We get the oil density from the stack.
//    However, we might need to interchange phases
// 3) First make a pass through all separators to
//    obtain the gas which was produced.  
// 4) We already have calculated the oil, so load it
//    Note that the oil at the last stage is the volume
//    and partial volumes with respect to the feed, i.e.,
//
//    V      = Sum V        ( O      + F    )
//     oil,n    i   oil,i,n    i,n-1    i,n
//
//    where O represents the oil from the last stage and
//    F is the feed at the last stage
//
// 5) Form the partial volumes for the gas.  Thus, at the
//    last stage, then
//
//    V      = Sum V        ( O      + F    )
//     gas,n    i   gas,i,n    i,n-1    i,n
//
//    where Q is the total feed into all separators
//
// 6) Then go backwards through the other separators.
//    At each stage j we have
//
//    O    = M  ( F  + O    )
//     j+1    j    j    j-1
//
//    where M is simply the derivative matrix of the total mass or
//    moles of components of the oil phase with respect to 
//    the feed + previous oil.
//
// 7) We can telescope the sums so that at the first stage
//
//    V      = Sum ( Prod M  ) V         F 
//     oil,1    i      j   j    oil,i,n    i,1
//
//    Thus
//
//    V      = Sum V         F 
//     oil,1    i   oil,i,n    i,1
//    
// 8) The products for gas are similar
//
//    V      = Sum ( I - Prod M  ) V         F    
//     gas,1    i          j   j    gas,i,n   i,1
//
//    Thus
//
//    V      = Sum V         F 
//     gas,1    i   gas,i,n    i,1
//
// 9) At each stage of this process we must write the results
//    to Application
*/
void EosPvtModel::SeparatorGas( int iNc, EosPvtTable **pPvtTable )
{
   double  dA;
   double  dB;
   double *pTa;
   int     iNi;
   int     iNj;
   int     iSep;

   /* Reset the volume indicator */
   ( m_pApplication )->ReadVolumeControl( EOS_STOCK_TANK );

   /* Reset the slice */
   iSep = ( m_pApplication )->SetSeparator( EOS_INIT_START );

   /* Load previous values */
   ( m_pApplication )->WriteSeparatorInfo( m_pY, m_pZx, m_pDZxda );

   /* If anything to do */
   while ( ( m_pApplication )->SetSeparator( EOS_INIT_UP ) >= 0 )
   {
      /* Load previous values */
      ( m_pApplication )->WriteSeparatorInfo( m_pX, m_pZx, m_pDZxda );

      /* Add to total gas */
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         m_pY[iNi] += m_pX[iNi];
      }
   }

   /* Reset the slice to end */
   iSep = ( m_pApplication )->SetSeparator( EOS_INIT_END );

   /* Set the pvt table */
   m_pEosPvtTable = pPvtTable[iSep];

   /* Load separator oil derivatives */
   ( m_pApplication )->WriteSeparatorDrv( EOS_OIL, m_pDXda );

   /* Set the composition */
   for ( iNi = 0; iNi < iNc; iNi++ )
   {
      m_pComposition[iNi] = m_pY[iNi];
   }

   /* Normalize */
   if ( m_iMolarFlash )
   {
      dA = m_pComposition[0];
      for ( iNi = 1; iNi < iNc; iNi++ )
      {
         dA += m_pComposition[iNi];
      }
   }
   else
   {
      m_pComposition[0] = m_pComposition[0] / m_pMolecularWeight[0];
      dA = m_pComposition[0];
      for ( iNi = 1; iNi < iNc; iNi++ )
      {
         m_pComposition[iNi] = m_pComposition[iNi] / m_pMolecularWeight[iNi];
         dA += m_pComposition[iNi];
      }
   }

   /* Only if something present */
   if ( dA > 1.0 / m_dEnorm )
   {
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         m_pComposition[iNi] = m_pComposition[iNi] / dA;
      }

      /* Z factor */
      /* TODO: The next line is too long. Please rewrite to make it shorter. */
      ( m_pEosPvtTable )->SolveCubic( 1, EOS_NOFUGACITY, EOS_NOHEAT, EOS_NOPOTENTIAL, m_iDrv, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset, m_pPressure, m_pTemperature, m_pComposition, m_pZy, m_pDZydp, m_pDZydt, m_pDZyda, m_pWork, m_pDYdp, m_pDYdt, m_pDYda, m_pHy, m_pDHydp, m_pDHydt, m_pDHyda, m_pATable, m_pPhaseId );

      /* Perform the volume translations */
      /* TODO: The next line is too long. Please rewrite to make it shorter. */
      ( m_pEosPvtTable )->Volume( 1, m_iDrv, m_iMultipleAbc, m_pAbcOffset, m_pPressure, m_pTemperature, m_pComposition, m_pZy, m_pDZydp, m_pDZydt, m_pDZyda, m_pWork );

      /* Density */
      dB = *m_pZy;
      *m_pZy = dA * dB;

      /* Density derivatives */
      if ( m_iDrv )
      {
         if ( m_iMolarFlash )
         {
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               m_pDZyda[iNi] = dB + m_pDZyda[iNi];
            }
         }
         else
         {
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               m_pDZyda[iNi] = ( dB + m_pDZyda[iNi] ) / m_pMolecularWeight[iNi];
            }
         }

         /* Set the derivatives */
         pTa = m_pDXda;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            dB = *pTa++ *m_pDZyda[0];
            for ( iNj = 1; iNj < iNc; iNj++ )
            {
               dB += *pTa++ *m_pDZyda[iNj];
            }

            m_pDMyda[iNi] = dB;
         }

         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            m_pDMuyda[iNi] = m_pDZyda[iNi];
            m_pDZyda[iNi] = m_pDMyda[iNi];
            m_pDMyda[iNi] = m_pDMuyda[iNi] - m_pDMyda[iNi];
         }
      }
   }

   /* Else set to zero */
   else
   {
      *m_pZy = 0.0;
      *m_pMy = 0.0;
      if ( m_iDrv )
      {
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            m_pDZyda[iNi] = 0.0;
            m_pDMyda[iNi] = 0.0;
            m_pDMuyda[iNi] = 0.0;
         }
      }
   }

   /* Retrieve gas phase derivative */
   ( m_pApplication )->WriteSeparatorDrv( EOS_GAS, m_pDYda );

   /* Store the data */
   *m_pPhaseId = EOS_SINGLE_PHASE_GAS;

   /* TODO: The next line is too long. Please rewrite to make it shorter. */
   ( m_pApplication )->ReadData( EOS_SINGLE_PHASE_OIL, 1, iNc, m_pPhaseId, m_pY, m_pDYdp, m_pMy, m_pDMydp, m_pZy, m_pDMydp, m_pMuy, m_pDMuydp, m_pIfy, m_pDIfydp, m_pHy, m_pDHydp, m_pBp, m_pDBpdp, m_pMWy );

   /* Reset the volume indicator */
   ( m_pApplication )->ReadVolumeControl( EOS_VOLUME );

   /* While anything more to do */
   while ( ( m_pApplication )->SetSeparator( EOS_INIT_DOWN ) >= 0 )
   {
      /* Load next compositions */
      ( m_pApplication )->WriteSeparatorDrv( EOS_OIL, m_pDXda );

      /* Set the derivatives */
      if ( m_iDrv )
      {
         pTa = m_pDXda;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            dA = *pTa * m_pDZxda[0];
            dB = *pTa++ *m_pDZyda[0];
            for ( iNj = 1; iNj < iNc; iNj++ )
            {
               dA += *pTa * m_pDZxda[iNj];
               dB += *pTa++ *m_pDZyda[iNj];
            }

            m_pDMxda[iNi] = dA;
            m_pDMyda[iNi] = dB;
         }

         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            m_pDZxda[iNi] = m_pDMxda[iNi];
            m_pDZyda[iNi] = m_pDMyda[iNi];
            m_pDMyda[iNi] = m_pDMuyda[iNi] - m_pDMyda[iNi];
         }
      }

      /* Write it out */
      /* TODO: The next line is too long. Please rewrite to make it shorter. */
      ( m_pApplication )->ReadData( EOS_SINGLE_PHASE_OIL, 1, iNc, m_pPhaseId, m_pY, m_pDYdp, m_pMy, m_pDMydp, m_pZy, m_pDMydp, m_pMuy, m_pDMuydp, m_pIfy, m_pDIfydp, m_pHy, m_pDHydp, m_pBp, m_pDBpdp, m_pMWy );

      /* TODO: The next line is too long. Please rewrite to make it shorter. */
      ( m_pApplication )->ReadData( EOS_SINGLE_PHASE_GAS, 1, iNc, m_pPhaseId, m_pX, m_pDXdp, m_pMx, m_pDMxdp, m_pZx, m_pDMxdp, m_pMux, m_pDMuxdp, m_pIfx, m_pDIfxdp, m_pHx, m_pDHxdp, m_pBp, m_pDBpdp, m_pMWx );
   }
}


/* 
// SeparatorHydrocarbon
//
// Volume calculations for a single phase hydrocarbon 
//
// iNc
//    Number of hydrocarbons
//
// 1) The flash results from the last separator are already
//    known.  Copy these to other separators in the train 
*/
void EosPvtModel::SeparatorHydrocarbon( int iNc )
{
   /* Move to the first separator */
   /*int iSep =*/
   ( m_pApplication )->SetSeparator( EOS_INIT_END );

   /* Set volume control to volume only */
   ( m_pApplication )->ReadVolumeControl( EOS_VOLUME );

   /* Load previous values */
   ( m_pApplication )->WriteSeparatorInfo( m_pX, m_pZx, m_pDZxda );

   /* Save the data for other separators */
   *m_pPhaseId = EOS_SINGLE_PHASE_OIL;
   while ( ( m_pApplication )->SetSeparator( EOS_INIT_DOWN ) >= 0 )
   {
      /* TODO: The next line is too long. Please rewrite to make it shorter. */
      ( m_pApplication )->ReadData( EOS_SINGLE_PHASE_OIL, 1, iNc, m_pPhaseId, m_pX, m_pDXdp, m_pMx, m_pDMxdp, m_pZx, m_pDZxdp, m_pMux, m_pDMuxdp, m_pIfx, m_pDIfxdp, m_pHx, m_pDHxdp, m_pBp, m_pDBpdp, m_pMWx );
   }
}


/* 
// SeparatorWater
//
// Separator water calculations
//
// pPvtTable
//    Array of pointers to pvt tables for each separator
//
// 1) For each separator calculate water properties
*/
void EosPvtModel::SeparatorWater( EosPvtTable **pPvtTable )
{
   int iSep;

   /* Set volume control to everything */
   ( m_pApplication )->ReadVolumeControl( EOS_STOCK_TANK );

   /* Move to last separator */
   iSep = ( m_pApplication )->SetSeparator( EOS_INIT_END );

   /* Set the pvt table */
   m_pEosPvtTable = pPvtTable[iSep];

   /* Initialize the properties */
   WaterProperties( EOS_OPTION_ON, EOS_OPTION_ON );

   /* Set volume control to volume only */
   ( m_pApplication )->ReadVolumeControl( EOS_VOLUME );

   /* Loop over other data */
   while ( ( m_pApplication )->SetSeparator( EOS_INIT_DOWN ) >= 0 )
   {
      /* Update water properties */
      WaterProperties( EOS_OPTION_ON, EOS_OPTION_OFF );
   }
}
