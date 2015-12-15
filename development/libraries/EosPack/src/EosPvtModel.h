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
*  Source module is EosPvtModel.c                                    *
*                                                                    *
*********************************************************************/
#ifndef EOSPVTMODEL_H
#define EOSPVTMODEL_H


// Revision history
//
// Each constructor has a "iVersion" parameter which
// allows for upward compatibility with old versions
// The following revisions have been made
//
//
// Class EosPvtModel
//
// Version 0
//    Initial release
//
// Equation of State Flasher Data Description
// Thom Potempa
//
// Here is a brief description of the data needed to drive the equation of state flasher designed for 
// MoReS.  It is also intended to be a more general application compute server. The issue of a data 
// server is not addressed.  
// 
// The water phase correlations are based upon a public domain correlation I found on the internet.  
// Within the source code there is a comment which must remain present in the code.  The correlations 
// were authored by Bernhard Spang, The Mining Company, http://chemengineer.miningco.com.
// 
// Classes
// 
// The following four classes are supported:
// 
// EosPvtModel (flash computations)
// EosPvtTable (main cache for equation-of-state data)
// EosApplication (input/output to Application)
//
// EosPvtTable will never be seen by the user
// 
// The EosApplication class has a constructor of the form
// 
// EosApplication::EosApplication ( int iVersion, int** pIArray, double** pRArray )
// 
// The EosPvtTable class has a constructor of the form
// 
// EosPvtTable::EosPvtTable ( int iVersion, int *piTables, 
//                            double *pdTables, double *pCritical, 
//                            double *pvtData, double *pSaltMW )
// 
// The EosPvtModel class has no constructor since it is pure method
// 
// By default the arrays point to zero, and provisions have been made to initialize the data outside of 
// the constructor, which also calls the same data initialization.  Since this is intended to be a 
// compute rather than a data server, no provisions have been made for data other than integer and real.  
// 
// The copy and equal constructers are turned off, as these are intended to be compute engine classes.
// 
// The EosApplication class is pure virtual.
// 
// More "C" interfaces will be required in the future to facilitate computational efficiency within 
// Application.  As of this point in time, the only relevant entry point into the flasher itself is
// 
// pEosApplication->DoFlash ( pEosPvtTable );
// 
// Thus, the general way to invoke the flasher is
// 
// pEosPvtTable = new EosPvtTable ( iVersion, piTables, pdTables, 
//                                  pCritical, pvtData, pSaltMW );
// pEosApplication = new EosApplication ( iVersion, pIArray, pRArray ); 
// pEosApplication->DoFlash ( pEosPvtTable );
// delete pEosApplication;
// delete pEosPvtTable;
// 
// The classes are not persistent in the current application, although in principle they can be made 
// to be so.  The EosPvtTable class should be persistent.
// 
// A few notes on the methods themselves. The flasher is faster than that present in MULTISIM.  It is 
// also numerically more stable; for instance, care has been taken to normalize the "ideal" 
// contribution to all Jacobian matrices to the identity matrix.  Within the flasher itself, 
// standard C++ naming conventions have been used: pointers begin with p; doubles begin with d; 
// integers begin with i.
// 
// Preprocessor Declarations
// 
// The preprocessor declarations are found in this file,
// EosPvtDefinitions.h 
//
// These are used internally within the flasher and 
// are not of interest for the interface.
// 
// EOS_NOCONVERGE           
// EOS_CONVERGE             
// EOS_INIT_NONE           
// EOS_INIT_COMP          
// EOS_INIT_DOWN           
// EOS_INIT_START           
// EOS_INIT_UP            
// EOS_INIT_END  
// 
// These are used internally within the flasher and are not of interest for the interface.
//
// Integer Arrays
// 
// Associated with each class is an array of integer data pIData. The pointer to the array is passed 
// within the array of pointers to integer arrays. This array can be destroyed after each class is 
// initialized.
// 
// Real Arrays
// 
// Associated with each class is an array of integer data pRData.  The pointer to the array is passed 
// within the array of pointers to real arrays. This array can be destroyed after each class is 
// initialized.  The following values must be set.
//

// Data for class EosPvtModel (read in class EosApplication)

// Define constants for flash states.
// Used internally within flasher and not of interest for the interface.
enum
{
   EOS_FL_1P_NCV = 1,
   EOS_FL_1P_CV  = 2,
   EOS_FL_2P_NCV = 3,
   EOS_FL_2P_CV  = 4,
   EOS_FL_BP_NCV = 5,
   EOS_FL_BP_CV  = 6,
   EOS_FL_BP_CHK = 7
};

// Define type of flash
// Used internally within flasher and not of interest for the interface.
#define EOS_TOF_0P - 1
#define EOS_TOF_2P - 2

// Define constants for final states
// Used internally within flasher and not of interest for the interface.
enum
{
   EOS_FL_0P = 0,
   EOS_FL_1P = 1,
   EOS_FL_2P = 2,
   EOS_FL_BP = 3,
   EOS_FL_AP = -9
};

// Define constants for loads
#define EOS_NOGETK 0
#define EOS_GETK   1

/* Define convergence flags */
#define EOS_NOCONVERGE 0
#define EOS_CONVERGE   1

/* Terms for initialization */
#define EOS_INIT_NONE - 1
#define EOS_INIT_COMP 0

/* Directions for initialization */
#define EOS_INIT_DOWN  -1
#define EOS_INIT_START 0
#define EOS_INIT_UP    1
#define EOS_INIT_END   2

// For class EosPvtModel (loaded in class EosApplication):
//
// piFlasher[PVTMETHOD] = EOS_PVT_MODEL
// Type of pvt model; currently only the equation of state flasher is available.                      
//
// piFlasher[EOS_MAXITN] 
// Maximum allowed iterations; 20 is reasonable.                     
//
// piFlasher[EOS_MAXFLASH] 
// Parameter controlling slicing of calculations; 32 is a reasonable value.  If the number is set too 
// low, additional overhead is introduced in function calls; if the number is set too high, additional 
// overhead is also introduced by various ways.
//                       
// piFlasher[EOS_MICHELSON] = EOS_OPTION_OFF || EOS_OPTION_ON
// Specifies whether the more robust Michelson stability analysis or a less robust but faster 
// analysis is used to determine the number of hydrocarbon phases.
// 
// piFlasher[EOS_SUBSTITUTIONS]
// Number of successive substitution iterations.  Substitution is not an efficient method.
// 1 is a reasonable default as there are some easy problems which converge quickly
// 
// piFlasher[EOS_OWNMEMORY]
// Not used
// 
// piFlasher[EOS_DEBUG] = EOS_OPTION_OFF || EOS_OPTION_ON
// Indicates whether the flasher is to generate debug printing.
//                    
// EOS_LAST_INTEGER 
// Number of elements in integer data for class EosPvtModel. 
enum
{
   PVTMETHOD           = 0,
   EOS_MAXITN          = 1,
   EOS_MAXFLASH        = 2,
   EOS_MICHELSON       = 3,
   EOS_SUBSTITUTIONS   = 4,
   EOS_OWNMEMORY       = 5,
   EOS_DEBUG           = 6,
   EOS_LAST_INTEGER    = 7
};

