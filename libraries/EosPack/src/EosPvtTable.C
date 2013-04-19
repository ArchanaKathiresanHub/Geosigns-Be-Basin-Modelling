// Copyright 2011, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include "EosApplication.h"
#include "EosPvtDefinitions.h"
#include "EosPvtModel.h"
#include "EosPvtTable.h"
#include "EosUtils.h"

#include <math.h>
#include <stdio.h>

/*
// New Cubic Method
*/
#define EOS_NEW_CUBIC


/* Coefficients defined for the corresponding state viscosity method */
static const double EOS_DIVFAC = 1.0e-80;

static const double  EOS_VCS00 = 0.0820561600e+0;
static const double  EOS_VCS01 = -1.8439486666e-2;
static const double  EOS_VCS02 = 1.0510162064e+0;
static const double  EOS_VCS03 = -1.6057820303e+1;
static const double  EOS_VCS04 = 8.4844027562e+2;
static const double  EOS_VCS05 = -4.2738409106e+4;
static const double  EOS_VCS06 = 7.6565285254e-4;
static const double  EOS_VCS07 = -4.8360724197e-1;
static const double  EOS_VCS08 = 8.5195473835e+1;
static const double  EOS_VCS09 = -1.6607434721e+4;
static const double  EOS_VCS10 = -3.7521074532e-5;
static const double  EOS_VCS11 = 2.8616309259e-2;
static const double  EOS_VCS12 = -2.8685285973e+0;
static const double  EOS_VCS13 = 1.1906973942e-4;
static const double  EOS_VCS14 = -8.5315715699e-3;
static const double  EOS_VCS15 = 3.8365063841e+0;
static const double  EOS_VCS16 = 2.4986828379e-5;
static const double  EOS_VCS17 = 5.7974531455e-6;
static const double  EOS_VCS18 = -7.1648329297e-3;
static const double  EOS_VCS19 = 1.2577853784e-4;
static const double  EOS_VCS20 = 2.2240102466e+4;
static const double  EOS_VCS21 = -1.4800512328e+6;
static const double  EOS_VCS22 = 5.0498054887e+1;
static const double  EOS_VCS23 = 1.6428375992e+6;
static const double  EOS_VCS24 = 2.1325387196e-1;
static const double  EOS_VCS25 = 3.7791273422e+1;
static const double  EOS_VCS26 = -1.1857016815e-5;
static const double  EOS_VCS27 = -3.1630780767e+1;
static const double  EOS_VCS28 = -4.1006782941e-6;
static const double  EOS_VCS29 = 1.4870043284e-3;
static const double  EOS_VCS30 = 3.1512261532e-9;
static const double  EOS_VCS31 = -2.1670774745e-6;
static const double  EOS_VCS32 = 2.4000551079e-5;
static const double  EOS_VCS33 = 0.0096000000e+0;

static const double  EOS_VCSCV = 1.0000000000e-7;
static const double  EOS_VCSCP = 9.8692300000e-6;
static const double  EOS_VCSMW = 1.6043000000e+1;
static const double  EOS_VCSDC = 1.6248000000e+2;

static const double  EOS_VCSPT = 1.1743567500e+4;
static const double  EOS_VCSTT = 9.0680000002e+1;
static const double  EOS_VCSRT = 2.8147000000e+1;

static const double  EOS_VCSPC = 4.6001840000e+6;
static const double  EOS_VCSTC = 1.9060000000e+2;
static const double  EOS_VCSRC = 1.0127780000e+1;

static const double  EOS_VCSSA = 4.7774858000e+0;
static const double  EOS_VCSSB = 1.7606536300e+0;
static const double  EOS_VCSSC = -0.5678889400e+0;
static const double  EOS_VCSSD = 1.3278623100e+0;

static const double  EOS_VCSLA = -0.1788601650e+0;
static const double  EOS_VCSLB = 0.0483847500e+0;
static const double  EOS_VCSLC = -0.0184898700e+0;
static const double  EOS_VCSLD = 0.3600000000e+0;

static const double  EOS_VCSA1 = -2.7036003000e+0;
static const double  EOS_VCSA2 = 3.1661552000e+0;
static const double  EOS_VCSA3 = -8.6573409000e+0;
static const double  EOS_VCSA4 = 5.2640362000e+0;
static const double  EOS_VCSA5 = -3.5269034000e+0;
static const double  EOS_VCSA6 = 0.4600000000e+0;

