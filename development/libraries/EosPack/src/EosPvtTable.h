// Copyright 2011, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef EOSPVTTABLE_H
#define EOSPVTTABLE_H

#include "EosWater.h"

////////////////////////////////////////////////////////////////////
// Revision history
//
// Class EosPvtTable
//
// Version 0
//    Initial release
// Version 1
//    EOS_METHOD_WATERMODEL added to enable all water phase
//    calculations to be done internal to the flasher
// Version 2
//    EOS_METHOD_VTUNE added for Jossi viscosity
// Version 3
//    EOS_METHOD_VTUNE0 etc added for Lohrenz viscosity
//    EOS_METHOD_TYPE_OF_C added to calculate standard
//    condition densities
// Version 4
//    Added corresponding states viscosity model
// Version 5
//    Added ability for linear interpolation in Bij
////////////////////////////////////////////////////////////////////
#define EOS_PVT_TABLE_VERSION 5

// Constants for getting density
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
enum
{
   EOS_DENSITY    = 0,
   EOS_VOLUME     = 1,
   EOS_SEPARATOR  = 2,
   EOS_STOCK_TANK = 3
};

/* Define whether to get fugacity */
#define EOS_NOFUGACITY 0
#define EOS_FUGACITY   1

/* Define whether to get heats - enthalpy */
#define EOS_NOHEAT 0
#define EOS_HEAT   1

/* Define whether to get chemical potential */
#define EOS_NOPOTENTIAL 0
#define EOS_POTENTIAL   1

/* Define whether to get phase identification */
#define EOS_NOPHASEID 0
#define EOS_PHASEID   1

////////////////////////////////////////////////////////////////////////////////
/// 
/// piTables[EOS_METHOD] = EOS_PVT_TABLE ||           <BR>
///                        EOS_PVT_PR ||              <BR>
///                        EOS_PVT_PRCORR ||          <BR>
///                        EOS_PVT_RK ||              <BR>
///                        EOS_PVT_SRK ||             <BR>
///                        EOS_PVT_IDEAL ||   
/// Selection of equation of state method.
///                        
/// piTables[EOS_METHOD_COMPONENTS]
/// Number of components used within the equation-of-
/// state flash calculations; does not 
/// include water and tracer components.
/// 
/// piTables[EOS_METHOD_PHASEID] = EOS_SINGLE_PHASE_GAS ||       <BR>
///                                EOS_SINGLE_PHASE_OIL ||       <BR>
///                                EOS_SINGLE_PHASE_DEFAULT ||   <BR>
///                                EOS_SINGLE_PHASE_ZMINUSB ||   <BR>
///                                EOS_SINGLE_PHASE_AOVERB 
/// Indicator for method used for single phase naming.
/// 
/// piTables[EOS_METHOD_NUMBERABC]
/// Number of temperature entries minus one in the ABC 
/// coefficient tables.  Thus, for an isothermal table, 
/// enter 0, for a table with two temperatures, enter 
/// 1, etc.  
/// 
/// piTables[EOS_METHOD_HEATDATA] = EOS_OPTION_OFF || EOS_OPTION_ON
/// Indicator as to whether reference state heat 
/// capacity data will be available to the flasher.
/// 
/// piTables[EOS_METHOD_OWNMEMORY] = EOS_OPTION_OFF || EOS_OPTION_ON
/// Indicator as to whether the class will provide its 
/// own memory or not.  Best practice 
/// would be to set to EOS_OPTION_ON as the class 
/// should provide its own memory.
///              
/// piTables[EOS_METHOD_SALTS]
/// Number of salt components.  If no salts are present the
/// water molecular weight is calculated from the salinity
///              
/// piTables[EOS_METHOD_WATERMODEL]
/// Type of water model.  Either
///    EOS_WATER_CORRELATION for correlation
///    EOS_WATER_LINEAR for linear model
/// Only used for release 2
///              
/// piTables[EOS_METHOD_TYPE_OF_C]
/// Enables calculations of c at standard conditions  <BR>
///    EOS_CONSTANT_C for constant                    <BR>
///    EOS_LINEAR_C for linear model                  <BR>
/// Only used for release 3 and later
/// 
/// piTables[EOS_METHOD_TYPE_OF_BIJ]
/// Enables calculations of bij at standard conditions <BR>
///    EOS_CONSTANT_C for constant                     <BR>
///    EOS_LINEAR_C for linear model                   <BR>
/// Only used for release 5 and later
///                  
////////////////////////////////////////////////////////////
enum
{
   EOS_METHOD              = 0,
   EOS_METHOD_COMPONENTS   = 1,
   EOS_METHOD_PHASEID      = 2,
   EOS_METHOD_NUMBERABC    = 3,
   EOS_METHOD_HEATDATA     = 4,
   EOS_METHOD_OWNMEMORY    = 5,
   EOS_METHOD_SALTS        = 6,
   EOS_METHOD_WATERMODEL   = 7,
   EOS_METHOD_TYPE_OF_C    = 8,
   EOS_METHOD_TYPE_OF_BIJ  = 9,
   EOS_METHOD_LAST_INTEGER = 10
};

/* For types of pvt table models */
enum
{
   EOS_PVT_TABLE  = 0, ///< Correlations are Application ABC tables
   EOS_PVT_PR     = 1, ///< Peng Robinson equation of state used
   EOS_PVT_PRCORR = 2, ///< Corrected Peng Robinson equation of state used
   EOS_PVT_RK     = 3, ///< Redlich Kwong equation of state used
   EOS_PVT_SRK    = 4, ///< Soave Redlich Kwong equation of state used
   EOS_PVT_IDEAL  = 5  ///< Ideal gas equation of state used

};

