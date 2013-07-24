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
*  Include file for Cauldron classes is EosCauldron.h                *
*  Source module for Cauldron implementation is EosCauldron.cpp      *
*                                                                    *
*********************************************************************/
#include "stdafx.h"

#include <math.h>
#include <malloc.h>
#include <stdlib.h>

#include "EosCauldron.h"
#include "EosPvtModel.h"
#include "EosPvtTable.h"

// Construction of pvttable using the Cauldron brew
//
// Note: All input should be in MKS SI units
// 1) Generate an instance of the pvt table from supplied data
EosPvtTable *EosCauldron::ConcoctBrew ( int iNc,            // Number of components
                                        int isRK,           // Set to 1 for Redlich Kwong; otherwise Peng Robinson correct
                                        double *pvtData,
                                        double *pT,         // default temperature in SI units
                                        double *pLohrenz,   // Lohrenz tuning terms
                                        double dCritAOverB,
                                        int    iPhaseMethod // Methods for labeling a single phase
                                      )
{
   double  pdTables[EOS_METHOD_LAST_DOUBLE];
   int     piTables[EOS_METHOD_LAST_INTEGER];
   /* Default temperature SI units */
   *pT = 373.16;

   /* Initialize integers for PVT */
   if ( isRK )
   {
      piTables[EOS_METHOD] = EOS_PVT_SRK;
   }
   else
   {
      piTables[EOS_METHOD] = EOS_PVT_PRCORR;
   }
   piTables[EOS_METHOD_COMPONENTS] = iNc;
   piTables[EOS_METHOD_PHASEID]    = iPhaseMethod;
   piTables[EOS_METHOD_NUMBERABC]  = 0;
   piTables[EOS_METHOD_HEATDATA]   = EOS_OPTION_OFF;
   piTables[EOS_METHOD_OWNMEMORY]  = EOS_OPTION_ON;
   piTables[EOS_METHOD_SALTS]      = 0;
   piTables[EOS_METHOD_WATERMODEL] = EOS_WATER_CORRELATIONS;
   piTables[EOS_METHOD_TYPE_OF_C]  = EOS_CONSTANT_C;

   /* Initialize doubles for PVT tables */
   if ( isRK )
   {
      pdTables[EOS_METHOD_KB1] = 1.0;
      pdTables[EOS_METHOD_KB2] = 0.0;
   }
   else
   {
      pdTables[EOS_METHOD_KB1] = 1.0 + sqrt ( 2.0 );
      pdTables[EOS_METHOD_KB2] = 1.0 - sqrt ( 2.0 );
   }
   pdTables[EOS_METHOD_SALINITY]    = 20000.0;
   pdTables[EOS_METHOD_WATERIFT]    = 35.0;
   pdTables[EOS_METHOD_PRESCONV]    = 1.0;
   pdTables[EOS_METHOD_TEMPCONV]    = 1.0;
   pdTables[EOS_METHOD_VOLCONV]     = 1.0; 
   pdTables[EOS_METHOD_VISCCONV]    = 1.0; 
   pdTables[EOS_METHOD_DENSCONV]    = 1.0; 
   pdTables[EOS_METHOD_HEATCONV]    = 1.0; 
   pdTables[EOS_METHOD_TENSCONV]    = 1.0; 
   pdTables[EOS_METHOD_CRITZMINUSB] = 0.25;
   pdTables[EOS_METHOD_CRITAOVERB]  = dCritAOverB;
   pdTables[EOS_METHOD_VTUNE]       = 0.0;
   pdTables[EOS_METHOD_VTUNE0]      = pLohrenz[0];
   pdTables[EOS_METHOD_VTUNE1]      = pLohrenz[1];
   pdTables[EOS_METHOD_VTUNE2]      = pLohrenz[2];
   pdTables[EOS_METHOD_VTUNE3]      = pLohrenz[3];
   pdTables[EOS_METHOD_VTUNE4]      = pLohrenz[4];

   /* Do the deed */
   EosPvtTable *pvttable = new EosPvtTable( 3, piTables, pdTables, pvtData, pT, NULL );

   /* Return pointer */
   return ( pvttable );
}

