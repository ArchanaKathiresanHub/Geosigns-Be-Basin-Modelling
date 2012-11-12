// Copyright 2011, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef EOS_APPLICATION_H
#define EOS_APPLICATION_H

#include "EosPvtDefinitions.h"

// Class EosApplication
//
// Version 0
//    Initial release
//

////////////////////////////////////////////////////////////////////////////////
// piApplication[APPLICATION] = EOS_MORES (Not used currently)
//                     
// piApplication[EOS_APPLICATION_COMPONENTS]
// Total number of components in the application, including components such as water which are not 
// flashed within the equation of state flasher. When the flasher is called in the general rather than 
// the MoReS specific mode, should be set to the number of flash components.
//                     
// piApplication[EOS_APPLICATION_NUMBERPHASES]
// Total number of phases in the application, including phases which are not treated within
// the flasher explicitly
// 
// piApplication[EOS_APPLICATION_FLASHES]
// Number of flashes to be done by the flasher.
// 
// piApplication[EOS_APPLICATION_THERMAL] = EOS_OPTION_OFF || EOS_OPTION_ON         
// Indicator as to whether an external temperature array is to be specified.  Otherwise, the 
// temperature associated with the class EosPvtTable is used.
// 
// piApplication[EOS_APPLICATION_WATER]
// C index of water component; set to -1 if not present.
// 
// piApplication[EOS_APPLICATION_LIQUID]
// C index of liquid phase; set to -1 if not present. The flasher can handle single phase oil or single 
// phase gas systems.
//  
// piApplication[EOS_APPLICATION_VAPOUR]
// C index of vapour phase; set to -1 if not present. The flasher can handle single phase oil or single 
// phase gas systems.
// 
// piApplication[EOS_APPLICATION_AQUEOUS]
// C index of aqueous phase; set to -1 if not present. Within MoReS, can be set to negative one even if 
// aqueous phase present in simulation in order to use another MoReS method to compute the aqueous phase 
// properties.
// 
// piApplication[EOS_APPLICATION_OBJECTS]
// Total number of objects in the Application arrays; indirection controls which objects are filled by flasher
// 
// piApplication[EOS_APPLICATION_RESTORE] = EOS_OPTION_OFF || EOS_OPTION_ON
// Indicator as to whether K values are to be restored from the previous K values at the beginning of each
// new flash.
// 
// piApplication[EOS_APPLICATION_OWNMEMORY] = EOS_OPTION_OFF || EOS_OPTION_ON
// 
// Indicator as to whether the memory for results to be controlled by the flasher or the external 
// application.  For the EosApplication class set to EOS_OPTION_OFF as the memory requirements are calculated
// by EOSGETMEMORY.  For other applications using the EosApplication class, best practice is to set to 
// EOS_OPTION_ON since each class should manage its own memory.
//  
// piApplication[EOS_APPLICATION_DERIVATIVES] = EOS_NODRV || 
//                                              EOS_DRV_N || 
//                                              EOS_DRV_P || 
//                                              EOS_DRV_T 
// Derivative level.  Set to EOS_DRV_P for MoRes.
// 
// piApplication[EOS_APPLICATION_VISCOSITY] = EOS_OPTION_OFF || EOS_OPTION_ON
// Indicator for whether viscosity to be calculated. Must currently be set to EOS_OPTION_ON in MoReS.
// 
// piApplication[EOS_APPLICATION_TENSION] = EOS_OPTION_OFF || EOS_OPTION_ON 
// Indicator for whether tension terms to be calculated. Must currently be set to EOS_OPTION_ON in MoReS.
// 
// piApplication[EOS_APPLICATION_HEAT] = EOS_OPTION_OFF || EOS_OPTION_ON
// Indicator for whether enthalpy terms to be calculated. Must currently be set to EOS_OPTION_OFF in MoReS.
// 
// piApplication[EOS_APPLICATION_VOLUMES] = EOS_OPTION_OFF || EOS_OPTION_ON 
// Indicator as to whether volumes or densities to be returned in the density array. Must currently be 
// set to EOS_OPTION_OFF in MoReS.  See note below.
// 
// piApplication[EOS_APPLICATION_COMPOSITIONS] = EOS_OPTION_OFF || EOS_OPTION_ON
// Indicator as to whether compositions to be returned. Must currently be set to EOS_OPTION_OFF 
// in MoReS. Note that this entry and the EOS_APPLICATION_VOLUMES entry work in tandem:
// 
//   - If EOS_APPLICATION_VOLUMES is EOS_OPTION_OFF and EOS_APPLICATION_COMPOSITIONS is EOS_OPTION_OFF, the 
//     flasher returns densities and compositions, the normal mode. 
// 
//   - If EOS_APPLICATION_VOLUMES is EOS_OPTION_ON and EOS_APPLICATION_COMPOSITIONS is EOS_OPTION_OFF, the 
//     flasher returns volumes in the density array. This is useful for explicit simulation. 
// 
//   - If EOS_APPLICATION_VOLUMES is EOS_OPTION_OFF and EOS_APPLICATION_COMPOSITIONS is EOS_OPTION_ON, the 
//     flasher returns only compositions. This is useful for separator calculations. 
// 
//   - If EOS_APPLICATION_VOLUMES is EOS_OPTION_ON and EOS_APPLICATION_COMPOSITIONS is EOS_OPTION_ON, the 
//     flasher returns volumes and compositions. This is useful for stock tank computations.
// 
// piApplication[EOS_APPLICATION_MOLARDENSITY] = EOS_OPTION_OFF || EOS_OPTION_ON
// Indicates that volume/density terms will be returned on a mass basis even when the compositions 
// are specified in molar terms. Not used for MoReS.
// 
// piApplication[EOS_APPLICATION_MOLEFRACTIONS] = EOS_OPTION_OFF || EOS_OPTION_ON
// Indicates whether compositions to be specified on a mass or mole basis.  Set to EOS_OPTIONS_OFF in 
// MoReS to indicate mass compositions used. 
// 
// piApplication[EOS_APPLICATION_MOLECULARWEIGHT] = EOS_OPTION_OFF || EOS_OPTION_ON
// Indicator as to whether the flasher will return the phase molecular weights.  Set to EOS_OPTION_OFF in MoReS.
// 
// piApplication[EOS_APPLICATION_FLASH] = EOS_OPTION_OFF || EOS_OPTION_ON
// Indicator as to whether the flasher will perform flash calculations.  Set to EOS_OPTION_ON in MoReS. 
// Useful in explicit simulations where the flash calculations were used to earlier compute the 
// volume balance and at the beginning of the next time step properties must be updated from saved K values.
// 
// piApplication[EOS_APPLICATION_PROPERTIES] = EOS_OPTION_OFF || EOS_OPTION_ON 
// Indicator as to whether the flasher will update properties, including volume balance, as determined 
// by the property flag.  Set to EOS_OPTION_ON in MoReS.  Useful if flasher invoked to obtain the 
// bubble point but properties are not desired.
// 
// piApplication[EOS_APPLICATION_GETBUB] = EOS_OPTION_OFF || EOS_OPTION_ON 
// 
// Indicator as to whether bubble point or dew point calculations will be performed as opposed to the 
// standard flash calculations. Set to EOS_OPTION_OFF in MoReS. This is slightly different than the 
// EOS_APPLICATION_BUBDEW entry for the class EosPvtModel; the EOS_APPLICATION_GETBUB entry means get the bubble 
// point no matter what the pressure and compute the properties at the bubble point pressure; the 
// EOS_APPLICATION_BUBDEW entry indicates track the bubble point.
// 
// piApplication[EOS_APPLICATION_INITIALIZE] = 
//                                      EOS_FLASH_CALCULATIONS || 
//                                      EOS_COMPOSITIONAL_GRADING ||
//                                      EOS_SEPARATOR_FLASH 
// Mode in which the flasher is invoked.  Must be EOS_FLASH_CALCULATIONS currently in MoReS.  
// 
// piApplication[EOS_APPLICATION_TABLEENTRY]
// Entry in depth table at which pressure is specified in initialization calculations.  For separator calculations
// also used internally.  
//
// piApplication[EOS_APPLICATION_PSEUDO] = EOS_OPTION_OFF || EOS_OPTION_ON
// Compute pseudo-phase properties if a phase is not present.  If turned off, the values of 
// EOS_DEFAULT_DENSITY, etc., are used.  If turned on, either the bubble point composition or an 
// approximation involving the Wilson K values is used to compute pseudo-phase properties, depending upon 
// whether EOS_METHOD_BUBDEW is specified.  Only of real use in MoReS, which requires a pseudo-phase property 
// for phases not present.
//                     
// piApplication[EOS_APPLICATION_BUBDEW] = EOS_OPTION_OFF || EOS_OPTION_ON
// Indicates that the bubble or dew point will be computed within the flash calculations.  If 
// specified, the pseudo phase composition is determined from the equilibrium values at the 
// bubble or dew point.
//                     
// EOS_APPLICATION_LAST_INTEGER 
// Number of elements in integer data for class EosApplication or EosApplication. 
enum
{
   APPLICATION                     = 0,
   EOS_APPLICATION_COMPONENTS      = 1,
   EOS_APPLICATION_NUMBERPHASES    = 2,
   EOS_APPLICATION_FLASHES         = 3,
   EOS_APPLICATION_THERMAL         = 4,
   EOS_APPLICATION_WATER           = 5,
   EOS_APPLICATION_LIQUID          = 6,
   EOS_APPLICATION_VAPOUR          = 7,
   EOS_APPLICATION_AQUEOUS         = 8,
   EOS_APPLICATION_OBJECTS         = 9,
   EOS_APPLICATION_RESTORE         = 10,
   EOS_APPLICATION_OWNMEMORY       = 11,
   EOS_APPLICATION_DERIVATIVES     = 12,
   EOS_APPLICATION_VISCOSITY       = 13,
   EOS_APPLICATION_TENSION         = 14,
   EOS_APPLICATION_HEAT            = 15,
   EOS_APPLICATION_VOLUMES         = 16,
   EOS_APPLICATION_COMPOSITIONS    = 17,
   EOS_APPLICATION_MOLARDENSITY    = 18,
   EOS_APPLICATION_MOLEFRACTIONS   = 19,
   EOS_APPLICATION_MOLECULARWEIGHT = 20,
   EOS_APPLICATION_FLASH           = 21,
   EOS_APPLICATION_PROPERTIES      = 22,
   EOS_APPLICATION_GETBUB          = 23,
   EOS_APPLICATION_INITIALIZE      = 24,
   EOS_APPLICATION_TABLEENTRY      = 25,
   EOS_APPLICATION_PSEUDO          = 26,
   EOS_APPLICATION_BUBDEW          = 27,
   EOS_APPLICATION_LAST_INTEGER    = 28
};