////////////////////////////////////////////////////////////////////
///    
/// pRArray[EOS_METHOD_CRITICAL]          
/// Pointer to array of critical values for each component. The array is of length 5 or 6 times the 
/// number of flash components.  The order of the array is first all molecular weights, then all critical 
/// pressures, then all critical temperatures, then all critical volumes, then all parachors.  If the class 
/// is initialized with its own memory, a pointer to the location for memory allocated for this array is 
/// returned.
///
/// If any equation of state other than EOS_PVT_TABLE or EOS_PVT_IDEAL is used, the array must be of length 6 times 
/// the number of flash components, as the acentric factor is the last member of this array
///
/// If EOS_PVT_TABLE or EOS_PVT_IDEAL is used as the equation of state, the acentric factors are not relevant and hence 
/// the length of the array is 5 times the number of flash components
/// 
/// pRArray[EOS_METHOD_RAWABC] 
/// Values of raw equation of state parameters for each temperature appearing in the table.  The order is, 
/// for each temperature, first the temperature, then the values of A, then the values of B, then the 
/// values of C, then the values of the interaction parameters.  Thus the number of entries is the 
/// number of temperatures times [ 1 + number of flash components * [ 3 + number of flash components ] ]. 
/// The flasher expects these in units of volume (why? Because that is what Libra did).  If the class is 
/// initialized with its own memory, a pointer to the  location for memory allocated for this array is returned.
///
/// If the pvt method is EOS_PVT_IDEAL, then this data need not be supplied, with the exception of the default temperature
///
/// If any option is used except for EOS_PVT_TABLE or EOS_PVT_IDEAL, the number of temperature entries must be 1.  In this
/// case the input is not A, B, and C but OMEGAA, OMEGAB, and the multiplicative factor for B for volume translation
/// 
/// pRArray[EOS_METHOD_ABC] 
/// Values of cached equation of state parameters for each temperature appearing in the table.  The order 
/// is slightly different as to whether one temperature entry is specified or several.  If only a single 
/// temperature is specified, then first the temperature, then the B parameter as a volume, then 
/// the C parameter as a volume, then the term with the A parameters and the interaction coefficients 
/// combined as pressure per volume.  Thus 1 + number of flash components * [ 2 + number of flash 
/// components ] ] entries are needed.  When more than a single temperature is used, then data associated 
/// with number of temperatures minus one entries is associated.   First the temperature, then the B 
/// parameter at zero temperature as a volume, then the C parameter at zero temperature as a volume, then 
/// the term with the A parameters and the interaction coefficients combined as pressure per volume at 
/// zero temperature, then the slope of the B parameter at as a volume, then the slope of the C parameter 
/// at as a volume, then slope of the term with the A parameters and the interaction coefficients 
/// combined as pressure per volume. This is sufficient information to specify the linear 
/// interpolation between each temperature. Thus the number of entries is the [ number of temperatures - 
/// 1 ] times [ 1 + number of flash components * [ 4 + 2 * number of flash components ] ].  If the class 
/// is initialized with its own memory, a pointer to the location for memory allocated for this array is returned.
///
/// If the method is not EOS_PVT_TABLE or EOS_PVT_IDEAL, the value of A and B are not cached as volumes but 
/// related values for increased performance.  
///
/// For the ideal gas case, A, B, and C are set to zero
///            
/// pRArray[EOS_METHOD_HEATS]    
/// Coefficients for zero state enthalpy values for each component.  The equation is specified as a 
/// fifth degree polynonial of the form: 
/// HeatI[T] = Heat1I * T + ... + 
///            Heat5I * T ^ 5
///
/// The corresponding entropy is
///
/// EntropyI[T] = Heat1I * ln ( T ) + 2 * Heat2I * T + 
///               3 * Heat3I * T * T / 2 + ...
///               5 * Heat5I * T ^ 4 / 4 + Heat6I
/// 
/// First Heat1 must be specified for each component, then Heat2, etc.  The terms must be in internal 
/// energy units.  Finally the entropy constant must be given.  Thus 6 times the number of flash 
/// components entries are expected. If the class is initialized with its own memory, a pointer to the 
/// location for memory allocated for this array is returned.
///            
/// pRArray[EOS_METHOD_SALTMW] 
/// Molecular weights of salt components, the number of which is governed by EOS_METHOD_SALTS.
enum
{
   EOS_METHOD_CRITICAL    = 1,
   EOS_METHOD_RAWABC      = 2,
   EOS_METHOD_ABC         = 3,
   EOS_METHOD_HEATS       = 4,
   EOS_METHOD_SALTMW      = 5,
   EOS_METHOD_LAST_DARRAY = 6
};

#define EOS_METHOD_LAST_IARRAY      1

/* 
// Class for cached pvt tables for equation of state 
*/
class EosPvtTable : public EosWater
{
   //--------------------------------------------------------------------// 
   // Routines to set up or transfer data
   //--------------------------------------------------------------------// 
public:
   // Constructors
   EosPvtTable( void );
   EosPvtTable( int iVersion, int *piTables, double *pdTables, double *pCritical, double *pvtData, double *pSaltMW );
   EosPvtTable( EosPvtTable *pCache, double dT );
   EosPvtTable( EosPvtTable &pCache );

   ///////////////////////////////////////////////////////////////////////////////
   /// \brief Constructor for corresponding state viscosity model. 
   ///        To be used for the entry point ViscosityCS
   ///
   /// \param iNcArg - Number of components (input)
   /// \param dCvpArg - Conversion factor for pressure from SI (input)
   /// \param dCvtArg - Conversion factor for temperature from SI (input)
   /// \param dCvvArg - Conversion factor for viscosity from SI (input)
   /// \param dTune1  - Tuning parameter 1
   /// \param dTune2  - Tuning parameter 2
   /// \param dTune3  - Tuning parameter 3
   /// \param dTune4  - Tuning parameter 4
   /// \param CSVscMethod - method number of viscosity model
   /// \param pPCArg  - Critical pressure
   /// \param pTCArg  - Critical temperature
   /// \param pMWArg  - Molecular weights
   ///////////////////////////////////////////////////////////////////////////////
   EosPvtTable( int iNcArg, double dCvpArg, double dCvtArg, double dCvvArg, double dTune1, double dTune2, double dTune3, 
                double dTune4, int CSVscMethod, double *pPCArg, double *pTCArg, double *pMWArg );