// Currently not used, but reserved for later use when multiple applications can access the flasher
//* For type of model */
#define EOS_MORES     0
#define EOS_PVT_MODEL 0

// pdFlasher[EOS_ENORM]      
// A very large number but well within the largest double precision number a machine can handle.  
// 1.0E80 is a reasonable default
//        
// pdFlasher[EOS_TINY]
// Number such that 1 + piFlasher[EOS_TINY] is equal to 1 on a computer.
//               
// pdFlasher[EOS_CONVERGENCE] 
// Convergence tolerance for flasher.  0.0001 is reasonable for reservoir simulation; 
// use 1000.0 * piFlasher[EOS_TINY] for very precise results.
// 
// pdFlasher[EOS_THERMALDIFFUSION]
// Multiplicative factor for thermal diffusion. If 0, thermal diffusion not used
// 
// pdFlasher[EOS_BUBBLEREDUCE]
// Factor for reduction in initial stage of bubble point calculations, usually set to 0.5.  Set to 0.1 if
// convergence is an issue.
// 
// pdFlasher[EOS_NEWTON_RELAX_COEFF]
// Used to scale update for solution on Newton iteration
enum
{
   EOS_ENORM                   = 0,
   EOS_TINY                    = 1,
   EOS_CONVERGENCE             = 2,
   EOS_THERMALDIFFUSION        = 3,
   EOS_BUBBLEREDUCE            = 4,
   EOS_NEWTON_RELAX_COEFF      = 5,
   EOS_LAST_DOUBLE             = 6
};

// pdTables[EOS_METHOD_SALINITY] 
// Default salinity in parts per million for aqueous phase calculations.
// 
// pdTables[EOS_METHOD_WATERIFT] 
// Default water/reference phase IFT.
// 
// pdTables[EOS_METHOD_KB1]
// KB1 parameter for equation of state (e.g., 1.0 for Redlich Kwong).
//           
// pdTables[EOS_METHOD_KB2]
// KB2 parameter for equation of state (e.g., 0.0 for Redlich Kwong).
//
// pdTables[EOS_METHOD_PRESCONV] 
// Conversion factor for pressure from flasher internal units to MKS units.
//
// pdTables[EOS_METHOD_TEMPCONV]
// Conversion factor for temperature from flasher internal units to MKS units.
// 
// pdTables[EOS_METHOD_VOLCONV]
// Conversion factor for volume from flasher internal units to MKS units.
// 
// pdTables[EOS_METHOD_VISCCONV] 
// Conversion factor for viscosity from flasher internal units to MKS units.
// 
// pdTables[EOS_METHOD_DENSCONV]
// Conversion factor for density from flasher internal units to MKS units.
// 
// pdTables[EOS_METHOD_HEATCONV]         
// Conversion factor for enthalpy from flasher internal units to MKS units.
// 
// pdTables[EOS_METHOD_TENSCONV] 
// Conversion factor for interfacial tension from flasher internal units to MKS units.
// 
// pdTables[EOS_METHOD_CRITZMINUSB] 
// Critical z factor minus b for naming a single phase mixture when piTables[EOS_METHOD_PHASEID] is  
// EOS_SINGLE_PHASE_ZMINUSB.  0.25 is the MoReS default.
// 
// pdTables[EOS_METHOD_CRITAOVERB]
// Critical a over b for naming a single phase mixture when piTables[EOS_METHOD_PHASEID] is  
// EOS_SINGLE_PHASE_AOVERB. 5.0 is a reasonable default value.
// 
// pdTables[EOS_METHOD_VTUNE]
// Value of the viscosity tuning parameter for the Jossi/GSS model. If set to zero Lohrenz viscosity 
// is used; if negative corresponding states viscosity used
// 
// pdTables[EOS_METHOD_VTUNE0] =   0.1023000
// pdTables[EOS_METHOD_VTUNE1] =   0.0233640
// pdTables[EOS_METHOD_VTUNE2] =   0.0585330
// pdTables[EOS_METHOD_VTUNE3] = - 0.0407580
// pdTables[EOS_METHOD_VTUNE4] =   0.0093324
// Values of the viscosity tuning parameter for the Lohrenz model. If set to zero Lohrenz viscosity 
// defaults given above are used
//
// pdTables[EOS_METHOD_VTERM1] =   1.0
// pdTables[EOS_METHOD_VTERM2] =   1.0
// pdTables[EOS_METHOD_VTERM3] =   1.0 
// pdTables[EOS_METHOD_VTERM4] =   1.0
// Tuning parameters for corresponding states viscosity model
//
// pdTables[EOS_METHOD_CSVSCMETHOD] =   0.0
// Flag for corresponding states viscosity version number
//   0 is old 2 term model  
//   1 is newer 4 term heavy oil extension model with special PVTSim Shell JIP formulation
//   2 is newer 4 term heavy oil extension model without the special JIP formulation 
enum
{
   EOS_METHOD_SALINITY         = 0,
   EOS_METHOD_WATERIFT         = 1,
   EOS_METHOD_KB1              = 2,
   EOS_METHOD_KB2              = 3,
   EOS_METHOD_PRESCONV         = 4,
   EOS_METHOD_TEMPCONV         = 5,
   EOS_METHOD_VOLCONV          = 6,
   EOS_METHOD_VISCCONV         = 7,
   EOS_METHOD_DENSCONV         = 8,
   EOS_METHOD_HEATCONV         = 9,
   EOS_METHOD_TENSCONV         = 10,
   EOS_METHOD_CRITZMINUSB      = 11,
   EOS_METHOD_CRITAOVERB       = 12,
   EOS_METHOD_VTUNE            = 13,
   EOS_METHOD_VTUNE0           = 14,
   EOS_METHOD_VTUNE1           = 15,
   EOS_METHOD_VTUNE2           = 16,
   EOS_METHOD_VTUNE3           = 17,
   EOS_METHOD_VTUNE4           = 18,
   EOS_METHOD_VTERM1           = 19,
   EOS_METHOD_VTERM2           = 20,
   EOS_METHOD_VTERM3           = 21,
   EOS_METHOD_VTERM4           = 22,
   EOS_METHOD_CSVSCMETHOD      = 23,
   EOS_METHOD_LAST_DOUBLE      = 24
};