// Define constants for loads
#define EOS_TO_APPLICATION 0
#define EOS_TO_INTERNAL    1

// Associated with class EosApplication is an array of pointers to real arrays pdApplication. This array can be destroyed 
// after the class is initialized. The following values either must be set or in some cases are 
// allocated when a class is initialized with its own memory. If the array is not to be used, can be 
// set to ( double* ) 0.
//
// pdApplication[EOS_APPLICATION_CHANGE]  
// Minimum relative change in overall compositions or pressures to perform a stability test on a single 
// phase mixture.  0.0001 is a reasonable default value.  
//
// pdApplication[EOS_APPLICATION_PMIN]
// Minimum pressure for flasher.  Used in class EosApplication since MoReS may send negative pressures 
// (!) to the flasher.
enum
{
   EOS_APPLICATION_CHANGE      = 0,
   EOS_APPLICATION_PMIN        = 1,
   EOS_APPLICATION_LAST_DOUBLE = 2
};

// Associated with class EosApplication is an array of pointers 
// to integer arrays pIArray. This array can be destroyed after the class is initialized.  The 
// following values either must be set or in some cases are allocated when a class is initialized 
// with its own memory.   If the array is not to be used, can be set to ( int* ) 0.
// 
// pIArray[INTEGERDATA]      
// Pointer to the piApplication array. The pointer to this array must ALWAYS be specified.
enum
{
   INTEGERDATA     = 0,
   EOS_LAST_IARRAY = 1
};