   /// \brief Destructor
   virtual ~EosPvtTable();

   /// \brief Get the number of hydrocarbons modelled by the Pvt tables for use by the flasher 
   int GetNumberHydrocarbons() const { return ( m_iEosComp ); }

   /// \brief Get the number of SALTS (not including water) used in aqueous phase computations by the flasher
   int GetNumberSalts() const { return ( m_iSaltComp ); }

   /// \brief Return gravity constant and gas constant to flasher for compositional grading calculations
   void GetGravity( double *pGravity, double *pGascon );

   /// \brief Print out an overview of the class data
   void PrintInputData() const;

   /// \brief Write out information contained in this class to the flasher class, e.g., molecular weights
   ///
   //  \param iM            Number of objects in slice
   /// \param i1
   /// \param i2
   /// \param isothermal    Indicator as to whether this is an isothermal system or there are temperature dependencies
   /// \param isSalt        Indicator as to whether this is for aqueous phase computations (1) or hydrocarbon phases
   /// \param iWaterComp    Indicator for the index of the water component within the aqueous phase components
   /// \param pAbcOffSetT   Pointer for the offset within the multiple ABC tables for each object.
   ///                      This will be the index for the temperature interpolation
   /// \param pMultipleAbcT Indicator as to whether there is a single ABC table involved in the computations or multiple.
   ///                      The computations within the model differ for the different models
   /// \param pT            Temperatures which are set in isothermal mode
   /// \param pMw           Return a copy of the molecular weights to the flasher
   //
   // 1) Set the molecular weights.  This depends upon which phase is being calculated and in addition as 
   //    to whether salt components or a salinity is used for water
   // 2) If isothermal set temperature from the temperature entry in the table
   // 3) If only one table entry or for water phase, only one table. Do not set temperature
   // 4) If multiple table entries do a table lookup and set the offset in the pvt tables
   // 5) If multiple tables see if more than a single table used
   void WritePvtInfo( int iM, int i1, int i2, int isothermal, int isSalt, int iWaterComp, int *pAbcOffSetT,
                      int *pMultipleAbcT, double *pT, double *pMw );

   /// \brief Provide an initial estimate of the bubble point for the flasher
   /// \return an initial guess for the bubble point calculations which is about 4000 PSIA
   double InitBubblePoint() { return ( 2.5e7 * m_dConvPressure ); }

   /// \brief Set the pointers to the ABC terms used in the flasher
   ///
   /// \param iN          Maximum number of flashes to perform in any slice
   /// \param pAbcOffsetT Pointer used for indicating the beginning of the temperature entry in the PVT tables
   /// \param pFinal      On input represents the end of the work array. Updated in this call to get a possible 
   ///                    new end of the work array
   //
   // This array is needed to ensure that there are no memory crashes between the flasher and the pvt tables, 
   // as they use the same workspace which is allocated in the flasher.  Thus
   //
   // 1) If more than one temperature entry we will need to store the entry in the table for each object being
   //    flasher. Thus allocate memory for this
   // 2) Update pFinal
   void SetAbcPointers( int iN, int **pAbcOffsetT, double **pFinal );

   /// \brief Set the pointers for temporary memory
   ///
   /// \param iN      Maximum number of flashes to perform in any slice
   /// \param pTCubic Pointer to use as start of work array when calling cubic equation of state routine 
   /// \param pTMu    Pointer to use as start of work array when calling viscosity routine 
   /// \param pTWork  Pointer to use as start of work array when calling other routines 
   /// \param pAT     Pointer to use as start of work array when calling cubic equation of state routine 
   /// \param pSumT   Pointer to use as start of work array when calling viscosity routine 
   /// \param pFinal  On input represents the end of the work array. Updated in this call to get a possible 
   ///                new end of the work array
   ///
   // This array is needed to ensure that there are no memory
   // crashes between the flasher and the pvt tables, as they
   // use the same workspace which is allocated in the flasher.  Thus
   //
   // 1) For each array, calculate the end 
   // 2) Return pFinal 
   void SetPointers( int iN, double *pTCubic, double *pTMu, double *pTWork, double **pATable, 
                     double **pSumTable, double **pFinal );

   /// \brief Writes out default temperature
   /// \returns the default temperature. The default temperature is that of the last pvt table
   double WriteTemperature() const;

protected:
   // Initialize the data
   void Initialize( int iVersion, int *piTables, double *pdTables, double *pCritical, double *pvtData, double *pSaltMW );

   // Copy the data
   void Copy( const EosPvtTable *pCache );

   // Read all the data supplied by the application for the pvt tables
   void ReadAllData( int iVersion, int **pointI, double **pointR );

   // Read the constant terms for the pvt tables
   int ReadConstants( int iVersion, int *pITerms, double *pTerms, double *pWaterData );

   // Allocate the temporary memory
   void AllocateMemory();

   // Set the terms dKb2 through dKb9 for the cubic equation 
   void SetCubicTerms( double dUserKb0, double dUserKb1 );

   // Set the array pointers
   void SetArrayPointers();

   // Set the volume translation flag
   void SetVolumeTranslation();

   // Initialize calculated terms for the pvt table
   void CachePvtTables( double *pAcentric );

   // Cache the ABC data supplied from the application into a more usuable form for computations
   void CacheAbcData();

   // Cache data supplied from the application into a more usuable form 
   // for computations for the Redlich Kwong EOS
   void CacheRKData( void );

   // Cache data supplied from the application into a more usuable form 
   // for computations for the Soave Redlich Kwong and Peng Robinson EOS
   void CacheEosData( double *pAcentric );

