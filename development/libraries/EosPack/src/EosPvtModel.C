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

#include "EosApplication.h"
#include "EosPvtModel.h"
#include "EosPvtTable.h"
#include "EosUtils.h"

/* 
// Do NOT include any application files here - standard include ONLY
// to keep EOS application stand-alone.
*/
//#include "Unit.h"

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <algorithm>

/* Define whether to restore terms */
#define EOS_NORESTORE 0
#define EOS_RESTORE   1

/* Constants for scaling K values */
#define EOS_NOSCALEK 0
#define EOS_SCALEK   1

/* Constants for retrograde bubble points */
#define EOS_NORETROGRADE 0
#define EOS_RETROGRADE   1

/* Constants for forcing bubble point calculation */
#define EOS_NOFORCEBP 0
#define EOS_FORCEBP   1

/* Constants for flash pressure */
#define EOS_NONORMALIZE 0
#define EOS_NORMALIZE   1

/* Define constants for phase identification */
#define EOS_GAS 0
#define EOS_OIL 1


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
   m_dEnorm            = self.m_dEnorm;
   m_dLnEnorm          = self.m_dLnEnorm;
   m_dTiny             = self.m_dTiny;
   m_dConverge         = self.m_dConverge;
   m_dThermalDiffusion = self.m_dThermalDiffusion;
   m_dBubbleReduce     = self.m_dBubbleReduce;
   m_dNewtonRelaxCoeff = self.m_dNewtonRelaxCoeff;
   m_iMaxIterations    = self.m_iMaxIterations;
   m_iFlashLength      = self.m_iFlashLength;
   m_iMichelson        = self.m_iMichelson;
   m_iSubstitutions    = self.m_iSubstitutions;
   m_iDebug            = self.m_iDebug;
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
   m_pApplication   = (EosApplication *)0;
   m_pEosPvtTable   = (EosPvtTable *)0;
   m_pWorkArray     = (double *)0;
   m_iDrv           = EOS_DRV_N;
   m_iHeat          = EOS_OPTION_OFF;
   m_iVolume        = EOS_OPTION_OFF;
   m_iMolarFlash    = EOS_OPTION_OFF;
   m_restoreKValues = EOS_OPTION_OFF;
   m_iWaterComp     = -1;

   /* Set default terms */
   if ( piFlasher == (int *)0 )
   {
      m_dEnorm            = FLT_MAX;
      m_dLnEnorm          = log( m_dEnorm );
      m_dTiny             = DBL_EPSILON;
      m_dThermalDiffusion = 0.0;
      m_dBubbleReduce     = 0.5;
      m_dNewtonRelaxCoeff = 1.0;
      m_dConverge         = 0.0001;
      m_iMichelson        = EOS_OPTION_ON;
      m_iBubbleDew        = EOS_OPTION_OFF;
      m_iPseudoProperties = EOS_OPTION_OFF;
      m_iDebug            = EOS_OPTION_OFF;
      m_iMaxIterations    = 20;
      m_iFlashLength      = 64;
      m_iSubstitutions    = 0;
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
   const int iNc = m_pEosPvtTable->GetNumberHydrocarbons();

   m_pApplication->WriteControlData( &iType, &m_restoreKValues, &iNObj, &iFlash, &iProperties,
                                  &m_iBubbleDewPoint, &iWater, &iInitialize, &m_iBubbleDew, &m_iPseudoProperties );

   /* Reset the maximum flash length */
   if ( iInitialize != EOS_FLASH_CALCULATIONS )
   {
      m_iFlashLength = 1;
   }

   /* Set the memory */
   m_pWorkArray = (double *)0;
   iSize = SetPointers(); // get the size...
   m_pWorkArray = CNEW ( double, iSize ); //...allocate it
   iSize = SetPointers(); //...set the pointers

   /* Standard flash calculations */
   if ( iInitialize == EOS_FLASH_CALCULATIONS )
   {
      /* Set minimum pressure */
      WriteMinimumPressure();

      /* Debug print */
      if ( m_iDebug )
      {
         PrintInputData( iSize );
      }

      /* Need to flash anyway if nothing restored */
      iFlashIt = ( ( iFlash || m_restoreKValues == EOS_OPTION_OFF ) && iNc > 0 ) ? 1 : 0;
      m_iBubbleDewPoint = m_iBubbleDewPoint && ( iNc > 0 );

      /* Check grid blocks for maximum changes, etc. */
      if ( iFlashIt || m_iBubbleDewPoint )
      {
         m_pApplication->ModifyPhaseIdentification( m_dEnorm );
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
            m_pApplication->SetTrivialSlice();
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
         m_pApplication->PrintOutputData();
      }
   }

   /* Compositional grading */
   else if ( iInitialize == EOS_COMPOSITIONAL_GRADING )
   {
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
   m_pApplication->WriteOutputData(
      &m_iDrv, &iGetViscosity, &iGetTension, &m_iHeat, &iGetMW, &m_iVolume, &m_iMolarFlash, &iMolarDensity
      );

   /* Bubble point initial guess */
   if ( iType == EOS_TOF_2P )
   {
      m_pApplication->ResetSlice();
      iM = m_pApplication->SetSlice( EOS_FL_1P_NCV, m_iFlashLength );
      while ( iM )
      {
         ReadData( iM, iM, iNc, EOS_OPTION_OFF, EOS_NOGETK, &iTherm );
         BubblePointInit( iM, iNc, EOS_NORETROGRADE );
         m_pApplication->ReadFlashResults( iM, iM, EOS_FL_BP_NCV, EOS_NORESTORE, m_pSplit, m_pPhase, m_pKValue );
         iM = m_pApplication->SetSlice( EOS_FL_1P_NCV, m_iFlashLength );
      }

      /* Bubble point iterations */
      m_pApplication->ResetSlice();
      iM = m_pApplication->SetSlice( EOS_FL_BP_NCV, m_iFlashLength );
      while ( iM )
      {
         ReadData( iM, iM, iNc, EOS_OPTION_OFF, EOS_GETK, &iTherm );
         BubblePointNewton( iM, iNc, EOS_FL_BP_CV );

         m_pApplication->ReadFlashResults(
            iM, iM, ( m_iDrv ? EOS_FL_BP_CV : EOS_FL_BP_NCV ), EOS_NORESTORE, m_pSplit, m_pPhase, m_pKValue
            );
         iM = m_pApplication->SetSlice( EOS_FL_BP_NCV, m_iFlashLength );
      }
   }

   /* Reset phase indicator */
   m_pApplication->ModifyPhaseIndicator( EOS_OPTION_ON );

   /* Set bubble point properties */
   if ( iType == EOS_TOF_2P )
   {
      m_pApplication->ResetSlice();
      iM = m_pApplication->SetSlice( EOS_FL_BP, m_iFlashLength );
      while ( iM )
      {
         AdjustPointers( iM, iNc, EOS_OPTION_OFF );
         ReadData( iM, iM, iNc, EOS_OPTION_OFF, EOS_GETK, &iTherm );
         BubblePoint( iM, iNc );
         WriteData( iM, iNc, iGetViscosity, iGetTension, iMolarDensity, iType, EOS_FL_2P );
         iM = m_pApplication->SetSlice( EOS_FL_BP, m_iFlashLength );
      }
   }

   /* Set single phase properties */
   m_pApplication->ResetSlice();
   iM = m_pApplication->SetSlice( EOS_FL_1P, m_iFlashLength );
   while ( iM )
   {
      AdjustPointers( iM, iNc, EOS_OPTION_OFF );
      ReadData( iM, iM, iNc, EOS_OPTION_OFF, EOS_NOGETK, &iTherm );
      OnePhase( iM, iNc );
      BubblePointZero( iM, iNc );
      WriteData( iM, iNc, iGetViscosity, iGetTension, iMolarDensity, iType, EOS_FL_1P );
      iM = m_pApplication->SetSlice( EOS_FL_1P, m_iFlashLength );
   }

   /* Set no hydrocarbon properties */
   m_pApplication->ResetSlice();
   iM = m_pApplication->SetSlice( EOS_FL_0P, m_iFlashLength );
   while ( iM )
   {
      WriteData( iM, iNc, iGetViscosity, iGetTension, iMolarDensity, iType, EOS_FL_0P );
      iM = m_pApplication->SetSlice( EOS_FL_0P, m_iFlashLength );
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
   printf( "\nEosPvtModel.dNewtonRelaxCoeff: " );
   printf( "%e", m_dNewtonRelaxCoeff );
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
   m_pEosPvtTable->PrintInputData();
   m_pApplication->PrintInputData();
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
   m_dEnorm            = pdFlasher[EOS_ENORM];
   m_dLnEnorm          = log( m_dEnorm );
   m_dTiny             = pdFlasher[EOS_TINY];
   m_dConverge         = pdFlasher[EOS_CONVERGENCE];
   m_dThermalDiffusion = pdFlasher[EOS_THERMALDIFFUSION];
   m_dBubbleReduce     = pdFlasher[EOS_BUBBLEREDUCE];
   m_dNewtonRelaxCoeff = pdFlasher[EOS_NEWTON_RELAX_COEFF];
   m_iMaxIterations    = piFlasher[EOS_MAXITN];
   m_iFlashLength      = piFlasher[EOS_MAXFLASH];
   m_iMichelson        = piFlasher[EOS_MICHELSON];
   m_iSubstitutions    = piFlasher[EOS_SUBSTITUTIONS];
   m_iDebug            = piFlasher[EOS_DEBUG];
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
   /* Loop bounds */
   int     i1;
   int     i2;
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
   int     iMolar;
   m_pApplication->WriteData( iM, i1, i2, iGetK, isSalt, m_pPressure, m_pTemperature, m_pComposition,
      m_pSplit, m_pKValue, m_pPhase, pThermal, &iMolar );

   /* Use correct molecular weight */
   double *pforMW = isSalt ? m_pSaltMW : m_pMolecularWeight;
   
   /* Set the pvt term assignments */
   m_pEosPvtTable->WritePvtInfo(
      iM, i1, i2, *pThermal, isSalt, m_iWaterComp, m_pAbcOffset, &m_iMultipleAbc, m_pTemperature, pforMW
      );
      
   /* Reset thermal indicator */
   *pThermal = ( *pThermal == EOS_OPTION_OFF ) ? 1 : 0;

   /* Calculate mole fraction and moles for multiple blocks */
   if ( iM > 1 )
   {
      if ( iNc == 0 )
      {
         for ( int i = i1; i < i2; i++ )
         {
            m_pMoles[i] = 0.0;
         }
      }
      else if ( iMolar )
      {
         double *pTa = m_pComposition;
         for ( int i = i1; i < i2; i++ )
         {
            m_pMoles[i] = pTa[i];
         }

         for ( int iNi = 1; iNi < iNc; iNi++ )
         {
            pTa += iM;
            for ( int i = i1; i < i2; i++ )
            {
               m_pMoles[i] += pTa[i];
            }
         }
      }
      else
      {
         double *pTa = m_pComposition;
         double dA = pforMW[0];
         for ( int i = i1; i < i2; i++ )
         {
            pTa[i] = pTa[i] / dA;
            m_pMoles[i] = pTa[i];
         }

         // the following loop is NOT relevant when m_iUseBlkSlt = ON (because iNc=1)
         for ( int iNi = 1; iNi < iNc; iNi++ )
         {
            pTa += iM;
            dA = pforMW[iNi];
            for ( int i = i1; i < i2; i++ )
            {
               pTa[i] = pTa[i] / dA;
               m_pMoles[i] += pTa[i];
            }
         }
      }

      /* Normalize */
      for ( int i = i1; i < i2; i++ )
      {
         double dA = m_pMoles[i];
         m_pWork[i] = 1.0 / ( dA > 0.0 ? dA : 1.0 );
      }

      double *pTa = m_pComposition;
      for ( int iNi = 0; iNi < iNc; iNi++ )
      {
         for ( int i = i1; i < i2; i++ )
         {
            double dA = pTa[i] * m_pWork[i];
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
         for ( int iNi = 1; iNi < iNc; iNi++ )
         {
            m_pMoles[0] += m_pComposition[iNi];
         }
      }
      else
      {
         m_pComposition[0] = m_pComposition[0] / pforMW[0];
         m_pMoles[0] = m_pComposition[0];
         for ( int iNi = 1; iNi < iNc; iNi++ )
         {
            m_pComposition[iNi] = m_pComposition[iNi] / pforMW[iNi];
            m_pMoles[0] += m_pComposition[iNi];
         }
      }

      /* Normalize */
      double dA = m_pMoles[0];
      double dB = 1.0 / ( dA > 0.0 ? dA : 1.0 );
      for ( int iNi = 0; iNi < iNc; iNi++ )
      {
         double dC = m_pComposition[iNi] * dB;
         m_pComposition[iNi] = ( dC >= m_dTiny ) ? dC : 0.0;
      }
   }

   /* If water check to see if anything there and take action */
   if ( isSalt && m_iWaterComp >= 0 )
   {
      int iNi = m_iWaterComp * iM;
      for ( int i = i1; i < i2; i++ )
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
//    Modified to have the minimum pressure capped at 1Bar (100000 Pascal).
//    This is because in ThermoMoReS with high temperature, the flash is used to
//    generate K-values,the water vapour pressure could be higher than user
//    specified flash pressure.
// 3) Send the results to Application
*/
void EosPvtModel::WriteMinimumPressure( void )
{
   double dT = m_pApplication->WriteIsothermal() ? m_pEosPvtTable->WriteTemperature() : m_pApplication->WriteMinimumTemperature();
   double minP = m_pEosPvtTable->WaterVaporPressure( dT );

   m_pApplication->ReadMinimumPressure( std::min( 100000.0, minP ) );
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
   iFlashes = m_pApplication->WriteNumberFlashes();
   iN = iFlashes > m_iFlashLength ? m_iFlashLength : iFlashes;

   /* Terms to set up locations */
   iNc = m_pEosPvtTable->GetNumberHydrocarbons();
   iSalts = m_pEosPvtTable->GetNumberSalts();
   iNc = ( iNc > iSalts ) ? iNc : ( iSalts + 1 );
   iNcm = iNc * iN;
   iNc2 = iNc * iNcm;

   /* Keep a local copy of the molecular weight */
   m_pMolecularWeight = m_pWorkArray;

   /* Set up the array for the phase indicator */
   m_pPhase = (int *)( m_pMolecularWeight + iNc );

   /* Set up locations for saved properties in work array */
   m_pPressure    = ( (double *)m_pPhase ) + iN;
   m_pTemperature = m_pPressure    + iN;
   m_pMoles       = m_pTemperature + iN;
   m_pSplit       = m_pMoles       + iN;
   m_pKValue      = m_pSplit       + iN;
   m_pComposition = m_pKValue      + iNcm;

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
   m_pDXdp   = m_pX      + iNcm;
   m_pY      = m_pDXdp   + i;
   m_pDYdp   = m_pY      + iNcm;
   m_pMx     = m_pDYdp   + i;
   m_pDMxdp  = m_pMx     + iN;
   m_pMy     = m_pDMxdp  + iJ;
   m_pDMydp  = m_pMy     + iN;
   m_pZx     = m_pDMydp  + iJ;
   m_pDZxdp  = m_pZx     + iN;
   m_pZy     = m_pDZxdp  + iJ;
   m_pDZydp  = m_pZy     + iN;
   m_pHx     = m_pDZydp  + iJ;
   m_pDHxdp  = m_pHx     + iN;
   m_pHy     = m_pDHxdp  + iJ;
   m_pDHydp  = m_pHy     + iN;
   m_pMWx    = m_pDHydp  + iJ;
   m_pMWy    = m_pMWx    + iN;
   m_pMux    = m_pMWy    + iN;
   m_pDMuxdp = m_pMux    + iN;
   m_pMuy    = m_pDMuxdp + iJ;
   m_pDMuydp = m_pMuy    + iN;
   m_pIfx    = m_pDMuydp + iJ;
   m_pDIfxdp = m_pIfx    + iN;
   m_pIfy    = m_pDIfxdp + iJ;
   m_pDIfydp = m_pIfy    + iN;
   m_pWork   = m_pDIfydp + iJ;

   /* Save bubble point locations */
   if ( m_iBubbleDewPoint == EOS_OPTION_OFF )
   {
      m_pDBpdp = m_pDMydp;
   }

   /* Set pvt table pointers */
   m_pEosPvtTable->SetPointers( iN, m_pMWy, m_pIfx, m_pWork, &m_pATable, &m_pSumTable, &pFinal );
   i = (int)( pFinal - m_pWorkArray );

   /* Give values to other pointers */
   if ( m_pWorkArray != ( (double *)0 ) )
   {
      AdjustPointers( iN, iNc, EOS_OPTION_OFF );
      m_pOSplit     = m_pWork;
      m_pFx         = m_pWork + iN;
      m_pG          = m_pMWx;
      m_pH          = m_pDXdp;
      m_pFy         = m_pDXdt;
      m_pGx         = m_pDHxdp;
      m_pGy         = m_pDHxdt;
      m_pBeta       = m_pDHydp;
      m_pGold       = m_pDHydt;
      m_pLow        = m_pDMydp;
      m_pHigh       = m_pDMydt;
      m_pXRhs       = m_pMx;
      m_pXMat       = m_pDMxdt;
      m_pTerm1      = m_pMux;
      m_pTerm2      = m_pMuy;
      m_pTerm3      = m_pDMuydp;
      m_pTermx      = m_pDMuxda;
      m_pTermy      = m_pDMuyda;
      m_pPotentialx = m_pDMxda;
      m_pPotentialy = m_pDMyda;
      m_pLastx      = pFinal - iNcm;
      m_pLasty      = m_pDHyda;
      m_pPoint      = m_pDMxdp;
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
   m_pApplication->SetPointers( iN, &pFinal );

   /* Set pvt pointers */
   m_pEosPvtTable->SetAbcPointers( iN, &m_pAbcOffset, &pFinal );

   /* Calculate the total length */
   i = (int)(pFinal - m_pWorkArray);
   i = ( i > i1 ? i : i1 );

   /* End of the routine */
   return ( i );
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
   iRestore = m_pApplication->WriteOldValues();
   if ( iRestore )
   {
      /* Reset bubble point terms */
      if ( m_iBubbleDew )
      {
         m_pApplication->ModifyBubble( EOS_OPTION_ON, EOS_FL_1P_NCV, EOS_FL_BP_NCV );

         /* Bubble point iterations */
         m_pApplication->ResetSlice();
         iM = m_pApplication->SetSlice( EOS_FL_BP_NCV, m_iFlashLength );
         while ( iM )
         {
            /* Set up locations in the work array */
            ReadData( iM, iM, iNc, EOS_OPTION_OFF, EOS_GETK, &iThermal );

            /* Calculate the bubble point */
            BubblePointNewton( iM, iNc, EOS_FL_BP_CHK );

            /* Store grid block values */
            m_pApplication->ReadFlashResults( iM, iM, EOS_FL_BP_CV, EOS_NORESTORE, m_pSplit, m_pPhase, m_pKValue );
            iM = m_pApplication->SetSlice( EOS_FL_BP_NCV, m_iFlashLength );
         }

         /* Check bubble point */
         m_pApplication->ResetSlice();
         iM = m_pApplication->SetSlice( EOS_FL_BP_CHK, m_iFlashLength );
         while ( iM )
         {
            /* Set up locations in the work array */
            ReadData( iM, iM, iNc, EOS_OPTION_OFF, EOS_GETK, &iThermal );

            /* Test the bubble point */
            BubblePointTest( iM, iNc );

            /* Store grid block values */
            m_pApplication->ReadFlashResults( iM, iM, EOS_FL_2P_NCV, EOS_NORESTORE, m_pSplit, m_pPhase, m_pKValue );
            iM = m_pApplication->SetSlice( EOS_FL_BP_CHK, m_iFlashLength );
         }
      }

      /* Successive substitution */
      if ( m_iSubstitutions )
      {
         m_pApplication->ResetSlice();
         iM = m_pApplication->SetSlice( EOS_FL_2P_NCV, m_iFlashLength );
         while ( iM )
         {
            /* Set up locations in the work array */
            ReadData( iM, iM, iNc, EOS_OPTION_OFF, EOS_GETK, &iThermal );

            /* Do the successive substitutions */
            Substitution( iM, iNc );

            /* Store grid block values */
            m_pApplication->ReadFlashResults( iM, iM, EOS_FL_2P_NCV, EOS_NORESTORE, m_pSplit, m_pPhase, m_pKValue );
            iM = m_pApplication->SetSlice( EOS_FL_2P_NCV, m_iFlashLength );
         }
      }

      /* Newton iterations */
      m_pApplication->ResetSlice();
      iM = m_pApplication->SetSlice( EOS_FL_2P_NCV, m_iFlashLength );
      while ( iM )
      {
         /* Set up locations in the work array */
         ReadData( iM, iM, iNc, EOS_OPTION_OFF, EOS_GETK, &iThermal );

         /* Do the Newton iterations */
         NewtonFlashMultipleObjects( iM, iNc, EOS_NORESTORE, EOS_FL_2P_CV );

         /* Store grid block values */
         m_pApplication->ReadFlashResults( iM, iM, EOS_FL_2P_CV, EOS_NORESTORE, m_pSplit, m_pPhase, m_pKValue );
         iM = m_pApplication->SetSlice( EOS_FL_2P_NCV, m_iFlashLength );
      }
   }

   /* Michelson stability analysis */
   if ( m_iMichelson )
   {
      m_pApplication->ResetSlice();
      iM = m_pApplication->SetSlice( EOS_FL_1P_NCV, m_iFlashLength );
      while ( iM )
      {
         /* Set up locations in the work array */
         ReadData( iM, iM, iNc, EOS_OPTION_OFF, EOS_NOGETK, &iThermal );

         /* Do the Michelson stability analysis */
         Michelson( iM, iNc );

         /* Store grid block values */
         m_pApplication->ReadFlashResults( iM, iM, EOS_FL_2P_NCV, iRestore, m_pSplit, m_pPhase, m_pKValue );
         iM = m_pApplication->SetSlice( EOS_FL_1P_NCV, m_iFlashLength );
      }
   }

   /* Fast stability analysis */
   if ( m_iMichelson == EOS_OPTION_OFF )
   {
      m_pApplication->ResetSlice();
      iM = m_pApplication->SetSlice( EOS_FL_1P_NCV, m_iFlashLength );
      while ( iM )
      {
         /* Set up locations in the work array */
         ReadData( iM, iM, iNc, EOS_OPTION_OFF, EOS_NOGETK, &iThermal );

         /* Do the fast stability analysis */
         FastInitialization( iM, iNc );

         /* Store grid block values */
         m_pApplication->ReadFlashResults( iM, iM, EOS_FL_2P_NCV, iRestore, m_pSplit, m_pPhase, m_pKValue );
         iM = m_pApplication->SetSlice( EOS_FL_1P_NCV, m_iFlashLength );
      }
   }

   /* Successive substitution */
   if ( m_iSubstitutions )
   {
      m_pApplication->ResetSlice();
      iM = m_pApplication->SetSlice( EOS_FL_2P_NCV, m_iFlashLength );
      while ( iM )
      {
         /* Set up locations in the work array */
         ReadData( iM, iM, iNc, EOS_OPTION_OFF, EOS_GETK, &iThermal );

         /* Do the successive substitutions */
         Substitution( iM, iNc );

         /* Store grid block values */
         m_pApplication->ReadFlashResults( iM, iM, EOS_FL_2P_NCV, iRestore, m_pSplit, m_pPhase, m_pKValue );
         iM = m_pApplication->SetSlice( EOS_FL_2P_NCV, m_iFlashLength );
      }
   }

   /* Newton iterations */
   m_pApplication->ResetSlice();
   iM = m_pApplication->SetSlice( EOS_FL_2P_NCV, m_iFlashLength );
   while ( iM )
   {
      /* Set up locations in the work array */
      ReadData( iM, iM, iNc, EOS_OPTION_OFF, EOS_GETK, &iThermal );

      /* Do the Newton iterations */
      NewtonFlashMultipleObjects( iM, iNc, iRestore, EOS_FL_2P_NCV );

      /* Store grid block values */
      m_pApplication->ReadFlashResults( iM, iM, EOS_FL_2P_NCV, iRestore, m_pSplit, m_pPhase, m_pKValue );
      iM = m_pApplication->SetSlice( EOS_FL_2P_NCV, m_iFlashLength );
   }

   /* Bubble point initial guess */
   if ( m_iBubbleDew )
   {
      m_pApplication->ResetSlice();
      iM = m_pApplication->SetSlice( EOS_FL_1P_NCV, m_iFlashLength );
      while ( iM )
      {
         /* Set up locations in the work array */
         ReadData( iM, iM, iNc, EOS_OPTION_OFF, EOS_NOGETK, &iThermal );

         /* Generate the initial guess */
         BubblePointInit( iM, iNc, EOS_NORETROGRADE );

         /* Store grid block values */
         m_pApplication->ReadFlashResults( iM, iM, EOS_FL_BP_NCV, iRestore, m_pSplit, m_pPhase, m_pKValue );
         iM = m_pApplication->SetSlice( EOS_FL_1P_NCV, m_iFlashLength );
      }

      /* Bubble point iterations */
      m_pApplication->ResetSlice();
      iM = m_pApplication->SetSlice( EOS_FL_BP_NCV, m_iFlashLength );
      while ( iM )
      {
         /* Set up locations in the work array */
         ReadData( iM, iM, iNc, EOS_OPTION_OFF, EOS_GETK, &iThermal );

         /* Calculate the bubble point */
         BubblePointNewton( iM, iNc, EOS_FL_BP_CV );

         /* Store grid block values */
         m_pApplication->ReadFlashResults( iM, iM, EOS_FL_BP_CV, EOS_NORESTORE, m_pSplit, m_pPhase, m_pKValue );
         iM = m_pApplication->SetSlice( EOS_FL_BP_NCV, m_iFlashLength );
      }
   }

   /* Reset phase indicator */
   m_pApplication->ModifyPhaseIndicator( EOS_OPTION_ON );

   /* Reset bubble point terms */
   if ( m_iBubbleDew )
   {
      m_pApplication->ModifyBubble( EOS_TO_APPLICATION, EOS_FL_BP, EOS_FL_1P );
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
   iRestore = m_pApplication->WriteOldValues();

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
         NewtonFlashOneObject( iNc, EOS_NORESTORE, EOS_FL_2P_NCV );
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
      NewtonFlashOneObject( iNc, iRestore, EOS_FL_2P_NCV );
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
      m_pApplication->ReadFlashResults( 1, 1, EOS_FL_1P_CV, iTested, m_pSplit, m_pPhase, m_pKValue );
      m_pApplication->ModifyPhaseIndicator( EOS_OPTION_OFF );
      *m_pPhase = ( *m_pPhase + 1 ) / 2;
   }
}


/*
// FlashEquations OneObject/MultipleObjects
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
void EosPvtModel::FlashEquationsOneObject( int iNc, int iUpdate )
{
   double  dTerm1;
   double  dTerm2;
   double  dA;
   double  dB;
   double  dC;
   double  dD;
   double  dCnv;
   double  dSplit;
   double  dOSplit;
   int     i;
   int     iNi;
   int     iter;
   int     iTemp;
   int     iConvrg;

   /* Constant */
   dA = 100.0 * m_dTiny;
   dB = m_dConverge * m_dConverge;
   dCnv = ( dA > dB ) ? dA : dB;

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
#if defined(__INTEL_COMPILER) && __INTEL_COMPILER < 1300 
   // With Intel 12.0.2.137 compiler without this pragma the compiler
   // generate a faulty code, by specifying this pragma the generated
   // code is corrected and the optimization working.
   #pragma ivdep 
#endif         

   for ( iNi = 0; iNi < iNc; iNi++ )
   {
      m_pKValue[iNi] = dTerm2 * m_pKValue[iNi] + dTerm1;
      m_pTermx[iNi] = m_pKValue[iNi] - 1.0;
      m_pTermy[iNi] = m_pTermx[iNi] * m_pComposition[iNi];
   }


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

void EosPvtModel::FlashEquationsMultipleObjects( int iM, int i1, int i2, int iNc, int iUpdate )
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

   for ( iNi = 1; iNi < iNc; iNi++ )
   {
      pTa += iM;
      pTb += iM;
      for ( i = i1; i < i2; i++ )
      {
         dA = pTa[i];
         dB = ( dA - 1.0 ) * pTb[i];
         m_pTerm1[i] += dB;
         m_pTerm2[i] += dB / dA;
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
#if defined(__INTEL_COMPILER) && __INTEL_COMPILER < 1300 
   // With Intel 12.0.2.137 compiler without this pragma the compiler
   // generate a faulty code, by specifying this pragma the generated
   // code is corrected and the optimization working.
   #pragma ivdep 
#endif         
     for ( i = i1; i < i2; i++ )
     {
         pTa[i] = m_pTerm2[i] * pTa[i] + m_pTerm1[i];
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
      for ( i = i1; i < i2; i++ )
      {
         dA = 1.0 / ( m_pOSplit[i] + m_pSplit[i] * pTa[i] );
         dB = pTc[i] * dA;
         m_pTerm1[i] = dB;
         m_pTerm2[i] = -m_dTiny - pTb[i] * dB * dA;
      }

      for ( iNi = 1; iNi < iNc; iNi++ )
      {
         pTa += iM;
         pTb += iM;
         pTc += iM;
         for ( i = i1; i < i2; i++ )
         {
            dA = 1.0 / ( m_pOSplit[i] + m_pSplit[i] * pTa[i] );
            dB = pTc[i] * dA;
            m_pTerm1[i] += dB;
            m_pTerm2[i] -= pTb[i] * dB * dA;
         }
      }

      /* New split and test for convergence */
      dF = 0.0;
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
void EosPvtModel::
OneSideStability( int iM, int iNc, double *pP, double *pT, double *pState, double *pFz )
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
      m_pEosPvtTable->SolveCubic(
         iM, EOS_FUGACITY, EOS_NOHEAT, EOS_NOPOTENTIAL, EOS_DRV_N, EOS_NOPHASEID,
         m_iMultipleAbc, m_pAbcOffset, pP, pT, m_pX, m_pZy, m_pDZydp, m_pDZydt, m_pDZyda, pFz,
         m_pDYdp, m_pDYdt, m_pDYda, m_pHy, m_pDHydp, m_pDHydt, m_pDHyda, m_pATable, m_pPhaseId
         );

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
      EosLinAlg::Cholesky( iM, iNc, m_dTiny, m_pDYda, m_pTerm3, m_pTerm2 );
      EosLinAlg::BackSolve( iM, iNc, m_pDYda, m_pDYdp );

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
   m_pEosPvtTable->WilsonKValues( iM, EOS_SCALEK, m_pTemperature, m_pKValue, m_pWork );

   /* Calculate invariant part of function */
   m_pEosPvtTable->SolveCubic(
      iM, EOS_FUGACITY, EOS_NOHEAT, EOS_NOPOTENTIAL, EOS_DRV_N, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset,
      m_pPressure, m_pTemperature, m_pComposition, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pFx,
      m_pDXdp, m_pDXdt, m_pDXda, m_pHx, m_pDHxdp, m_pDHxdt, m_pDHxda, m_pATable, m_pPhaseId
      );

   /* Store invariant part of function for multiple blocks */
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
   if ( m_restoreKValues != EOS_OPTION_ON ) {
      m_pEosPvtTable->WilsonKValues( iM, EOS_SCALEK, m_pTemperature, m_pKValue, m_pWork );
   }

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

   m_pEosPvtTable->SolveCubic(
      iM, EOS_FUGACITY, EOS_NOHEAT, EOS_NOPOTENTIAL, EOS_NODRV, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset,
      m_pPressure, m_pTemperature, m_pX, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda,
      m_pFx, m_pDXdp, m_pDXdt, m_pDXda, m_pHx, m_pDHxdp, m_pDHxdt, m_pDHxda, m_pATable, m_pPhaseId
      );

   m_pEosPvtTable->SolveCubic(
      iM, EOS_FUGACITY, EOS_NOHEAT, EOS_NOPOTENTIAL, EOS_NODRV, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset,
      m_pPressure, m_pTemperature, m_pY, m_pZy, m_pDZydp, m_pDZydt, m_pDZyda,
      m_pFy, m_pDYdp, m_pDYdt, m_pDYda, m_pHy, m_pDHydp, m_pDHydt, m_pDHyda, m_pATable, m_pPhaseId
      );

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

      EosUtils::VectorExp( iNc * iM, m_pKValue, m_pKValue );

      for ( i = 0; i < iM; i++ )
      {
         m_pSplit[i] = 0.5;
         m_pPhase[i] = EOS_FL_2P_NCV;
      }

      /* Perform a single flash */
      FlashEquationsMultipleObjects( iM, 0, iM, iNc, EOS_NONORMALIZE );
   }

   /* Reset K values for single grid block case */
   else
   {

      if ( m_restoreKValues != EOS_OPTION_ON ) {

         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            dC = m_pFx[iNi] - m_pFy[iNi];
            m_pKValue[iNi] = dC > dB ? dB : ( dC < dA ? dA : dC );
         }

         EosUtils::VectorExp( iNc, m_pKValue, m_pKValue );
      }

      *m_pSplit = 0.5;
      *m_pPhase = EOS_FL_2P_NCV;

      /* Perform a single flash */
      FlashEquationsOneObject( iNc, EOS_NONORMALIZE );
   }
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
      if ( iM > 1 )
      {
         FlashEquationsMultipleObjects( iM, 0, iM, iNc, EOS_NORMALIZE );
      }
      else
      {
         FlashEquationsOneObject( iNc, EOS_NORMALIZE );
      }

      /* Get the x phase properties */
      m_pEosPvtTable->SolveCubic(
         iM, EOS_FUGACITY, EOS_NOHEAT, EOS_NOPOTENTIAL, EOS_NODRV, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset,
         m_pPressure, m_pTemperature, m_pX, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda,
         m_pFx, m_pDXdp, m_pDXdt, m_pDXda, m_pHx, m_pDHxdp, m_pDHxdt, m_pDHxda, m_pATable, m_pPhaseId
         );

      /* Get the y phase properties */
      m_pEosPvtTable->SolveCubic(
         iM, EOS_FUGACITY, EOS_NOHEAT, EOS_NOPOTENTIAL, EOS_NODRV, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset,
         m_pPressure, m_pTemperature, m_pY, m_pZy, m_pDZydp, m_pDZydt, m_pDZyda,
         m_pFy, m_pDYdp, m_pDYdt, m_pDYda, m_pHy, m_pDHydp, m_pDHydt, m_pDHyda, m_pATable, m_pPhaseId
         );

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
   if ( iM > 1 )
   {
      FlashEquationsMultipleObjects( iM, 0, iM, iNc, EOS_NONORMALIZE );
   }
   else
   {
      FlashEquationsOneObject( iNc, EOS_NONORMALIZE );
   }
}


/*
// NewtonFlash OneObject/MultipleObjects
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
void EosPvtModel::NewtonFlashOneObject( int iNc, int iRestore, int iLevel )
{
   int     iter;
   int     iNi;
   int     iNj;
   int     iNk;
   int     iConvrg;
   int     iThermal;
   int     iAnyConvrg;
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
   double  theta = m_dNewtonRelaxCoeff; // relax. coefficient to update solution in Newton

   /* With a single element slice don't bother with next one */

   /* Set terms */
   dCnv = m_dConverge * m_dConverge;
   dVeryTiny = 1.0 / m_dEnorm;
   dE = 0.0;

   /* Newton Loop */
   iConvrg = EOS_NOCONVERGE;
   for ( iter = 0; iter < m_iMaxIterations && iConvrg == EOS_NOCONVERGE; iter++ )
   {
      /* Solve the flah equations */
      FlashEquationsOneObject( iNc, EOS_NORMALIZE );

      /* Chemical potential for the x phase */
      m_pEosPvtTable->SolveCubic(
         1, EOS_FUGACITY, EOS_NOHEAT, EOS_NOPOTENTIAL, EOS_DRV_N, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset,
         m_pPressure, m_pTemperature, m_pX, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda,
         m_pFx, m_pDXdp, m_pDXdt, m_pDXda, m_pHx, m_pDHxdp, m_pDHxdt, m_pDHxda, m_pATable, m_pPhaseId
         );

      /* Chemical potential for the y phase */
      m_pEosPvtTable->SolveCubic(
         1, EOS_FUGACITY, EOS_NOHEAT, EOS_NOPOTENTIAL, EOS_DRV_N, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset,
         m_pPressure, m_pTemperature, m_pY, m_pZy, m_pDZydp, m_pDZydt, m_pDZyda,
         m_pFy, m_pDYdp, m_pDYdt, m_pDYda, m_pHy, m_pDHydp, m_pDHydt, m_pDHyda, m_pATable, m_pPhaseId
         );

      /* Scale terms for single grid block case */
      dE = ( *m_pPhase != EOS_FL_1P_NCV ) ? 1.0 : 0.0;
      dB = *m_pOSplit;
      dC = *m_pSplit;
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
         for ( iNj = iNi; iNj < iNc; iNj++ )
         {
            pTa[iNj] = m_pDYdp[iNj] * m_pDYdp[iNi] * ( pTa[iNj] * dC + pTb[iNj] * dB - 1.0 );
         }

         pTa += iNc;
         pTb += iNc;
      }

      /* Diagonal terms and right hand side */
      pTa = m_pDXda;
      EosUtils::VectorLog( iNc, m_pKValue, m_pKValue );
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         *pTa += 1.0;
         m_pFy[iNi] = dE * ( ( m_pFx[iNi] - m_pFy[iNi] ) - m_pKValue[iNi] );
         m_pFx[iNi] = m_pDYdp[iNi] * m_pFy[iNi];
         pTa += iNc + 1;
      }

      /* Factor the matrix */
      EosLinAlg::Cholesky( 1, iNc, m_dTiny, m_pDXda, m_pTerm3, m_pTerm2 );

      /* Perform back substitution */
      EosLinAlg::BackSolve( 1, iNc, m_pDXda, m_pFx );

      /* Transform solution with the matrix and get maximum residual */
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
      if ( theta < 1.0 )
      {
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            dC = m_pKValue[iNi] + m_pFx[iNi] * theta;
            m_pKValue[iNi] = dC > m_dLnEnorm ? m_dLnEnorm : ( dC < dB ? dB : dC );               
         }
         theta *= 1.1; // increase relax coefficient on each Newton iteration till 1.0

      }
      else
      {
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            dC = m_pKValue[iNi] + m_pFx[iNi];
            m_pKValue[iNi] = dC > m_dLnEnorm ? m_dLnEnorm : ( dC < dB ? dB : dC );
         }
      }

      EosUtils::VectorExp( iNc, m_pKValue, m_pKValue );
   }

   /* Solve the flash equations once more before exiting */
   FlashEquationsOneObject( iNc, EOS_NONORMALIZE );
}

void EosPvtModel::NewtonFlashMultipleObjects( int iM, int iNc, int iRestore, int iLevel )
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
   double  theta = m_dNewtonRelaxCoeff; // relax. coefficient to update solution in Newton

   /* Prepare for iterations for more than one block */
   iNextExists = m_pApplication->Aandebeurt( -1, EOS_FL_2P_NCV );
   iNi = -m_iMaxIterations;
   for ( i = 0; i < iM; i++ )
   {
      m_pPhase[i] = iNi;
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
      FlashEquationsMultipleObjects( iM, 0, iM, iNc, EOS_NORMALIZE );

      /* Chemical potential for the x phase */
      m_pEosPvtTable->SolveCubic(
         iM, EOS_FUGACITY, EOS_NOHEAT, EOS_NOPOTENTIAL, EOS_DRV_N, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset,
         m_pPressure, m_pTemperature, m_pX, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda,
         m_pFx, m_pDXdp, m_pDXdt, m_pDXda, m_pHx, m_pDHxdp, m_pDHxdt, m_pDHxda, m_pATable, m_pPhaseId
         );

      /* Chemical potential for the y phase */
      m_pEosPvtTable->SolveCubic(
         iM, EOS_FUGACITY, EOS_NOHEAT, EOS_NOPOTENTIAL, EOS_DRV_N, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset,
         m_pPressure, m_pTemperature, m_pY, m_pZy, m_pDZydp, m_pDZydt, m_pDZyda,
         m_pFy, m_pDYdp, m_pDYdt, m_pDYda, m_pHy, m_pDHydp, m_pDHydt, m_pDHyda, m_pATable, m_pPhaseId
         );

      /* Scale terms for multiple grid block case */
      for ( i = 0; i < iM; i++ )
      {
         m_pG[i] = ( m_pPhase[i] != EOS_FL_1P_NCV ) ? 1.0 : 0.0;
      }

      pTa = m_pKValue;
      pTb = m_pComposition;
      pTc = m_pDYdp;
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         for ( i = 0; i < iM; i++ )
         {
            dA = pTa[i];
            pTc[i] = sqrt( pTb[i] * dA ) * m_pG[i] / ( m_pOSplit[i] + m_pSplit[i] * dA );
         }

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
            for ( i = 0; i < iM; i++ )
            {
               pTa[i] = pTd[i] * pTc[i] * ( pTa[i] * m_pSplit[i] + pTb[i] * m_pOSplit[i] - 1.0 );
            }

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
      EosUtils::VectorLog( iNc * iM, m_pKValue, m_pKValue );
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         for ( i = 0; i < iM; i++ )
         {
            dA = m_pG[i] * ( ( pTb[i] - pTc[i] ) - pTa[i] );
            pTc[i] = dA;
            pTb[i] = pTd[i] * dA;
            pTe[i] += 1.0;
         }

         pTa += iM;
         pTb += iM;
         pTc += iM;
         pTd += iM;
         pTe += iM + iNcm;
      }

      /* Factor the matrix */
      EosLinAlg::Cholesky( iM, iNc, m_dTiny, m_pDXda, m_pTerm3, m_pTerm2 );

      /* Perform back substitution */
      EosLinAlg::BackSolve( iM, iNc, m_pDXda, m_pFx );

      /* Transform solution with the matrix and get maximum residual */
      pTa = m_pDYdp;
      pTb = m_pFx;
      for ( i = 0; i < iM; i++ )
      {
         m_pOSplit[i] = pTb[i] * pTa[i];
      }

      for ( iNi = 1; iNi < iNc; iNi++ )
      {
         pTa += iM;
         pTb += iM;
         for ( i = 0; i < iM; i++ )
         {
            m_pOSplit[i] += pTb[i] * pTa[i];
         }
      }

      for ( i = 0; i < iM; i++ )
      {
         m_pOSplit[i] *= m_pG[i];
      }

      pTa = m_pFx;
      pTb = m_pFy;
      pTc = m_pDYdp;
      pTd = m_pComposition;
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
         for ( i = 0; i < iM; i++ )
         {
            dB = pTc[i];
            dA = ( pTa[i] - dB * m_pOSplit[i] ) / ( dB > dVeryTiny ? dB : dVeryTiny );
            dC = ( pTd[i] > 0.0 ? dA : pTb[i] );
            m_pDYdt[i] += dC * dC;
            pTa[i] = dC;
         }
      }

      /* Test for convergence */
      iConvrg = EOS_CONVERGE;
      iAnyConvrg = EOS_NOCONVERGE;
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
      if ( theta < 1.0 )
      {
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               dC = pTa[i] + pTb[i] * theta;
               pTa[i] = dC > m_dLnEnorm ? m_dLnEnorm : ( dC < dB ? dB : dC );
            }

            pTa += iM;
            pTb += iM;
         }
         theta *= 1.1; // increase relax coefficient on each Newton iteration till 1.0
      }
      else
      {
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               dC = pTa[i] + pTb[i];
               pTa[i] = dC > m_dLnEnorm ? m_dLnEnorm : ( dC < dB ? dB : dC );
            }

            pTa += iM;
            pTb += iM;
         }
      }

      EosUtils::VectorExp( iM * iNc, m_pKValue, m_pKValue );

      /* If anything "aan de beurt" */
      iAnyConvrg = ( iConvrg ? 0 : iAnyConvrg ) ? iM : 0;
      for ( i = 0; i < iNextExists * iAnyConvrg; i++ )
      {
         /* For converged blocks */
         if ( m_pPhase[i] > 0 )
         {
            /* One more check with flash equations */
            FlashEquationsMultipleObjects( iM, i, i + 1, iNc, EOS_NONORMALIZE );

            /* Store grid block values */
            m_pApplication->ReadFlashResults( i, iM, iLevel, iRestore, m_pSplit, m_pPhase, m_pKValue );

            /* Set the indirection location */
            iNextExists = m_pApplication->Aandebeurt( i, EOS_FL_2P_NCV );

            /* Load the next location */
            ReadData( i, iM, iNc, EOS_OPTION_OFF, EOS_GETK, &iThermal );

            /* Reset iteration counters */
            m_pPhase[i] = -m_iMaxIterations;
            iter = -1;
         }
      }
   }

   /* Solve the flash equations once more before exiting */
   FlashEquationsMultipleObjects( iM, 0, iM, iNc, EOS_NONORMALIZE );
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
   dB = m_pEosPvtTable->InitBubblePoint();
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
   m_pEosPvtTable->WilsonKValues( iM, EOS_SCALEK, m_pTemperature, m_pKValue, m_pWork );

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

      /* Invariant part of function */
      m_pEosPvtTable->SolveCubic(
         iM, EOS_FUGACITY, EOS_NOHEAT, EOS_NOPOTENTIAL, EOS_NODRV, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset,
         m_pBp, m_pTemperature, m_pComposition, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda,
         m_pPotentialy, m_pDXdp, m_pDXdt, m_pDXda, m_pHx, m_pDHxdp, m_pDHxdt, m_pDHxda, m_pATable, m_pPhaseId
         );

      /* Store invariant part of function */
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
      m_pEosPvtTable->SolveCubic(
         iM, EOS_FUGACITY, EOS_NOHEAT, EOS_NOPOTENTIAL, EOS_DRV_P, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset,
         m_pBp, m_pTemperature, m_pComposition, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pFx,
         m_pDXdp, m_pDXdt, m_pDXda, m_pHx, m_pDHxdp, m_pDHxdt, m_pDHxda, m_pATable, m_pPhaseId
         );

      /* Calculate y phase fugacity */
      m_pEosPvtTable->SolveCubic(
         iM, EOS_FUGACITY, EOS_NOHEAT, EOS_NOPOTENTIAL, EOS_DRV_P, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset,
         m_pBp, m_pTemperature, m_pX, m_pZy, m_pDZydp, m_pDZydt, m_pDZyda, m_pFy,
         m_pDYdp, m_pDYdt, m_pDYda, m_pHy, m_pDHydp, m_pDHydt, m_pDHyda, m_pATable, m_pPhaseId
         );

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

            dC = dG * ( ( m_pFx[iNi] + log( dB > dVeryTiny ? dB : dVeryTiny ) )
                        - ( m_pFy[iNi] + log( dA > dVeryTiny ? dA : dVeryTiny ) ) );
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
      EosLinAlg::Cholesky(  iM, iNc, m_dTiny, m_pDYda, m_pTerm3, m_pTerm2 );
      EosLinAlg::BackSolve( iM, iNc, m_pDYda, m_pDYdp );
      EosLinAlg::BackSolve( iM, iNc, m_pDYda, m_pFy );

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

   /* Invariant part of function */
   m_pEosPvtTable->SolveCubic( iM, EOS_FUGACITY, EOS_NOHEAT, EOS_NOPOTENTIAL, EOS_NODRV, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset,
                               m_pPressure, m_pTemperature, m_pComposition, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pFx,
                               m_pDXdp, m_pDXdt, m_pDXda, m_pHx, m_pDHxdp, m_pDHxdt, m_pDHxda, m_pATable, m_pPhaseId );

   /* Store invariant part of function */
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
   m_pApplication->WriteOutputData(
      &m_iDrv, &iGetViscosity, &iGetTension, &m_iHeat, &iGetMW, &m_iVolume, &m_iMolarFlash, &iMolarDensity
      );

   /* See if a bubble point possible */
   iBubble = m_iPseudoProperties && m_iBubbleDew && m_iVolume == EOS_DENSITY && iType == EOS_TOF_2P;

   /* Fix bubble point terms */
   if ( iBubble )
   {
      m_pApplication->ModifyBubble( EOS_TO_INTERNAL, EOS_FL_1P, EOS_FL_BP );
   }

   /* Two phase calculations */
   if ( iType == EOS_TOF_2P )
   {
      m_pApplication->ResetSlice();
      iM = m_pApplication->SetSlice( EOS_FL_2P, m_iFlashLength );
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
         iM = m_pApplication->SetSlice( EOS_FL_2P, m_iFlashLength );
      }
   }

   /* Single phase calculations with known bubble point */
   if ( iBubble )
   {
      m_pApplication->ResetSlice();
      iM = m_pApplication->SetSlice( EOS_FL_BP, m_iFlashLength );
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
         iM = m_pApplication->SetSlice( EOS_FL_BP, m_iFlashLength );
      }
   }

   /* Single phase calculations with a pseudo phase */
   if ( m_iPseudoProperties && m_iVolume == EOS_DENSITY && iType == EOS_TOF_2P )
   {
      m_pApplication->ResetSlice();
      iM = m_pApplication->SetSlice( EOS_FL_1P, m_iFlashLength );
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
         iM = m_pApplication->SetSlice( EOS_FL_1P, m_iFlashLength );
      }
   }

   /* Single phase calculations without a pseudo phase */
   if ( m_iVolume != EOS_DENSITY || m_iPseudoProperties == EOS_OPTION_OFF || iType != EOS_TOF_2P )
   {
      m_pApplication->ResetSlice();
      iM = m_pApplication->SetSlice( EOS_FL_1P, m_iFlashLength );
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
         iM = m_pApplication->SetSlice( EOS_FL_1P, m_iFlashLength );
      }
   }

   /* Nothing present */
   m_pApplication->ResetSlice();
   iM = m_pApplication->SetSlice( EOS_FL_0P, m_iFlashLength );
   while ( iM )
   {
      WriteData( iM, iNc, iGetViscosity, iGetTension, iMolarDensity, iType, EOS_FL_0P );
      iM = m_pApplication->SetSlice( EOS_FL_0P, m_iFlashLength );
   }

   /* Fix bubble point terms */
   if ( iBubble )
   {
      m_pApplication->ModifyBubble( EOS_TO_APPLICATION, EOS_FL_BP, EOS_FL_1P );
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
   m_pApplication->WriteOutputData(
      &m_iDrv, &iGetViscosity, &iGetTension, &m_iHeat, &iGetMW, &m_iVolume, &m_iMolarFlash, &iMolarDensity
      );

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
#if defined(__INTEL_COMPILER) && __INTEL_COMPILER < 1300 
         // With Intel 12.0.2.137 compiler without this pragma the compiler
         // generate a faulty code, by specifying this pragma the generated
         // code is corrected and the optimization working.
         #pragma distribute_point
#endif         
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
void EosPvtModel::
WriteData( int iM, int iNc, int iGetViscosity, int iGetTension, int iMolarDensity,
           int iType, int iPhase )
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
            m_pEosPvtTable->Viscosity(
               iM, m_iDrv, m_iMolarFlash, m_dTiny, m_pPressure, m_pTemperature, m_pX, m_pDXdp, m_pDXdt, m_pDXda,
               m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pMux, m_pDMuxdp, m_pDMuxdt, m_pDMuxda, m_pSumTable
               );
         }

         /* Get the parachor terms */
         if ( iGetTension )
         {
            m_pEosPvtTable->Tension(
               iM, m_iDrv, m_iMolarFlash, EOS_OPTION_OFF, m_pX, m_pDXdp, m_pDXdt, m_pDXda,
               m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pIfx, m_pDIfxdp, m_pDIfxdt, m_pDIfxda, m_pWork
               );
         }

         /* Get the mass density */
         if ( m_iMolarFlash && iMolarDensity == EOS_OPTION_OFF )
         {
            m_pEosPvtTable->MassDensity(
               iM, m_iDrv, m_pDXdp, m_pDXdt, m_pDXda, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pMWx, m_pWork
               );
         }

         /* Set results */
         m_pApplication->ReadData(
            iType, iM, iNc, m_pPhaseId, m_pX, m_pDXdp, m_pMx, m_pDMxdp, m_pZx, m_pDZxdp,
            m_pMux, m_pDMuxdp, m_pIfx, m_pDIfxdp, m_pHx, m_pDHxdp, m_pBp, m_pDBpdp, m_pMWx
            );
      }
      else
      {
         m_pApplication->ReadNull( iType, iM, m_pPhaseId );
      }
   }

   /* Two phase flash results */
   else if ( iPhase == EOS_FL_2P )
   {
      /* Get the viscosity */
      if ( iGetViscosity )
      {
         double *pP = m_iBubbleDewPoint ? m_pBp : m_pPressure;
         m_pEosPvtTable->Viscosity(
            iM, m_iDrv, m_iMolarFlash, m_dTiny, pP, m_pTemperature, m_pX, m_pDXdp, m_pDXdt, m_pDXda,
            m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pMux, m_pDMuxdp, m_pDMuxdt, m_pDMuxda, m_pSumTable
            );

         m_pEosPvtTable->Viscosity(
            iM, m_iDrv, m_iMolarFlash, m_dTiny, pP, m_pTemperature, m_pY, m_pDYdp, m_pDYdt, m_pDYda,
            m_pZy, m_pDZydp, m_pDZydt, m_pDZyda, m_pMuy, m_pDMuydp, m_pDMuydt, m_pDMuyda, m_pSumTable
            );
      }

      /* Get the parachor terms */
      if ( iGetTension )
      {
         m_pEosPvtTable->Tension(
            iM, m_iDrv, m_iMolarFlash, EOS_OPTION_ON, m_pX, m_pDXdp, m_pDXdt, m_pDXda,
            m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pIfx, m_pDIfxdp, m_pDIfxdt, m_pDIfxda, m_pWork
            );

         m_pEosPvtTable->Tension(
            iM, m_iDrv, m_iMolarFlash, EOS_OPTION_ON, m_pY, m_pDYdp, m_pDYdt, m_pDYda,
            m_pZy, m_pDZydp, m_pDZydt, m_pDZyda, m_pIfy, m_pDIfydp, m_pDIfydt, m_pDIfyda, m_pWork
            );

         m_pEosPvtTable->SetTension(
            iM, m_iDrv, m_pIfx, m_pDIfxdp, m_pDIfxdt, m_pDIfxda,
            m_pIfy, m_pDIfydp, m_pDIfydt, m_pDIfyda, m_pWork
            );
      }

      /* Get the mass density */
      if ( m_iMolarFlash && iMolarDensity == EOS_OPTION_OFF )
      {
         m_pEosPvtTable->MassDensity(
            iM, m_iDrv, m_pDXdp, m_pDXdt, m_pDXda, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pMWx, m_pWork
            );

         m_pEosPvtTable->MassDensity(
            iM, m_iDrv, m_pDYdp, m_pDYdt, m_pDYda, m_pZy, m_pDZydp, m_pDZydt, m_pDZyda, m_pMWy, m_pWork
            );
      }

      /* Set results */
      m_pApplication->ReadData(
         EOS_SINGLE_PHASE_OIL, iM, iNc, m_pPhaseId, m_pX, m_pDXdp, m_pMx, m_pDMxdp,
         m_pZx, m_pDZxdp, m_pMux, m_pDMuxdp, m_pIfx, m_pDIfxdp, m_pHx, m_pDHxdp, m_pBp, m_pDBpdp, m_pMWx
         );

      m_pApplication->ReadData(
         EOS_SINGLE_PHASE_GAS, iM, iNc, m_pPhaseId, m_pY, m_pDYdp, m_pMy, m_pDMydp,
         m_pZy, m_pDZydp, m_pMuy, m_pDMuydp, m_pIfy, m_pDIfydp, m_pHy, m_pDHydp, m_pBp, m_pDBpdp, m_pMWy
         );
   }

   /* Single phase */
   else if ( iPhase == EOS_FL_1P )
   {
      /* Get the viscosity */
      if ( iGetViscosity )
      {
         m_pEosPvtTable->Viscosity(
            iM, m_iDrv, m_iMolarFlash, m_dTiny, m_pPressure, m_pTemperature, m_pX, m_pDXdp, m_pDXdt, m_pDXda,
            m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pMux, m_pDMuxdp, m_pDMuxdt, m_pDMuxda, m_pSumTable
            );
      }

      /* Get the parachor terms */
      if ( iGetTension )
      {
         m_pEosPvtTable->Tension(
            iM, m_iDrv, m_iMolarFlash, EOS_OPTION_OFF, m_pX, m_pDXdp, m_pDXdt, m_pDXda,
            m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pIfx, m_pDIfxdp, m_pDIfxdt, m_pDIfxda, m_pWork
            );
      }

      /* Get the mass density */
      if ( m_iMolarFlash && iMolarDensity == EOS_OPTION_OFF )
      {
         m_pEosPvtTable->MassDensity(
            iM, m_iDrv, m_pDXdp, m_pDXdt, m_pDXda, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pMWx, m_pWork
            );
      }

      /* Set results */
      m_pApplication->ReadData(
         EOS_SINGLE_PHASE_OIL, iM, iNc, m_pPhaseId, m_pX, m_pDXdp, m_pMx, m_pDMxdp,
         m_pZx, m_pDZxdp, m_pMux, m_pDMuxdp, m_pIfx, m_pDIfxdp, m_pHx, m_pDHxdp,
         m_pBp, m_pDBpdp, m_pMWx
         );
      m_pApplication->ReadNull( EOS_SINGLE_PHASE_GAS, iM, m_pPhaseId );
   }

   /* No hydrocarbons */
   else
   {
      for ( i = 0; i < iM; i++ )
      {
         m_pPhaseId[i] = EOS_SINGLE_PHASE_GAS;
      }

      m_pApplication->ReadNull( EOS_SINGLE_PHASE_OIL, iM, m_pPhaseId );
      m_pApplication->ReadNull( EOS_SINGLE_PHASE_GAS, iM, m_pPhaseId );
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
      m_pEosPvtTable->MassHeat( iM, iDrvp, m_pMWx, m_pHx, m_pDHxdp, m_pDHxdt, m_pDHxda );
      m_pEosPvtTable->MassHeat( iM, iDrvp, m_pMWy, m_pHy, m_pDHydp, m_pDHydt, m_pDHyda );
   }

   /* Convert to a density */
   m_pEosPvtTable->Density( iM, iDrvp, m_iMolarFlash, m_pMWx, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda );
   m_pEosPvtTable->Density( iM, iDrvp, m_iMolarFlash, m_pMWy, m_pZy, m_pDZydp, m_pDZydt, m_pDZyda );

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
      m_pEosPvtTable->SolveCubic(
         iM, iFugacity, iGetH, EOS_NOPOTENTIAL, iDrvp, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset,
         m_pBp, m_pTemperature, m_pX, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda,
         m_pWork, m_pDXdp, m_pDXdt, m_pDXda, m_pHx, m_pDHxdp, m_pDHxdt, m_pDHxda, m_pATable, m_pPhaseId
         );

      /* Z factor for the y phase */
      m_pEosPvtTable->SolveCubic(
         iM, iFugacity, iGetH, EOS_NOPOTENTIAL, iDrvp, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset,
         m_pBp, m_pTemperature, m_pY, m_pZy, m_pDZydp, m_pDZydt, m_pDZyda,
         m_pWork, m_pDYdp, m_pDYdt, m_pDYda, m_pHy, m_pDHydp, m_pDHydt, m_pDHyda, m_pATable, m_pPhaseId
         );
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

      m_pEosPvtTable->SolveCubic(
         iM, EOS_NOFUGACITY, m_iHeat, EOS_NOPOTENTIAL, m_iDrv, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset,
         m_pPressure, m_pTemperature, m_pX, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda,
         m_pWork, m_pDYdp, m_pDXdt, m_pDYda, m_pHx, m_pDHxdp, m_pDHxdt, m_pDHxda, m_pATable, m_pPhaseId
         );

      m_pEosPvtTable->SolveCubic(
         iM, EOS_NOFUGACITY, m_iHeat, EOS_NOPOTENTIAL, m_iDrv, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset,
         m_pPressure, m_pTemperature, m_pY, m_pZy, m_pDZydp, m_pDZydt, m_pDZyda,
         m_pWork, m_pDYdp, m_pDXdt, m_pDYda, m_pHy, m_pDHydp, m_pDHydt, m_pDHyda, m_pATable, m_pPhaseId
         );
   }

   /* Convert to a volume */
   m_pEosPvtTable->Volume(
      iM, iDrvp, m_iMultipleAbc, m_pAbcOffset, pP, m_pTemperature, m_pX,
      m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pWork
      );

   m_pEosPvtTable->Volume(
      iM, iDrvp, m_iMultipleAbc, m_pAbcOffset, pP, m_pTemperature, m_pY,
      m_pZy, m_pDZydp, m_pDZydt, m_pDZyda, m_pWork
      );
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
      EosLinAlg::Cholesky( iM, iNc, m_dTiny, m_pDYda, m_pTerm3, m_pTerm2 );

      /* Solve matrix for pressure derivatives */
      EosLinAlg::BackSolve( iM, iNc, m_pDYda, m_pDYdp );

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
         EosLinAlg::BackSolve( iM, iNc, m_pDYda, pTa );

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
            EosLinAlg::BackSolve( iM, iNc, m_pDYda, m_pDYdt );

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
      EosLinAlg::Cholesky( iM, iNc, m_dTiny, m_pDYda, m_pTerm3, m_pTerm2 );

      /* Solve matrix for pressure derivatives */
      EosLinAlg::BackSolve( iM, iNc, m_pDYda, m_pDYdp );

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
         EosLinAlg::BackSolve( iM, iNc, m_pDYda, pTa );

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
            EosLinAlg::BackSolve( iM, iNc, m_pDYda, m_pDYdt );

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
   m_pEosPvtTable->SolveCubic(
      iM, EOS_NOFUGACITY, m_iHeat, EOS_NOPOTENTIAL, m_iDrv, EOS_PHASEID, m_iMultipleAbc, m_pAbcOffset,
      m_pPressure, m_pTemperature, m_pComposition, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pWork,
      m_pDXdp, m_pDXdt, m_pDXda, m_pHx, m_pDHxdp, m_pDHxdt, m_pDHxda, m_pATable, m_pPhaseId
      );

   /* Perform the volume translations */
   m_pEosPvtTable->Volume(
      iM, m_iDrv, m_iMultipleAbc, m_pAbcOffset, m_pPressure, m_pTemperature, m_pComposition,
      m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pWork
      );

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
      m_pEosPvtTable->MassHeat( iM, m_iDrv, m_pMWx, m_pHx, m_pDHxdp, m_pDHxdt, m_pDHxda );
   }

   /* Convert to a density */
   m_pEosPvtTable->Density( iM, m_iDrv, m_iMolarFlash, m_pMWx, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda );

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
   m_pEosPvtTable->SolveCubic(
      iM, EOS_NOFUGACITY, m_iHeat, EOS_NOPOTENTIAL, m_iDrv, EOS_PHASEID, m_iMultipleAbc, m_pAbcOffset,
      m_pPressure, m_pTemperature, m_pX, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pWork,
      m_pDXdp, m_pDXdt, m_pDXda, m_pHx, m_pDHxdp, m_pDHxdt, m_pDHxda, m_pATable, m_pPhaseId
      );

   /* Get Wilson k values */
   m_pEosPvtTable->WilsonKValues( iM, EOS_NOSCALEK, m_pTemperature, m_pKValue, m_pWork );

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