// Interface routine to compute the phase properties 
//
// Set EosCauldron.h for description of input
//
// 1) Load data into pointI and pointR array
// 2) Construct EosCauldron instance
// 3) Construct EosPvtModel instance
// 4) Do flash
// 5) Deconstruct EosCauldron and EosPvtModel
void EosCauldron::EosGetProperties( int iFlashes,          // Number of flashes
                                    int iOil,              // C index of oil phase 
                                    int iGas,              // C index of gas phase 
                                    double *pPressure,     // Pointer to array of reference phase pressures (SI units) of size the number of objects
                                    double *pTemperature,  // Pointer to array of temperatures  (SI units) of size the number of objects.
                                    double *pAccumulation, // Pointer to array of mass accumulations (SI units). The array is over all objects and is of size 
                                                           // number of objects (varies first) 
                                                           // components (varies last)
                                    double* pKValues,      // K-values used to initialise the Newton solver.
                                                           // If this pointer is null then the can be no k-value initialisation.
                                    double *pPhaseAcc,     // Pointer to array of phase accumulations on a mass basis (SI units). The array is 
                                                           // over all objects and is of size number of 
                                                           // objects (varies first)
                                                           // phases (varies last)
                                                           // Can be set to NULL
                                    double *pMassFraction, // Pointer to array of phase mass fractions. The array is over all objects and is of size
                                                           // number of objects (varies first)
                                                           // number of components (+1 if enthalpies required)
                                                           // phases (varies last)
                                                           // Can be set to NULL
                                    double *pDensity,      // Pointer to array of densities on a mass basis (SI units). The array is over all objects and is of size
                                                           // number of objects (varies first) 
                                                           // phases (varies last)
                                                           // Can be set to NULL
                                    double *pViscosity,    // Pointer to array of viscosities (SI units). The array is over all objects and is of size
                                                           // number of objects (varies first)
                                                           // phases (varies last)
                                                           // Can be set to NULL if viscosities not required
                                    EosPvtTable *pvttable ,// Pointer to a cached Pvt table
                                    int iItersNum,         // Maximal number of iterations for nonlinear solver
                                    double dStopTol,       // Convergence tolerance value for nonlinear solver
                                    double dNewtonRelCoef  // Relaxation coefficient for the Newton nonlinear solver
                                 )
{
   int          piApplication[EOS_APPLICATION_LAST_INTEGER];
   double      *pointR[EOS_APPLICATION_LAST_DARRAY];
   int         *pointI[EOS_APPLICATION_LAST_IARRAY];
   double       pdFlasher[EOS_LAST_DOUBLE];
   int          piFlasher[EOS_LAST_INTEGER];
   EosPvtModel *pvtmodel;
   EosCauldron *program;

   /* Set the integer data */
   piApplication[EOS_APPLICATION_COMPONENTS] = pvttable->GetNumberHydrocarbons ( );
   piApplication[EOS_APPLICATION_FLASHES]    = iFlashes;
   piApplication[EOS_APPLICATION_LIQUID]     = iOil;
   piApplication[EOS_APPLICATION_VAPOUR]     = iGas;

   // Indicate whether to use the user supplied k-values or not.
   if ( pKValues == 0 ) { // or ( pKValues != 0 and pKValues [ 0 ] == -1.0 )) {
      piApplication[EOS_APPLICATION_RESTORE] = EOS_OPTION_OFF;
   } else if ( pKValues [ 0 ] == -1.0 ) {
      piApplication[EOS_APPLICATION_RESTORE] = EOS_OPTION_INITIALISE;
   } else {
      piApplication[EOS_APPLICATION_RESTORE] = EOS_OPTION_ON;
   }

   /* Set integer pointers */
   pointI[INTEGERDATA]                 = piApplication;
   pointI[EOS_APPLICATION_EOSCOMPS]    = NULL;
   pointI[EOS_APPLICATION_INDIRECTION] = NULL;
   pointI[EOS_APPLICATION_PHASES]      = NULL;

   /* Set real pointers */
   pointR[REALDATA]                      = NULL;
   pointR[EOS_APPLICATION_PRESSURE]      = pPressure;
   pointR[EOS_APPLICATION_TEMPERATURE]   = pTemperature;
   pointR[EOS_APPLICATION_ACCUMULATION]  = pAccumulation;
   pointR[EOS_APPLICATION_DENSITY]       = pDensity;
   pointR[EOS_APPLICATION_DDENSITY]      = NULL;
   pointR[EOS_APPLICATION_TOTALPHASE]    = pPhaseAcc;
   pointR[EOS_APPLICATION_DTOTALPHASE]   = NULL;
   pointR[EOS_APPLICATION_VISCOSITIES]   = pViscosity;
   pointR[EOS_APPLICATION_DVISCOSITIES]  = NULL;
   pointR[EOS_APPLICATION_TENSIONS]      = NULL;
   pointR[EOS_APPLICATION_DTENSIONS]     = NULL;
   pointR[EOS_APPLICATION_FRACTION]      = pMassFraction;
   pointR[EOS_APPLICATION_DFRACTION]     = NULL;
   pointR[EOS_APPLICATION_DEPTH]         = NULL;
   pointR[EOS_APPLICATION_SPLIT]         = NULL;

   // Set the k-values array. 
   // If this value is null then it will not be used by the solver.
   pointR[EOS_APPLICATION_KVALUES]    = pKValues;

   pointR[EOS_APPLICATION_BPRESSURE]     = NULL;
   pointR[EOS_APPLICATION_DPRESSURE]     = NULL;
   pointR[EOS_APPLICATION_MW]            = NULL;
   pointR[EOS_APPLICATION_PHASEPRESSURE] = NULL;

   /* Flasher integer data */
   piFlasher[PVTMETHOD]           = EOS_PVT_MODEL;
   piFlasher[EOS_MAXITN]          = iItersNum;
   piFlasher[EOS_MAXFLASH]        = 32;
   piFlasher[EOS_MICHELSON]       = EOS_OPTION_OFF;
   piFlasher[EOS_SUBSTITUTIONS]   = 0;
   piFlasher[EOS_OWNMEMORY]       = EOS_OPTION_ON;
   piFlasher[EOS_DEBUG]           = EOS_OPTION_OFF;

   /* Flasher real data */
   pdFlasher[EOS_ENORM]            = 1.0e80;
   pdFlasher[EOS_TINY]             = 1.0e-15;
   pdFlasher[EOS_CONVERGENCE]      = dStopTol;
   pdFlasher[EOS_THERMALDIFFUSION] = 0.0;
   pdFlasher[EOS_BUBBLEREDUCE]     = 0.5;
   pdFlasher[EOS_NEWTON_RELAX_COEFF] = dNewtonRelCoef;


   /* Read Cauldron data */
   program = new EosCauldron( 0, pointI, pointR );

   /* Construct model */
   pvtmodel = new EosPvtModel( 0, piFlasher, pdFlasher );

   /* Do the flash */
   pvtmodel->DoFlash( program, &pvttable );

   /* Destruct the class */
   delete pvtmodel;
   delete program;
}

// Constructor application object
EosCauldron::EosCauldron()
{
   this->Initialize( 0, NULL, NULL );
}