   //--------------------------------------------------------------------// 
   // Methods for cubic equation of state
   //--------------------------------------------------------------------// 

public:
   // Driving routine for solving the cubic equation of state
   void SolveCubic( int iM, int iGetF, int iGetH, int iGetP, int iDrv, int iPhase, 
      int iMultipleAbc, int *pAbcOffset, double *pP, double *pT, double *pX, 
      double *pZ, double *pDZdp, double *pDZdt, double *pDZdz,
      double *pPot, double *pDPotdp, double *pDPotdt, double *pDPotdz, 
      double *pHeat, double *pDHeatdp, double *pDHeatdt, double *pDHeatdz, 
      double *pA, int *pPhase );
protected:
   // Get the A and B parameters for ideal gas
   void GetIdeal( int iM, int iGetF, int iGetH, int iDrv, double *pAT, double *pBT, 
      double *pAOverBT, double *pAiOverAT, double *pBiOverBT, double *pAijOverAT, 
      double *pD2Adt2OverAT, double *pD2Bdt2OverBT, double *pDAidtOverAiT, 
      double *pDBidtOverBiT, double *pDAdtOverAT, double *pDBdtOverBT );

   // Driving routine for getting the A and B terms
   void GetABTerms( int iM, int iGetF, int iGetH, int iDrv, int iMultipleAbcT,
      int *pAbcOffsetT, double *pP, double *pT, double *pX, double *pAT, double *pBT,
      double *pAOverBT, double *pAiOverAT, double *pBiOverBT, double *pAijOverAT, 
      double *pD2Adt2OverAT, double *pD2Bdt2OverBT, double *pDAidtOverAiT, double *pDBidtOverBiT,
      double *pDAdtOverAT, double *pDBdtOverBT, double *pHelp1T, double *pHelp2T,
      double *pHelp3T, double *pHelp4T, double *pHelp5T, double *pHelp6T );

   // Get the A and B parameters for cubic equation of state tables
   void GetAB( int iM, int iGetF, int iGetH, int iDrv, int iMultipleAbcT, int *pAbcOffsetT, double *pP, double *pT,
      double *pX, double *pAT, double *pBT, double *pAOverBT, double *pAiOverAT, double *pBiOverBT,
      double *pAijOverAT, double *pD2Adt2OverAT, double *pD2Bdt2OverBT, double *pDAidtOverAiT,
      double *pDBidtOverBiT, double *pDAdtOverAT, double *pDBdtOverBT, double *pHelp1T, double *pHelp2T,
      double *pHelp3T, double *pHelp4T );

   // Get the A and B parameters for cubic equation of state
   void GetEos( int iM, int iGetF, int iGetH, int iDrv, int iMultipleAbcT, double *pP, double *pT, double *pX,
      double *pAT, double *pBT, double *pAOverBT, double *pAiOverAT, double *pBiOverBT, double *pAijOverAT,
      double *pD2Adt2OverAT, double *pD2Bdt2OverBT, double *pDAidtOverAiT, double *pDBidtOverBiT,
      double *pDAdtOverAT, double *pDBdtOverBT, double *pHelp1T, double *pHelp2T, double *pHelp3T,
      double *pHelp4T, double *pHelp5T, double *pHelp6T );

   // Get the A and B parameters for Redlich Kwong equation of state
   void GetRK( int iM, int iGetF, int iGetH, int iDrv, double *pP, double *pT, double *pX, double *pAT, double *pBT,
      double *pAOverBT, double *pAiOverAT, double *pBiOverBT, double *pAijOverAT, double *pD2Adt2OverAT,
      double *pD2Bdt2OverBT, double *pDAidtOverAiT, double *pDBidtOverBiT, double *pDAdtOverAT,
      double *pDBdtOverBT, double *pHelp1T, double *pHelp2T );

   // Get the z factor
   void ZFactor( int iM, int iDrv, int iEnergy, int iPhase, double *pP, int *pPhase, double *pZ, double *pDZdp,
      double *pDZdt, double *pDZdz, double *pAT, double *pBT, double *pAOverBT, double *pAiOverAT,
      double *pBiOverBT, double *pDAdtOverAT, double *pDBdtOverBT, double *pHelp1T, double *pHelp2T,
      double *pHelp3T, double *pHelp4T, double *pHelp5T );

   // Get the nonideal contribution to the heat
   void NonIdealHeat( int iM, int iDrv, double *pP, double *pT, double *pZ, double *pDZdp, double *pDZdt, double *pDZdz,
      double *pHeat, double *pDHeatdp, double *pDHeatdt, double *pDHeatdz, double *pBT,
      double *pAOverBT, double *pAiOverAT, double *pBiOverBT, double *pDAdtOverAT, double *pDBdtOverBT,
      double *pD2Adt2OverAT, double *pD2Bdt2OverBT, double *pDAidtOverAiT, double *pDBidtOverBiT,
      double *pHelp1T, double *pHelp2T, double *pHelp3T, double *pHelp4T, double *pHelp5T,
      double *pHelp6T );

   // Get the ideal contribution to the heat
   void IdealHeat( int iM, int iDrv, double *pT, double *pX, double *pHeat, double *pDHeatdt, double *pDHeatdz,
      double *pHelp1T );

   // DOCUMENT ME!
   void VolumeTranslationHeat( int iM, int iDrv, double *pCi, double *pDCidt, double *pD2Cidt2, double *pP, double *pT,
      double *pX, double *pHeat, double *pDHeatdp, double *pDHeatdt, double *pDHeatdz,
      double *pHelp1T, double *pHelp2T, double *pHelp3T );

   // Get either the fugacity or chemical potential
   void Potential( int iM, int iDrv, int iGetP, double *pP, double *pT, double *pZ, double *pDZdp, double *pDZdt,
      double *pDZdz, double *pPot, double *pDPotdp, double *pDPotdt, double *pDPotdz, double *pBT,
      double *pAOverBT, double *pAiOverAT, double *pBiOverBT, double *pAijOverAT, double *pDAdtOverAT,
      double *pDBdtOverBT, double *pDAidtOverAiT, double *pDBidtOverBiT, double *pHelp1T, double *pHelp2T,
      double *pHelp3T, double *pHelp4T, double *pHelp5T, double *pHelp6T, double *pHelp7T );

