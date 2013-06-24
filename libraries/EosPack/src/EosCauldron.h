// Copyright 2011, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef EOSCAULDRON_H
#define EOSCAULDRON_H

#include "EosApplication.h"

class EosPvtTable;

/// \class EosCauldron 
///
/// \brief Interface to the outside world
///
class EosCauldron : public EosApplication 
{
public:
   /// \brief Construction of pvttable using the Cauldron brew
   ///
   /// \param iNc                  Number of components
   /// \param isRK                 Set to 1 for Redlich Kwong; otherwise Peng Robinson corrected
   /// \param pvtData              pointer to array wich contains the following data (doubles):
   ///                                MolecularWeight[NC]     - Molecular weight of components
   ///                                CriticalPressure[NC]    - Critical pressure of components
   ///                                CriticalTemperature[NC] - Critical temperatures of components
   ///                                CriticalVolume[NC]      - Critical volume of components
   ///                                Ift[NC]                 - Interfacial tension array, filled in ConcoctBrew and used in PVT library
   ///                                Acentric[NC]            - Acentric factor of components
   ///                                T[1]                    - dummy, filled in ConcoctBrew and used in PVT library
   ///                                OmegaA[NC]              - Omega A for components
   ///                                OmegaB[NC]              - Omega B for components
   ///                                ShiftC[NC]              - Volume shift for components
   ///                                Binary[NC][NC]          - Binary interaction terms
   ///                             pvtData is used to pass data to PVT library further that is why it contains Ift and T 
   ///                             which will be defined inside of ConcoctBrew.
   /// \param pT                   default temperature, assigned in ConcotBrew and used in PVT library should point to T in pvtData
   /// \param pLohrenz             Lohrenz tuning terms
   /// \param dCritAoverB              
   /// \param iPhaseMethod         Methods for labeling a single phase
   ///
   /// \note All input should be in MKS SI units
   static EosPvtTable* ConcoctBrew ( int iNc, int isRK, double *pvtData, double *pT, double *pLohrenz, double dCritAoverB, int iPhaseMethod );


   /// \brief Entry point for Cauldron 
   ///
   /// \param[in]  iFlashes       Number of flashes
   /// \param[in]  iOil           C index of oil phase 
   /// \param[in]  iGas           C index of gas phase 
   /// \param[in]  pPressure      Pointer to array of reference phase pressures (SI units) of size the number of objects
   /// \param[in]  pTemperature   Pointer to array of temperatures  (SI units) of size the number of objects.
   /// \param[in]  pAccumulation  Pointer to array of mass accumulations (SI units). The array is over all objects and is of size 
   ///                            number of objects (varies first) 
   ///                            components (varies last)
   ///
   /// \param[out] pPhaseAcc      Pointer to array of phase accumulations on a mass basis (SI units). The array is 
   ///                            over all objects and is of size number of 
   ///                            objects (varies first)
   ///                            phases (varies last)
   ///                            Can be set to NULL
   ///
   /// \param[out] pMassFraction  Pointer to array of phase mass fractions. The array is over all objects and is of size
   ///                            number of objects (varies first)
   //                             number of components (+1 if enthalpies required)
   ///                            phases (varies last)
   ///                            Can be set to NULL
   ///
   /// \param[out] pDensity       Pointer to array of densities on a mass basis (SI units). The array is over all objects and is of size
   ///                            number of objects (varies first) 
   ///                            phases (varies last)
   ///                            Can be set to NULL
   ///
   /// \param[out] pViscosity     Pointer to array of viscosities (SI units). The array is over all objects and is of size
   ///                            number of objects (varies first)
   ///                            phases (varies last)
   ///                            Can be set to NULL if viscosities not required
   ///
   /// \param[in]  pvtTable       Pointer to a cached Pvt table
   /// \param[in]  iItersNum      Maximal number of iterations for nonlinear solver
   /// \param[in]  dStopTol       Convergence tolerance value for nonlinear solver
   /// \param[in]  dNewtonRelCoef Relaxation coefficient for the Newton nonlinear solver
   static void EosGetProperties( int iFlashes, int iOil, int iGas, double *pPressure, double *pTemperature,
                                 double *pAccumulation, double *pPhaseAcc, double *pMassFraction, double *pDensity,
                                 double *pViscosity, EosPvtTable *pvttable, int iItersNum, double dStopTol, double dNewtonRelCoef );