// Constructor application object
// 
// 1) Allocate memory
// 2) Set current flash to zero
// 3) If no input data supply defaults
// 4) Else call ReadAllData to read in the data
EosCauldron::EosCauldron( int iVersion,   // Version of flasher (set to zero)
                          int **pointI,   // (in) Array of pointers to integer arrays as is described above when using external memory
                                          // (ou) Except for the first integer array, array of pointers to integer arrays
                          double **pointR // (in) Array of pointers to double precision arrays as is described above when using external memeory
                                          // (out) Except for the first double array, array of pointers to double arrays
                         )
{
   /* Initialize current flash */
   this->Initialize ( iVersion, pointI, pointR );
}

// Copy constructor
//
// 1) Make a copy, but do not set iOwnMemory
EosCauldron::EosCauldron( const EosCauldron & self )
{   
   this->iCurrentFlash      = self.iCurrentFlash;
   this->iFlashes           = self.iFlashes;
   this->iLiquidPhase       = self.iLiquidPhase;
   this->iVapourPhase       = self.iVapourPhase;
   this->iComponents        = self.iComponents;
   this->m_iRestore         = self.m_iRestore;
   this->pInd               = self.pInd;
   this->pInd1              = self.pInd1;
   this->pInd2              = self.pInd2;
   this->pSavedPhase        = self.pSavedPhase;
   this->dMinPressure       = self.dMinPressure;
   this->pPressure          = self.pPressure;
   this->pTemperature       = self.pTemperature;
   this->pAccumulation      = self.pAccumulation;
   this->pMassFraction      = self.pMassFraction;
   this->pPhaseAccumulation = self.pPhaseAccumulation;
   this->pDensity           = self.pDensity;
   this->pViscosity         = self.pViscosity;
   this->pSavedSplit        = self.pSavedSplit;
   this->pSavedKvalue       = self.pSavedKvalue;
}

// Initialize constructor application object
//
// 1) Allocate memory
// 2) Set current flash to zero
// 3) If no input data supply defaults
// 4) Else call ReadAllData to read in the data
void EosCauldron::Initialize( int iVersion, int **pointI, double **pointR )
{
   /* Initialize current flash */
   this->iCurrentFlash = 0;

   /* Set defaults */
   if ( pointI == (int **)0 )
   {
      this->pInd = 0;
      this->iFlashes = 0;
      this->iComponents = 0;
      this->m_iRestore  = EOS_OPTION_OFF;
      this->iLiquidPhase = EOS_SINGLE_PHASE_OIL;
      this->iVapourPhase = EOS_SINGLE_PHASE_GAS;
      this->dMinPressure = 1.0;

      /* Pointers */
      this->pPressure = (double *)0;
      this->pTemperature = this->pPressure;
      this->pAccumulation = this->pPressure;
      this->pMassFraction = this->pPressure;
      this->pPhaseAccumulation = this->pPressure;
      this->pDensity = this->pPressure;
      this->pViscosity = this->pPressure;
      this->pSavedSplit = this->pPressure;
      this->pSavedKvalue = this->pPressure;
   }
   else /* Read application data */
   {
      this->ReadAllData( iVersion, pointI, pointR );
   }
}

// Routine to write out the index of the water component 
//
// Returns Cauldron index of water component
int EosCauldron::WriteWaterIndex( )
{
   return( -1 );
}

// See if we need to restore old K values
//
// Returns value of iRestore
int EosCauldron::WriteOldValues( )
{
   if ( m_iRestore != EOS_OPTION_ON ) {
      return EOS_OPTION_OFF;
   } else {
      return EOS_OPTION_ON;
   }

}

// Write out number of flashes 
//
// Returns total number of flashes
int EosCauldron::WriteNumberFlashes( )
{
   return( this->iFlashes );
}

// Write out whether we are isothermal
//
// Returns 1 if isothermal, 0 if thermal
int EosCauldron::WriteIsothermal()
{
   return ( EOS_OPTION_OFF );
}

// Write the minimum temperature
//
// Returns the minimum temperature
//
// 1) Determine the minimum temperature
double EosCauldron::WriteMinimumTemperature()
{
   double dA, dB;

   /* Set minimum */
   dA = this->pTemperature[0];
   for ( int i = 1; i < this->iFlashes; ++i )
   {
      dB = this->pTemperature[i];
      dA = dA < dB ? dA : dB;
   }

   /* Return the minimum */
   return( dA );
}

// Returns output controls needed by flasher
//
// Various terms needed for output 
// 1) Get data
// 2) Do some checks for consistency
void EosCauldron::WriteOutputData( int *pDrv,         // pDrv          derivative level
                                   int *pGetV,        // pGetV         get viscosity
                                   int *pGetT,        // pGetT         get tension
                                   int *pGetH,        // pGetH         get enthalpies
                                   int *pGetM,        // pGetM         get molecular weights
                                   int *pVolume,      // pVolume       indicator for whether densities or volumes returned
                                   int *pMolar,       // pMolar        output is to be molar
                                   int *pMolarDensity // pMolarDensity keep molar output as molar densities, otherwise convert to mass
                                 )
{
   /* Set the terms */
   *pDrv          = EOS_NODRV;
   *pGetV         = ( this->pViscosity != NULL );
   *pGetT         = EOS_OPTION_OFF;
   *pGetH         = EOS_OPTION_OFF;
   *pGetM         = EOS_OPTION_OFF;
   *pVolume       = EOS_OPTION_OFF;
   *pMolar        = EOS_OPTION_OFF;
   *pMolarDensity = EOS_OPTION_OFF;
}