// pIArray[EOS_APPLICATION_EOSCOMPS]    
// Pointer to array giving translation from internal flasher numbering to external application numbering 
// of components used in the two phase flasher. The pointer to this array must ALWAYS be specified.
//
// pIArray[EOS_APPLICATION_INDIRECTION] 
// Pointer to indirection array.  The pointer to this array must ALWAYS be specified for flash and separator 
// calculations; not used for initialization computations.
//  
// pIArray[EOS_APPLICATION_PHASES]   
// Pointer to array used to save state of system for generating initial guess at next time step when
// the restore option is used.  The pointer to this array must be specified when saving information from the 
// last flash.
//  
// EOS_APPLICATION_LAST_IARRAY 
// Number of elements in array of pointers to integer arrays in class EosApplication or EosMoReS.
enum
{
   EOS_APPLICATION_EOSCOMPS    = 1,
   EOS_APPLICATION_INDIRECTION = 2,
   EOS_APPLICATION_PHASES      = 3,
   EOS_APPLICATION_LAST_IARRAY = 4
};

// pRArray[REALDATA]        
// For all pointer to the pRData array
//
// pRArray[EOS_APPLICATION_TEMPERATURE]    
// Pointer to temperature array of length number of objects.  Can be set to the null pointer in an 
// isothermal situation. If the class is initialized with its own memory, a pointer to the location for 
// memory allocated for this array is returned.
// 
// pRArray[EOS_APPLICATION_ACCUMULATION] 
// Pointer to accumulation array (mass or moles) of length number of objects times the number of 
// application components, stored in that order. This is input to the flasher.  If the class is 
// initialized with its own memory, a pointer to the location for memory allocated for this array is 
// returned.  In the case of compositional grading calculations, this array need only be specified at 
// the reference depth.
// 
// pRArray[EOS_APPLICATION_FRACTION] 
// Pointer to phase component mass or moles fraction array of length number of objects times the number 
// of application components times the number of phases, stored in that order. If the class is 
// EosApplication, note that enthalpy information is stored as the last component in this array.  When using 
// separator or surface flashes the information is returned as total mass or moles in the phase.  If 
// the class is initialized with its own memory, a pointer to the location for memory allocated for 
// this array is returned.
//
// The enthalpy will be returned in the last component entry as an extra component if requested
// 
// pRArray[EOS_APPLICATION_DFRACTION] 
// Pointer to derivatives of phase component mass or moles fraction array of length number of objects 
// times the number of application components times the number of derivatives times the number of 
// phases, stored in that order. Derivative information is stored as first the pressure, then 
// the compositions, and finally the temperature. If pressure derivatives are not requested, note that 
// the composition derivative is in the first location. If the class is EosApplication, note that 
// enthalpy information is stored as the last component in this array.  If the class is 
// initialized with its own memory, a pointer to the location for memory allocated for this array is returned.
//
// The enthalpy derivatives will be returned in the last component entry as an extra component if requested
// 
// pRArray[EOS_APPLICATION_TOTALPHASE] 
// Pointer to phase total mass or moles array of length number of objects times the number of 
// phases, stored in that order. Only returned in the standard flash mode, and not for volume or 
// separator modes.  If the class is initialized with its own memory, a pointer to the location for 
// memory allocated for this array is returned.
// 
// pRArray[EOS_APPLICATION_DTOTALPHASE] 
// Pointer to derivatives of phase total mass or moles array of length number of objects times the number 
// of derivatives times the number of phases, stored in that order. Derivative information is stored as 
// first the pressure, then the compositions, and finally the temperature.  If pressure derivatives 
// are not requested, note that the composition derivative is in the first location. Only returned 
// in the standard flash mode, and not for volume or separator modes.  If the class is initialized with 
// its own memory, a pointer to the location for memory allocated for this array is returned.
// 
// pRArray[EOS_APPLICATION_DENSITY] 
// Pointer to phase density array of length number of objects times the number of phases, stored in that 
// order. Returned as a density in the standard flash  mode, volume in volume or surface modes, and not 
// returned for separator mode.  If the class is initialized with its own memory, a pointer to the 
// location for memory allocated for this array is returned.
// 
// pRArray[EOS_APPLICATION_DDENSITY] 
// Pointer to derivatives of phase density array of length number of objects times the number of 
// derivatives times the number of phases, stored in that order. Derivative information is stored as 
// first the pressure, then the compositions, and finally the temperature.  If pressure derivatives 
// are not requested, note that the composition derivative is in the first location. Returned as a 
// density in the standard flash mode, volume in volume or surface modes, and not returned for 
// separator mode.  If the class is initialized with its own memory, a pointer to the location for 
// memory allocated for this array is returned.
// 
// pRArray[EOS_APPLICATION_VISCOSITIES] 
// Pointer to phase viscosity array of length number of objects times the number of phases, stored in 
// that order. Only returned if information requested. If the class is initialized with its own memory, a 
// pointer to the location for memory allocated for this array is returned.
// 
// pRArray[EOS_APPLICATION_DVISCOSITIES] 
// Pointer to derivatives of phase viscosity array of length number of objects times the number of 
// derivatives times the number of phases, stored in that order. Derivative information is stored as 
// first the pressure, then the compositions, and finally the temperature.  If pressure derivatives 
// are not requested, note that the composition derivative is in the first location.  Only returned 
// if requested.  If the class is initialized with its own memory, a pointer to the location for memory 
// allocated for this array is returned.
// 
// pRArray[EOS_APPLICATION_TENSIONS] 
// Pointer to phase tension array of length number of objects times the number of phases, stored in that 
// order. Only returned if information requested.  If the class is initialized with its own memory, a 
// pointer to the location for memory allocated for this array is returned.
// 
// pRArray[EOS_APPLICATION_DTENSIONS] 
// Pointer to derivatives of phase tension array of length number of objects times the number of 
// derivatives times the number of phases, stored in that order. Derivative information is stored as 
// first the pressure, then the compositions, and finally the temperature.  If pressure derivatives 
// are not requested, note that the composition derivative is in the first location.  Only returned 
// if requested.  If the class is initialized with its own memory, a pointer to the location for memory 
// allocated for this array is returned.
// 
// pRArray[EOS_APPLICATION_DEPTH]       
// Pointer to depth array of length number of objects, used in initialization calculations.  Must be 
// monotonic. If the class is initialized with its own memory, a pointer to the location for memory 
// allocated for this array is returned.
//   
// pRArray[EOS_APPLICATION_PRESSURE] 
// Pointer to reference pressure array of length number of objects.  If initialization calculations, 
// need only be specified at the reference depth. If the class is initialized with its own memory, a 
// pointer to the location for memory allocated for this array is returned.
// 
// pRArray[EOS_APPLICATION_SPLIT] 
// Pointer to saved phase split array of length number of objects.  Only used when in restore mode, i.e., the 
// results of flash calculations are saved as initial values for the next iteration. 
// 
// pRArray[EOS_APPLICATION_KVALUES] 
// Pointer to saved K-Value array of length number of objects times the number of application components.  
// Only used in restore mode, i.e., when the results of flash calculations are saved as initial values for 
// the next iteration. 
// 
// pRArray[EOS_APPLICATION_BPRESSURE] 
// Pointer to bubble point pressure array of length number of objects. Only returned if requested.  If 
// the class is initialized with its own memory, a pointer to the location for memory allocated for 
// this array is returned.
// 
// pRArray[EOS_APPLICATION_DPRESSURE] 
// Pointer to derivatives of bubble point pressure array of length number of objects times the number 
// of derivatives, in that order. Derivative information is stored as first the pressure, then 
// the compositions, and finally the temperature. If pressure derivatives are not requested, note that 
// the composition derivative is in the first location. Only returned if requested.  If the class 
// is initialized with its own memory, a pointer to the location for memory allocated for this array is 
// returned.
// 
// pRArray[EOS_APPLICATION_MW] 
// Pointer to molecular weight array of length number of objects times the number of phases. Only 
// returned if requested.  If the class is initialized with its own memory, a pointer to the location for 
// memory allocated for this array is returned.
// 
// pRArray[EOS_APPLICATION_PHASEPRESSURE] 
// Pointer to phase pressure array of length number of objects times the number of phases. Only returned 
// from initialization computations.  If the class is initialized with its own memory, a pointer to the 
// location for memory allocated for this array is returned.
// 
enum
{
   REALDATA                      = 0,
   EOS_APPLICATION_TEMPERATURE   = 1,
   EOS_APPLICATION_ACCUMULATION  = 2,
   EOS_APPLICATION_FRACTION      = 3,
   EOS_APPLICATION_DFRACTION     = 4,
   EOS_APPLICATION_TOTALPHASE    = 5,
   EOS_APPLICATION_DTOTALPHASE   = 6,
   EOS_APPLICATION_DENSITY       = 7,
   EOS_APPLICATION_DDENSITY      = 8,
   EOS_APPLICATION_VISCOSITIES   = 9,
   EOS_APPLICATION_DVISCOSITIES  = 10,
   EOS_APPLICATION_TENSIONS      = 11,
   EOS_APPLICATION_DTENSIONS     = 12,
   EOS_APPLICATION_DEPTH         = 13,
   EOS_APPLICATION_PRESSURE      = 14,
   EOS_APPLICATION_SPLIT         = 15,
   EOS_APPLICATION_KVALUES       = 16,
   EOS_APPLICATION_BPRESSURE     = 17,
   EOS_APPLICATION_DPRESSURE     = 18,
   EOS_APPLICATION_MW            = 19,
   EOS_APPLICATION_PHASEPRESSURE = 20,
   EOS_APPLICATION_LAST_DARRAY   = 21
};