static const double  EOS_VCSB1 = -2.0909750000e+5;
static const double  EOS_VCSB2 = 2.6472690000e+5;
static const double  EOS_VCSB3 = -1.4728180000e+5;
static const double  EOS_VCSB4 = 4.7167400000e+4;
static const double  EOS_VCSB5 = -9.4918720000e+3;
static const double  EOS_VCSB6 = 1.2199790000e+3;
static const double  EOS_VCSB7 = -9.6279930000e+1;
static const double  EOS_VCSB8 = 4.2741520000e+0;
static const double  EOS_VCSB9 = -8.1415310000e-2;

static const double  EOS_VCSC1 = 1.6969859270e-3;
static const double  EOS_VCSC2 = -0.1333723460e-3;
static const double  EOS_VCSC3 = 1.4000000000e+0;
static const double  EOS_VCSC4 = 1.6800000000e+2;

static const double  EOS_VCSJ1 = -1.0350605860e+1;
static const double  EOS_VCSJ2 = 8.8066614295e+0;
static const double  EOS_VCSJ3 = -1.5132806564e+3;
static const double  EOS_VCSJ4 = 1.8873011594e+2;
static const double  EOS_VCSJ5 = 1.3567312582e-3;
static const double  EOS_VCSJ6 = 4.5944806261e+0;
static const double  EOS_VCSJ7 = 1.9377431488e+2;

static const double  EOS_VCSK1 = -9.7460200000e+0;
static const double  EOS_VCSK2 = 9.0631692206e+0;
static const double  EOS_VCSK3 = -2.0682293095e+3;
static const double  EOS_VCSK4 = 4.4605500000e+1;
static const double  EOS_VCSK5 = 3.0881032754e-2;
static const double  EOS_VCSK6 = 2.5871668712e+0;
static const double  EOS_VCSK7 = 4.9489329154e+2;

static const int  EOS_VCSMI  = 100;   // max number of iterations

////////////////////////////////////////////////////////////////////////////////
//  Construction of cached pvttable. 
///////////////////////////////////////////////////////////////////////////////
EosPvtTable::EosPvtTable()
{
   Initialize( 0, NULL, NULL, NULL, NULL, NULL );
}

////////////////////////////////////////////////////////////////////////////////
// Construction of cached pvttable. 
//
// See EosPvtModel.h EosPvtTable__Constructor for argument description
////////////////////////////////////////////////////////////////////////////////
EosPvtTable::EosPvtTable( int iVersion, int *piTables, double *pdTables, double *pCritical, double *pvtData, double *pTSaltMW )
{
   Initialize( iVersion, piTables, pdTables, pCritical, pvtData, pTSaltMW );
}


////////////////////////////////////////////////////////////////////////////////
// Make a copy of the cache
//
// pCache Pointer to original cache
// dT     Temperature
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
/////////////////////////////////////////////////////////////////////////////////
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
      pCache->GetABTerms( 1, EOS_FUGACITY, EOS_NOHEAT, EOS_DRV_N, iMultipleAbc, &iAbcOffset, &dP, m_pTemperature, pX, &dA, &dB, &dAOverB, 
                          m_pCdata, m_pBdata, m_pAdata, NULL, NULL, NULL, pHelp, NULL, NULL, &dHelp1, &dHelp2, &dHelp3, &dHelp4, &dHelp5, &dHelp6 );

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


///////////////////////////////////////////////////////////////////////////////
// Make a copy of the cache
//
// pCache Pointer to original cache
// returns a pointer to the new cache
///////////////////////////////////////////////////////////////////////////////
EosPvtTable::EosPvtTable( EosPvtTable &pCache ) : EosWater()
{
   /* Make the raw copy */
   Copy( &pCache );
}


///////////////////////////////////////////////////////////////////////////////
//
// Constructor for corresponding state viscosity model. 
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
//
///////////////////////////////////////////////////////////////////////////////
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


///////////////////////////////////////////////////////////////////////////////
///
/// \brief Destruction of pvt cache 
//
// 1) Delete temporary memory
// 2) Delete instance
///
///////////////////////////////////////////////////////////////////////////////
EosPvtTable::~EosPvtTable()
{
   /* Free work area */
   if ( m_pOwnMemory )
   {
      CDELETE( m_pOwnMemory );
      m_pOwnMemory = NULL;
   }
}


///////////////////////////////////////////////////////////////////////////////
//
// Make a copy of the cache
//
// pCache Pointer to original cache
//
// Note: only the pointers are copied for arrays;
// and hence the original data is used from its
// original location
//
// 1) Create a new instance
// 2) Make a copy of the data
//
///////////////////////////////////////////////////////////////////////////////
void EosPvtTable::Copy( const EosPvtTable *pCache ) 
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