// Write Control Data
// 
// Get control terms for flasher
// 1) Access and return data
void EosCauldron::WriteControlData( int *pType,            // Number of hydrocarbon phases allowed in flasher, either
                                                           // EOS_TOF_2P           - Two hydrocarbon phases may be present
                                                           // EOS_TOF_0P           - No hydrocarbon phases present
                                                           // EOS_SINGLE_PHASE_OIL - Single phase oil only possible
                                                           // EOS_SINGLE_PHASE_GAS - Single phase gas only possible
                                                           //
                                    int *pSaved,           // Indicator for whether saved K values used
                                    int *pNobj,            // Total number of objects
                                    int *pFlash,           // Do the flash
                                    int *pProp,            // Get properties
                                    int *pBubble,          // Do a bubble dew point calculation instead of a flash
                                    int *pWater,           // Do water phase calculations
                                    int *pInit,            // Do initialization or separator calculations; values are
                                                           // EOS_FLASH_CALCULATIONS    - flasher will be used for flash 
                                                           // EOS_COMPOSITIONAL_GRADING - flasher will be used for compositional grading calculations.
                                                           // EOS_SEPARATOR_FLASH       - flasher will be used for separator calculations.
                                                           //
                                    int *pBubbleDew,       // pBubbleDew        bubble point tracking to be used for a single phase mixture
                                    int *pPseudoProperties // pPseudoProperties pseudo phase properties to be generated for Application
                                  )
{
   /* Set the control terms */
   *pType             = EOS_TOF_2P;
   *pSaved            = WriteOldValues ();
   // *pSaved            = m_iRestore;
   *pNobj             = this->iFlashes;
   *pFlash            = EOS_OPTION_ON;
   *pProp             = EOS_OPTION_ON;
   *pBubble           = EOS_OPTION_OFF;
   *pWater            = EOS_OPTION_OFF;
   *pInit             = EOS_OPTION_OFF;
   *pBubbleDew        = EOS_OPTION_OFF;
   *pPseudoProperties = EOS_OPTION_OFF;
}

// Load grid block properties into temporary vectors 
// 1) Set simple indicators
// 2) Two similar loops, one for hydrocarbons and one for water phase, load pressure,
//    temperature, and saved split information
// 3) A second loop for compositional information including saved K values if required
void EosCauldron::WriteData( int iM,           // Number of elements in slice
                             int i1,           // Starting index between 0 and iM - 1
                             int i2,           // Ending index between 0 and iM - 1
                             int iGetK,        // Indicator if estimate of K values required
                             int isSalt,       // Indicator if we are getting hydrocarbon or water information
                             double *pP,       // Pointer to pressure to be loaded
                             double *pT,       // Pointer to temperature to be loaded
                             double *pZ,       // Pointer to composition to be loaded
                             double *pSplit,   // Pointer to phase split to be loaded
                             double *pValueK,  // Pointer to K values to be loaded
                             int *pPhases,     // Pointer to phase indicator (2 phase, etc.) to be loaded
                             int *pIsothermal, // If temperature data is present then returns that this is not isothermal.
                             int *pMolar       // Indicates that the returned data is in terms of mole fractions
                           )
{
   int i, iJ, iNi, iNj;
   double dA, *pTa, *pTb;

   /* Set flags */
   *pIsothermal = EOS_OPTION_OFF;
   *pMolar = EOS_OPTION_OFF;

   /* Loop over indirection indicator and store terms */
   if ( iM > 1 )
   {
      for ( i = i1; i < i2; i++ )
      {
         iJ = this->pInd[i];
         dA = this->pPressure[iJ];
         pP[i] = dA > this->dMinPressure ? dA : this->dMinPressure;
         pT[i] = this->pTemperature[iJ];
         pSplit[i] = this->pSavedSplit[iJ];
         pPhases[i] = this->pSavedPhase[iJ];
      }

      /* Store vector terms */
      if ( iGetK )
      {
         iNj = 0;
         pTa = pZ;
         pTb = pValueK;
         for ( iNi = 0; iNi < this->iComponents; iNi++ )
         {
            for ( i = i1; i < i2; i++ )
            {
               iJ = iNj + this->pInd[i];
               dA = this->pAccumulation[iJ];
               pTa[i] = ( dA > 0.0 ? dA : 0.0 );
               pTb[i] = this->pSavedKvalue[iJ];
            }
            pTa += iM;
            pTb += iM;
            iNj += this->iFlashes;
         }
      }
      else
      {
         iNj = 0;
         pTa = pZ;
         for ( iNi = 0; iNi < this->iComponents; iNi++ )
         {
            for ( i = i1; i < i2; i++ )
            {
               iJ = iNj + this->pInd[i];
               dA = this->pAccumulation[iJ];
               pTa[i] = ( dA > 0.0 ? dA : 0.0 );
            }
            pTa += iM;
            iNj += this->iFlashes;
         }
      }
   }

   /* Store for a single grid block */
   else
   {
      iJ = *this->pInd;
      dA = this->pPressure[iJ];
      *pP = ( dA > this->dMinPressure ) ? dA : this->dMinPressure;
      *pPhases = this->pSavedPhase[iJ];
      *pSplit = this->pSavedSplit[iJ];
      *pT = this->pTemperature[iJ];

      /* Store vector terms */
      if ( iGetK )
      {
         iNj = iJ;
         pTa = pZ;
         pTb = pValueK;
         for ( iNi = 0; iNi < this->iComponents; iNi++ )
         {
            dA = this->pAccumulation[iNj];
            *pTa++ = ( dA > 0.0 ? dA : 0.0 );
            *pTb++ = this->pSavedKvalue[iNj];
            iNj += this->iFlashes;
         }
      }
      else
      {
         iNj = iJ;
         pTa = pZ;
         for ( iNi = 0; iNi < this->iComponents; iNi++ )
         {
            dA = this->pAccumulation[iNj];
            *pTa++ = ( dA > 0.0 ? dA : 0.0 );
            iNj += this->iFlashes;
         }
      }
   }
}