/// \brief General application interface API
class EosApplication
{
public:
   EosApplication() {}
   virtual ~EosApplication() {}

   // Write out the index of the water component 
   // Returns Application index of water component
   virtual int  WriteWaterIndex( void ) { return ( 0 ); }

   // See if we need to restore old K values
   // Returns value of 1 if to restore from old K values
   virtual int WriteOldValues( void )  { return ( 0 ); }

   // Write/return total number of active flashes 
   virtual int WriteNumberFlashes( void ) { return ( 0 ); }

   // Write out whether we are isothermal
   // Returns 1 if isothermal, 0 if thermal
   virtual int WriteIsothermal( void ) { return ( 0 ); }

   // Write/returns the minimum temperature
   virtual double WriteMinimumTemperature( void )  { return ( 0.0 ); }

   /// \brief Various terms needed for output 
   /// \param pDrv           derivative level
   /// \param pGetV          get viscosity
   /// \param pGetT          get tension
   /// \param pGetH          get enthalpies
   /// \param pGetM          get molecular weights
   /// \param pVolume        indicator for whether densities or volumes returned
   /// \param pMolar         output is to be molar
   /// \param pMolarDensity  keep molar output as molar densities, otherwise convert to mass
   virtual void WriteOutputData( int *pDrv, int *pGetV, int *pGetT, int *pGetH, int *pGetM, int *pVolume, int *pMolar, int *pMolarDensity )
   {
      USE_PARAM_EOSPVT( pDrv );
      USE_PARAM_EOSPVT( pGetV );
      USE_PARAM_EOSPVT( pGetT );
      USE_PARAM_EOSPVT( pGetH );
      USE_PARAM_EOSPVT( pGetM );
      USE_PARAM_EOSPVT( pVolume );
      USE_PARAM_EOSPVT( pMolar );
      USE_PARAM_EOSPVT( pMolarDensity );
   }

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
   /// \param pBubbleDew Indicates whether bubble point tracking to be used for a single phase mixture
   /// \param pPseudoPropertiesIndicates whether pseudo phase properties to be generated for Application
   ///
   // 1) Access and return data
   virtual void WriteControlData( int *pType, int *pSaved, int *pNobj, int *pFlash, int *pProp, int *pBubble, int *pWater, int *pInit, int *pBubbleDew, int *pPseudoProperties )
   {
      USE_PARAM_EOSPVT( pType );
      USE_PARAM_EOSPVT( pSaved );
      USE_PARAM_EOSPVT( pNobj );
      USE_PARAM_EOSPVT( pFlash );
      USE_PARAM_EOSPVT( pProp );
      USE_PARAM_EOSPVT( pBubble );
      USE_PARAM_EOSPVT( pWater );
      USE_PARAM_EOSPVT( pInit );
      USE_PARAM_EOSPVT( pBubbleDew );
      USE_PARAM_EOSPVT( pPseudoProperties );
   }