//
// Initialize construction of cached pvttable. 
//
// See EosPvtModel.h EosPvtTable__Constructor for argument description
//
// 1) Generate an instance of the pvt cache
// 2) Set volumn translation off
// 3) Set defaults if no data supplied
// 5) Initialize the data
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
void EosPvtTable::SetPointers( int iN, double *pTCubic, double *pTMu, double *pTWork, double **pAT, double **pSumT, double **pFinal )
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
void EosPvtTable::PrintInputData( void ) const
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


// Write the default temperature for the table
//
// Returns the default temperature
//
// 1) The default temperature is that of the last pvt
//    table
double EosPvtTable::WriteTemperature() const
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

         EosUtils::VectorLog( iM, pHelp5T, pHelp5T );
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

      EosUtils::VectorLog( iM, pHelp3T, pHelp3T );

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

         EosUtils::VectorPow( iM, pMu, dSixth, pMu );
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

         EosUtils::VectorPow( iM, pMu, dSixth, pMu );
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

         EosUtils::VectorPowX( iM, pTf, pTg, pTg );
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

            EosUtils::VectorPowX( iM, pTf, pTg, pTg );
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

         EosUtils::VectorPowX( iM, pTf, pTg, pTg );
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

            EosUtils::VectorPowX( iM, pTf, pTg, pTg );
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

      EosUtils::VectorPow( iM, pT3, 3.5, pT3 );
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

      EosUtils::VectorPow( iM, pT3, 3.5, pT3 );
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

      EosUtils::VectorExp( iM, pT5, pT5 );
      EosUtils::VectorExp( iM, pT6, pT6 );

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
   EosUtils::VectorExp( iM, pMu, pMu );
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

      EosUtils::VectorExp( iM, pTa, pTa );
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

         EosUtils::VectorExp( iM, pTa, pTa );
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

      EosUtils::VectorExp( m_iEosComp, pKValue, pKValue );
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
      EosUtils::VectorPow( iM, pSum4, dE, pWrk1 ); // pWrk1 = (Mn)^(Visfac2-1)
      EosUtils::VectorPow( iM, pSum3, dE, pWrk2 ); // pWrk2 = (Mw)^(Visfac2-1) 
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
      EosUtils::VectorPow( iM, pSum4, m_dVTerm2, pWrk1 );   // Mn^2.303
      EosUtils::VectorPow( iM, pSum3, m_dVTerm2, pWrk2 );   // Mw^2.303
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

   EosUtils::VectorPow( iM, pTd, 1.847, pTa );     // pTa = rhor^1.847
   EosUtils::VectorPow( iM, pMWMix, 0.5173, pTb ); // pTb = MWmix^0.5173
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

   EosUtils::VectorPow( iM, pTa, dC, pTa );
   EosUtils::VectorPow( iM, pTb, dD, pTb );
   EosUtils::VectorPow( iM, pTc, 0.5, pTc );
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
   EosUtils::VectorLog( iM, pTb, pTb );         // log(T/F) 
   EosUtils::VectorPow( iM, pTZero, d5, pTa );  // T^(1/3)
   EosUtils::VectorPow( iM, pRZero, 0.5, pTg ); // sqrt(rho)
   EosUtils::VectorPow( iM, pTc, 1.5, pTe );    // (1/T)^(3/2)
   EosUtils::VectorPow( iM, pRZero, 0.1, pTf ); // rho^(0.1)
   EosUtils::VectorExp( iM, pTj, pTj );         // exp(j1+j4/T)
   EosUtils::VectorExp( iM, pTp, pTp );         // exp(k1+k4/T)

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
   EosUtils::VectorExp( iM, pTm, pTm ); 
   EosUtils::VectorExp( iM, pTs, pTs );
   EosUtils::VectorExp( iM, pTu, pTu ); // exp(DT)

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
   EosUtils::VectorPow( iM, pTd, 1.5, pSaturationPressure );
   EosUtils::VectorPow( iM, pTa, EOS_VCSLD, pLiquidDensity );
   EosUtils::VectorPow( iM, pTa, EOS_VCSA6, pGasDensity );
   EosUtils::VectorPow( iM, pTa, dG, pTc );

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
   EosUtils::VectorExp( iM, pSaturationPressure, pSaturationPressure );
   EosUtils::VectorExp( iM, pTa, pTa );
   EosUtils::VectorExp( iM, pTb, pGasDensity );

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
   EosUtils::VectorPow( iM, pTemperature, 0.5, pWork );

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

      EosUtils::VectorExp( iM, pWork, pWork );

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

      EosUtils::VectorExp( iM, pWork, pWork );

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