   /// \brief Default constructor
   EosCauldron();

   /// \brief Constructor application object
   /// 
   /// \param          iVersion Version of flasher (set to zero)
   /// \param[in][out] pointI   On input, array of pointers to integer arrays as is described above when using external memory
   ///                          On output, except for the first integer array, array of pointers to integer arrays
   ///                          as is described above when using internal memory
   /// \param[in][out] pointR   On input array of pointers to double precision arrays as is described above.
   ///                          On ooutput, except for the first double array, array of pointers to double arrays
   ///                          as is described above when using internal memory
   EosCauldron( int iVersion, int **pointI, double **pointR );

   /// \brief Copy constructor
   EosCauldron( const EosCauldron & self );
   
   virtual ~EosCauldron() {;}

   // Methods to write out data from application to the flasher

   /// \brief Returns index of water component
   virtual int WriteWaterIndex();

   /// \brief Returns whether saving K values
   virtual int WriteOldValues();

   /// \brief Returns the number of flashes
   virtual int WriteNumberFlashes();

   /// \brief Returns whether isothermal
   virtual int WriteIsothermal();

   virtual double WriteMinimumTemperature( );
   
   /// \brief Returns output controls needed by flasher
   ///
   // Various terms needed for output 
   /// \param pDrv          derivative level
   /// \param pGetV         get viscosity
   /// \param pGetT         get tension
   /// \param pGetH         get enthalpies
   /// \param pGetM         get molecular weights
   /// \param pVolume       indicator for whether densities or volumes returned
   /// \param pMolar        output is to be molar
   /// \param pMolarDensity keep molar output as molar densities, otherwise convert to mass
   virtual void WriteOutputData( int *pDrv, int *pGetV, int *pGetT, int *pGetH, int *pGetM,
                                 int *pVolume, int *pMolar, int *pMolarDensity );

   /// \brief Write Control Data -  Get control terms for flasher
   ///
   /// \param pType Number of hydrocarbon phases allowed in flasher, either
   ///              EOS_TOF_2P           - Two hydrocarbon phases may be present
   ///              EOS_TOF_0P           - No hydrocarbon phases present
   ///              EOS_SINGLE_PHASE_OIL - Single phase oil only possible
   ///              EOS_SINGLE_PHASE_GAS - Single phase gas only possible
   ///
   /// \param pSaved  Indicator for whether saved K values used
   /// \param pNobj   Total number of objects
   /// \param pFlash  Do the flash
   /// \param pProp   Get properties
   /// \param pBubble Do a bubble dew point calculation instead of a flash
   /// \param pWater  Do water phase calculations
   /// \parm pInit    Do initialization or separator calculations; values are
   ///                EOS_FLASH_CALCULATIONS    - Indicates that the flasher will be used for flash 
   ///                EOS_COMPOSITIONAL_GRADING - Indicates that the flasher will be used for compositional grading calculations.
   ///                EOS_SEPARATOR_FLASH       - Indicates that the flasher will be used for separator calculations.
   ///
   /// \param pBubbleDew        Indicates whether bubble point tracking to be used for a single phase mixture
   /// \param pPseudoProperties Indicates whether pseudo phase properties to be generated for Application
   virtual void WriteControlData( int *pType, int *pSaved, int *pNobj, int *pFlash, int *pProp, int *pBubble, int *pWater,
                                  int *pInit, int *pBubbleDew, int *pPseudoProperties );

   /// \brief Routine to transfer pressure, temperature and accumulation
   ///        data from external to internal format over slices
   ///
   /// \param iM          Number of elements in slice
   /// \param i1          Starting index between 0 and iM - 1
   /// \param i2          Ending index between 0 and iM - 1
   /// \param iGetK       Indicator if estimate of K values required
   /// \param isSalt      Indicator if we are getting hydrocarbon or water information
   /// \param pP          Pointer to pressure to be loaded
   /// \param pT          Pointer to temperature to be loaded
   /// \param pZ          Pointer to composition to be loaded
   /// \param pSplit      Pointer to phase split to be loaded
   /// \param pValueK     Pointer to K values to be loaded
   /// \param pPhases     Pointer to phase indicator (2 phase, etc.) to be loaded
   /// \param pIsothermal If temperature data is present then returns that this is not isothermal.
   /// \param pMolar      Indicates that the returned data is in terms of mole fractions
   virtual void WriteData( int iM, int i1, int i2, int iGetK, int isSalt, double *pP, double *pT, double *pZ,
                           double *pSplit, double *pValueK, int *pPhases, int *pIsothermal, int *pMolar );