   /// \brief Load grid block properties into temporary vectors 
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
   virtual void WriteData( int iM, int i1, int i2, int iGetK, int isSalt, double *pP, double *pT, 
                           double *pZ, double *pSplit, double *pValueK, int *pPhases, int *pIsothermal, int *pMolar )
   {
      USE_PARAM_EOSPVT( iM );
      USE_PARAM_EOSPVT( i1 );
      USE_PARAM_EOSPVT( i2 );
      USE_PARAM_EOSPVT( iGetK );
      USE_PARAM_EOSPVT( isSalt );
      USE_PARAM_EOSPVT( pP );
      USE_PARAM_EOSPVT( pT );
      USE_PARAM_EOSPVT( pZ );
      USE_PARAM_EOSPVT( pSplit );
      USE_PARAM_EOSPVT( pValueK );
      USE_PARAM_EOSPVT( pPhases );
      USE_PARAM_EOSPVT( pIsothermal );
      USE_PARAM_EOSPVT( pMolar );
   }

   /// \brief Load grid block properties into temporary vectors for grading 
   /// 
   /// \param iDirection  Indicator which direction we are going
   /// \param pD          (constant) Depth of current entry
   /// \param pP          Pressure, only loaded at initial node
   /// \param pT          Temperature
   /// \param pZ          Composition, only loaded at initial node
   /// \param pIsothermal Indicator whether the temperature array has been specified
   /// \param pMolar      Indicator as to whether molar or mass values being returned
   virtual void WriteGradingData( int iDirection, double *pD, double *pP, double *pT, double *pZ, int *pIsothermal, int *pMolar )
   {
      USE_PARAM_EOSPVT( iDirection );
      USE_PARAM_EOSPVT( pD );
      USE_PARAM_EOSPVT( pP );
      USE_PARAM_EOSPVT( pT );
      USE_PARAM_EOSPVT( pZ );
      USE_PARAM_EOSPVT( pIsothermal );
      USE_PARAM_EOSPVT( pMolar );
   }