   // Add the ideal contributions to the chemical potentials
   void IdealPotential( int iM, int iDrv, double *pT, double *pPot, double *pDPotdt, double *pHelp1T );

   // Add the volume translation contribution to chemical potentials
   void VolumeTranslationPotential( int iM, int iDrv, double *pCi, double *pDCidt, double *pP, double *pX, double *pZ,
      double *pDZdp, double *pDZdt, double *pDZdz, double *pPot, double *pDPotdp,
      double *pDPotdt, double *pHelp1T, double *pHelp2T, double *pHelp3T );

   //--------------------------------------------------------------------// 
   // Methods for other properties
   //--------------------------------------------------------------------// 
public:
   // Convert a specific volume to density
   void Density( int iM, int iDrv, int iMolar, double *pMw, double *pZ, double *pDZdp, double *pDZdt, double *pDZdx );

   // Convert a molar density to a mass density
   void MassDensity( int iM, int iDrv, double *pDXdp, double *pDXdt, double *pDXda, double *pRho, double *pDRhodp,
      double *pDRhodt, double *pDRhoda, double *pMw, double *pWorkT );

   //Convert a molar heat to a mass heat
   void MassHeat( int iM, int iDrv, double *pMW, double *pHeat, double *pDHeatdp, double *pDHeatdt, double *pDHeatda );

   // Set the interfacial tension between two phases
   void SetTension( int iM, int iDrv, double *pIftx, double *pDIftxdp, double *pDIftxdt, double *pDIftxda,
      double *pIfty, double *pDIftydp, double *pDIftydt, double *pDIftyda, double *pWorkT );

   // Set the tension parameters for each phase
   void Tension( int iM, int iDrv, int iMolar, int iCompute, double *pX, double *pDXdp, double *pDXdt, double *pDXda,
      double *pRho, double *pDRhodp, double *pDRhodt, double *pDRhoda, double *pIft, double *pDIftdp,
      double *pDIftdt, double *pDIftda, double *pWorkT );

   // Compute the thermal diffusion term for compositional grading
   void ThermalDiffusion( int iM, double dT, double dDiffusion, double *pZx, double *pDZxda, double *pHx,
      double *pDHxda, double *pDiffusion );

   // Compute the viscosity
   void Viscosity( int iM, int iDrv, int iMolar, double dTiny, double *pP, double *pT, double *pZ, double *pDZdp,
      double *pDZdt, double *pDZda, double *pRho, double *pDRhodp, double *pDRhodt, double *pDRhoda,
      double *pMu, double *pDMudp, double *pDMudt, double *pDMuda, double *pSumT );

   // Compute the volume from a z factor
   void Volume( int iM, int iDrv, int iMultipleAbcT, int *pAbcOffsetT, double *pP, double *pT, double *pX, double *pZ,
      double *pDZdp, double *pDZdt, double *pDZdx, double *pWork );

   // Compute the ideal Wilson K values
   void WilsonKValues( int iM, int iStretch, double *pT, double *pKValue, double *pWork );

protected:
   // Get the pseudo-compositions for viscosity
   void GetComps( int iM, int iDrv, int iMolar, int iLights, double dTiny, double *pZ, double *pDZdp, double *pDZdt,
      double *pDZda, double *pX, double *pDXdp, double *pDXdt, double *pDXda, int *pNComp, double *pRho,
      double *pDRhodp, double *pDRhodt, double *pDRhoda, double *pRo, double *pDRodp, double *pDRodt,
      double *pDRoda, double *pMWC, double *pPCC, double *pTCC, double *pVCC, double *pTmp );

   // Compute the Lohrenz-Bray-Clark viscosity
   void LohrenzBC( int iM, int iDrv, int iMolar, int iNLight, double *pMWC, double *pPCC, double *pTCC, double *pVCC,
      double *pT, double *pZ, double *pDZdp, double *pDZdt, double *pDZda, double *pRho, double *pDRhodp,
      double *pDRhodt, double *pDRhoda, double *pMu, double *pDMudp, double *pDMudt, double *pDMuda,
      double *pSumT );

   // Obtain the mobility ratio between heavy and light viscosity
   void MobilityRatio( int iM, int iDrv, double *pML, double *pDMLdp, double *pDMLdt, double *pDMLda, double *pMH,
      double *pDMHdp, double *pDMHdt, double *pDMHda, double *pMR, double *pDMRdp, double *pDMRdt,
      double *pDMRda, double *pTmp );

   // Viscosity for heavy components from Glen Shealy correlation
   void ViscosityGSS( int iM, int iDrv, int iNHeavy, double *pMWC, double *pP, double *pT, double *pZ, double *pDZdp,
      double *pDZdt, double *pDZda, double *pMu, double *pDMudp, double *pDMudt, double *pDMuda,
      double *pATerm, double *pTmp );

   // DOCUMENT ME!
   void ViscosityMix( int iM, int iDrv, int iMolar, double *pZ, double *pDZdp, double *pDZdt, double *pDZda, double *pX,
      double *pDXdp, double *pDXdt, double *pDXda, double *pML, double *pDMLdp, double *pDMLdt,
      double *pDMLda, double *pMR, double *pDMRdp, double *pDMRdt, double *pDMRda, double *pMu,
      double *pDMudp, double *pDMudt, double *pDMuda, double *pTmp );

   // Add the volume translation term
   void VolumeTranslation( int iM, int iDrv, int iMultipleAbcT, int *pAbcOffsetT, double *pT, double *pX, double *pZ,
      double *pDZdt, double *pDZdx, double *pWork );

   // Get volume translation C term for each component
   void VolumeTranslationC( int iM, int iDrv, int iGetH, int iMultipleAbcT, int *pAbcOffsetT, double *pT, double *pCi,
      double *pDCidt, double *pD2Cidt2 );

   //--------------------------------------------------------------------// 
   // Methods for corresponding state viscosity model
   //--------------------------------------------------------------------// 

public:
   // Get the size of the work array needed for the computations
   int  ViscosityWorkArraySize( int iM );