class EosApplication;
class EosPvtTable;

/* 
// Class for pvt data equation of state 
*/
class EosPvtModel
{
   //--------------------------------------------------------------------// 
   // Organizational Routines 
   //--------------------------------------------------------------------// 
public:
   EosPvtModel( void );
   EosPvtModel( int iVersion, int *piFlasher, double *pdFlasher );
   EosPvtModel( const EosPvtModel &self );
   virtual ~EosPvtModel( void )  { }

   // Entry point/Supervisor for the flasher
   void DoFlash( EosApplication *pTApplication, EosPvtTable **pTEosPvtTable );

private:   
   // Initialize the flasher
   void Initialize( int iVersion, int *piFlasher, double *pdFlasher );

   // Organization routine for bubble point calculations
   void BubblePointCalculations( int iNc, int iType );

   // Routine to adjust derivative pointers to ensure that 
   // results will be located in contiguous memory locations
   void AdjustPointers( int iM, int iNc, int isWater );

   // Print input data into the flasher for debugging purposes
   void PrintInputData( int iSize );

   // Read all data from the outside world into the flasher
   void ReadAllData( int iVersion, int *piFlasher, double *pdFlasher );

   // Read data from EosApplication and EosPvtTable data structures into the flasher
   void ReadData( int iS, int iM, int iNc, int isSalt, int iGetK, int *pThermal );

   // Write minimum pressure to Application data structure
   void WriteMinimumPressure( void );

   // Set pointers to temporary arrays
   int SetPointers( void );

   //--------------------------------------------------------------------// 
   // Methods to perform flash calculations
   //--------------------------------------------------------------------// 

   // Perform flash for multiple objects
   void FlashMultipleObjects( int iNc );

   // Perform flash for one object
   void FlashOneObject( int iNc );

   // Solve flash equations for mole fractions and splits
   void FlashEquationsOneObject( int iNc, int iUpdate );
   void FlashEquationsMultipleObjects( int iM, int i1, int i2, int iNc, int iUpdate );

   // Perform on side of the Michelson stability analysis
   void OneSideStability( int iM, int iNc, double *pP, double *pT, double *pState, double *pFz );

   // Perform a Michelson stability analysis
   void Michelson( int iM, int iNc );

   // DOCUMENT ME!
   void FastInitialization( int iM, int iNc );

   // Successive substitutions for two phase K values
   void Substitution( int iM, int iNc );

   // Newton's method for two phase K values
   void NewtonFlashOneObject( int iNc, int iRestore, int iLevel );
   void NewtonFlashMultipleObjects( int iM, int iNc, int iRestore, int iLevel );

   // Generate an initial guess for the bubble point pressure
   void BubblePointInit( int iM, int iNc, int iRetrograde );

   // Use Newton's method to solve for the bubble point pressure
   void BubblePointNewton( int iM, int iNc, int iCheck );

   // Test if an object with a known bubble point is two phase
   void BubblePointTest( int iM, int iNc );

   //--------------------------------------------------------------------// 
   // Methods for computing properties 
   //--------------------------------------------------------------------// 

   // Set the properties for multiple objects
   void PropertiesMultipleObjects( int iNc, int iType );

   // Set the properties for one object
   void PropertiesOneObject( int iNc, int iType );

   // Determine which phase is oil and which is gas in a two phase system
   void PhaseId( int iM, int iNc, int iMolar, int iGetPhase );

   // Write property data to simulator data structures
   void WriteData( int iM, int iNc, int iGetViscosity, int iGetTension, 
      int iMolarDensity, int iType, int iPhase );

   // Organizing routine for bubble point calculations
   void BubblePoint( int iM, int iNc );

   // Routine to get the cubic equation of state terms
   // for bubble point calculations
   void BubblePointCubicTerms( int iM, int iNc );

   // Assemble the matrix for bubble point calculations
   void BubblePointMatrix( int iM, int iNc );

   // DOCUMENT ME!
   void BubblePointZero( int iM, int iNc );

   //Compute the derivatives for bubble point calculations
   void BubblePointDerivatives( int iM, int iNc );

   // Computations for a single phase
   void OnePhase( int iM, int iNc );

   // Properties for a single phase
   void OnePhaseProperties( int iM, int iNc );

   // Compute volumes and derivatives for a one phase system
   void OnePhaseVolume( int iM, int iNc );

   // Computations for a single phase with a pseudo phase
   // set for the other phase composition
   void PseudoPhase( int iM, int iNc );

   // Properties for a single phase with a pseudo phase
   // for the other phase
   void PseudoPhaseProperties( int iM, int iNc );

   // Organizing routine for two phase property calculations
   void TwoPhase( int iM, int iNc );

   // Form and solve the matrix for the two phase properties
   void TwoPhaseMatrix( int iM, int iNc );

   // Organizing routine for two phase property calculations
   // in standard mode, i.e., with densities
   void TwoPhaseProperties( int iM, int iNc );

   // Derivatives of two phase properties
   void TwoPhaseDerivatives( int iM, int iNc, double *pDX, double *pDY, double *pDZx, double *pDZy,
      double *pDHx, double *pDHy, double *pDMx, double *pDMy );

   // Density derivatives for two phase property calculations
   void TwoPhaseDensity( int iM, int iNc, double *pDZ, double *pDX );

   // Composition derivatives for two phase property calculations
   void TwoPhaseComposition( int iM, int iNc );

   // Two phase volume calculations
   void TwoPhaseVolume( int iM, int iNc );

   //--------------------------------------------------------------------// 
   // Methods for water properties 
   //--------------------------------------------------------------------// 