// Routine to change minimum allowed pressure
//
// 1) Do reset
//
// Cauldron does specify a minimum pressure. The flasher,
// being smarter than Cauldron, can make a more intelligent
// guess and hence the value is overwritten
void EosCauldron::ReadMinimumPressure( double dMinP )
{
   this->dMinPressure = dMinP;
}

// Set all data from the outside world
//
// 1) Set constant terms at beginning of each array
// 2) Allocate memory if requested
// 3) Store pointers to memory when memory is allocated
// 4) Initialize indirection and hydrocarbon arrays when
//    memory is allocated
// 5) Set internal pointers to external values if external
//    memory used
// 6) Set index of water component and number of implicit
//    derivatives
void EosCauldron::ReadAllData( int iVersion,    // Version number, currently 0
                               int    **pointI, // Pointer to integer arrays.  See EosPvtModel.h for a description
                               double **pointR  // Pointer to double arrays.  See EosPvtModel.h for a description
                             )
{
   int *pITerms;

   /* Set arrays */
   pITerms = pointI[INTEGERDATA];

   /* Loop over integer terms */
   this->iVapourPhase = pITerms[EOS_APPLICATION_VAPOUR];
   this->iLiquidPhase = pITerms[EOS_APPLICATION_LIQUID];
   this->iComponents  = pITerms[EOS_APPLICATION_COMPONENTS];
   this->iFlashes     = pITerms[EOS_APPLICATION_FLASHES];
   this->m_iRestore   = pITerms [EOS_APPLICATION_RESTORE];

   /* Initialization terms */
   this->dMinPressure = 1.0;

   /* Simulator real variables */
   this->pPressure          = pointR[EOS_APPLICATION_PRESSURE];
   this->pTemperature       = pointR[EOS_APPLICATION_TEMPERATURE];
   this->pAccumulation      = pointR[EOS_APPLICATION_ACCUMULATION];
   this->pDensity           = pointR[EOS_APPLICATION_DENSITY];
   this->pPhaseAccumulation = pointR[EOS_APPLICATION_TOTALPHASE];
   this->pMassFraction      = pointR[EOS_APPLICATION_FRACTION];
   this->pViscosity         = pointR[EOS_APPLICATION_VISCOSITIES];
   this->pSavedPhase        = NULL;
   this->pSavedSplit        = NULL;
   this->pSavedKvalue       = pointR[EOS_APPLICATION_KVALUES];

}

// ReadData
//
// Routine to store phase properties into Cauldron permanent arrays 
//
// 1) Set output control terms
// 2) Set an indirection term corresponding to the proper phase
// 3) Load required phase properties
//
// Note that enthalpy terms loaded into composition arrays
//
// This is all relatively straight forward, except that
//
// a) Remember that what is contiguous in the flasher
//    is controlled by the indirection pInd;
// b) iPvtUse controls which components were present in
//    the flasher
// c) Only take loops over implicit derivative components
void EosCauldron::ReadData( int iPhaseId, /// Type of phase to be processed
                                          ///    EOS_SINGLE_PHASE_OIL - Phase stored in "x" arrays in flasher
                                          ///    EOS_SINGLE_PHASE_GAS - Phase stored in "y" arrays in flasher
                                          ///    EOS_WATER            - Aqueous phase
                            int iM,       /// Number of objects to read
                            int iNc,      /// Number of components used in the flasher
                            int *pPhases, /// (constant) Indentifier for phases
                                          ///    EOS_SINGLE_PHASE_OIL - Phase is a liquid
                                          ///    EOS_SINGLE_PHASE_GAS - Phase is a gas
                                          ///    Not used for an aqueous phase load
                            double *pX,   /// (constant) Pointer to compositions or overall mass/moles from the flasher
                            double *pDX,  /// (constant) Pointer to derivatives of pX
                            double *pMx,  /// (constant) Pointer to total mass/moles from the flasher
                            double *pDMx, /// (constant) Pointer to derivatives of pMx
                            double *pZx,  /// (constant) Pointer to density or volume from the flasher
                            double *pDZx, /// (constant) Pointer to derivatives of pZx
                            double *pMux, /// (constant) Pointer to viscosity from the flasher
                            double *pDMux,/// (constant) Pointer to derivatives of pMux
                            double *pIfx, /// (constant) Pointer to tension from the flasher
                            double *pDIfx,/// (constant) Pointer to derivatives of pIfx
                            double *pHx,  /// (constant) Pointer to enthalpies from flasher
                            double *pDHx, /// (constant) Pointer to derivatives of pHx
                            double *pP,   /// (constant) Pointer to bubble or dew point from flasher
                            double *pDP,  /// (constant) Pointer to derivatives of pP
                            double *pMwx  /// (constant) Pointer to molecular weight from flasher
                          )
{
   int iNi, iNl, i, iJ, iK, i1, i2;
   double *pTa, *pTb;

   /* Code for multiple grid blocks */
   if ( iM > 1 )
   {
      if ( iPhaseId == EOS_SINGLE_PHASE_OIL )
      {
         iJ = this->iLiquidPhase * this->iFlashes;
         iK = this->iVapourPhase * this->iFlashes;
         for ( i = 0; i < iM; i++ )
         {
            this->pInd1[i] = pPhases[i] == EOS_SINGLE_PHASE_OIL ? iJ : iK;
         }
      }
      else
      {
         iJ = this->iLiquidPhase * this->iFlashes;
         iK = this->iVapourPhase * this->iFlashes;
         for ( i = 0; i < iM; i++ )
         {
            this->pInd1[i] = pPhases[i] == EOS_SINGLE_PHASE_OIL ? iK : iJ;
         }
      }

      /* Single offset properties */
      for ( i = 0; i < iM; i++ )
      {
         this->pInd2[i] = this->pInd[i] + this->pInd1[i];
      }

      /* Phase accumulation */
      if ( this->pPhaseAccumulation )
      {
         for ( i = 0; i < iM; i++ )
         {
            this->pPhaseAccumulation[this->pInd2[i]] = pMx[i];
         }
      }

      /* Density */
      if ( this->pDensity )
      {
         for ( i = 0; i < iM; i++ )
         {
            this->pDensity[this->pInd2[i]] = pZx[i];
         }
      }

      /* Viscosity */
      if ( this->pViscosity )
      {
         for ( i = 0; i < iM; i++ )
         {
            this->pViscosity[this->pInd2[i]] = pMux[i];
         }
      }

      /* Set pointers for compositions */
      if ( this->pMassFraction )
      {
         for ( i = 0; i < iM; i++ )
         {
            this->pInd2[i] = this->pInd[i] + this->iComponents * this->pInd1[i];
         }

         /* Compositions */
         pTa = pX;
         pTb = this->pMassFraction;
         for ( iNi = 0; iNi < this->iComponents; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               pTb[this->pInd2[i]] = *pTa++;
            }
            pTb += this->iFlashes;
         }
      }
   }

   /* Code for single grid block */
   else
   {
      /* Set up terms for phases */
      if ( iPhaseId == EOS_SINGLE_PHASE_OIL )
      {
         i1 = this->iFlashes * ( (*pPhases == EOS_SINGLE_PHASE_OIL) ? this->iLiquidPhase : this->iVapourPhase );
      }
      else
      {
         i1 = this->iFlashes * ( (*pPhases == EOS_SINGLE_PHASE_OIL) ? this->iVapourPhase : this->iLiquidPhase );
      }

      /* Single offset properties */
      iK = *this->pInd;
      i2 = iK + i1;

      /* Phase Accumulation */
      if ( this->pPhaseAccumulation )
      {
         this->pPhaseAccumulation[i2] = *pMx;
      }

      /* Viscosity */
      if ( this->pDensity )
      {
         this->pDensity[i2] = *pZx;
      }

      /* Viscosity */
      if ( this->pViscosity )
      {
         this->pViscosity[i2] = *pMux;
      }

      /* Compositions */
      if ( this->pMassFraction )
      {
         pTa = pX;
         iNl = iK + this->iComponents * i1;
         for ( iNi = 0; iNi < this->iComponents; iNi++ )
         {
            this->pMassFraction[iNl] = *pTa++;
            iNl += this->iFlashes;
         }
      }
   }
}