   // Public entry point for the calculations
   void ViscosityCS( int iM, int iMolar, double *pP, double *pT, double *pZ, double *pViscosity, double *pDViscosityDX,
      double *pDViscosityDP, double *pDViscosityDT, double *pWork );
protected:
   // DOCUMENT ME!
   void   ViscosityCorrespondingStates( int iM, int iMolar, int iDrv, double *pP, double *pT, double *pZ, double *pDZdp,
      double *pDZdt, double *pDZda, double *pViscosity, double *pDViscosityDX,
      double *pDViscosityDP, double *pDViscosityDT, double *pWork );

   // Initialize all the cached terms for the model
   void   InitializeViscosityTerms( void );

   // Organizing routine for the methane density
   void MethaneDensity( int iM, double *pTemperatureArg, double *pPressure, double *pRho, double *pDDensityDP,
      double *pDDensityDT, double *pWork );

   // Organizing routine for the viscosity calculation
   void   CalculateViscosity( int iM, double *pP, double *pT, double *pPMix, double *pTMix, double *pMWMix,
      double *pViscosity, double *pDViscosityDP, double *pDViscosityDT,
      double *pDViscosityDPMix, double *pDViscosityDTMix, double *pDViscosityDWMMix,
      double *pWork, double *Tmeth, double *dTmethdT, double *dTmethdP,
      double *dTmethdMWmix );

   // BWRS function for methane
   void   BWRSFunction( int iM, int *iInd, double *pPressure, double *pRho, double *pFunction, double *pDerivative,
      double *pCoef0, double *pCoef1, double *pCoef2, double *pCoef3, double *pCoef4, double *pCoef5,
      double *pCoef6, double *pCoef7, double *pCoef8, double *pCoef9, double *pCoef10,
      double *pCoef11, double *pCoef12, double *pCoef13, double *pCoef14, double *pCoef15,
      double *pCoef16, double *pWork );

   // Viscosity correlation for methane
   void   ViscosityCorrelation( int iM, double *pTZero, double *pRZero, double *pViscosity, double *pDViscosityDT,
      double *pDViscosityDR, double *pWork );

   void   VscCorrHeavy( const int iM, const int iDrv, const double *T, const double *P, 
      const double *MWn, double *MWw, double *Vsc, 
      double *dVscdT, double *dVscdP, double *dVscdMWn, double *dVscdMWw );

   // Calculates the BWRS coefficients
   void   BWRSCoefficients( int iM, int iDrv, double *pTemperatureArg, double *pCoef0, double *pCoef1, double *pCoef2,
      double *pCoef3, double *pCoef4, double *pCoef5, double *pCoef6, double *pCoef7,
      double *pCoef8, double *pCoef9, double *pCoef10, double *pCoef11, double *pCoef12,
      double *pCoef13, double *pCoef14, double *pCoef15, double *pCoef16, double *pWork );

   // Calculates the saturation curve and densities
   void   SaturationCurve( int iM, double *pTemperatureArg, double *pSaturationPressure, double *pLiquidDensity,
      double *pGasDensity, double *pWork );

   //--------------------------------------------------------------------// 
   // Description of variables
   //--------------------------------------------------------------------// 

   /// Type of equation of state used.  Options include   <BR>
   //   EOS_PVT_TABLE: ACB tables
   //   EOS_PVT_PR: Peng Robinson
   //   EOS_PVT_PRCORR: Corrected Peng-Robinson
   //   EOS_PVT_RK: Redlich Kwong
   //   EOS_PVT_SRK: Soave Redlich Kwong
   //   EOS_PVT_IDEAL: Ideal gas
   // Controlled by piTables[EOS_METHOD]
   int m_iMethod; 

   // Number of components used within the equation-of-state flash; 
   // generally does not include water and tracer components.
   // Controlled by piTables[EOS_METHOD_COMPONENTS]
   int m_iEosComp;

   // Number of salt components.  If no salts are present the
   // water molecular weight is calculated from the salinity
   // and is controlled by piTables[EOS_METHOD_SALTS]
   // Note: the number of salts does NOT include the water component
   int m_iSaltComp;

   int m_iAllocLength; ///< Length of allocated memory for bookkeepping purposes

   // Pointer to allocated memory in the class if class has its own memory. 
   // Best practice would be to set to EOS_OPTION_ON as the class 
   // should provide its own memory.  Controlled by
   //        piTables[EOS_METHOD_OWNMEMORY] = EOS_OPTION_OFF || 
   //                                         EOS_OPTION_ON
   double *m_pOwnMemory;

   /// Pointer to an array of length NC of molecular weights.
   /// Controlled by the first NC entries of pRArray[EOS_METHOD_CRITICAL] 
   double *m_pMolecularWeight;

   // Pointer to an array of length NC of critical pressures
   // Controlled by the second NC entries of pRArray[EOS_METHOD_CRITICAL] 
   double *m_pCriticalPressure;

   /// Pointer to an array of length NC of critical temperatures
   /// Controlled by the third NC entries of pRArray[EOS_METHOD_CRITICAL] 
   double *m_pCriticalTemperature;

   /// Pointer to an array of length NC of critical volumes
   /// Controlled by the fourth NC entries of pRArray[EOS_METHOD_CRITICAL] 
   double *m_pCriticalVolume;

   /// Pointer to an array of length NC of parachors
   /// Controlled by the fifth NC entries of pRArray[EOS_METHOD_CRITICAL] 
   double *m_pParachor;

   /// Indicator as to whether reference state heat capacity data 
   /// will be available to the flasher. Controlled by          <BR>
   // piTables[EOS_METHOD_HEATDATA] = EOS_OPTION_OFF || EOS_OPTION_ON <BR>
   // The standard state enthalpy is calculated from
   //       H = H1 * T + H2 * T^2 + H3 * T^3 +
   //                    H4 * T^4 + H5 * T^5
   // The standard state entropy is calculated from
   //       S = H1 * ln(T) + 2 * H2 * T + 3/2 * H3 * T^2
   //           4/3 * H4 * T^3 + 5/4 * H5 * T^4 + H6
   // If heat constants are not specified the ideal
   // contribution to the enthalpy will not be calculated
   int m_iHeat;