   // Organizational routine for water properties
   void WaterProperties( int iUseCurrent, int iStatus );

   // Calculates the water phase compositions
   void WaterPhase( int iM, int iNc );

   // Converts water terms to a molar value if required
   void WaterMolarTerms( int iM, int iNc, int iType, double *pRo, double *pDRodp, 
      double *pDRodt, double *pDRoda );

   // Gets the total volume of the water phase
   void WaterVolume( int iM, int iNc );

   //--------------------------------------------------------------------// 
   // Compositional Grading Routines 
   //--------------------------------------------------------------------// 

   // Driver routine for compositional grading
   void CompositionalGrading( int iNc, int iType );

   // Read in data from simulator to local arrays
   void ReadGradingData( int iNc, int iDirection );

   // Write out data from local arrays to simulator
   void WriteGradingData( int iNc, int iMolar );

   // Calculate properties for compositional grading
   void CompositionalGradingDensity( int iNc, int iMolar, double *pP, double *pComp, double *pZfactor,
      double *pHt, double *pMW );

   // Compute the bubble point with respect to the phase present
   void GradingBubblePoint( int iNc, int iType, int iForceBP, int iRetrograde );

   // Take one step in compositional grading algorithm
   void CompositionalGradingDo( int iNc, int iType, int *iMove );

   // Newton iterations for compositional grading
   int CompositionalGradingNewton( int iNc );

   // Routine to calculate the chemical potential
   void ChemicalPotentials( int iNc, int iDrvt, int iGetH, int iNorm );

   //--------------------------------------------------------------------// 
   // Separator routines 
   //--------------------------------------------------------------------// 

   // Perform the separator flash
   void SeparatorFlash( int iNc, int iType, int iWater, int iFlash, EosPvtTable **pPvtTable );

   // Routine to collect all gas produced by the separator and to obtain partial volumes
   void SeparatorGas( int iNc, EosPvtTable **pPvtTable );

   // Routine when the hydrocarbons appear in only one phase
   // for calculating partial volumes
   void SeparatorHydrocarbon( int iNc );

   // Routine to handle aqueous phase components
   void SeparatorWater( EosPvtTable **pPvtTable );

   /*
   // 
   // Memory is used and reused.  
   // Thus a map is needed; it is mapped as follows:
   //
   // 1 - Size ( N = number in slice, C = components, T = total )
   // 2 - Used in organizing routines
   // 3 - Used in flash routines
   // 4 - Used in property EOS call and matrix setup
   // 5 - Used in property routines
   // 6 - Used in compositional grading
   // 7 - Used in separator routines 
   // 8 - Used in water routines 
   //
   // Note: O in table means "orthogonal"; * means okay
   //
   //                         1   2   3   4   5   6   7   8 
   // ----------------------+---+---+---+---+---+---+---+---+
   // pMolecularWeight       C    *       *   *   *   *    
   //    pSaltMW                                          *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pPhase                 N    *   *           *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pPressure              N    *   *   *   *   *   *   *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pTemperature           N    *   *   *   *   *   *   *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pMoles                 N    *       *   *   *   *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pSplit                 N    *   *   *   *   *   *
   //    pBp                      O   O   O   O   O   O
   // ----------------------+---+---+---+---+---+---+---+---+
   // pKValue                NC   *   *   *   *   *   *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pComposition           NC   *   *   *   *   *   *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pDBpdp (bp)            N            *   *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pDBpda (bp)            N            *   *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pDBpdt (bp)            N            *   *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pPhaseId               N        *   *   *   *   *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pX                     NC       *   *   *   *   *
   //    pMW                                              *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pDXdp                  NC       *   *   *   *   
   //    pH                           O
   //    pDMWdp                                           *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pDXda                  NCC      *   *   *   *   *
   //    pDMWda                                           *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pDXdt                  NC           *   *   *
   //    pFy                          *
   //    pDMWdt                                           *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pY                     NC       *   *   *   *   *
   //    pW                                               *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pDYdp                  NC       *   *   *    
   //    pDWdp                                            *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pDYda                  NCC      *   *   *   *   *
   //    pDWda                                            *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pDYdt                  NC       *   *   *       *
   //    pDWdt                                            *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pMx                    N            *   *       *
   //    pXMat                        *               O
   //    pRhoW                                            *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pDMxdp                 N                *
   //    pPoint                       *           *   *
   //    pDRhoWdp                                         *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pDMxda                 NC               *   *
   //    pPotentialx                  *           O
   //    pDRhoWda                                         *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pDMxdt                 N                *
   //    pXRhs                        *               *
   //    pDRhoWdt                                         *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pMy                    N        *   *   *       *   
   //    pMuW                                         O   *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pDMydp                 N                *
   //    pDBpdp (nobp)                    *   O
   //    pLow                         *           *    
   //    pDMuWdp                                          * 
   // ----------------------+---+---+---+---+---+---+---+---+
   // pDMyda                 NC               *       *
   //    pDBpda (nobp)                    *   O
   //    pPotentialy                  *           *
   //    pDMuWda                                          *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pDMydt                 N                *
   //    pDBpdt (nobp)                    *   O
   //    pHigh                        *           *
   //    pDMuWdt                                          *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pZx                    N        *   *   *   *   *
   //    pIftW                                            *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pDZxdp                 N        *   *   *   *   *
   //    pDIftWdp                                         *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pDZxda                 NC       *   *   *   *   *
   //    pDIftWda                                         *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pDZxdt                 N        *   *   *   *   *
   //    pDIftWdt                                         *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pZy                    N        *   *   *   *   *
   //    pHeatW                                           *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pDZydp                 N        *   *   *   *   *
   //    pDHeatWdp                                        *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pDZyda                 NC       *   *   *   *   * 
   //    pDHeatWda                                        *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pDZydt                 N        *   *   *   *   * 
   //    pDHeatWdt                                        *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pHx                    N            *   *   *
   //    pDRhoWds                                         *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pDHxdp                 N            *   *   *
   //    pGx                          *           O   *
   //    pDMuWds                                          *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pDHxda                 NC           *   *   *
   //    pDHeatWds                                        *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pDHxdt                 N            *   *   *
   //    pGy                          *           O   *
   //    pWorkW                                           *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pHy                    N            *   *   *
   //    pWorkW                                           *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pDHydp                 N            *   *   
   //    pBeta                        *           *   *
   //    pWorkW                                           *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pDHyda                 NC           *   *   
   //    pLasty                                   *
   //    pWorkW              N                            *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pDHydt                 N            *   *   
   //    pGold                        *           *   *
   //    pWorkW                                           *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pMWx                   N                *   O
   //    pG                           *           *   *
   //    pWorkW                                           *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pMWy                   N                *   O
   //    pATable                      *   *       *   *
   //    pWorkW                                           *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pMux                   N                *    
   //    pATable                      *   *       *   *
   //    pTerm1                       O           O   O
   //    pWorkW                                           *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pDMuxdp                N                *
   //    pATable                      *   *       *   *
   //    pWorkW                                           *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pDMuxda                NC               *
   //    pATable                      *   *       *   *
   //    pTermx                       O           O   O
   //    pWorkW              N                            *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pDMuxdt                N                *
   //    pATable                      *   *       *   *
   //    pWorkW                                           *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pMuy                   N                *
   //    pATable                      *   *       *   *
   //    pTerm2                       O   O       O   O
   //    pWorkW                                           *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pDMuydp                N                *
   //    pATable                      *   *       *   *
   //    pTerm3                       O   O       O   O
   //    pWorkW                                           *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pDMuyda                NC               *       O
   //    pATable                      *   *       *   *
   //    pTermy                       O   O       O   O
   //    pWorkW              N                            *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pDMuydt                N                *
   //    pATable                      *   *       *   *
   //    pWorkW                                           *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pIfx                   N                *
   //    pATable                      *   *       *   *
   //    pSumTable                            O
   //    pWorkW                                           *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pDIfxdp                N                *
   //    pATable                      *   *       *   *
   //    pSumTable                            O
   //    pWorkW                                           *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pDIfxda                NC               *
   //    pATable                      *   *       *   *
   //    pSumTable                            O
   //    pWorkW              N                            *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pDIfxdt                N                *
   //    pATable                      *   *       *   *
   //    pSumTable                            O
   //    pWorkW                                           *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pIfy                   N                *
   //    pATable                      *   *       *   *
   //    pSumTable                            O
   //    pWorkW                                           *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pDIfydp                N                *
   //    pATable                      *   *       *   *
   //    pSumTable                            O
   //    pSalinity                *                   O   *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pDIfyda                NC               *
   //    pATable                      *   *       *   *
   //    pSumTable                            O
   //    pDSda                    *                   O   *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pDIfydt                N                *
   //    pATable                      *   *       *   *
   //    pSumTable                            O
   // ----------------------+---+---+---+---+---+---+---+---+
   // pWork                  NC   O   *   *   *   *   *
   //    pSumTable                            O
   //    pFx                      *
   //    MoReS->pInd1        N                O       *
   // ----------------------+---+---+---+---+---+---+---+---+
   // pWork (beyond)         N    O   *   *   *   *   *
   //    pSumTable                            *
   //    pOSplit                  *
   //    pMWW                     *                   O   *
   //    pLastx                                   *    
   //    MoReS->pInd2                         O       *
   // ----------------------+---+---+---+---+---+---+---+---+
   // MoReS->pInd            N    *   *   *   *   ?   ?   *
   // ----------------------+---+---+---+---+---+---+---+---+
   // MoReS->pSavedPhase     T    *   *   *   *   ?   ?   
   // ----------------------+---+---+---+---+---+---+---+---+
   // MoReS->pSavedSplit     T    *   *   *   *   ?   ?   
   // ----------------------+---+---+---+---+---+---+---+---+
   // MoReS->pSavedKValue    TC   *   *   *   *   ?   ?
   // ----------------------+---+---+---+---+---+---+---+---+
   // pAbcOffset             N    *   *   *       *   *
   // ----------------------+---+---+---+---+---+---+---+---+
   //
   */

private:  
   // The maximum flash length.  Default is 64 (?) 
   // Controlled by piFlasher[EOS_MAXFLASH]
   int m_iFlashLength;