// Routine to store phase properties into Cauldron permanent arrays 
// when a phase is not present
//
// 1) Set output control terms
// 2) Set an indirection term corresponding to the proper phase
// 3) Load required phase properties
//
// Note that enthalpy terms loaded into composition arrays
void EosCauldron::ReadNull( int iPhaseId, // Type of phase to be processed
                                          //    EOS_SINGLE_PHASE_OIL  Phase stored in "x" arrays in flasher
                                          //    EOS_SINGLE_PHASE_GAS  Phase stored in "y" arrays in flasher
                                          //    EOS_WATER  Aqueous phase
                            int iM,       // Number of objects to read
                            int *pPhases  // Indentifier for phases
                                          //    EOS_SINGLE_PHASE_OIL Phase is a liquid
                                          //    EOS_SINGLE_PHASE_GAS Phase is a gas
                                          //    Not used for an aqueous phase load
                          )
{
   int iNi, i, iJ, iK, i1, i2;
   double *pTa;

   /* Code for multiple grid blocks */
   if ( iM > 1 )
   {
      /* Set up terms for phases */
      if ( iPhaseId == EOS_SINGLE_PHASE_OIL )
      {
         iK = this->iFlashes * this->iLiquidPhase;
         iJ = this->iFlashes * this->iVapourPhase;
         for ( i = 0; i < iM; i++ )
         {
            this->pInd1[i] = pPhases[i] == EOS_SINGLE_PHASE_OIL ? iK : iJ;
         }
      }
      else
      {
         iK = this->iFlashes * this->iLiquidPhase;
         iJ = this->iFlashes * this->iVapourPhase;
         for ( i = 0; i < iM; i++ )
         {
            this->pInd1[i] = pPhases[i] == EOS_SINGLE_PHASE_OIL ? iJ : iK;
         }
      }

      /* Single offset properties */
      for ( i = 0; i < iM; i++ )
      {
         this->pInd2[i] = this->pInd[i] + this->pInd1[i];
      }

      /* Phase accumulation */
      if ( this->pPhaseAccumulation )
      {
         for ( i = 0; i < iM; i++ )
         {
            this->pPhaseAccumulation[this->pInd2[i]] = 0.0;
         }
      }

      /* Density */
      if ( this->pDensity )
      {
         for ( i = 0; i < iM; i++ )
         {
            this->pDensity[this->pInd2[i]] = EOS_DEFAULT_DENSITY;
         }
      }

      /* Viscosity */
      if ( this->pViscosity )
      {
         for ( i = 0; i < iM; i++ )
         {
            this->pViscosity[this->pInd2[i]] = EOS_DEFAULT_VISCOSITY;
         }
      }

      /* Set pointers for compositions */
      if ( this->pMassFraction )
      {
         for ( i = 0; i < iM; i++ )
         {
            this->pInd2[i] = this->pInd[i] + this->iComponents * this->pInd1[i];
         }

         /* Compositions */
         pTa = this->pMassFraction;
         for ( iNi = 0; iNi < this->iComponents; iNi++ )
         {
            for ( i = 0; i < iM; i++ )
            {
               pTa[this->pInd2[i]] = 0.0;
            }
            pTa += this->iFlashes;
         }
      }
   }

   /* Code for single grid block */
   else
   {
      /* Set up terms for phases */
      if ( iPhaseId == EOS_SINGLE_PHASE_OIL )
      {
         i1 = this->iFlashes * ( (*pPhases == EOS_SINGLE_PHASE_OIL) ? this->iLiquidPhase : this->iVapourPhase );
      }
      else
      {
         i1 = this->iFlashes * ( (*pPhases == EOS_SINGLE_PHASE_OIL) ? this->iVapourPhase : this->iLiquidPhase );
      }

      /* Single offset properties */
      iK = *this->pInd;
      i2 = iK + i1;

      /* Phase accumulation */
      if ( this->pPhaseAccumulation )
      {
         this->pPhaseAccumulation[i2] = 0.0;
      }

      /* Density */
      if ( this->pDensity )
      {
         this->pDensity[i2] = EOS_DEFAULT_DENSITY;
      }

      /* Viscosity */
      if ( this->pViscosity )
      {
         this->pViscosity[i2] = EOS_DEFAULT_VISCOSITY;
      }

      /* Compositions */
      if ( this->pMassFraction )
      {
         i = iK + this->iComponents * i1;
         for ( iNi = 0; iNi < this->iComponents; iNi++ )
         {
            this->pMassFraction[i] = 0.0;
            i += this->iFlashes;
         }
      }
   }
}