   /// \brief Writes the gas oil contact. Returns GOC. If one is not present returns a number higher than highest entry
   virtual double WriteGOC()  { return ( 0.0 ); }

   /// \brief Routine to load composition of gas and density of oil at previous separator
   /// 
   /// \param pY     Gas composition to be output
   /// \param pZx    Oil Z factor
   /// \param pDZxda Derivative of oil z factor
   virtual void WriteSeparatorInfo( double *pY, double *pZx, double *pDZxda )
   {
      USE_PARAM_EOSPVT( pY );
      USE_PARAM_EOSPVT( pZx );
      USE_PARAM_EOSPVT( pDZxda );
   }

   /// \brief Routine to load separator derivatives
   /// 
   /// \param iPhase Index of phase to be returned
   /// \param pDX    Pointer to derivative to be stored
   virtual void WriteSeparatorDrv( int iPhase, double *pDX )
   { 
      USE_PARAM_EOSPVT( iPhase ); 
      USE_PARAM_EOSPVT( pDX ); 
   }

   /// \brief Write separator data to application structures
   ///
   /// \param iM     Total number of feed streams
   /// \param iFeed  Index of the feed stream
   /// \param iInlet (constant) Inlet for this flash
   /// \param pFeed  (constant) The actual feeds
   virtual void WriteSeparatorData( int iM, int iFeed, int *pInlet, double *pFeed )
   {
      USE_PARAM_EOSPVT( iM );
      USE_PARAM_EOSPVT( iFeed );
      USE_PARAM_EOSPVT( pInlet );
      USE_PARAM_EOSPVT( pFeed );
   }

   /// \brief Routine to change volume control as we step through separator calculations
   ///
   /// \param iTVolume Term to set volume control to
   ///       EOS_DENSITY    Density and mole/mass fractions
   ///       EOS_VOLUME     Volumes only
   ///       EOS_SEPARATOR  Total moles/mass only
   ///       EOS_STOCK_TANK Volumes and total moles/mass
   virtual void ReadVolumeControl( int iTVolume )
   { 
      USE_PARAM_EOSPVT( iTVolume );
   }

   /// \brief Routine to change minimum allowed pressure
   ///
   /// \param dMinP Minimum allowed pressure
   virtual void ReadMinimumPressure( double dMinP )
   {
      USE_PARAM_EOSPVT( dMinP ); 
   }

   /// \brief Read separator data to Application structures
   /// 
   /// \param iM   Total number of feed streams
   /// \param iFeed  Index of the feed stream
   /// \param iDrv   Derivative level for Application run
   /// \param pQv    Phase surface volumes
   /// \param pDQv   Derivatives of phase surface volumes
   /// \param pQm    Phase surface mass
   /// \param pComp  Phase surface compositions
   virtual void ReadSeparatorData( int iM, int iFeed, int iDrv, double *pQv, double *pDQv, double *pQm, double *pComp )
   {
      USE_PARAM_EOSPVT( iM );
      USE_PARAM_EOSPVT( iFeed );
      USE_PARAM_EOSPVT( iDrv );
      USE_PARAM_EOSPVT( pQv );
      USE_PARAM_EOSPVT( pDQv );
      USE_PARAM_EOSPVT( pQm );
      USE_PARAM_EOSPVT( pComp );
   }

   /// \brief Routine to store phase properties into application permanent arrays 
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
   virtual void ReadData( int iPhaseId, int iM, int iNc, int *pPhases, double *pX, double *pDX, double *pMx, double *pDMx, 
                          double *pZx, double *pDZx, double *pMux, double *pDMux, double *pIfx, double *pDIfx, double *pHx, 
                          double *pDHx, double *pP, double *pDP, double *pMwx )
   {
      USE_PARAM_EOSPVT( iPhaseId );
      USE_PARAM_EOSPVT( iM );
      USE_PARAM_EOSPVT( iNc );
      USE_PARAM_EOSPVT( pPhases );
      USE_PARAM_EOSPVT( pX );
      USE_PARAM_EOSPVT( pDX );
      USE_PARAM_EOSPVT( pMx );
      USE_PARAM_EOSPVT( pDMx );
      USE_PARAM_EOSPVT( pZx );
      USE_PARAM_EOSPVT( pDZx );
      USE_PARAM_EOSPVT( pMux );
      USE_PARAM_EOSPVT( pDMux );
      USE_PARAM_EOSPVT( pIfx );
      USE_PARAM_EOSPVT( pDIfx );
      USE_PARAM_EOSPVT( pHx );
      USE_PARAM_EOSPVT( pDHx );
      USE_PARAM_EOSPVT( pP );
      USE_PARAM_EOSPVT( pDP );
      USE_PARAM_EOSPVT( pMwx );
   }