   // Specifies whether the more robust Michelson stability analysis 
   // or a less robust but faster analysis is used to determine the 
   // number of hydrocarbon phases.  
   // Controlled by piFlasher[EOS_MICHELSON] = EOS_OPTION_OFF || EOS_OPTION_ON
   int  m_iMichelson;

   // Maximum allowed iterations; 20 is reasonable for simple problems.  
   // Controlled by piFlasher[EOS_MAXITN]
   int m_iMaxIterations;

   // Number of successive substitution iterations.  
   // Substitution is not an efficient method. 1 is a reasonable
   // default as there are some easy problems which converge quickly.  
   // Controlled by piFlasher[EOS_SUBSTITUTIONS]
   int m_iSubstitutions;

   // Local copy of indicator for whether the bubble point is
   // to be calculated for single phase mixtures passed from
   // the EosApplication class
   int m_iBubbleDew;

   // Local copy of indicator for whether a pseudo phase composition
   // is to be calculated for single phase mixtures passed from
   // the EosApplication class
   int m_iPseudoProperties;

   // A very large number but well within the largest double 
   // precision number a machine can handle.  1.0E80 is a reasonable
   // default.  
   // Controlled by pdFlasher[EOS_ENORM]      
   double m_dEnorm;

   double m_dLnEnorm; ///< Log(m_dEnorm)

   // Number such that 1 + piFlasher[EOS_TINY] is equal to 1 
   // Controlled by pdFlasher[EOS_TINY]   
   double m_dTiny;

   // Convergence tolerance for flasher.  0.0001 is 
   // reasonable for reservoir simulation; 
   // use 1000.0 * piFlasher[EOS_TINY] for very precise results.
   // Controlled by pdFlasher[EOS_CONVERGENCE] 
   double m_dConverge;

   // Multiplicative factor for thermal diffusion in compositional
   // grading calculations.  If 0, thermal diffusion not used.  
   // Controlled by pdFlasher[EOS_THERMALDIFFUSION]
   double m_dThermalDiffusion;

   // Factor for reduction in initial stage of bubble point calculations, 
   // usually set to 0.5.  Set to 0.1 if convergence is an issue.
   // Controlled by pdFlasher[EOS_BUBBLEREDUCE]
   double m_dBubbleReduce;