   // Methods to read in data by application from the flasher or from the outside world
   
   /// \brief Read the minimum pressure
   virtual void ReadMinimumPressure( double dMinP );

   /// \brief Routine to store phase properties into Cauldron permanent arrays 
   ///
   /// \param iPhaseId Type of phase to be processed
   ///                 EOS_SINGLE_PHASE_OIL - Phase stored in "x" arrays in flasher
   ///                 EOS_SINGLE_PHASE_GAS - Phase stored in "y" arrays in flasher
   ///                 EOS_WATER            - Aqueous phase
   /// \param iM       Number of objects to read
   /// \param iNc      Number of components used in the flasher
   /// \parm pPhases  (constant) Indentifier for phases
   ///                EOS_SINGLE_PHASE_OIL - Phase is a liquid
   ///                EOS_SINGLE_PHASE_GAS - Phase is a gas
   ///                Not used for an aqueous phase load
   /// \param pX      (constant) Pointer to compositions or overall mass/moles from the flasher
   /// \param pDX     (constant) Pointer to derivatives of pX
   /// \param pMx     (constant) Pointer to total mass/moles from the flasher
   /// \param pDMx    (constant) Pointer to derivatives of pMx
   /// \param pZx     (constant) Pointer to density or volume from the flasher
   /// \param pDZx    (constant) Pointer to derivatives of pZx
   /// \param pMux    (constant) Pointer to viscosity from the flasher
   /// \param pDMux   (constant) Pointer to derivatives of pMux
   /// \param pIfx    (constant) Pointer to tension from the flasher
   /// \param pDIfx   (constant) Pointer to derivatives of pIfx
   /// \param pHx     (constant) Pointer to enthalpies from flasher
   /// \param pDHx    (constant) Pointer to derivatives of pHx
   /// \param pP      (constant) Pointer to bubble or dew point from flasher
   /// \param pDP     (constant) Pointer to derivatives of pP
   /// \parm pMW      (constant) Pointer to molecular weight from flasher
   virtual void ReadData( int iPhaseId, int iM, int iNc, int *pPhases, double *pX, double *pDX,
                          double *pMx, double *pDMx, double *pZx, double *pDZx, double *pMux, double *pDMux,
                          double *pIfx, double *pDIfx, double *pHx, double *pDHx, double *pP, double *pDP,
                          double *pMwx );

   /// \brief Read properties that are set to zero
   ///
   /// \param iPhaseId Type of phase to be processed
   ///                    EOS_SINGLE_PHASE_OIL  Phase stored in "x" arrays in flasher
   ///                    EOS_SINGLE_PHASE_GAS  Phase stored in "y" arrays in flasher
   ///                    EOS_WATER  Aqueous phase
   /// \param iM       Number of objects to read
   /// \param pPhases  Indentifier for phases
   ///                    EOS_SINGLE_PHASE_OIL Phase is a liquid
   ///                    EOS_SINGLE_PHASE_GAS Phase is a gas
   ///                    Not used for an aqueous phase load
   virtual void ReadNull( int iPhaseId, int iM, int *pPhases );

   /// \brief Read flash results into arrays to save K values and phase splits
   /// \param iS
   /// \param iM     Number of elements to load
   /// \param iSet   Integer above which success is declared. Generally when the pPhase array is of following form
   ///                  - EOS_2P_NCV - Nonconverged two phase flash
   ///                  - EOS_2P_CV  - Converged two phase flash
   ///                  - EOS_BP_NCV - Nonconverged bubble point calculation
   ///                  - EOS_BP_CV  - Converged bubble point calculation
   ///               It is better to load partially converged blocks than  to ditch them!
   /// \param iReset Integer controlling what happens if not successful. Lack of success is generally indicated by the following
   ///                  - EOS_1P_NCV - Nonconverged one phase system
   ///               If iReset is on then the current compositions and pressures are saved in order to be used later 
   ///               to see if we need to do the stability testing again.
   /// \param pSplit  Phase split from the flasher.  Overloaded with bubble or dew point for a bubble point calculation
   /// \param pPhases Current phase indicator from flasher; see above
   /// \param pValueK Current estimate of K values
   virtual void ReadFlashResults( int iS,
                                  int iM,
                                  int iSet,
                                  int iReset,
                                  double *pSplit,
                                  int *pPhases,
                                  double *pValueK
                                );