   /// \brief Routine to store phase properties into Application permanent arrays when a phase is not present
   ///
   /// \param iPhaseId  Type of phase to be processed
   ///                  EOS_SINGLE_PHASE_OIL - Phase stored in "x" arrays in flasher
   ///                  EOS_SINGLE_PHASE_GAS - Phase stored in "y" arrays in flasher
   ///                  EOS_WATER            - Aqueous phase
   /// \param iM        Number of objects to read
   /// \param pPhases   (constant) Indentifier for phases
   ///                  EOS_SINGLE_PHASE_OIL - Phase is a liquid
   ///                  EOS_SINGLE_PHASE_GAS - Phase is a gas
   ///                  Not used for an aqueous phase load
   virtual void ReadNull( int iPhaseId, int iM, int *pPhases )
   {
      USE_PARAM_EOSPVT( iPhaseId );
      USE_PARAM_EOSPVT( iM );
      USE_PARAM_EOSPVT( pPhases );
   }


   /// \brief Subroutine to load flash results into permanent arrays 
   ///
   /// \param iS      Index of element to load; if iM then load all
   /// \param iM      Total number of elements to load
   /// \param iSet    Integer above which success is declared.  Generally when the  pPhase array is of following form   
   ///                EOS_2P_NCV   Nonconverged two phase flash
   ///                EOS_2P_CV    Converged two phase flash
   ///                EOS_BP_NCV   Nonconverged bubble point calculation
   ///                EOS_BP_CV    Converged bubble point calculation
   ///                It is better to load partially converged blocks than to ditch them!
   /// \param iReset  Integer controlling what happens if not successful. Lack of success is generally indicated by the following
   ///                EOS_1P_NCV  Nonconverged one phase system
   ///                If iReset is on then the current compositions and pressures are saved in order to be used later to see
   ///                if we need to do the stability testing again.
   /// \param pSplit  (constant) Phase split from the flasher.  Overloaded with bubble or dew point for a bubble point calculation
   /// \parm pPhases  (constant) Current phase indicator from flasher; see above
   /// \param pValueK (constant) Current estimate of K values
   virtual void ReadFlashResults( int iS, int iM, int iSet, int iReset, double *pSplit, int *pPhases, double *pValueK )
   {
      USE_PARAM_EOSPVT( iS );
      USE_PARAM_EOSPVT( iM );
      USE_PARAM_EOSPVT( iSet );
      USE_PARAM_EOSPVT( iReset );
      USE_PARAM_EOSPVT( pSplit );
      USE_PARAM_EOSPVT( pPhases );
      USE_PARAM_EOSPVT( pValueK );
   }

   /// \brief Load grading results into permanent arrays 
   /// 
   /// \param dDepth       Current depth returned from flasher
   /// \param dTemperature Current temperature returned from flasher
   /// \param dPressure    Current reference phase pressure returned from flasher
   /// \param dPx          Current oil phase pressure returned from flasher; either reference phase pressure or dew point
   /// \param dPy          Current gas phase pressure returned from flasher; either reference phase pressure or bubble point
   /// \param dRox         Current oil density at oil phase pressure
   /// \param dRoy         Current gas density at gas phase pressure
   /// \param dHx          Current oil enthalpy at oil phase pressure
   /// \param dHy          Current gas density at gas phase pressure
   /// \param pZ           (constant) Composition of reference phase
   /// \param pX           (constant) Composition of oil phase, either reference phase or at dew point
   /// \param pY           (constant) Composition of gas phase, either reference phase or at bubble point
   virtual void ReadGradingData( double dDepth, double dTemperature, double dPressure, double dPx, double dPy, 
                                 double dRox, double dRoy, double dHx, double dHy, double *pZ, double *pX, double *pY )
   {
      USE_PARAM_EOSPVT( dDepth );
      USE_PARAM_EOSPVT( dTemperature );
      USE_PARAM_EOSPVT( dPressure );
      USE_PARAM_EOSPVT( dPx );
      USE_PARAM_EOSPVT( dPy );
      USE_PARAM_EOSPVT( dRox );
      USE_PARAM_EOSPVT( dRoy );
      USE_PARAM_EOSPVT( dHx );
      USE_PARAM_EOSPVT( dHy );
      USE_PARAM_EOSPVT( pZ );
      USE_PARAM_EOSPVT( pX );
      USE_PARAM_EOSPVT( pY );
   }

   /// \brief Reads the gas oil contact 
   ///
   /// \param dGOCT Gas oil contact computed by flasher
   virtual void ReadGOC( double dGOCT )
   { 
      USE_PARAM_EOSPVT( dGOCT ); 
   }

   /// \brief Print application Input Data used for debugging purposes
   virtual void PrintInputData() {}


   /// \brief Print application Output Data used for debugging purposes
   virtual void PrintOutputData() {}