   // Factor for scaling update to solution on Newton iteration
   // usually set to 1.0 (Newton convntional method) or between 0.0 and 1.0 to make method more stable
   // On each Newton iteration it set as min( 1.0, RelCoef * 0.1 * IterNum)
   // Controlled byt pdFlasher[EOS_NEWTON_RELAX_COEFF]
   double m_dNewtonRelaxCoeff;

   // Indicator for derivative level obtained from class EosApplication
   // The values for derivatives returned by flasher are:
   //  EOS_NODRV - No derivatives
   //  EOS_DRV_N - Only composition derivatives
   //  EOS_DRV_P - Composition and pressure derivatives.
   //  EOS_DRV_T - Composition, pressure, and temperature derivatives 
   int m_iDrv;

   // Indicator for whether enthalpy calculations will be undertaken.
   // Obtained from class EosApplication
   int m_iHeat;

   // Indicates whether the flasher is to generate debug printing.
   // Controlled by piFlasher[EOS_DEBUG] = EOS_OPTION_OFF || EOS_OPTION_ON
   int m_iDebug;

   // Indicates whether the flasher should use the k-values supplied by the user
   // or initialise them internally.
   int m_restoreKValues;

   // Indicator for what type of composition/densities will be returned.  
   // Obtained from class EosApplication.  Values are:
   //    EOS_DENSITY
   //       If EOS_APPLICATION_VOLUMES is EOS_OPTION_OFF and 
   //       EOS_APPLICATION_COMPOSITIONS is EOS_OPTION_OFF, the 
   //       flasher returns densities and compositions, the 
   //       normal mode. 
   //    EOS_VOLUME
   //       If EOS_APPLICATION_VOLUMES is EOS_OPTION_ON and 
   //       EOS_APPLICATION_COMPOSITIONS is EOS_OPTION_OFF, the 
   //       flasher returns volumes in the density array.  This 
   //       is useful for explicit simulation. 
   //    EOS_SEPARATOR
   //       If EOS_APPLICATION_VOLUMES is EOS_OPTION_OFF and 
   //       EOS_APPLICATION_COMPOSITIONS is EOS_OPTION_ON, the 
   //       flasher returns only compositions.  This is useful 
   //       for separator calculations. 
   //    EOS_STOCK_TANK
   //       If EOS_APPLICATION_VOLUMES is EOS_OPTION_ON and 
   //       EOS_APPLICATION_COMPOSITIONS is EOS_OPTION_ON, the 
   //       flasher returns volumes and compositions.  This is 
   //       useful for stock tank computations.
   int m_iVolume;

   //  Index of the water component within the components 
   //  involved in the aqueous phase.  Thus, if there are
   //  two salt components followed by a water component in
   //  the total component ordering, the index will be 2.
   //  Obtained from class EosApplication
   int m_iWaterComp;

   // Indicator as to whether molar or mass values to be
   // returned.  Obtained from class EosApplication
   int m_iMolarFlash;

   // Indicator as to bubble/dew point calculations to be
   // done, or regular flash.  Obtained from class EosApplication
   int m_iBubbleDewPoint;

   // Location of work array.  
   // DOCUMENT ME!
   double *m_pWorkArray;

   // Type of phase of the objects for each slice.  Values include:
   //    EOS_FL_0P                
   //       On input indicates that the K values are unknown.  Thus 
   //       pSavedPhase should be initialized to 0 before the first 
   //       call to the flasher.  On output indicates
   //       that no hydrocarbons are present
   //    EOS_FL_1P 
   //       On input and output indicates that a single hydrocarbon 
   //       phase is present
   //    EOS_FL_2P 
   //       On input and output indicates that two hydrocarbon phases 
   //       are present
   // During the iterations other values are posssible
   //    EOS_FL_1P_NCV   
   //       Single phase block but needs checking         
   //    EOS_FL_1P_CV        
   //       Definitely single phase     
   //    EOS_FL_2P_NCV   
   //       Two phase block but needs checking         
   //    EOS_FL_2P_CV            
   //       Definitely two phase 
   //    EOS_FL_BP_NCV          
   //       Has a bubble point but needs checking  
   //    EOS_FL_BP_CV             
   //       Definitely has a bubble point
   //    EOS_FL_BP_CHK      
   //       Check to see if pressure has decreased below bubble 
   //       point pressure
   int *m_pPhase;

   // Indicator for type of phase for objects in each slice.  
   // Values include:
   //   EOS_SINGLE_PHASE_GAS (0): Single phase gas
   //   EOS_SINGLE_PHASE_OIL (1): Single phase oil
   int *m_pPhaseId;

   // Local copy of the molecular weights of each component,
   // obtained from class EosPvtTable
   double *m_pMolecularWeight;

   double *m_pPressure;    ///< Pressure of objects in each slice
   double *m_pTemperature; ///< Temperature of objects in each slice   
   double *m_pMoles;       ///< Total moles of objects in each slice
   double *m_pSplit;       ///< Phase split of objects in each slice

   // K values of objects in each slice.  
   // Array order is: objects (first), components (last)
   double *m_pKValue;

   // Molar composition of objects in each slice.
   // Array order is: objects (first), components (last)
   double *m_pComposition;

   // Bubble point of objects in each slice.
   double *m_pBp;

   // Pressure derivative of bubble point of objects in each slice.
   double *m_pDBpdp;

   // Accumulation derivatives of bubble point of objects in each slice.
   // Array order is: objects (first), components (last)
   double *m_pDBpda;

   // Temperature derivative of bubble point of objects in each slice.
   double *m_pDBpdt;

   // Mole or mass fraction of x phase objects in each slice.  
   // Array order is: objects (first), components (last)
   double *m_pX;

   // Pressure derivative of mole or mass fraction of x-phase objects in each slice.
   // Array order is: objects (first), components (last)
   double *m_pDXdp;

   // Accumulation derivatives of mole or mass fraction of x-phase objects in each slice.  
   // The array order is: objects (first), components, component derivatives (last)
   double *m_pDXda;

   // Temperature derivative of mole or mass fraction of x-phase objects in each slice.
   // Array order is: objects (first), components (last)
   double *m_pDXdt;

   // Mole or mass fraction of y-phase objects in each slice.  
   // Array order is: objects (first), components (last)
   double *m_pY;

   //Pressure derivative of mole or mass fraction of y-phase objects in each slice.
   // Array order is: objects (first), components (last)
   double *m_pDYdp;