// Subroutine to load flash results into permanent arrays 
//
// 1) Check to see if success
// 2) If successful load K values and "split" terms as well 
//    as current phase flag
// 3) If needs a reset on lack of success store the current
//    fractions and pressure for later testing
// 4) Otherwise indicate a single phase system
void EosCauldron::ReadFlashResults( int iS,   
                                    int iM,          // Number of elements to load
                                    int iSet,        // Integer above which success is declared. Generally when the pPhase array is of following form
                                                     //    - EOS_2P_NCV - Nonconverged two phase flash
                                                     //    - EOS_2P_CV  - Converged two phase flash
                                                     //    - EOS_BP_NCV - Nonconverged bubble point calculation
                                                     //    - EOS_BP_CV  - Converged bubble point calculation
                                                     // It is better to load partially converged blocks than  to ditch them!
                                    int iReset,      // Integer controlling what happens if not successful. Lack of success is generally indicated by the following
                                                     //    - EOS_1P_NCV - Nonconverged one phase system
                                                     // If iReset is on then the current compositions and pressures are saved in order to be used later 
                                                     // to see if we need to do the stability testing again.
                                    double *pSplit,  // Phase split from the flasher.  Overloaded with bubble or dew point for a bubble point calculation
                                    int    *pPhases, // Current phase indicator from flasher; see above
                                    double *pValueK  // Current estimate of K values
                                  )
{
   int i, i1, i2, iJ, iK, iNi, iTemp;

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

   /* Loop over indirection indicator store terms */
   for ( i = i1; i < i2; i++ )
   {
      iJ = this->pInd[i];
      iTemp = pPhases[i];

      /* Success!! */
      if ( iTemp >= iSet )
      {
         iK = i;
         this->pSavedPhase[iJ] = iTemp;
         this->pSavedSplit[iJ] = pSplit[iK];
         for ( iNi = 0; iNi < this->iComponents; iNi++ )
         {
            this->pSavedKvalue[iJ] = pValueK[iK];
            iK += iM;
            iJ += this->iFlashes;
         }
      }

      /* Set to single phase non converged */
      else
      {
         this->pSavedPhase[iJ] = EOS_FL_1P_NCV;
      }
   }
}

// Routine to modify phase property indicators 
//
// 1) Divide the current phase indicator by 2 after adding
//    1.  The definitions of the numbers EOS_FL_1P_NCV and
//    EOS_FL_1P_CV are such that when divided by two they
//    result in EOS_FL_1P.  This is necessary since the
//    flasher needs to know whether a flash is converged
//    or not, whereas the Cauldron storage can care less.
void EosCauldron::ModifyPhaseIndicator( int iDoAll // Indicator as to whether to modify all phase indicators or only the current flash
                                      )
{
   /* Reset objects */
   if ( iDoAll )
   {
      for ( int iN = 0; iN < this->iFlashes; iN++ )
      {
         this->pSavedPhase[iN] = ( this->pSavedPhase[iN] + 1 ) / 2;
      }
   }
   else
   {
      int iJ = *this->pInd;
      this->pSavedPhase[iJ] = ( this->pSavedPhase[iJ] + 1 ) / 2;
   }
}

// Check to see which grid blocks to do stability test 
//
// 1) If no hydrocarbon phases then do nothing!
// 2) Then test to see if any hydrocarbons present.  If not
//    then the block is EOS_FL_0P
// 3) If only one component is present then block is EOS_FL_1P_CV
// 4) If previous results are unknown block is EOS_FL_1P_NCV
// 5) If previous results are known and components and pressures
//    have not changed much, block is EOS_FL_1P_CV
// 6) Else block is either EOS_FL_1P_NCV or EOS_FL_2P_NCV
//
// Note that EOS_FL_1P_CV means that no testing will be done
void EosCauldron::ModifyPhaseIdentification( double dEnorm )
{
   double dA, dB, dMass, dMaxMass, dVeryTiny;
   int i, iK, iNi;

   /* Set a very tiny number */
   dVeryTiny = -1.0 / dEnorm;

   /* Check grid blocks when not in restore mode */
   for ( i = 0; i < this->iFlashes; i++ )
   {

      /* Sum up the mass and see if anything there */
      iK = i;
      dMass = dVeryTiny;
      dMaxMass = dVeryTiny;

      for ( iNi = 0; iNi < this->iComponents; iNi++ )
      {
         dA = this->pAccumulation[iK];
         dB = ( dA > 0.0 ? dA : 0.0 );
         dMass += dB;
         dMaxMass = ( dB > dMaxMass ? dB : dMaxMass );

         if ( m_iRestore != EOS_OPTION_ON ) {
            this->pSavedKvalue[iK] = 0.0;
         }

         iK += this->iFlashes;
      }

      /* Set terms */
      if ( dMass <= 0.0 )
      {
         this->pSavedSplit[i] = 0.0;
         this->pSavedPhase[i] = EOS_FL_0P;
      }
      else if ( dMass <= dMaxMass )
      {
         this->pSavedSplit[i] = 0.0;
         this->pSavedPhase[i] = EOS_FL_1P_CV;
      }
      else
      {
         this->pSavedSplit[i] = 0.0;
         this->pSavedPhase[i] = EOS_FL_1P_NCV;
      }
   }
}