   // Routines to perform direct modifications on Cauldron data
  
   /// \brief Modify phase identification array to Cauldron style to flasher internal style
   /// \param iDoAll Indicator as to whether to modify all phase indicators or only the current flash
   virtual void ModifyPhaseIndicator( int iDoAll );

   /// \brief Modify phase identification array from Cauldron style to flasher internal style
   virtual void ModifyPhaseIdentification ( double dEnorm );

   /// \brief Modify phase identification array from Cauldron style to flasher internal style for a single block
   virtual void ModifyOnePhaseIdentification ( double dEnorm );

   // Routines which have an administrative function

   /// \brief Set a slice for computations
   /// \param iTestPhase Phase indicator for which to form the slice
   /// \param iMVL       Slice length
   /// \return           the length of the slice
   virtual int SetSlice( int iTestPhase, int iMVL );

   /// \brief See if any more objects present to compute
   /// 
   /// \param iM         Set the indirection "aan de beurt" to this value if greater than zero
   /// \param iTestPhase Phase indicator for which to check if anything more present for calculations
   /// \return 1 if anything aan de beurt, 0 otherwise
   virtual int Aandebeurt( int iM, int iTestPhase );

   /// \brief Set a slice of length 1 
   virtual void SetTrivialSlice();

   /// \brief Sets current temporary pointers   
   /// \param iN     Length of calculations
   /// \param pFinal On input the end of the temporary memory. On output the new end
   virtual void SetPointers( int iN, double **pFinal );

   /// \brief Resets the slice back to the beginning of the objects 
   virtual void ResetSlice( );

private:
   /// \brief Does the initialization
   void Initialize ( int iVersion, int **pointI, double **pointR );

   /// \brief Read all data from the constructor
   /// \param iVersion  Version number, currently 0
   /// \param pointI    Pointer to integer arrays. See EosPvtModel.h for a description 
   /// \parm pointR     Pointer to double arrays.  See EosPvtModel.h for a description
   void ReadAllData( int iVersion, int **pointI, double **pointR );


   int iCurrentFlash;  /*!< Internal indicator for the last object processed */
   
   int iFlashes;       /*!< Number of flashes to be done by the flasher. Controlled by piApplication[EOS_APPLICATION_FLASHES] */

   int iLiquidPhase;   /*!< C index of liquid phase; set to -1 if not present. The flasher can handle single phase oil or single 
                            phase gas systems.  Controlled by piApplication[EOS_APPLICATION_LIQUID] */
   int iVapourPhase;   /*!< C index of vapour phase; set to -1 if not present. The flasher can handle single phase oil or single 
                            phase gas systems. Controlled by piApplication[EOS_APPLICATION_VAPOUR] */
   int iComponents;    /*!< Total number of components in the application, including tracer components which are not 
                            flashed within the equation of state flasher. When the flasher is called in the general rather than 
                            the Cauldron specific mode, should be set to the number of flash components. Controlled by
                            piApplication[EOS_APPLICATION_COMPONENTS] */
   int *pInd;          /*!< This array gives the indirection indicator over each slice of the calculation and is dynamically allocated */
   int *pInd1;         /*!< This array is used in storing results in the interface and is dynamically allocated */
   int *pInd2;         /*!< This array is used in storing results in the interface and is dynamically allocated */

   int *pSavedPhase;   /*!< Pointer to array of integers giving the last known state of the system when using saved K values. 
                            When the K values are to be restored, location given externally by pIArray[EOS_APPLICATION_PHASES]
                            If K values not to be restored, location generated internally
                            Options are as follows:
                               EOS_FL_0P     On input indicates that the K values are unknown.Thus pSavedPhase should be initialized to 0 
                                             before the first call to the flasher. On output indicates that no hydrocarbons are present
                               EOS_FL_1P     On input and output indicates that a single hydrocarbon phase is present
                               EOS_FL_2P     On input and output indicates that two hydrocarbon phases are present During the iterations 
                                             other values are posssible
                               EOS_FL_1P_NCV Single phase block but needs checking         
                               EOS_FL_1P_CV  Definitely single phase     
                               EOS_FL_2P_NCV Two phase block but needs checking         
                               EOS_FL_2P_CV  Definitely two phase 
                               EOS_FL_BP_NCV Has a bubble point but needs checking  
                               EOS_FL_BP_CV  Definitely has a bubble point
                               EOS_FL_BP_CHK Check to see if pressure has decreased below bubble point pressure
                            This array is over the entire space of objects */