   // Accumulation derivatives of mole or mass fraction of y-phase objects in each slice.
   // The array order is: objects (first), components, component derivatives (last)
   double *m_pDYda;

   // Temperature derivative of mole or mass fraction of y-phase objects in each slice.
   // The array order is: objects (first), components(last)
   double *m_pDYdt;

   // Mass or moles in x-phase of objects in each slice
   double *m_pMx;

   // Pressure derivative of mass or moles in x-phase of objects in each slice
   double *m_pDMxdp;

   // Accumulation derivatives of mass or moles in x phase of objects in each slice.
   // The array order is: objects (first), components(last)
   double *m_pDMxda;

   // Temperature derivative of mass or moles in x phase of objects in each slice
   double *m_pDMxdt;

   // Mass or moles in y-phase of objects in each slice
   double *m_pMy;

   // Pressure derivative of mass or moles in y-phase of objects in each slice
   double *m_pDMydp;

   // Accumulation derivatives of mass or moles in y phase of objects in each slice.
   // The array order is: objects (first), components(last)
   double *m_pDMyda;

   // Temperature derivative of mass or moles in y phase of objects in each slice
   double *m_pDMydt;

   // Z factor, volume, or density of x phase in each slice
   double *m_pZx;

   // Pressure derivative of Z factor, volume, or density of x phase in each slice
   double *m_pDZxdp;

   // Accumulation derivative of Z factor, volume, or density of x phase in each slice.
   // The array order is: objects (first), components(last)
   double *m_pDZxda;

   // Temperature derivative of Z factor, volume, or density of x phase in each slice
   double *m_pDZxdt;

   // Z factor, volume, or density of y phase in each slice
   double *m_pZy;

   // Pressure derivative of Z factor, volume, or density of y phase in each slice
   double *m_pDZydp;

   // Accumulation derivative of Z factor, volume, or density of y phase in each slice.
   // The array order is: objects (first), components(last)
   double *m_pDZyda;

   // Temperature derivative of Z factor, volume, or density of y phase in each slice
   double *m_pDZydt;

   // Molecular weight of x phase in each slice
   double *m_pMWx;

   // Molecular weight of y phase in each slice
   double *m_pMWy;

   // Enthalpy of x phase in each slice
   double *m_pHx;

   // Pressure derivative of enthalpy of x phase in each slice
   double *m_pDHxdp;

   // Accumulation derivatives of enthalpy of x phase in each slice
   // The array order is: objects (first), components(last)
   double *m_pDHxda;

   // Temperature derivative of enthalpy of x phase in each slice
   double *m_pDHxdt;

   // Enthalpy of y phase in each slice
   double *m_pHy;

   // Pressure derivative of enthalpy of y phase in each slice
   double *m_pDHydp;

   // Accumulation derivatives of enthalpy of y phase in each slice
   // The array order is: objects (first), components(last)
   double *m_pDHyda;

   // Temperature derivative of enthalpy of y phase in each slice
   double *m_pDHydt;

   // Viscosity of x phase of objects in each slice
   double *m_pMux;

   // Pressure derivative of viscosity of x phase of objects in each slice
   double *m_pDMuxdp;

   // Accumulation derivatives of viscosity of x phase of objects in each slice.
   // The array order is: objects (first), components(last)
   double *m_pDMuxda;

   // Pressure derivative of viscosity of x phase of objects in each slice
   double *m_pDMuxdt;

   // Viscosity of y phase of objects in each slice
   double *m_pMuy;

   // Pressure derivative of viscosity of y phase of objects in each slice
   double *m_pDMuydp;

   // Accumulation derivatives of viscosity of y phase of objects in each slice.
   // The array order is: objects (first), components(last)
   double *m_pDMuyda;

   // Pressure derivative of viscosity of y phase of objects in each slice
   double *m_pDMuydt;

   // Interfacial tension between x and y phases of objects in each slice.
   double *m_pIfx;

   // Pressure derivative of interfacial tension between x and 
   // y phases of objects in each slice.
   double *m_pDIfxdp;

   // Accumulation derivatives of interfacial tension between x and 
   // y phases of objects in each slice.
   // The array order is: objects (first), components(last)
   double *m_pDIfxda;

   // Temperature derivative of interfacial tension between x and 
   // y phases of objects in each slice.
   double *m_pDIfxdt;

   // Interfacial tension between x and y phases of 
   // objects in each slice.
   double *m_pIfy;

   // Pressure derivative of interfacial tension between x and 
   // y phases of objects in each slice.
   double *m_pDIfydp;

   // Accumulation derivatives of interfacial tension between x and 
   // y phases of objects in each slice.
   // The array order is: objects (first), components(last)
   double *m_pDIfyda;

   // Temperature derivative of interfacial tension between x and 
   // y phases of objects in each slice.
   double *m_pDIfydt;

   // Temporary term usually corresponding to free energy of objects in each slice
   double *m_pG;

   // Temporary term usually corresponding to free energy of objects in each slice
   double *m_pH;

   // Temporary term usually corresponding to fugacities of objects in each slice.
   // The array order is: objects (first), components(last)
   double *m_pFx;

   // Temporary term usually corresponding to fugacities of objects in each slice.
   // The array order is: objects (first), components(last)
   double *m_pFy;

   // Temporary term usually corresponding to free energy of objects in each slice
   double *m_pGx;

   // Temporary term usually corresponding to free energy of objects in each slice
   double *m_pGy;

   // Temporary term corresponding to lowest good estimate
   // of bubble point pressure of objects in each slice
   double *m_pLow;

   // Temporary term corresponding to highest good estimate
   // of bubble point pressure of objects in each slice
   double *m_pHigh;

   // Temporary term corresponding to an additional matrix row that 
   // arises when bubble point calculations are being undertaken. 
   // The array order is: objects (first), components(last)
   double *m_pXMat;

   // Temporary term corresponding to the right hand side.  
   // The array order is: objects (first), components(last)
   double *m_pXRhs;

   // Temporary term used in the Michelson stability test
   // for objects in each slice
   double *m_pBeta;

   // Temporary term corresponding to last iteration free energy
   // of objects in each slice
   double *m_pGold;

   // Work array for objects in each slice
   // DOCUMENT ME!
   double *m_pWork;

   // Used in K value flash calculations for objects in each slice.  
   double *m_pTerm1;

   // Used in K value flash calculations for objects in each slice.  
   double *m_pTerm2;