// Check to see which grid blocks to do stability test 
//
// 1) If no hydrocarbon phases then do nothing!
// 2) Then test to see if any hydrocarbons present.  If not
//    then the block is EOS_FL_0P
// 3) If only one component is present then block is EOS_FL_1P_CV
// 4) Else block is EOS_FL_1P_NCV
//
// This routine does not use previous results
void EosCauldron::ModifyOnePhaseIdentification
   (
         double dEnorm
   )
{
   double dA, dB, dMass, dMaxMass;
   int iJ, iK, iNi;

   /* Check grid block */
   iJ = *this->pInd;

   /* Sum up the mass and see if anything there */
   iK = iJ;
   dMass = -1.0 / dEnorm;
   dMaxMass = dMass;
   for ( iNi = 0; iNi < this->iComponents; iNi++ )
   {
      dA = this->pAccumulation[iK];
      dB = ( dA > 0.0 ? dA : 0.0 );
      dMass += dB;
      dMaxMass = ( dB > dMaxMass ? dB : dMaxMass );
      this->pSavedKvalue[iK] = 0.0;
      iK += this->iFlashes;
   }

   /* Set terms */
   this->pSavedSplit[iJ] = 0.0;
   if ( dMass <= 0.0 )
   {
      this->pSavedPhase[iJ] = EOS_FL_0P;
   }
   else if ( dMass <= dMaxMass )
   {
      this->pSavedPhase[iJ] = EOS_FL_1P_CV;
   }
   else
   {
      this->pSavedPhase[iJ] = EOS_FL_1P_NCV;
   }
}

// Slice objects and return the length of the slice
//
// 1) Fill up a slice of length up to iMVL based upon
//    the value of iTestPhase as compared to the current
//    state of the phase indicator 
int EosCauldron::SetSlice( int iTestPhase, // Phase indicator for which to form the slice
                           int iMVL        // Slice length
                         )
{
   /* Load up the blocks */
   int i = 0;
   while ( i < iMVL && this->iCurrentFlash < this->iFlashes )
   {
      int iJ = this->iCurrentFlash++;
      if ( this->pSavedPhase[iJ] == iTestPhase )
      {
         this->pInd[i++] = iJ;
      }
   }

   return( i );
}


// See if any more objects present to compute
//
// Returns 1 if anything aan de beurt, 0 otherwise
//
// 1) If iM greater than zero reset the iM place
// 2) Get the next object aan de beurt
int EosCauldron::Aandebeurt( int iM,        // Set the indirection "aan de beurt" to this value if greater than zero
                             int iTestPhase // Phase indicator for which to check if anything more present for calculations
                           )
{
   int i = 0;

   /* Set the indirection */
   if ( iM >= 0 )
   {
      this->pInd[iM] = this->iCurrentFlash++;
   }

   /* Load up the blocks */
   while ( i == 0 && this->iCurrentFlash < this->iFlashes )
   {
      if ( this->pSavedPhase[this->iCurrentFlash] == iTestPhase )
      {
         i = 1;
      }
      else
      {
         this->iCurrentFlash++;
      }
   }

   /* End of function */
   return ( i );
}

//
// Trivial slice 
// 
// 1) Set slice for a single block
void EosCauldron::SetTrivialSlice()
{
   /* Set indirection indicator */
   *this->pInd = 0;
}

// Set the pointers to temporary memory
//
// 1) Need to set aside some memory for indirection arrays
//    Note that these integer arrays are allowed to have
//    double space just to ensure that the allocation
//    does not result in overwrites
// 2) If we are not saving K values, we still need these
//    arrays during the flash
//
// The memory will actually be allocated from within the
// EosPvtModel class.  This routine gets two passes; the
// first calculates the memory assuming that the pointers
// start at null to get the length, the second after the
// allocation to set the correct locations
void EosCauldron::SetPointers( int iN,          // Length of calculations
                               double **pFinal  // On input the end of the temporary memory. On output the new end
                             )
{
   /* Indirection terms */
   pInd1 = (int *) *pFinal;
   pInd2 = (int *)( ((double *)this->pInd1) + iN );
   pInd = (int *)( ((double *)this->pInd2) + iN );
   *pFinal = ( (double *)this->pInd ) + iN;
   pSavedPhase = (int *) *pFinal;
   pSavedSplit = ( ( double * ) pSavedPhase ) + iFlashes;

   /* Saved K values, etc. */
   if ( m_iRestore == EOS_OPTION_OFF ) {
      pSavedKvalue = pSavedSplit + iFlashes;
      *pFinal = pSavedKvalue + iFlashes * iComponents;
   } else {
      *pFinal =  pSavedSplit + iFlashes;
   }

}

// Reset the slice back to the beginning of objects
//
// 1) Set the current flash to the beginning of the objects
void EosCauldron::ResetSlice()
{
   this->iCurrentFlash = 0;
}