   double dMinPressure;   /*!< Minimum pressure for flasher. Used in class EosCauldron since Cauldron may send 
                               negative pressures (!) to the flasher. Controlled by pdApplication[EOS_APPLICATION_PMIN] */
   double *pPressure;     /*!< Pointer to array of reference phase pressures of size the number of objects.  When compositional 
                               grading calculations are used, the reference pressure of all depth entries except the reference depth 
                               is likewise returned. The location is either externally supplied or generated at construction and 
                               is controlled by pRArray[EOS_APPLICATION_PRESSURE] */

   double *pTemperature;  /*!< Pointer to array of temperatures of size the number of objects. The location is either externally 
                               supplied or generated at construction and is controlled by pRArray[EOS_APPLICATION_TEMPERATURE] */
   double *pAccumulation; /*!< Pointer to array of accumulations (molar or mass depending on setting of EOS_APPLICATION_MOLEFRACTIONS).
                               The array is over all objects and is of size  number of objects (varies first) x components (varies last)
                               For separator calculations, corresponds to the total feed for separator 1, then for additional feed for 
                               separator 2, etc. When compositional grading calculations are used, the accumulation of all depth entries 
                               (even the reference depth if composition not single phase) are likewise returned.
                               The location is either externally supplied or generated at construction and is controlled by 
                               pRArray[EOS_APPLICATION_ACCUMULATION] 
                               For separator calculations, corresponds to the total feed for separator 1, then for additional feed for 
                               separator 2, etc. For grading calculations the accumulation at only one entry should be supplied */

   double *pMassFraction; /*!< Pointer to array of phase mole or mass fractions or total moles/mass. The array is over all objects and 
                            is of size number of objects (varies first) x number of components (+1 if enthalpies required) phases (varies last)
                            The exact output is determined by iVolume  Note that enthalpies are returned as the last component
                            The location is either externally supplied or generated at construction and is controlled by
                            pRArray[EOS_APPLICATION_FRACTION] */

   double *pPhaseAccumulation; /*!< Pointer to array of phase accumulations on a mole or mass basis. The array is over all objects and is 
                                 of size number of objects (varies first) x phases (varies last)
                                 Whether something is returned is controlled by iVolume.
                                 The location is either externally supplied or generated at construction and is controlled by
                                 pRArray[EOS_APPLICATION_TOTALPHASE] */

   double *pDensity;      /*!< Pointer to array of densities or volumes on a mole or mass basis. The array is over all objects and is of size
                               number of objects (varies first) x phases (varies last)
                               Whether densties or volumes are returned is controlled by iVolume The location is either externally 
                               supplied or generated at construction and is controlled by pRArray[EOS_APPLICATION_DENSITY] */

   double *pViscosity;    /*!< Pointer to array of viscosities. The array is over all objects and is of size 
                            number of objects (varies first) x  phases (varies last)
                            The location is either externally supplied or generated at construction and is controlled by
                            pRArray[EOS_APPLICATION_VISCOSITIES] */

   double *pSavedSplit;   /*!< Pointer to array of saved phase mole/mass fractions. The array is over all objects. This array is completely
                               ignorant of which phase is the oil phase and which phase is the gas phase. 
                               Oninput/output (except when *pSavedPhase is 0) constains
                                 - for two phase blocks: phase split
                                 - for single phase blocks with known bubble point: bubble or dew point pressure stored as a negative number
                                 - for other single phase blocks: the pressure corresponding to the last stability test
                               If K values not to be restored, location generated internally
                               If the location is externally supplied controlled by pRArray[EOS_APPLICATION_SPLIT] */
   double *pSavedKvalue;  /*!< Pointer to array of saved K values on a mole basis.  The array is over all objects and is of size
                               number of objects (varies first) x components (varies last)
                               This array is completely ignorant of which phase is the oil phase and which phase is the gas phase.
                               On input/output (except when *pSavedPhase is 0) contains
                                 - for two phase blocks: k values
                                 - for single phase blocks with known bubble point: k value at bubble point
                                 - for other single phase blocks: the mole/mass fraction corresponding to the last stability test
                               If K values not to be restored, location generated internally
                               If the location is externally supplied controlled by pRArray[EOS_APPLICATION_KVALUES] */
};
#endif
