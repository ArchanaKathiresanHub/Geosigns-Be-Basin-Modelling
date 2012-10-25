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
*                                                                    *
* Package:        MoReS Common System Package                        *
*                                                                    *
* Dependencies:   No platform dependencies                           *
*                                                                    *
* Usage notes:    See Technical Documentation                        *
*                                                                    *
*  Include file for classes is EosPvtModel.h                         *
*  Private include file for classes is EosCauldron.h                 *
*  Source module is EosPvtModel.cpp                                  *
*  Source module for Cauldron implementation is EosCauldron.cpp      *
*                                                                    *
*********************************************************************/

#ifndef EOSCAULDRON_H
#define EOSCAULDRON_H

/*
// Include the external definitions 
*/
#ifndef EOSPVTMODEL_H
#include "EosPvtModel.h"
#endif

/* 
// Class EosCauldron 
//
// Interface to the outside world
*/
class EosCauldron : public EosApplication 
{
/* 
// Concoct brew
//
//   Construction of pvttable using the Cauldron brew
//
//   iNc                   Number of components
//   isRK                  Set to 1 for Redlich Kwong; otherwise Peng Robinson corrected
//   pMolecularWeight      Molecular weight of components
//   pCriticalPressure     Critical pressure of components
//   pCriticalTemperature  Critical temperatures of components
//   pCriticalVolume       Critical volume of components
//   pAcentric             Acentric factor of components
//   pOmegaA               Omega A for components
//   pOmegaB               Omega B for components
//   pShiftC               Volume shift for components
//   pBinary               Binary interaction terms
//   pLohrenz              Lohrenz tuning terms
//
//   Note: All input should be in MKS SI units
*/
public:
   static EosPvtTable* ConcoctBrew ( int iNc, int isRK, double *pMolecularWeight, 
                                     double *pCriticalPressure, 
                                     double *pCriticalTemperature,
                                     double *pCriticalVolume, 
                                     double *pAcentricFactor,
                                     double *pOmegaA, double *pOmegaB, 
                                     double *pShiftC, double *pBinary,
                                     double *pLohrenz );
/*
// EosGetProperties
//
// Entry point for Cauldron 
//
// iFlashes (input):        Number of flashes
// iOil (input):            C index of oil phase 
// iGas (input):            C index of gas phase 
// pPressure (input):       Pointer to array of reference phase 
//                          pressures (SI units) of size the number of objects
// pTemperature (input):    Pointer to array of temperatures  (SI units)
//                          of size the number of objects.  
// pAccumulation (input):   Pointer to array of mass accumulations (SI units). The
//                          array is over all objects and is of
//                          size
//                             number of objects (varies first)
//                             components (varies last)
// pPhaseAcc (output):      Pointer to array of phase accumulations
//                          on a mass basis (SI units).  The array is 
//                          over all objects and is of size
//                             number of objects (varies first)
//                             phases (varies last)
//                          Can be set to NULL
// pMassFraction (output):  Pointer to array of phase mass
//                          fractions.  The array
//                          is over all objects and is of size
//                             number of objects (varies first)
//                             number of components (+1 if enthalpies 
//                                                   required)
//                             phases (varies last)
//                          Can be set to NULL
// pDensity (output):       Pointer to array of densities
//                          on a mass basis (SI units).  The array is 
//                          over all objects and is of size
//                             number of objects (varies first)
//                             phases (varies last)
//                          Can be set to NULL
// pViscosity (output):     Pointer to array of viscosities (SI units).  The 
//                          array is over all objects and is of size
//                             number of objects (varies first)
//                             phases (varies last)
//                          Can be set to NULL if viscosities not
//                          required
// pvtTable (input):        Pointer to a cached Pvt table
*/
   static void EosGetProperties ( int iFlashes,
      int iOil,
      int iGas,
      double *pPressure,
      double *pTemperature,
      double *pAccumulation,
      double *pPhaseAcc,
      double *pMassFraction,
      double *pDensity,
      double *pViscosity,
      EosPvtTable *pvttable
   );
/*
// Methods to write out data from application to the flasher
//
// WriteWaterIndex
//    Returns index of water component
// WriteOldValues
//    Returns whether saving K values
// WriteNumberFlashes
//    Returns the number of flashes
// WriteIsothermal
//    Returns whether isothermal
// WriteOutputData
//    Returns output controls needed by flasher
// WriteControlData
//    Returns control data for flasher
// WriteData
//    Routine to transfer pressure, temperature and accumulation
//    data from external to internal format over slices
*/
   EosCauldron ( );
   EosCauldron ( int iVersion, int **pointI, double **pointR );
   EosCauldron ( EosCauldron &self );
   virtual ~EosCauldron( );
   virtual int WriteWaterIndex( );
   virtual int WriteOldValues( );
   virtual int WriteNumberFlashes( );
   virtual int WriteIsothermal( );
   virtual double WriteMinimumTemperature( );
   virtual void WriteOutputData( int *iDrv, int *iGetV,
               int *iGetT, int *iGetH, int *iGetM,
               int *iVolume, int *iMolar,
               int *iMolarDensity );
   virtual void WriteControlData ( 
               int *pType,
               int *pSaved,
               int *pNobj,
               int *pFlash,
               int *pProp,
               int *pBubble,
               int *pWater,
               int *pInit,
               int *pBubbleDew,
               int *pPseudoProperties
            );
   virtual void WriteData( int iM, int i1, int i2,
               int iGetK, int isSalt, double *pP, double *pT, double *pZ,
               double *pSplit, double *pValueK, int *pPhases,
               int *pIsothermal, int *pMolar );
/*
// Methods to read in data by application from the flasher or
// from the outside world
//
// ReadMinimumPressure
//    Read the minimum pressure
// ReadData
//    Read nonzero properties and derivatives
// ReadNull
//    Read properties that are set to zero
// ReadFlashResults
//    Read flash results into arrays to save K values and
//    phase splits
*/
   virtual void ReadMinimumPressure( double dMinP );
   virtual void ReadData( int iPhaseId, int iM, int iNc,
               int *pPhases, double *pX, double *pDX,
               double *pMx, double *pDMx, double *pZx,
               double *pDZx, double *pMux, double *pDMux,
               double *pIfx, double *pDIfx, double *pHx,
               double *pDHx, double *pP, double *pDP,
               double *pMwx );
   virtual void ReadNull ( 
               int iPhaseId,
               int iM,
               int *pPhases
            );
   virtual void ReadFlashResults ( 
               int iS,
               int iM,
               int iSet,
               int iReset,
               double *pSplit,
               int *pPhases,
               double *pValueK
            );
/*
// Routines to perform direct modifications on Cauldron data
//
// ModifyPhaseIndicator
//    Modify phase identification array to Cauldron
//    style to flasher internal style
// ModifyPhaseIdentification
//    Modify phase identification array from Cauldron
//    style to flasher internal style
// ModifyOnePhaseIdentification
//    Modify phase identification array from Cauldron
//    style to flasher internal style for a single block
*/
   virtual void ModifyPhaseIndicator( int iDoAll );
   virtual void ModifyPhaseIdentification ( double dEnorm );
   virtual void ModifyOnePhaseIdentification ( double dEnorm );
/*
// Routines which have an administrative function
//
// SetSlice
//    Set a slice for computations
// SetTrivialSlice
//    Set a slice of length 1 
// SetPointers
//    Sets current temporary pointers   
// ResetSlice
//    Resets the slice back to the beginning of the objects 
*/
   virtual int SetSlice( int iTestPhase, int iMVL );
   virtual int Aandebeurt( int iM, int iTestPhase );
   virtual void SetTrivialSlice( );
   virtual void SetPointers( int iN, double **pFinal );
   virtual void ResetSlice( );
/*
// Private methods 
//
// Initialize
//    Does the initialization
// ReadAllData
//    Read all data from the constructor
*/
private:
   void Initialize ( int iVersion, int **pointI, double **pointR );
   void ReadAllData( int iVersion, int **pointI,
               double **pointR );
/*
// Description of variables
//
// int iCurrentFlash
//    Internal indicator for the last object processed
// int iFlashes ** constant **
//    Number of flashes to be done by the flasher.  Controlled by
//       piApplication[EOS_APPLICATION_FLASHES]
// int iLiquidPhase ** constant **
//    C index of liquid phase; set to -1 if not present. 
//    The flasher can handle single phase oil or single 
//    phase gas systems.  Controlled by
//       piApplication[EOS_APPLICATION_LIQUID]
// int iVapourPhase ** constant **
//    C index of vapour phase; set to -1 if not present. 
//    The flasher can handle single phase oil or single 
//    phase gas systems.  Controlled by
//       piApplication[EOS_APPLICATION_VAPOUR]
// int iComponents ** constant **
//    Total number of components in the application, 
//    including tracer components which are not 
//    flashed within the equation of state flasher. When 
//    the flasher is called in the general rather than 
//    the Cauldron specific mode, should be set to the 
//    number of flash components.  Controlled by
//       piApplication[EOS_APPLICATION_COMPONENTS]
// int *pSavedPhase
//    Pointer to array of integers giving the last known state of 
//    the system when using saved K values.  When the K values
//    are to be restored, location given externally by
//       pIArray[EOS_APPLICATION_PHASES]
//    If K values not to be restored, location generated internally
//    Options are as follows
//       EOS_FL_0P                
//          On input indicates that the K values are unknown.  Thus 
//          pSavedPhase should be initialized to 0 before the first 
//          call to the flasher.  On output indicates
//          that no hydrocarbons are present
//       EOS_FL_1P 
//          On input and output indicates that a single hydrocarbon 
//          phase is present
//       EOS_FL_2P 
//          On input and output indicates that two hydrocarbon phases 
//          are present
//    During the iterations other values are posssible
//       EOS_FL_1P_NCV   
//          Single phase block but needs checking         
//       EOS_FL_1P_CV        
//          Definitely single phase     
//       EOS_FL_2P_NCV   
//          Two phase block but needs checking         
//       EOS_FL_2P_CV            
//          Definitely two phase 
//       EOS_FL_BP_NCV          
//          Has a bubble point but needs checking  
//       EOS_FL_BP_CV             
//          Definitely has a bubble point
//       EOS_FL_BP_CHK      
//          Check to see if pressure has decreased below bubble 
//          point pressure
//    This array is over the entire space of objects
// int *pInd
//    This array gives the indirection indicator over each
//    slice of the calculation and is dynamically allocated
// int *pInd1
//    This array is used in storing results in the interface
//    and is dynamically allocated
// int *pInd2
//    This array is used in storing results in the interface
//    and is dynamically allocated
// double dMinPressure ** constant **
//    Minimum pressure for flasher.  Used in class 
//    EosCauldron since Cauldron may send negative pressures 
//    (!) to the flasher.  Controlled by
//       pdApplication[EOS_APPLICATION_PMIN]
// double *pPressure  ** constant **
//    Pointer to array of reference phase pressures of size the 
//    number of objects.  When compositional grading calculations
//    are used, the reference pressure of all
//    depth entries except the reference depth is likewise returned
//    The location is either externally supplied or generated
//    at construction and is controlled by
//       pRArray[EOS_APPLICATION_PRESSURE] 
// double *pTemperature ** constant ** 
//    Pointer to array of temperatures of size the number of objects.
//    The location is either externally supplied or generated
//    at construction and is controlled by
//       pRArray[EOS_APPLICATION_TEMPERATURE]   
// double *pAccumulation ** constant **  
//    Pointer to array of accumulations (molar or mass depending on 
//    setting of EOS_APPLICATION_MOLEFRACTIONS).  The
//    array is over all objects and is of size
//       number of objects (varies first)
//       components (varies last)
//    For separator calculations, corresponds to the total feed for 
//    separator 1, then for additional feed for separator 2, etc. 
//    When compositional grading calculations are used, the 
//    accumulation of all depth entries (even the reference
//    depth if composition not single phase) are likewise returned
//    The location is either externally supplied or generated
//    at construction and is controlled by
//        pRArray[EOS_APPLICATION_ACCUMULATION] 
//                          For separator calculations, corresponds
//                          to the total feed for separator 1,
//                          then for additional feed for separator
//                          2, etc.  For grading calculations the
//                          accumulation at only one entry should
//                          be supplied
// double *pMassFraction ** constant ** 
//    Pointer to array of phase mole or mass fractions or total 
//    moles/mass.  The array is over all objects and is of size
//       number of objects (varies first)
//       number of components (+1 if enthalpies required)
//       phases (varies last)
//    The exact output is determined by iVolume  Note that
//    enthalpies are returned as the last component
//    The location is either externally supplied or generated
//    at construction and is controlled by
//       pRArray[EOS_APPLICATION_FRACTION] 
// double *pPhaseAccumulation ** constant ** 
//    Pointer to array of phase accumulations on a mole or mass 
//    basis.  The array is over all objects and is of size
//       number of objects (varies first)
//       phases (varies last)
//    Whether something is returned is controlled by iVolume
//    The location is either externally supplied or generated
//    at construction and is controlled by
//       pRArray[EOS_APPLICATION_TOTALPHASE] 
// double *pDensity ** constant ** 
//    Pointer to array of densities or volumes on a mole or mass 
//    basis.  The array is over all objects and is of size
//       number of objects (varies first)
//       phases (varies last)
//    Whether densties or volumes are returned is controlled by 
//    iVolume The location is either externally supplied or generated
//    at construction and is controlled by
/        pRArray[EOS_APPLICATION_DENSITY] 
// double *pViscosity ** constant ** 
//    Pointer to array of viscosities.  The array is over 
//    all objects and is of size
//       number of objects (varies first)
//       phases (varies last)
//    The location is either externally supplied or generated
//    at construction and is controlled by
//       pRArray[EOS_APPLICATION_VISCOSITIES] 
// double *pSavedSplit;
//    Pointer to array of saved phase mole/mass fractions.  The array 
//    is over all objects. This array is completely 
//    ignorant of which phase is the oil phase 
//    and which phase is the gas phase.  On
//    input/output (except when *pSavedPhase is 0) constains
//       for two phase blocks: phase split
//       for single phase blocks with known bubble point: bubble or
//          dew point pressure stored as a negative number
//       for other single phase blocks: the pressure corresponding
//          to the last stability test
//    If K values not to be restored, location generated internally
//    If the location is externally supplied controlled by
//       pRArray[EOS_APPLICATION_SPLIT] 
// double *pSavedKvalue; 
//    Pointer to array of saved K values on a mole basis.  The array 
//    is over all objects and is of size
//       number of objects (varies first)
//       components (varies last)
//    This array is completely ignorant of which phase is the oil 
//    phase and which phase is the gas phase.  On
//    input/output (except when *pSavedPhase is 0) contains
//       for two phase blocks: k values
//       for single phase blocks with known bubble point: k value at
//          bubble point
//       for other single phase blocks: the mole/mass fraction 
//          corresponding to the last stability test
//    If K values not to be restored, location generated internally
//    If the location is externally supplied controlled by
//       pRArray[EOS_APPLICATION_KVALUES] 
*/
   int iCurrentFlash;
   int iFlashes;
   int iLiquidPhase;
   int iVapourPhase;
   int iComponents;
   int *pInd;
   int *pInd1;
   int *pInd2;
   int *pSavedPhase;
   double dMinPressure;
   double *pPressure;
   double *pTemperature;
   double *pAccumulation;
   double *pMassFraction;
   double *pPhaseAccumulation;
   double *pDensity;
   double *pViscosity;
   double *pSavedSplit;
   double *pSavedKvalue;
};
#endif