   /// \brief Modify bubble point indicator 
   ///
   /// \param iType Direction in which the conversion occurs
   ///              EOS_TO_INTERNAL    Change from Application storage (negative split for bubble point) to flasher storage 
   ///              EOS_TO_APPLICATION Change to Application storage (negative split for bubble point) from flasher storage
   /// \param iFrom Phase indicator on which to perform conversion
   ///              EOS_FL_1P, EOS_FL_1P_CV, or EOS_FL_1P_NCV  General input on from Application
   ///              EOS_FL_BP, EOS_FL_BP_CV, or EOS_FL_BP_NCV  General input on to Application
   /// \param iTo   Phase indicator on which to perform conversion
   ///              EOS_FL_1P, EOS_FL_1P_CV, or EOS_FL_1P_NCV General input on to Application
   ///              EOS_FL_BP, EOS_FL_BP_CV, or EOS_FL_BP_NCV General input on from Application
   virtual void ModifyBubble( int iType, int iFrom, int iTo )
   {
      USE_PARAM_EOSPVT( iType );
      USE_PARAM_EOSPVT( iFrom );
      USE_PARAM_EOSPVT( iTo );
   }

   /// \brief Routine to modify phase property indicators 
   ///
   /// \param iDoAll Indicator as to whether to modify all phase indicators or only the current flash
   virtual void ModifyPhaseIndicator( int iDoAll )
   { 
      USE_PARAM_EOSPVT( iDoAll ); 
   }

   /// \brief Check to see which grid blocks to do stability test 
   /// 1) If no hydrocarbon phases then do nothing!
   /// 2) Then test to see if any hydrocarbons present. If not then the block is EOS_FL_0P
   /// 3) If only one component is present then block is EOS_FL_1P_CV
   /// 4) If previous results are unknown block is EOS_FL_1P_NCV
   /// 5) If previous results are known and components and pressures have not changed much, block is EOS_FL_1P_CV
   /// 6) Else block is either EOS_FL_1P_NCV or EOS_FL_2P_NCV
   ///
   /// \param dEnorm TODO Document parameter
   virtual void ModifyPhaseIdentification( double dEnorm )
   { 
      USE_PARAM_EOSPVT( dEnorm ); 
   }

   /// \brief Check to see which grid blocks to do stability test 
   /// 1) If no hydrocarbon phases then do nothing!
   /// 2) Then test to see if any hydrocarbons present. If not then the block is EOS_FL_0P
   /// 3) If only one component is present then block is EOS_FL_1P_CV
   /// 4) Else block is EOS_FL_1P_NCV
   ///
   /// \param dEnorm TODO Document parameter
   virtual void ModifyOnePhaseIdentification( double dEnorm )
   { 
      USE_PARAM_EOSPVT( dEnorm ); 
   }

   /// \brief Routine to load liquids from previous separator into the current and add in any additional feed
   virtual void ModifySeparatorAccumulations() {}

   /// \brief Slice objects 
   /// 
   /// \param iTestPhase Phase indicator for which to form the slice
   /// \param iMVL       Slice length
   ///
   /// \return the length of the slice
   virtual int  SetSlice( int iTestPhase, int iMVL )
   { 
      USE_PARAM_EOSPVT( iTestPhase ); 
      USE_PARAM_EOSPVT( iMVL ); 
      return ( 0 ); 
   }

   /// \brief See if any more objects present to compute
   /// 
   /// \param iM         Set the indirection "aan de beurt" to this value if greater than zero
   /// \parm iTestPhase Phase indicator for which to check if anything more present for calculations
   ///
   /// \return 1 if anything aan de beurt, 0 otherwise
   virtual int Aandebeurt( int iM, int iTestPhase )
   { 
      USE_PARAM_EOSPVT( iM ); 
      USE_PARAM_EOSPVT( iTestPhase ); 
      return ( 0 ); 
   }

   /// \brief Set trivial slice 
   virtual void SetTrivialSlice() {}

   /// \brief Routine to set current pointer to next separator 
   ///
   /// \param iDirection - direction to move
   ///                     EOS_INIT_START - set to first separator
   ///                     EOS_INIT_END   - set to stock tank
   ///                     EOS_INIT_UP    - go up one step
   ///                     EOS_INIT_DOWN  - go down one step
   ///
   /// \return the index of the next separator
   virtual int SetSeparator( int iDirection )
   { 
      USE_PARAM_EOSPVT( iDirection ); 
      return ( 0 ); 
   }

   /// \brief Reset table entry in compositional grading
   /// 
   /// \param iDirection Direction to move in the table
   ///                   EOS_INIT_START - Go to the reference entry with data
   ///                   EOS_INIT_UP    - Increment pointer to a higher number
   ///                   EOS_INIT_DOWN  - Increment pointer to a lower number
   ///
   /// \return 1 if there is another entry in table
   virtual int SetTableEntry( int iDirection )
   { 
      USE_PARAM_EOSPVT( iDirection ); 
      return ( 0 );
   }

   /// \brief Set the pointers to temporary memory
   ///
   /// \full The memory will actually be allocated from within the
   /// EosPvtModel class.  This routine gets two passes; the
   /// first calculates the memory assuming that the pointers
   /// start at null to get the length, the second after the
   /// allocation to set the correct locations
   ///
   /// \param iN     Length of calculations
   /// \param pFinal On input the end of the temporary memory. On output the new end
   ///
   virtual void SetPointers( int iN, double **pFinal )
   { 
      USE_PARAM_EOSPVT( iN ); 
      USE_PARAM_EOSPVT( pFinal ); 
   }

   /// \brief Reset the slice back to the beginning of objects
   virtual void ResetSlice()  {}

private:
   /// Copy constructor turned off
   EosApplication( EosApplication &self );
};

#endif