   /// Pointer to an array of length NC of heat constants
   /// Controlled by the sixth NC entries of pRArray[EOS_METHOD_CRITICAL] 
   /// Only loaded if iHeat is specified as EOS_OPTION_ON
   double *m_pHeat0;

   /// Pointer to an array of length NC of heat constants
   /// Controlled by the seventh NC entries of pRArray[EOS_METHOD_CRITICAL] 
   /// Only loaded if iHeat is specified as EOS_OPTION_ON
   double *m_pHeat1;

   /// Pointer to an array of length NC of heat constants
   /// Controlled by the eighth NC entries of pRArray[EOS_METHOD_CRITICAL] 
   /// Only loaded if iHeat is specified as EOS_OPTION_ON
   double *m_pHeat2;

   /// Pointer to an array of length NC of heat constants
   /// Controlled by the ninth NC entries of pRArray[EOS_METHOD_CRITICAL] 
   /// Only loaded if iHeat is specified as EOS_OPTION_ON
   double *m_pHeat3;

   /// Pointer to an array of length NC of heat constants
   /// Controlled by the tenth NC entries of pRArray[EOS_METHOD_CRITICAL] 
   /// Only loaded if iHeat is specified as EOS_OPTION_ON
   double *m_pHeat4;

   /// Pointer to an array of length NC of heat constants
   /// Controlled by the eleventh NC entries of pRArray[EOS_METHOD_CRITICAL] 
   /// Only loaded if iHeat is specified as EOS_OPTION_ON
   double *m_pHeat5;

   /// Pointer to an array of length NC of heat constants
   /// Controlled by the twelfth NC entries of pRArray[EOS_METHOD_CRITICAL] 
   /// Only loaded if iHeat is specified as EOS_OPTION_ON
   double *m_pHeat6;

   /// Term for cubic equation of state.  
   /// For Redlich-Kwong set to one; for Peng Robinson set to 1+2^0.5.
   /// Controlled by pdTables[EOS_METHOD_KB0]  
   double m_dKb0;

   /// Term for cubic equation of state.  
   /// For Redlich-Kwong set to 0; for Peng Robinson set to 1-2^0.5. 
   /// Controlled by pdTables[EOS_METHOD_KB1]  
   double m_dKb1;

   double m_dKb2; ///< Calculated term for cubic equation of state equal to critical A/B
   double m_dKb3; ///< Calculated term for cubic equation of state equal to critical B   
   double m_dKb4; ///< Calculated term appearing in equation of state   
   double m_dKb5; ///< Calculated term appearing in equation of state
   double m_dKb6; ///< Calculated term appearing in equation of state
   double m_dKb7; ///< Calculated term appearing in equation of state
   double m_dKb8; ///< Calculated term appearing in equation of state
   double m_dKb9; ///< Calculated term appearing in equation of state

   // Tension conversion constant from MKS units to flasher internal units. 
   // For dynes/cm set to 1.0.  Controlled by pdTables[EOS_METHOD_TENSCONV] 
   double m_dConvTension;

   double m_dGravity; ///< Gravity constant
   double m_dGascon;  ///< Gas constant

   /// Salinity in PPM.  
   /// Used in water phase calculations when salts are not supplied.
   /// Controlled by pdTables[EOS_METHOD_SALINITY] 
   double m_dSalinity;

   /// Linear interpolation in temperature of C terms. 
   /// Controlled by piTables[EOS_METHOD_TYPE_OF_C].
   /// Options are:                                  <BR>
   ///   EOS_CONSTANT_C: constant                    <BR>
   ///   EOS_LINEAR_C: use linear model
   int m_iTypeOfC;

   /// Linear interpolation in temperature of Bij terms. 
   /// Controlled by piTables[EOS_METHOD_TYPE_OF_BIJ]
   /// Options are:                                  <BR>
   ///   EOS_CONSTANT_C: constant                    <BR>
   ///   EOS_LINEAR_C: use linear model
   int m_iTypeOfBij;

   /// Viscosity tuning parameter for Jossi/GSS model.
   /// If set to zero use Lohrenz viscosity.         <BR>
   /// Only for version 2 of flasher
   double m_dVTune;

   double m_dVTune0; ///< DOCUMENT ME!
   double m_dVTune1; ///< DOCUMENT ME!
   double m_dVTune2; ///< DOCUMENT ME!
   double m_dVTune3; ///< DOCUMENT ME!
   double m_dVTune4; ///< DOCUMENT ME!
   
   double m_dVTerm1; ///< Tuning parameter for corresponding states viscosity model
   double m_dVTerm2; ///< Tuning parameter for corresponding states viscosity model
   double m_dVTerm3; ///< Tuning parameter for corresponding states viscosity model
   double m_dVTerm4; ///< Tuning parameter for corresponding states viscosity model

   /// Version of corresponding states model.                          <BR>
   ///  0 is 2 parameter,                                              <BR>
   ///  1 is 4 parameter heavy oil PVTsim-Shell JIP formulation,       <BR>
   ///  2 is 4 parameter heavy oil non JIP formulation,
   int m_CSVscMethod;

   /// Molecular weights of salt components, the number of
   /// which is governed by EOS_METHOD_SALTS.  
   /// Controlled by pRArray[EOS_METHOD_SALTMW] 
   /// If this term is negative, it is interpretted as a salinity
   double *m_pSaltMW;