   // Used in Choleski factorization for objects in each slice.  
   double *m_pTerm3;

   // Used in K value flash calculations for objects in each slice.
   // The array order is: objects (first), components(last)
   double *m_pTermx;

   // Used in K value flash calculations for objects in each slice.  
   // The array order is: objects (first), components(last)
   double *m_pTermy;

   // Temporary term containing (1 - phase split) for objects in each slice
   double *m_pOSplit;

   // Temporary term giving which phase is active for Michelson stability analysis
   double *m_pPoint;

   // Salinity of objects in each slice, defined as: 1 + PPM / 1000000
   double *m_pSalinity;

   // DOCUMENT ME!
   double *m_pDSdp;

   // DOCUMENT ME!
   double *m_pDSdt;

   // Accumulation derivative of salinity of objects in each slice.
   // The array order is: objects (first), components(last)
   double *m_pDSda;

   // Local copy of salt molecular weights.  
   // The water molecular weight must also be stored
   double *m_pSaltMW;

   // Water phase molecular weight of objects in each slice
   double *m_pMWW;

   // Total mass/moles in aqueous phase of objects in each slice
   double *m_pMW;

   // Pressure derivative of total mass/moles in aqueous phase of 
   // objects in each slice
   double *m_pDMWdp;

   // Accumulation derivatives of total mass/moles in aqueous phase of 
   // objects in each slice  
   // The array order is: objects (first), components(last)
   double *m_pDMWda;

   // Temperature derivative of total mass/moles in aqueous phase of 
   // objects in each slice
   double *m_pDMWdt;

   // Mass or mole fractions in water phase of objects in each slice.
   // The array order is: objects (first), components(last)
   double *m_pW;

   // Pressure derivative of mass or mole fractions in water phase of 
   // objects in each slice.
   // The array order is: objects (first), components(last)
   double *m_pDWdp;

   // Accumulation derivatives of mass or mole fractions in water 
   // phase of objects in each slice.
   // The array order is: objects (first), components, component derivatives (last) 
   double *m_pDWda;

   // Temperature derivatives of mass or mole fractions in water 
   // phase of objects in each slice.  
   // The array order is: objects (first), components(last)
   double *m_pDWdt;

   // Density or volume of water phase for objects in each slice.
   double *m_pRhoW;

   // Pressure derivative of density or volume of water phase 
   // for objects in each slice.
   double *m_pDRhoWdp;

   // Accumulation derivatives of density or volume of water phase 
   // for objects in each slice.
   // The array order is: objects (first), components(last)
   double *m_pDRhoWda;

   // Temperature derivative of density or volume of water phase 
   // for objects in each slice.
   double *m_pDRhoWdt;

   // Water phase viscosity for objects in each slice
   double *m_pMuW;

   // Pressure derivative of water phase viscosity for objects in each slice
   double *m_pDMuWdp;

   // Accumulation derivatives of water phase viscosity for objects in 
   // each slice.
   // The array order is: objects (first), components(last)
   double *m_pDMuWda;

   // Temperature derivative of water phase viscosity for objects in each slice
   double *m_pDMuWdt;

   // Interfacial tension between water- and reference-phase for objects in each slice
   double *m_pIftW;

   // Pressure derivative of interfacial tension between water-
   // and reference-phase for objects in each slice
   double *m_pDIftWdp;

   // Accumulation derivatives of interfacial tension between water- 
   // and reference-phase for objects in each slice
   // The array order is: objects (first), components(last)
   double *m_pDIftWda;

   // Temperature derivative of interfacial tension between water- 
   // and reference-phase for objects in each slice
   double *m_pDIftWdt;

   // Enthalpy of water phase for objects in each slice
   double *m_pHeatW;

   // Pressure derivative of enthalpy of water phase for objects in each slice
   double *m_pDHeatWdp;

   // Accumulation derivatives of enthalpy of water phase for objects in each slice.
   // The array order is: objects (first), components(last)
   double *m_pDHeatWda;

   // Temperature derivative of enthalpy of water phase for objects in each slice
   double *m_pDHeatWdt;

   // Salinity derivative of volume or density for objects in each slice
   double *m_pDRhoWds;

   // Salinity derivative of viscosity for objects in each slice
   double *m_pDMuWds;

   // Salinity derivative of enthalpy for objects in each slice
   double *m_pDHeatWds;

   // Work array for water phase calculations
   // DOCUMENT ME!
   double *m_pWorkW;

   // Local copy of gravity constant obtained from the EosPvtTable class
   double m_dGravity;

   // Local copy of gas constant obtained from the EosPvtTable class
   double m_dGascon;

   // Depth used in compositional grading calculations
   double m_dDepthx;

   // Depth used in compositional grading calculations
   double m_dDepthy;

   // Pressure used in compositional grading calculations
   double m_dPressure;

   //Pressure used in compositional grading calculations
   double  m_dPressurex;

   // Pressure used in compositional grading calculations
   double m_dPressurey;

   // Bubble point pressure used in compositional grading calculations 
   double m_dBubblePoint;

   // Temperature used in compositional grading calculations
   double m_dTemperature;

   // Fugacities of x phase used in compositional grading calculations.
   // The length of the array is the number of components
   double *m_pPotentialx;

   // Fugacities of y phase used in compositional grading calculations
   // The length of the array is the number of components
   double *m_pPotentialy;

   // Composition of x phase used in compositional grading calculations. 
   // The length of the array is the number of components
   double *m_pLastx;

   // Composition of y phase used in compositional grading calculations.
   // The length of the array is the number of components
   double *m_pLasty;

   // Location of the work array to pass to EosPvtTable when
   // calling the SolveCubic routine
   double *m_pATable;

   // Location of the work array to pass to EosPvtTable when
   // calling the Viscosity routine
   double *m_pSumTable;

   // Indicator if there is more than one temperature entry
   // used in the table lookups on temperature
   int m_iMultipleAbc;

   // Offset for each object in the slice in the temperature tables
   int *m_pAbcOffset;

   //--------------------------------------------------------------------// 
   // IMPORTANT: CLASS MAINTAINS HANDLES TO THESE:
   //--------------------------------------------------------------------// 

   //Pointer to the application class
   EosApplication *m_pApplication;

   //Pointer to the pvt table class 
   EosPvtTable *m_pEosPvtTable;
};
#endif