#if defined(__INTEL_COMPILER) && __INTEL_COMPILER < 1300 
      // With Intel 12.1.6.233 compiler and without this pragma the compiler
      // generate a poorly accurate code, by specifying this pragma the generated
      // code is more accurate and the optimization is still enabled.
      #pragma distribute_point
#endif         
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
   m_pEosPvtTable->SolveCubic(
      iM, EOS_NOFUGACITY, m_iHeat, EOS_NOPOTENTIAL, m_iDrv, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset,
      m_pPressure, m_pTemperature, m_pY, m_pZy, m_pDZydp, m_pDZydt, m_pDZyda, m_pWork,
      m_pDYdp, m_pDYdt, m_pDYda, m_pHy, m_pDHydp, m_pDHydt, m_pDHyda, m_pATable, m_pPhaseId
      );

   /* Perform the volume translations */
   m_pEosPvtTable->Volume(
      iM, m_iDrv, m_iMultipleAbc, m_pAbcOffset, m_pPressure, m_pTemperature, m_pX,
      m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pWork
      );

   m_pEosPvtTable->Volume(
      iM, m_iDrv, m_iMultipleAbc, m_pAbcOffset, m_pPressure, m_pTemperature, m_pY,
      m_pZy, m_pDZydp, m_pDZydt, m_pDZyda, m_pWork
      );

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
      m_pEosPvtTable->MassHeat( iM, m_iDrv, m_pMWx, m_pHx, m_pDHxdp, m_pDHxdt, m_pDHxda );
      m_pEosPvtTable->MassHeat( iM, m_iDrv, m_pMWy, m_pHy, m_pDHydp, m_pDHydt, m_pDHyda );
   }

   /* Convert to a density */
   m_pEosPvtTable->Density( iM, m_iDrv, m_iMolarFlash, m_pMWx, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda );
   m_pEosPvtTable->Density( iM, m_iDrv, m_iMolarFlash, m_pMWy, m_pZy, m_pDZydp, m_pDZydt, m_pDZyda );

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
                     *pTa = *pTa / ( dA * m_pMoles[i] );
                     *pTc = *pTc **pTd++ / ( dA * m_pSplit[i] );
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
   m_pEosPvtTable->Volume(
      iM, m_iDrv, m_iMultipleAbc, m_pAbcOffset, m_pPressure, m_pTemperature, m_pX,
      m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pWork
      );

   m_pEosPvtTable->Volume(
      iM, m_iDrv, m_iMultipleAbc, m_pAbcOffset, m_pPressure, m_pTemperature,
      m_pY, m_pZy, m_pDZydp, m_pDZydt, m_pDZyda, m_pWork
      );

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
//     wrong sign that [ dNydP / S ] will have the correct sign
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

   m_pEosPvtTable->SolveCubic(
      iM, iFugacity, m_iHeat, EOS_NOPOTENTIAL, m_iDrv, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset,
      m_pPressure, m_pTemperature, m_pX, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pWork,
      m_pDXdp, m_pDXdt, m_pDXda, m_pHx, m_pDHxdp, m_pDHxdt, m_pDHxda, m_pATable, m_pPhaseId
      );

   /* Chemical potential for the y phase */
   m_pEosPvtTable->SolveCubic(
      iM, iFugacity, m_iHeat, EOS_NOPOTENTIAL, m_iDrv, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset,
      m_pPressure, m_pTemperature, m_pY, m_pZy, m_pDZydp, m_pDZydt, m_pDZyda, m_pWork,
      m_pDYdp, m_pDYdt, m_pDYda, m_pHy, m_pDHydp, m_pDHydt, m_pDHyda, m_pATable, m_pPhaseId
      );

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
            for ( iNi = 0; iNi < iNc; iNi++ )
            {
               m_pDXdp[iNi] = m_pPotentialx[iNi] * dC * ( m_pDXdp[iNi] - m_pDYdp[iNi] );
            }

            if ( m_iDrv >= EOS_DRV_T )
            {
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
      EosLinAlg::Cholesky( iM, iNc, m_dTiny,  m_pDXda, m_pTerm3, m_pTerm2 );

      /* Backsolve matrices multiple grid block */
      if ( iM > 1 )
      {
         pTf = m_pDYda;
         for ( iNi = 0; iNi < iNc; iNi++ )
         {
            EosLinAlg::BackSolve( iM, iNc, m_pDXda, pTf );
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
            EosLinAlg::BackSolve( iM, iNc, m_pDXda, m_pDXdp );
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
               EosLinAlg::BackSolve( iM, iNc, m_pDXda, m_pDXdt );
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
            for ( i = 0; i < iM; i++ )
            {
               dA = *pTf;
               *pTd++ = dA;
               *pTf++ = -m_pPotentialx[i] * dA;
            }
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
            EosLinAlg::BackSolve( 1, iNc, m_pDXda, pTf );
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
            EosLinAlg::BackSolve( 1, iNc, m_pDXda, m_pDXdp );
            for ( iNj = 0; iNj < iNc; iNj++ )
            {
               m_pDXdp[iNj] *= m_pPotentialx[iNj];
            }

            /* Temperature derivatives */
            if ( m_iDrv >= EOS_DRV_T )
            {
               EosLinAlg::BackSolve( 1, iNc, m_pDXda, m_pDXdt );
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
// 1) First set the phase identifications and the molecular weights
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
      m_pEosPvtTable->MassHeat( iM, m_iDrv, m_pMWx, m_pHx, m_pDHxdp, m_pDHxdt, m_pDHxda );
      m_pEosPvtTable->MassHeat( iM, m_iDrv, m_pMWy, m_pHy, m_pDHydp, m_pDHydt, m_pDHyda );
   }

   /* Convert to a density */
   m_pEosPvtTable->Density( iM, m_iDrv, m_iMolarFlash, m_pMWx, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda );
   m_pEosPvtTable->Density( iM, m_iDrv, m_iMolarFlash, m_pMWy, m_pZy, m_pDZydp, m_pDZydt, m_pDZyda );

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
         TwoPhaseDerivatives(
            iM, iNc, m_pDXdp, m_pDYdp, m_pDZxdp, m_pDZydp, m_pDHxdp, m_pDHydp, m_pDMxdp, m_pDMydp
            );

         /* Now for temperature derivatives */
         if ( m_iDrv >= EOS_DRV_T )
         {
            TwoPhaseDerivatives(
               iM, iNc, m_pDXdt, m_pDYdt, m_pDZxdt, m_pDZydt, m_pDHxdt, m_pDHydt, m_pDMxdt, m_pDMydt
               );
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
void EosPvtModel::
TwoPhaseDerivatives( int iM, int iNc, double *pDX, double *pDY, double *pDZx, double *pDZy,
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

#if defined(__INTEL_COMPILER) && __INTEL_COMPILER < 1300 
         // with intel 11.1 this loop crashes the program when it's vectorized
         // it appears that the register which contains iNi is not properly
         // initilized to 1. That's should reflect a compiler issue.
         // Unrolling the loop prevents it and keeps optimization on.
         #pragma unroll(4)
#endif
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
               for ( i = 0; i < iM; i++ )
               {
                  pTa[i] += *pTb++ **pTc++;
               }
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

         for ( i = 0; i < iM; i++ )
         {
            m_pWork[i] = 1.0 / m_pMWx[i];
         }

         iNj = iNc * iNc;
         pTe = m_pDXda;
         for ( iNi = 0; iNi < iNj; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               pTe[i] *= m_pWork[i];
            }

            pTe += iM;
         }

         for ( i = 0; i < iM; i++ )
         {
            m_pWork[i] = 1.0 / m_pMWy[i];
         }

         pTe = m_pDYda;
         for ( iNi = 0; iNi < iNj; iNi++ )
         {
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
            for ( iNj = 1; iNj < iNc; iNj++ )
            {
               dA = m_pMolecularWeight[iNj] / dB;
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
            for ( iNj = 0; iNj < iNc; iNj++ )
            {
               pTe[iNj] = ( pTe[iNj] - m_pX[iNj] * dA ) * dD;
               pTf[iNj] = ( pTf[iNj] - m_pY[iNj] * dB ) * dE;
            }

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
               for ( i = 0; i < iM; i++ )
               {
                  dB = pTb[i];
                  m_pDZxdp[i] -= pTa[i] * pTc[i];
                  m_pDZydp[i] += dB * pTd[i];
                  m_pDMydp[i] += dB;
               }
            }

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
   m_pApplication->WriteOutputData( &m_iDrv, &iGetViscosity, &iGetTension, &m_iHeat, &iGetMW, &m_iVolume, &m_iMolarFlash, &iMolarDensity );
   iMolarDensity = iMolarDensity && m_iMolarFlash;

   /* Get number of components */
   iSalts = m_pEosPvtTable->GetNumberSalts();
   m_iWaterComp = m_pApplication->WriteWaterIndex();
   iNc = iSalts + ( ( m_iWaterComp >= 0 ) ? 1 : 0 );

   /* Start anew if not using current */
   if ( iUseCurrent )
   {
      iM = 1;
   }
   else
   {
      m_pApplication->ResetSlice();
      iM = m_pApplication->SetSlice( EOS_FL_AP, m_iFlashLength );
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
         m_pEosPvtTable->Water( iM, iNc, m_iDrv, m_iVolume, iGetViscosity, iGetTension, m_iHeat, iTherm, m_pPressure, m_pTemperature, m_pSalinity, m_pDSda, m_pRhoW, m_pDRhoWdp, m_pDRhoWdt, m_pDRhoWds, m_pDRhoWda, m_pMuW, m_pDMuWdp, m_pDMuWdt, m_pDMuWds, m_pDMuWda, m_pIftW, m_pDIftWdp, m_pDIftWdt, m_pDIftWda, m_pHeatW, m_pDHeatWdp, m_pDHeatWdt, m_pDHeatWds, m_pDHeatWda, m_pWorkW );

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
      m_pApplication->ReadData( EOS_WATER, iM, iNc, m_pPhaseId, m_pW, m_pDWdp, m_pMW, m_pDMWdp, m_pRhoW, m_pDRhoWdp, m_pMuW, m_pDMuWdp, m_pIftW, m_pDIftWdp, m_pHeatW, m_pDHeatWdp, pDummy, pDummy, m_pMWW );

      /* Next slice */
      if ( iUseCurrent )
      {
         iM = 0;
      }
      else
      {
         iM = m_pApplication->SetSlice( EOS_FL_AP, m_iFlashLength );
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
   m_pApplication->WriteOutputData( &iTemp, &iTemp, &iTemp, &m_iHeat, &iTemp, &iTemp, &iMolar, &iTemp );

   /* Set the gravity constant in internal units */
   m_pEosPvtTable->GetGravity( &m_dGravity, &m_dGascon );

   /* Set pointer to the starting table entry */
   m_pApplication->SetTableEntry( EOS_INIT_START );

   /* Read the compositional grading data */
   ReadGradingData( iNc, EOS_INIT_COMP );

   /* Adjust to the bubble point */
   GradingBubblePoint( iNc, iType, EOS_NOFORCEBP, EOS_RETROGRADE );

   /* Tell Application that this is a GOC if two phases present */
   if ( *m_pPhase == EOS_FL_2P_CV )
   {
      m_pApplication->ReadGOC( m_dDepthx );
   }

   /* Write out results */
   WriteGradingData( iNc, iMolar );

   /* Go one direction */
   while ( m_pApplication->SetTableEntry( EOS_INIT_UP ) )
   {
      /* Iterate using Newton's method */
      CompositionalGradingDo( iNc, iType, &iMove );

      /* Write out results */
      WriteGradingData( iNc, iMolar );
   }

   /* Set pointer again to the starting table entry */
   m_pApplication->SetTableEntry( EOS_INIT_START );

   /* Read the compositional grading data */
   ReadGradingData( iNc, EOS_INIT_COMP );

   /* Adjust to the bubble point */
   GradingBubblePoint( iNc, iType, EOS_NOFORCEBP, EOS_RETROGRADE );

   /* Go other direction */
   while ( m_pApplication->SetTableEntry( EOS_INIT_DOWN ) )
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
   m_pApplication->WriteGradingData(
      iDirection, &m_dDepthx, m_pPressure, m_pTemperature, m_pComposition, &isothermal, &iMolar
      );

   /* Set the pvt term assignments */
   m_pEosPvtTable->WritePvtInfo(
      1, 0, 1, isothermal, EOS_OPTION_OFF, 0, m_pAbcOffset, &m_iMultipleAbc, m_pTemperature, m_pMolecularWeight
      );

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
      m_pApplication->ReadGradingData(
         m_dDepthx, *m_pTemperature, *m_pPressure, m_dPressurex, m_dPressurey,
         *m_pZx, *m_pZy, *m_pHx, *m_pHy, m_pComposition, m_pLastx, m_pLasty
         );
   }
   else
   {
      m_pApplication->ReadGradingData(
         m_dDepthx, *m_pTemperature, *m_pPressure, m_dPressurey, m_dPressurex,
         *m_pZy, *m_pZx, *m_pHy, *m_pHx, m_pComposition, m_pLasty, m_pLastx
         );
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
   m_pEosPvtTable->SolveCubic(
      1, EOS_NOFUGACITY, m_iHeat, EOS_NOPOTENTIAL, EOS_NODRV, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset,
      pP, m_pTemperature, pComp, pZfactor, m_pDZxdp, m_pDZxdt, m_pDZxda,
      m_pPotentialx, m_pDXdp, m_pDXdt, m_pDXda, pHt, m_pDHxdp, m_pDHxdt, m_pDHxda, m_pATable, m_pPhaseId
      );

   /* Perform the volume translations */
   m_pEosPvtTable->Volume(
      1, EOS_NODRV, m_iMultipleAbc, m_pAbcOffset, pP, m_pTemperature, pComp,
      pZfactor, m_pDZxdp, m_pDZxdt, m_pDZxda, m_pWork
      );

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
         NewtonFlashOneObject( iNc, EOS_NORESTORE, EOS_FL_2P_NCV );
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
         m_pApplication->ReadGOC( m_dDepthy );
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
         m_pApplication->ReadGOC( m_dDepthx );
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


////////////////////////////////////////////////////////////////////////////////
///
/// @brief   Routine to take Newton iterations for compositional grading
///
/// @param   iNc Number of components
///
/// 1) Load properties from the previous step.
/// 2) Determine whether enthalpy calculations are
///    required.  The enthalpy is required if thermal
///    diffusion is used AND the temperature is not
///    constant between steps.
/// 3) Calculate the chemical potential at the initial guess
/// 4) Calculate the thermal diffusion term if required
/// 5) Restore current properties
/// 6) Calculate the constant terms used in the iterations
///    At this point we will describe the system to be solved.
///    The equation to be solved is
///
///    dMu  = - M g dH - K  dlnT
///       i      i        i
///
///    where Mu is the chemical potential, M is the molecular
///    weight, and K is the thermal diffusion term.  The
///    thermal diffusion term is based upon the work of
///    Kempers (L.J.T.M Kempers, "A thermodynamic theory of the Soret
///    effect in a multicomponent liquid", J. Chem. Phys, 90,
///    June 1989.), while the basic form of the equation is
///    due to Schulte (A.M. Schulte, "Compositional Variations
///    within a Hydrocarbon Column Due to Gravity", SPE 9235,
///    1980)
/// 7) This differential equation can be integrated to
///    yield
///
///      +     -          +    -              +    -
///    Mu  = Mu  - M g ( H  - H  ) - K  ln ( T  / T  )
///      i     i    i                 i
///
///    The thermal diffusion term will always be lagged
///    at the last level.  Also, the thermal diffusion
///    can be tuned by the same constant for all components
/// 8) The term dMu depends upon pressure, composition,
///    and temperature.  The temperature field is known,
///    and thus the temperature dependence in the differential
///    of the chemical potential must be estimated.  It
///    is estimated in a second order correct way, namely,
///
///                    +   +      -   -           +    -
///    dMu/dT = 1/2 ( T dMu/dT + T dMu/dT ) ln ( T  / T  )
///       i                i          i
///
///    Note that in the iteration scheme these terms
///    are lagged by one iteration at the new level.  Actually,
///    since the temperature derivative of the chemical potential
///    is the non-ideal partial entropy, we are doing a midpoint
///    rule for estimating S for S dT
/// 9) Upon expansion we get
///
///          +         +        -     +
///    Sum Mu  dX  + Mu  dP = Mu  - Mu  - M g dH - AdlnT
///     j    ij  j     ip       i     i    i
///
/// 10) Note, however, that this condition means that there
///     are more unknowns than equations.  The appropriate
///     additional constraint is that the sum of the mole
///     fractions at the next level must be unity.  This
///     is ensured since
///
///     Sum dMu  = V dP = - MW g dH
///      i     i
///
///     and thus
///
///     dP = - rho g dH
///
///     meaning that the system is in mechanical equilibrium.
///     Also, the thermal diffusion terms sum to zero when
///     performing the sum over all components
/// 11) Use as an initial guess at the new level the current
///     composition and the pressure
/// 12) When forming the Jacobian matrix we choose the
///     square root of the mole fraction divided by a
///     scaling term involving units to ensure that
///     the matrix will be the identity matrix for the
///     ideal gas case
/// 13) The equations can be written in the form
///
///     A P dX = G
///     1 0 dP   H
///
///     The pressure derivatives appearing in the matrix
///     P are obtained by multiplying the standard
///     pressure derivatives by the current pressure,
///     which produces the derivative wrt the log of the
///     pressure.  But instead of this, we will use as
///     primary unknowns PX (the partial pressure) and
///     the pressure.  Thus the equations will be
///
///     A  P dPX = G
///     1 -1 dP    H
///
/// 14) By choosing a scaling factor of S = sqrt ( PX / RT )
///     above equation is rewritten as
///
///             -1
///     SAS SP S  dPX = SG
///      S  -1    dP     H
///
///     In this case the matrix SAS will have the property
///     that it will be a diagonal matrix for an ideal gas
/// 15) Let
///
///          -1
///     y = S  dPX
///
///     Then solve
///
///     SAS z = SG and SAS w = SP
///
///     The equations are then of the form
///
///     I   w   y  = z
///     S  -1  dP    H
///
/// 16) Now solve for dP.  We have
///
///     dP = ( Sz - H ) / ( 1 + Sw )
///
///     The solution is not allowed to grow more than one log
///     cycle.  The equations are now of the form
///
///     I  w  y   = z
///     0  1  dP    dP
///
///     and thus
///
///     y = z - w dP
///
/// 17) dPX is not used; rather dlnPX is used.  Thus
///
///     dlnPX = S y / PX = y / S
///
///     New partial pressures can then be set.
/// 18) The iterations are stopped when the pressure
///     has converged.  As noted above the system is then
///     in gravitational equilibrium owing to the definition
///     of the chemical potential
///
////////////////////////////////////////////////////////////////////////////////
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
      m_pEosPvtTable->ThermalDiffusion(
         1, dT, m_dThermalDiffusion, m_pZx, m_pDZxda, m_pHx, m_pDHxda, m_pPotentialy
         );
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
      EosLinAlg::Cholesky ( 1, iNc, m_dTiny, m_pDXda, m_pTerm3, m_pTerm2 );
      EosLinAlg::BackSolve( 1, iNc, m_pDXda, m_pDXdp );
      EosLinAlg::BackSolve( 1, iNc, m_pDXda, m_pDXdt );

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


////////////////////////////////////////////////////////////////////////////////
///
/// @brief   Routine to get the chemical potentials
///
/// @param   iNc Number of components
/// @param   iDrvt Indicator for derivative level.  Either
///                EOS_NODRV - isothermal case
///                EOS_DRV_T - thermal case
/// @param   iGetH Indicator as to whether we need enthalpy for thermal
///                diffusion
///
/// 1) Call SolveCubic to get the chemical potential
/// 2) Add the term RT log PXi
///
////////////////////////////////////////////////////////////////////////////////
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
   m_pEosPvtTable->SolveCubic(
      1, EOS_FUGACITY, iGetH, EOS_POTENTIAL, iDrvt, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset,
      m_pPressure, m_pTemperature, m_pY, m_pZx, m_pDZxdp, m_pDZxdt, m_pDZxda,
      m_pPotentialx, m_pDXdp, m_pDXdt, m_pDXda, m_pHx, m_pDHxdp, m_pDHxdt, m_pDHxda, m_pATable, m_pPhaseId
      );

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


////////////////////////////////////////////////////////////////////////////////
///
/// @brief   Routine to perform a surface flash through a separator train
///
/// @param   iNc       Number of hydrocarbon components
/// @param   iType     Type of hydrocarbon phases that can be present.  If
///                    EOS_TOF_2P, two phases can be present
/// @param   iWater    Indicator if a water phase is present
/// @param   iFlash    Indicator whether we are to flash
/// @param   pPvtTable Array of pointers to pvt tables for each separator
///
/// 1) The following type of system is modeled
///
///          [stock tank oil]
///                  ^
///                  |
///    More feed--> [ ] -----------------+
///                  ^                   |
///              oil |                   |
///                  |     gas           |
///    More feed--> [ ] -----------------+
///                  ^                   |
///              oil |                   |
///                  |     gas           |
///    Feed ------> [ ] ------------------->[stock tank gas]
///
/// 1) For each separator...
/// 2) First read in the previous values present in the oil
///    phase, and add in the new feed.  Of course, for the
///    first separator, there is only feed
/// 3) Then flash the mixture, obtaining only mole fractions,
///    except at the last stage
/// 4) Move on to the next separator
/// 5) Now for a pass in the other direction.  We must gather
///    the gas, and form the volumetric terms.  The computations
///    are different for a system that can have two hydrocarbon
///    phases and a single phase hydrocarbon system
/// 6) Finally, add in the water properties
////////////////////////////////////////////////////////////////////////////////
void EosPvtModel::SeparatorFlash( int iNc, int iType, int iWater, int iFlash, EosPvtTable **pPvtTable )
{
   int iSep;
   int iCount;
   int iThermal;

   /* See which simulator data needed */
   m_pApplication->WriteOutputData(
      &m_iDrv, &iCount, &iCount, &m_iHeat, &iCount, &m_iVolume, &m_iMolarFlash, &iCount
      );

   /* Reset the slice */
   iSep = m_pApplication->SetSeparator( EOS_INIT_START );

   /* If anything to do */
   iCount = 0;
   while ( iSep >= 0 )
   {
      iCount++;

      /* Set the pvt table */
      m_pEosPvtTable = pPvtTable[iSep];

      /* Load previous values */
      m_pApplication->ModifySeparatorAccumulations();

      /* If hydrocarbon present */
      if ( iType != EOS_TOF_0P )
      {
         /* Set the phase identification flag */
         m_pApplication->ModifyOnePhaseIdentification( m_dEnorm );

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
            m_pApplication->ModifyPhaseIndicator( EOS_OPTION_OFF );
            *m_pPhase = ( *m_pPhase + 1 ) / 2;
         }

         /* Calculate and store properties if requested */
         PropertiesOneObject( iNc, iType );
      }

      /* Reset the slice */
      iSep = m_pApplication->SetSeparator( EOS_INIT_UP );
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
   m_pApplication->ReadVolumeControl( m_iVolume );
}


////////////////////////////////////////////////////////////////////////////////
///
/// @brief   Load separator gas
///
/// @param   iNc       Number of components
/// @param   pPvtTable Array of pointers to pvt tables for each separator
///
/// 1) Ensure that the routine is going to calculate
///    both volumes and total moles/mass of each component
/// 2) We get the oil density from the stack.
///    However, we might need to interchange phases
/// 3) First make a pass through all separators to
///    obtain the gas which was produced.
/// 4) We already have calculated the oil, so load it
///    Note that the oil at the last stage is the volume
///    and partial volumes with respect to the feed, i.e.,
///
///    V      = Sum V        ( O      + F    )
///     oil,n    i   oil,i,n    i,n-1    i,n
///
///    where O represents the oil from the last stage and
///    F is the feed at the last stage
///
/// 5) Form the partial volumes for the gas.  Thus, at the
///    last stage, then
///
///    V      = Sum V        ( O      + F    )
///     gas,n    i   gas,i,n    i,n-1    i,n
///
///    where Q is the total feed into all separators
///
/// 6) Then go backwards through the other separators.
///    At each stage j we have
///
///    O    = M  ( F  + O    )
///     j+1    j    j    j-1
///
///    where M is simply the derivative matrix of the total mass or
///    moles of components of the oil phase with respect to
///    the feed + previous oil.
///
/// 7) We can telescope the sums so that at the first stage
///
///    V      = Sum ( Prod M  ) V         F
///     oil,1    i      j   j    oil,i,n    i,1
///
///    Thus
///
///    V      = Sum V         F
///     oil,1    i   oil,i,n    i,1
///
/// 8) The products for gas are similar
///
///    V      = Sum ( I - Prod M  ) V         F
///     gas,1    i          j   j    gas,i,n   i,1
///
///    Thus
///
///    V      = Sum V         F
///     gas,1    i   gas,i,n    i,1
///
/// 9) At each stage of this process we must write the results
///    to Application
///
////////////////////////////////////////////////////////////////////////////////
void EosPvtModel::SeparatorGas( int iNc, EosPvtTable **pPvtTable )
{
   double  dA;
   double  dB;
   double *pTa;
   int     iNi;
   int     iNj;
   int     iSep;

   /* Reset the volume indicator */
   m_pApplication->ReadVolumeControl( EOS_STOCK_TANK );

   /* Reset the slice */
   iSep = m_pApplication->SetSeparator( EOS_INIT_START );

   /* Load previous values */
   m_pApplication->WriteSeparatorInfo( m_pY, m_pZx, m_pDZxda );

   /* If anything to do */
   while ( m_pApplication->SetSeparator( EOS_INIT_UP ) >= 0 )
   {
      /* Load previous values */
      m_pApplication->WriteSeparatorInfo( m_pX, m_pZx, m_pDZxda );

      /* Add to total gas */
      for ( iNi = 0; iNi < iNc; iNi++ )
      {
         m_pY[iNi] += m_pX[iNi];
      }
   }

   /* Reset the slice to end */
   iSep = m_pApplication->SetSeparator( EOS_INIT_END );

   /* Set the pvt table */
   m_pEosPvtTable = pPvtTable[iSep];

   /* Load separator oil derivatives */
   m_pApplication->WriteSeparatorDrv( EOS_OIL, m_pDXda );

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
      m_pEosPvtTable->SolveCubic(
         1, EOS_NOFUGACITY, EOS_NOHEAT, EOS_NOPOTENTIAL, m_iDrv, EOS_NOPHASEID, m_iMultipleAbc, m_pAbcOffset,
         m_pPressure, m_pTemperature, m_pComposition, m_pZy, m_pDZydp, m_pDZydt, m_pDZyda, m_pWork,
         m_pDYdp, m_pDYdt, m_pDYda, m_pHy, m_pDHydp, m_pDHydt, m_pDHyda, m_pATable, m_pPhaseId
         );

      /* Perform the volume translations */
      m_pEosPvtTable->Volume(
         1, m_iDrv, m_iMultipleAbc, m_pAbcOffset, m_pPressure, m_pTemperature, m_pComposition,
         m_pZy, m_pDZydp, m_pDZydt, m_pDZyda, m_pWork
         );

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
   m_pApplication->WriteSeparatorDrv( EOS_GAS, m_pDYda );

   /* Store the data */
   *m_pPhaseId = EOS_SINGLE_PHASE_GAS;

   m_pApplication->ReadData(
      EOS_SINGLE_PHASE_OIL, 1, iNc, m_pPhaseId, m_pY, m_pDYdp, m_pMy, m_pDMydp,
      m_pZy, m_pDMydp, m_pMuy, m_pDMuydp, m_pIfy, m_pDIfydp,
      m_pHy, m_pDHydp, m_pBp, m_pDBpdp, m_pMWy
      );

   /* Reset the volume indicator */
   m_pApplication->ReadVolumeControl( EOS_VOLUME );

   /* While anything more to do */
   while ( m_pApplication->SetSeparator( EOS_INIT_DOWN ) >= 0 )
   {
      /* Load next compositions */
      m_pApplication->WriteSeparatorDrv( EOS_OIL, m_pDXda );

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
      m_pApplication->ReadData(
         EOS_SINGLE_PHASE_OIL, 1, iNc, m_pPhaseId, m_pY, m_pDYdp, m_pMy, m_pDMydp,
         m_pZy, m_pDMydp, m_pMuy, m_pDMuydp, m_pIfy, m_pDIfydp,
         m_pHy, m_pDHydp, m_pBp, m_pDBpdp, m_pMWy
         );

      m_pApplication->ReadData(
         EOS_SINGLE_PHASE_GAS, 1, iNc, m_pPhaseId, m_pX, m_pDXdp, m_pMx, m_pDMxdp,
         m_pZx, m_pDMxdp, m_pMux, m_pDMuxdp, m_pIfx, m_pDIfxdp,
         m_pHx, m_pDHxdp, m_pBp, m_pDBpdp, m_pMWx
         );
   }
}


////////////////////////////////////////////////////////////////////////////////
///
/// @brief   Volume calculations for a single phase hydrocarbon
///
/// @param   iNc Number of hydrocarbons
///
///  1) The flash results from the last separator are already
///    known.  Copy these to other separators in the train
///
////////////////////////////////////////////////////////////////////////////////
void EosPvtModel::SeparatorHydrocarbon( int iNc )
{
   /* Move to the first separator */
   /*int iSep =*/
   m_pApplication->SetSeparator( EOS_INIT_END );

   /* Set volume control to volume only */
   m_pApplication->ReadVolumeControl( EOS_VOLUME );

   /* Load previous values */
   m_pApplication->WriteSeparatorInfo( m_pX, m_pZx, m_pDZxda );

   /* Save the data for other separators */
   *m_pPhaseId = EOS_SINGLE_PHASE_OIL;
   while ( m_pApplication->SetSeparator( EOS_INIT_DOWN ) >= 0 )
   {
      m_pApplication->ReadData(
         EOS_SINGLE_PHASE_OIL, 1, iNc, m_pPhaseId, m_pX, m_pDXdp, m_pMx, m_pDMxdp,
         m_pZx, m_pDZxdp, m_pMux, m_pDMuxdp, m_pIfx, m_pDIfxdp,
         m_pHx, m_pDHxdp, m_pBp, m_pDBpdp, m_pMWx
         );
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
   m_pApplication->ReadVolumeControl( EOS_STOCK_TANK );

   /* Move to last separator */
   iSep = m_pApplication->SetSeparator( EOS_INIT_END );

   /* Set the pvt table */
   m_pEosPvtTable = pPvtTable[iSep];

   /* Initialize the properties */
   WaterProperties( EOS_OPTION_ON, EOS_OPTION_ON );

   /* Set volume control to volume only */
   m_pApplication->ReadVolumeControl( EOS_VOLUME );

   /* Loop over other data */
   while ( m_pApplication->SetSeparator( EOS_INIT_DOWN ) >= 0 )
   {
      /* Update water properties */
      WaterProperties( EOS_OPTION_ON, EOS_OPTION_OFF );
   }
}