   /// Indicator for method used for single phase naming.
   ///  piTables[EOS_METHOD_PHASEID] = EOS_SINGLE_PHASE_GAS ||
   ///                                 EOS_SINGLE_PHASE_OIL ||
   ///                                 EOS_SINGLE_PHASE_DEFAULT || 
   ///                                 EOS_SINGLE_PHASE_ZMINUSB || 
   ///                                 EOS_SINGLE_PHASE_AOVERB       <BR>
   ///  EOS_SINGLE_PHASE_GAS                                         <BR>
   ///     Single phase mixture to be treated as a gas               <BR>
   ///  EOS_SINGLE_PHASE_OIL                                         <BR>
   ///     Single phase mixture to be treated as an oil              <BR>
   ///  EOS_SINGLE_PHASE_DEFAULT                                     <BR>
   ///     Single phase mixture to be named from critical A/B        <BR>
   ///  EOS_SINGLE_PHASE_ZMINUSB                                     <BR>
   ///     Single phase mixture to be named from Z-B                 <BR>
   ///  EOS_SINGLE_PHASE_AOVERB                                      <BR>
   ///     Single phase mixture to be named from user supplied A/B   <BR>
   int m_iTypeOfSingle;

   /// Critical z factor minus b for naming a single phase mixture 
   /// when piTables[EOS_METHOD_PHASEID] is EOS_SINGLE_PHASE_ZMINUSB. <BR>
   /// 0.25 is the MoReS default.                                     <BR>
   /// Controlled by pdTables[EOS_METHOD_CRITZMINUSB]                 <BR>
   double m_dCritZMinusB;

   /// Critical a over b for naming a single phase mixture when 
   /// piTables[EOS_METHOD_PHASEID] is EOS_SINGLE_PHASE_AOVERB.       <BR>
   /// 5.0 is a reasonable default value.                             <BR>
   /// Controlled by pdTables[EOS_METHOD_CRITAOVERB]                  <BR>
   double m_dCritAOverB;

   int m_iTranslate;///< Internal indicator as to whether volume translation is being used

   /// Number of temperature entries minus one in the ABC coefficient tables.  
   /// For an isothermal table, enter 0, for a table with two temperatures, enter  1, etc.
   /// Controlled by piTables[EOS_METHOD_NUMBERABC]
   int m_iTables;

   /// Values of raw equation of state parameters for each temperature appearing in the table.
   /// The order is, for each temperature, first the temperature, then the values of A, 
   /// then the values of B, then the values of C, then the values of the interaction 
   /// parameters.  Thus the number of entries is the number of temperatures times 
   /// [ 1 + number of flash components * [ 3 + number of flash components ] ]. 
   /// The flasher expects these in units of volume (why? Because that is what Libra did).
   /// If the class is initialized with its own memory, a pointer to the location for 
   /// memory allocated for this array is returned.  
   /// Controlled by pRArray[EOS_METHOD_RAWABC] 
   double *m_pRawAbcData;

   /// Array of cached temperatures for the PVT table.
   /// If a single temperature, then one entry.  Otherwise iTables entries
   double *m_pTemperature;

   /// Array of volumes*R*T (AiAj)^0.5*(1-bij) for the PVT table
   /// at zero temperature.    If a single temperature, then NC*NC 
   /// entries.  Otherwise iTables*NC*NC entries
   double *m_pAdata;

   /// Array of volumes Bi for the PVT table at zero temperature. 
   /// If a single temperature, then NC entries. Otherwise iTables*NC entries
   double *m_pBdata;

   /// Array of volumes Ci for volume translation in the PVT table
   /// at zero temperature. If a single temperature, then NC 
   /// entries.  Otherwise iTables*NC entries
   double *m_pCdata;

   /// Array of A values at default temperature for volumes*R*T 
   /// (AiAj)^0.5*(1-bij) for the PVT table.  Always used if 
   /// iMethod is not EOS_PVT_TABLE, EOS_PVT_IDEAL, or EOS_PVT_RK.
   /// Otherwise not used
   double *m_pADefault;

   /// Array of square root of temperature terms of volumes*R*T 
   /// (AiAj)^0.5*(1-bij) for the PVT table.  Always used if 
   /// iMethod is not EOS_PVT_TABLE or EOS_PVT_IDEAL. 
   /// Otherwise not used
   double *m_pDAdsdata;

   /// Array of linear temperature terms of volumes*R*T 
   /// (AiAj)^0.5*(1-bij) for the PVT table.  Always used if 
   /// iMethod is not EOS_PVT_TABLE or EOS_PVT_IDEAL. Otherwise if a 
   /// single temperature no entries since constant.  Otherwise 
   /// iTables*NC*NC entries Linear interpolation is then used for 
   /// each temperature
   double *m_pDAdtdata;

   /// Array of 3/2 power temperature terms of volumes*R*T 
   /// (AiAj)^0.5*(1-bij) for the PVT table.  Only used if 
   /// iMethod is not EOS_PVT_TABLE or EOS_PVT_IDEAL and temperature
   /// dependent binary interaction terms used. Otherwise if a 
   /// single temperature no entries since constant.  
   /// iTables*NC*NC entries.
   double *m_pDAdrdata;

   /// Array of 2 power temperature terms of volumes*R*T 
   /// (AiAj)^0.5*(1-bij) for the PVT table.  Only used if 
   /// iMethod is not EOS_PVT_TABLE or EOS_PVT_IDEAL and temperature
   /// dependent binary interaction terms used. Otherwise if a 
   /// single temperature no entries since constant.  
   /// iTables*NC*NC entries.
   double *m_pDAdldata;

   /// Array of temperature derivative of volumes Bi for the PVT table.
   /// If a single temperature no entries since constant.  
   /// Otherwise iTables*NC entries.
   /// Linear interpolation is then used for each temperature
   double *m_pDBdtdata;

   /// Array of temperature derivative of volume translation Ci
   /// for the PVT table.  If a single temperature
   /// no entries since constant unless the iLinearC flag is
   /// on.  Otherwise iTables*NC entries
   /// Linear interpolation is then used for each temperature
   double *m_pDCdtdata;
   
   double *m_pPCMod; ///< Cached term for corresponding states viscosity model
   double *m_pTCMod; ///< Cached term for corresponding states viscosity mode
};


#endif
