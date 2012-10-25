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
/*
// Revision history
//
// Each constructor has a "iVersion" parameter which
// allows for upward compatibility with old versions
// The following revisions have been made
//
// Class EosApplication
//
// Version 0
//    Initial release
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
*/
#ifndef EOSPVTMODEL_H
#define EOSPVTMODEL_H
#define EOS_PVT_TABLE_VERSION 5

/*
//
// Class EosPvtModel
//
// Version 0
//    Initial release
//
// Equation of State Flasher Data Description
// Thom Potempa
//
// Here is a brief description of the data needed to 
// drive the equation of state flasher designed for 
// MoReS.  It is also intended to be a more general 
// application compute server.  The issue of a data 
// server is not addressed.  
// 
// The water phase correlations are based upon a 
// public domain correlation I found on the internet.  
// Within the source code there is a comment which 
// must remain present in the code.  The correlations 
// were authored by Bernhard Spang, The Mining 
// Company, http://chemengineer.miningco.com.
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
// By default the arrays point to zero, and provisions 
// have been made to initialize the data outside of 
// the constructor, which also calls the same data 
// initialization.  Since this is intended to be a 
// compute rather than a data server, no provisions 
// have been made for data other than integer and 
// real.  
// 
// The copy and equal constructers are turned off, as 
// these are intended to be compute engine classes.
// 
// The EosApplication class is pure virtual.
// 
// More "C" interfaces will be required in the future 
// to facilitate computational efficiency within 
// Application.  As of this point in time, the only relevant 
// entry point into the flasher itself is
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
// The classes are not persistent in the current Application 
// application, although in principle they can be made 
// to be so.  The EosPvtTable class should be persistent.
// 
// A few notes on the methods themselves.  The flasher 
// is faster than that present in MULTISIM.  It is 
// also numerically more stable; for instance, care 
// has been taken to normalize the "ideal" 
// contribution to all Jacobian matrices to the 
// identity matrix.  Within the flasher itself, 
// standard C++ naming conventions have been used: 
// pointers begin with p; doubles begin with d; 
// integers begin with i.
// 
// Preprocessor Declarations
// 
// The preprocessor declarations are found in this file,
// EosPvtModelDefinitions.h.  They are as follows:
// 
// EOS_STANDALONE (0 || 1)
// 
// Controls whether the flasher is run stand alone
// 
// EOS_FORTRAN (0 || 1)
// 
// Controls whether FORTRAN indicing used in indirection
// 
// EOS_OPTION_UNKNOWN (-1)
// 
// Only used internally
// 
// EOS_OPTION_OFF (0)
// 
// Turn an option off.
// 
// EOS_OPTION_ON (1)
// 
// Turn and option on.
// 
// EOS_HYDROCARBON (0)
// 
// Indicator whether a component is a hydrocarbon
// 
// EOS_AQUEOUS (1)
// 
// Indicator whether a component is water or a salt
// 
// EOS_NOTUSED (-1)
// 
// Indicator for a component which is not used in PVT model
// 
// EOS_EXPLICIT (-2)
// 
// Indicator for a passive component which does not have 
// any room in the derivative arrays
// 
// EOS_DEBUG_PRINTING (0 || 1)
// 
// Controls whether some debug printing will be generated.
// 
// EOS_DEBUG_OUTPUT (0 || 1)
// 
// Controls whether some debug output will be generated.stand alone
// 
// EOS_DEFAULT_DENSITY (1000.0)
// 
// Default density in flasher internal units if phase 
// not present.
// 
// EOS_DEFAULT_VISCOSITY (0.001)
// 
// Default viscosity in flasher internal units if 
// phase not present.
// 
// EOS_DEFAULT_TENSION (0.0)
// 
// Default parachor term in flasher internal units if 
// phase not present.
// 
// EOS_JOSSI_MW_SPLIT (100.0)
// 
// Molecular weight to divide heavy and light components
// 
// EOS_KVALUE_SPECTRUM (5.0)
// 
// Minimum factor of difference in ideal K values used to
// generate initial estimates for the flasher.  If 
// the ideal K values generated are too close together
// the methods may not converge
// 
// EOS_SINGLE_PHASE_GAS (0)
// 
// Indicates that all single phase hydrocarbon 
// mixtures will be considered gas; not used if only 
// one hydrocarbon phase present.
// 
// EOS_SINGLE_PHASE_OIL (1)
// 
// Indicates that all single phase hydrocarbon 
// mixtures will be considered oil; not used if only 
// one hydrocarbon phase present.
// 
// EOS_SINGLE_PHASE_DEFAULT (2)
// 
// Indicates that single phase hydrocarbon mixtures 
// will be assigned a phase name based upon the lumped 
// A and B.  This is rigorous for a single component 
// and will result in the same phase name being used 
// as pressure varies, but can be in error in 
// detecting whether the fluid has a bubble point or 
// dew point. Not used if only one hydrocarbon phase 
// present.
// 
// EOS_SINGLE_PHASE_ZMINUSB (3)
// 
// Indicates that single phase hydrocarbon mixtures 
// will be assigned a phase name based upon a user 
// supplied value for the difference of Z and B.  This 
// is not rigorous for a single component and might 
// result in different phase names being used as 
// pressure varies, and can be in error in detecting 
// whether the fluid has a bubble point or dew point. 
// This is the Application default as of this time.  Not 
// used if only one hydrocarbon phase present.
// 
// EOS_SINGLE_PHASE_AOVERB (4)
// 
// Indicates that single phase hydrocarbon mixtures 
// will be assigned a phase name based upon a user 
// supplied value for the quotient of the lumped A and 
// B.  This will result in the same phase name being 
// used as pressure varies and if properly specified 
// by the user can be made consistent with whether the 
// fluid has a bubble point or dew point. Not used if 
// only one hydrocarbon phase present.
//
// EOS_WATER_CORRELATION (0)
//
// Indicator that water correlations to be used
//
// EOS_WATER_LINEAR (1)
//
// Indicator that linear water model to be used
//
// EOS_CONSTANT_C (0)
//
// Indicator that constant c value is used
//
// EOS_LINEAR_C (1)
//
// Indicator that linear c function to be used
//
// EOS_ZERO_C (-1)
//
// Indicator that linear c function to be used but zero data supplied
//
// EOS_EXPANSION (0)
//
// Do a constant composition expansion
//
// EOS_DIFFLIB (1)
//
// Do a differential liberation
//
// EOS_CONSTANTVOLUME (2)
//
// Do a constant volume expansion
// 
// EOS_WATER
// EOS_FL_1P_NCV            
// EOS_FL_1P_CV             
// EOS_FL_2P_NCV            
// EOS_FL_2P_CV             
// EOS_FL_BP_NCV            
// EOS_FL_BP_CV             
// EOS_FL_BP_CHK            
// EOS_TOF_0P
// EOS_TOF_2P              
// 
// These are used internally within the flasher and 
// are not of interest for the interface.
// 
// EOS_FLASH_CALCULATIONS (0)
// 
// Indicates that the flasher will be used for flash 
// 
// EOS_COMPOSITIONAL_GRADING (1)
// 
// Indicates that the flasher will be used for 
// compositional grading calculations.
// 
// EOS_SEPARATOR_FLASH (2)
// 
// Indicates that the flasher will be used for 
// separator calculations.
// 
// EOS_FL_0P                
// EOS_FL_1P                
// EOS_FL_2P                
// EOS_FL_BP                
// EOS_FL_AP               
// EOS_TO_APPLICATION       
// EOS_TO_INTERNAL          
// EOS_NORETROGRADE               
// EOS_RETROGRADE              
// EOS_NOFORCEBP               
// EOS_FORCEBP                  
// EOS_NOSCALEK               
// EOS_SCALEK                     
// EOS_NOGETK               
// EOS_GETK                 
// EOS_NONORMALIZE          
// EOS_NORMALIZE 
// EOS_DENSITY 
// EOS_VOLUME 
// EOS_SEPARATOR 
// EOS_STOCK_TANK 
// EOS_GAS                 
// EOS_OIL                  
// 
// These are used internally within the flasher and 
// are not of interest for the interface.
// 
// EOS_NODRV (0)
// 
// No derivatives will be returned by the flasher.
// 
// EOS_DRV_N (1)
// 
// Only composition derivatives will be returned by 
// the flasher.
// 
// EOS_DRV_P (2)
// 
// Composition and pressure derivatives will be 
// returned by the flasher.
// 
// EOS_DRV_T (3)
// 
// Composition, pressure, and temperature derivatives 
// will be returned by the flasher.
// 
// EOS_NORESTORE (0)
// 
// Indicates that old values are not available to the 
// flasher as an initial guess.
// 
// EOS_RESTORE (1)
// 
// Indicates that old values are available to the 
// flasher as an initial guess.
// 
// EOS_NOFUGACITY           
// EOS_FUGACITY             
// EOS_NOHEAT               
// EOS_HEAT                 
// EOS_NOPOTENTIAL          
// EOS_POTENTIAL            
// EOS_NOPHASEID            
// EOS_PHASEID              
// EOS_NOCONVERGE           
// EOS_CONVERGE             
// EOS_INIT_NONE           
// EOS_INIT_COMP          
// EOS_INIT_DOWN           
// EOS_INIT_START           
// EOS_INIT_UP            
// EOS_INIT_END  
// 
// These are used internally within the flasher and 
// are not of interest for the interface.
// 
// EOS_PVT_TABLE (0)
// 
// Correlations are Application ABC tables
//
// EOS_PVT_PR (1)
// 
// Peng Robinson equation of state used
//
// EOS_PVT_PRCORR (2)
// 
// Corrected Peng Robinson equation of state used
// 
// EOS_PVT_RK (3)
//
// Redlich Kwong equation of state used
//
// EOS_PVT_SRK (4)
//
// Soave Redlich Kwong equation of state used
//
// EOS_PVT_IDEAL (5)
//
// Ideal gas equation of state used
// 
// EOS_MORES  
// EOS_PVT_MODEL 
// 
// Currently not used, but reserved for later use
// when multiple applications can access the flasher
// 
// Integer Arrays
// 
// Associated with each class is an array of integer 
// data pIData.  The pointer to the array is passed 
// within the array of pointers to integer arrays.  
// This array can be destroyed after each class is 
// initialized.  The following values must be set.
// 
// For class EosPvtModel (loaded in class EosApplication):
// 
// piFlasher[PVTMETHOD] = EOS_PVT_MODEL
// 
// Type of pvt model; currently only the equation of 
// state flasher is available.
//                        
// piFlasher[EOS_MAXITN]
// 
// Maximum allowed iterations; 20 is reasonable.
//                       
// piFlasher[EOS_MAXFLASH]
// 
// Parameter controlling slicing of calculations; 32 
// is a reasonable value.  If the number is set too 
// low, additional overhead is introduced in function 
// calls; if the number is set too high, additional 
// overhead is also introduced by various ways.
//                       
// piFlasher[EOS_MICHELSON] = EOS_OPTION_OFF || EOS_OPTION_ON
// 
// Specifies whether the more robust Michelson 
// stability analysis or a less robust but faster 
// analysis is used to determine the number of 
// hydrocarbon phases.
// 
// piFlasher[EOS_SUBSTITUTIONS]
// 
// Number of successive substitution iterations.  
// Substitution is not an efficient method.  1 is a reasonable
// default as there are some easy problems which converge
// quickly
// 
// piFlasher[EOS_OWNMEMORY]
// 
// Not used
// 
// piFlasher[EOS_DEBUG] = EOS_OPTION_OFF || EOS_OPTION_ON
// 
// Indicates whether the flasher is to generate debug
// printing.
//                    
// EOS_LAST_INTEGER 
// 
// Number of elements in integer data for class 
// EosPvtModel. 
// 
// For class EosPvtTable:
// 
// piTables[EOS_METHOD] = EOS_PVT_TABLE || 
//                        EOS_PVT_PR ||
//                        EOS_PVT_PRCORR ||
//                        EOS_PVT_RK ||
//                        EOS_PVT_SRK ||
//                        EOS_PVT_IDEAL ||
//
// Selection of equation of state method.
//                        
// piTables[EOS_METHOD_COMPONENTS]
// 
// Number of components used within the equation-of-
// state flash calculations; generally does not 
// include water and tracer components.
// 
// piTables[EOS_METHOD_PHASEID] = EOS_SINGLE_PHASE_GAS ||
//                                EOS_SINGLE_PHASE_OIL ||
//                                EOS_SINGLE_PHASE_DEFAULT || 
//                                EOS_SINGLE_PHASE_ZMINUSB || 
//                                EOS_SINGLE_PHASE_AOVERB 
// 
// Indicator for method used for single phase naming.
// 
// piTables[EOS_METHOD_NUMBERABC]
// 
// Number of temperature entries minus one in the ABC 
// coefficient tables.  Thus, for an isothermal table, 
// enter 0, for a table with two temperatures, enter 
// 1, etc.  
// 
// piTables[EOS_METHOD_HEATDATA] = EOS_OPTION_OFF || EOS_OPTION_ON
// 
// Indicator as to whether reference state heat 
// capacity data will be available to the flasher.
// 
// piTables[EOS_METHOD_OWNMEMORY] = EOS_OPTION_OFF || EOS_OPTION_ON
// 
// Indicator as to whether the class will provide its 
// own memory or not.  Best practice 
// would be to set to EOS_OPTION_ON as the class 
// should provide its own memory.
//              
// piTables[EOS_METHOD_SALTS]
// 
// Number of salt components.  If no salts are present the
// water molecular weight is calculated from the salinity
//              
// piTables[EOS_METHOD_WATERMODEL]
// 
// Type of water model.  Either
//    EOS_WATER_CORRELATION for correlation
//    EOS_WATER_LINEAR for linear model
// Only used for release 2
//              
// piTables[EOS_METHOD_TYPE_OF_C]
// 
// Enables calculations of c at standard conditions
//    EOS_CONSTANT_C for constant
//    EOS_LINEAR_C for linear model
// Only used for release 3 and later
// 
// piTables[EOS_METHOD_TYPE_OF_BIJ]
//              
// Enables calculations of bij at standard conditions
//    EOS_CONSTANT_C for constant
//    EOS_LINEAR_C for linear model
// Only used for release 5 and later
//                  
// EOS_METHOD_LAST_INTEGER          
// 
// Number of elements in integer data for class 
// EosPvtTable. 
// 
// For class EosApplication:
// 
// piApplication[APPLICATION] = EOS_MORES
// 
// Not used currently. 
//                     
// piApplication[EOS_APPLICATION_COMPONENTS]
// 
// Total number of components in the application, 
// including components such as water which are not 
// flashed within the equation of state flasher. When 
// the flasher is called in the general rather than 
// the MoReS specific mode, should be set to the 
// number of flash components.
//                     
// piApplication[EOS_APPLICATION_NUMBERPHASES]
// 
// Total number of phases in the application, 
// including phases which are not treated within
// the flasher explicitly
// 
// piApplication[EOS_APPLICATION_FLASHES]
// 
// Number of flashes to be done by the flasher.
// 
// piApplication[EOS_APPLICATION_THERMAL] = EOS_OPTION_OFF || 
//                                          EOS_OPTION_ON         
// 
// Indicator as to whether an external temperature 
// array is to be specified.  Otherwise, the 
// temperature associated with the class EosPvtTable 
// is used.
// 
// piApplication[EOS_APPLICATION_WATER]
// 
// C index of water component; set to -1 if not 
// present.
// 
// piApplication[EOS_APPLICATION_LIQUID]
// 
// C index of liquid phase; set to -1 if not present. 
// The flasher can handle single phase oil or single 
// phase gas systems.
// 
// piApplication[EOS_APPLICATION_VAPOUR]
//
// C index of vapour phase; set to -1 if not present. 
// The flasher can handle single phase oil or single 
// phase gas systems.
// 
// piApplication[EOS_APPLICATION_AQUEOUS]
// 
// C index of aqueous phase; set to -1 if not present.  
// Within MoReS, can be set to negative one even if 
// aqueous phase present in simulation in order to use 
// another MoReS method to compute the aqueous phase 
// properties.
// 
// piApplication[EOS_APPLICATION_OBJECTS]
// 
// Total number of objects in the Application arrays; indirection
// controls which objects are filled by flasher
// 
// piApplication[EOS_APPLICATION_RESTORE] = EOS_OPTION_OFF || 
//                                          EOS_OPTION_ON
// 
// Indicator as to whether K values are to be restored
// from the previous K values at the beginning of each
// new flash.
// 
// piApplication[EOS_APPLICATION_OWNMEMORY] = EOS_OPTION_OFF || 
//                                            EOS_OPTION_ON
// 
// Indicator as to whether the memory for results to 
// be controlled by the flasher or the external 
// application.  For the EosApplication class set to 
// EOS_OPTION_OFF as the memory requirements are calculated
// by EOSGETMEMORY.  For other applications using the 
// EosApplication class, best practice is to set to 
// EOS_OPTION_ON since each class should manage its 
// own memory.
//  
// piApplication[EOS_APPLICATION_DERIVATIVES] = EOS_NODRV || 
//                                              EOS_DRV_N || 
//                                              EOS_DRV_P || 
//                                              EOS_DRV_T 
// 
// Derivative level.  Set to EOS_DRV_P for MoRes.
// 
// piApplication[EOS_APPLICATION_VISCOSITY] = EOS_OPTION_OFF || 
//                                            EOS_OPTION_ON
// 
// Indicator for whether viscosity to be calculated. 
// Must currently be set to EOS_OPTION_ON in MoReS.
// 
// piApplication[EOS_APPLICATION_TENSION] = EOS_OPTION_OFF || 
//                                          EOS_OPTION_ON 
// 
// Indicator for whether tension terms to be 
// calculated. Must currently be set to EOS_OPTION_ON 
// in MoReS.
// 
// piApplication[EOS_APPLICATION_HEAT] = EOS_OPTION_OFF || 
//                                       EOS_OPTION_ON
// 
// Indicator for whether enthalpy terms to be 
// calculated. Must currently be set to EOS_OPTION_OFF 
// in MoReS.
// 
// piApplication[EOS_APPLICATION_VOLUMES] = EOS_OPTION_OFF || 
//                                          EOS_OPTION_ON 
// 
// Indicator as to whether volumes or densities to be 
// returned in the density array. Must currently be 
// set to EOS_OPTION_OFF in MoReS.  See note below.
// 
// piApplication[EOS_APPLICATION_COMPOSITIONS] = EOS_OPTION_OFF || 
//                                               EOS_OPTION_ON
// 
// Indicator as to whether compositions to be 
// returned. Must currently be set to EOS_OPTION_OFF 
// in MoReS. Note that this entry and the 
// EOS_APPLICATION_VOLUMES entry work in tandem:
// 
//   If EOS_APPLICATION_VOLUMES is EOS_OPTION_OFF and 
//   EOS_APPLICATION_COMPOSITIONS is EOS_OPTION_OFF, the 
//   flasher returns densities and compositions, the 
//   normal mode. 
// 
//   If EOS_APPLICATION_VOLUMES is EOS_OPTION_ON and 
//   EOS_APPLICATION_COMPOSITIONS is EOS_OPTION_OFF, the 
//   flasher returns volumes in the density array.  This 
//   is useful for explicit simulation. 
// 
//   If EOS_APPLICATION_VOLUMES is EOS_OPTION_OFF and 
//   EOS_APPLICATION_COMPOSITIONS is EOS_OPTION_ON, the 
//   flasher returns only compositions.  This is useful 
//   for separator calculations. 
// 
//   If EOS_APPLICATION_VOLUMES is EOS_OPTION_ON and 
//   EOS_APPLICATION_COMPOSITIONS is EOS_OPTION_ON, the 
//   flasher returns volumes and compositions.  This is 
//   useful for stock tank computations.
// 
//  piApplication[EOS_APPLICATION_MOLARDENSITY] = EOS_OPTION_OFF || 
//                                                EOS_OPTION_ON
// 
// Indicates that volume/density terms will be 
// returned on a mass basis even when the compositions 
// are specified in molar terms.  Not used for MoReS.
// 
// piApplication[EOS_APPLICATION_MOLEFRACTIONS] = EOS_OPTION_OFF || 
//                                                EOS_OPTION_ON
// 
// Indicates whether compositions to be specified on a 
// mass or mole basis.  Set to EOS_OPTIONS_OFF in 
// MoReS to indicate mass compositions used. 
// 
// piApplication[EOS_APPLICATION_MOLECULARWEIGHT] = EOS_OPTION_OFF || 
//                                                  EOS_OPTION_ON
// 
// Indicator as to whether the flasher will return the 
// phase molecular weights.  Set to EOS_OPTION_OFF in 
// MoReS.
// 
// piApplication[EOS_APPLICATION_FLASH] = EOS_OPTION_OFF || 
//                                        EOS_OPTION_ON
// 
// Indicator as to whether the flasher will perform 
// flash calculations.  Set to EOS_OPTION_ON in MoReS. 
// Useful in explicit simulations where the flash 
// calculations were used to earlier compute the 
// volume balance and at the beginning of the next 
// time step properties must be updated from saved K 
// values.
// 
// piApplication[EOS_APPLICATION_PROPERTIES] = EOS_OPTION_OFF || 
//                                             EOS_OPTION_ON 
// 
// Indicator as to whether the flasher will update 
// properties, including volume balance, as determined 
// by the property flag.  Set to EOS_OPTION_ON in 
// MoReS.  Useful if flasher invoked to obtain the 
// bubble point but properties are not desired.
// 
// piApplication[EOS_APPLICATION_GETBUB] = EOS_OPTION_OFF || 
//                                         EOS_OPTION_ON 
// 
// Indicator as to whether bubble point or dew point 
// calculations will be performed as opposed to the 
// standard flash calculations. Set to EOS_OPTION_OFF 
// in MoReS.  This is slightly different than the 
// EOS_APPLICATION_BUBDEW entry for the class EosPvtModel; the 
// EOS_APPLICATION_GETBUB entry means get the bubble 
// point no matter what the pressure and compute the 
// properties at the bubble point pressure; the 
// EOS_APPLICATION_BUBDEW entry indicates track the bubble point.
// 
// piApplication[EOS_APPLICATION_INITIALIZE] = 
//                                      EOS_FLASH_CALCULATIONS || 
//                                      EOS_COMPOSITIONAL_GRADING ||
//                                      EOS_SEPARATOR_FLASH 
// 
// Mode in which the flasher is invoked.  Must be 
// EOS_FLASH_CALCULATIONS currently in MoReS.  
// 
// piApplication[EOS_APPLICATION_TABLEENTRY]
// 
// Entry in depth table at which pressure is specified 
// in initialization calculations.  For separator calculations
// also used internally.  
//
// piApplication[EOS_APPLICATION_PSEUDO] = EOS_OPTION_OFF || 
//                                         EOS_OPTION_ON
// 
// Compute pseudo-phase properties if a phase is not 
// present.  If turned off, the values of 
// EOS_DEFAULT_DENSITY, etc., are used.  If turned on, 
// either the bubble point composition or an 
// approximation involving the Wilson K values is used 
// to compute pseudo-phase properties, depending upon 
// whether EOS_METHOD_BUBDEW is specified.  Only of real use 
// in MoReS, which requires a pseudo-phase property 
// for phases not present.
//                     
// piApplication[EOS_APPLICATION_BUBDEW] = EOS_OPTION_OFF || 
//                                         EOS_OPTION_ON
// 
// Indicates that the bubble or dew point will be 
// computed within the flash calculations.  If 
// specified, the pseudo phase composition is 
// determined from the equilibrium values at the 
// bubble or dew point.
//                     
// EOS_APPLICATION_LAST_INTEGER 
// 
// Number of elements in integer data for class 
// EosApplication or EosApplication. 
// 
// Real Arrays
// 
// Associated with each class is an array of integer 
// data pRData.  The pointer to the array is passed 
// within the array of pointers to real arrays.  This 
// array can be destroyed after each class is 
// initialized.  The following values must be set.
// 
// Data for class EosPvtModel (read in class EosApplication)
// 
// pdFlasher[EOS_ENORM]      
// 
// A very large number but well within the largest 
// double precision number a machine can handle.  
// 1.0E80 is a reasonable default
//        
// pdFlasher[EOS_TINY]      
// 
// Number such that 1 + piFlasher[EOS_TINY] is equal to 1 
// on a computer.
//               
// pdFlasher[EOS_CONVERGENCE] 
// 
// Convergence tolerance for flasher.  0.0001 is 
// reasonable for reservoir simulation; use 1000.0 *
// piFlasher[EOS_TINY] for very precise results.
// 
// pdFlasher[EOS_THERMALDIFFUSION]
// 
// Multiplicative factor for thermal diffusion.  If
// 0, thermal diffusion not used
// 
// pdFlasher[EOS_BUBBLEREDUCE]
// 
// Factor for reduction in initial stage of bubble point
// calculations, usually set to 0.5.  Set to 0.1 if
// convergence is an issue.
//              
// EOS_LAST_DOUBLE 
// 
// Number of elements in real data for class 
// EosPvtModel. 
// 
// For class EosPvtTable
// 
// pdTables[EOS_METHOD_SALINITY] 
// 
// Default salinity in parts per million for aqueous 
// phase calculations.
// 
// pdTables[EOS_METHOD_WATERIFT] 
// 
// Default water/reference phase IFT.
// 
// pdTables[EOS_METHOD_KB1]  
//   
// KB1 parameter for equation of state (e.g., 1.0 for 
// Redlich Kwong).
//           
// pdTables[EOS_METHOD_KB2]        
// 
// KB2 parameter for equation of state (e.g., 0.0 for 
// Redlich Kwong).
//        
// pdTables[EOS_METHOD_PRESCONV]      
// 
// Conversion factor for pressure from flasher 
// internal units to MKS units.
//     
// pdTables[EOS_METHOD_TEMPCONV]          
// 
// Conversion factor for temperature from flasher 
// internal units to MKS units.
// 
// pdTables[EOS_METHOD_VOLCONV]           
// 
// Conversion factor for volume from flasher internal 
// units to MKS units.
// 
// pdTables[EOS_METHOD_VISCCONV]         
// 
// Conversion factor for viscosity from flasher 
// internal units to MKS units.
// 
// pdTables[EOS_METHOD_DENSCONV]          
// 
// Conversion factor for density from flasher internal 
// units to MKS units.
// 
// pdTables[EOS_METHOD_HEATCONV]         
// 
// Conversion factor for enthalpy from flasher 
// internal units to MKS units.
// 
// pdTables[EOS_METHOD_TENSCONV]         
// 
// Conversion factor for interfacial tension from flasher 
// internal units to MKS units.
// 
// pdTables[EOS_METHOD_CRITZMINUSB]       
// 
// Critical z factor minus b for naming a single phase 
// mixture when piTables[EOS_METHOD_PHASEID] is  
// EOS_SINGLE_PHASE_ZMINUSB.  0.25 is the MoReS 
// default.
// 
// pdTables[EOS_METHOD_CRITAOVERB] 
// 
// Critical a over b for naming a single phase mixture 
// when piTables[EOS_METHOD_PHASEID] is  
// EOS_SINGLE_PHASE_AOVERB.  5.0 is a reasonable 
// default value.
// 
// pdTables[EOS_METHOD_VTUNE] 
// 
// Value of the viscosity tuning parameter for the 
// Jossi/GSS model. If set to zero Lohrenz viscosity 
// is used; if negative corresponding states viscosity used
// 
// pdTables[EOS_METHOD_VTUNE0] =   0.1023000
// pdTables[EOS_METHOD_VTUNE1] =   0.0233640
// pdTables[EOS_METHOD_VTUNE2] =   0.0585330
// pdTables[EOS_METHOD_VTUNE3] = - 0.0407580
// pdTables[EOS_METHOD_VTUNE4] =   0.0093324
// 
// Values of the viscosity tuning parameter for the 
// Lohrenz model. If set to zero Lohrenz viscosity 
// defaults given above are used
//
// pdTables[EOS_METHOD_VTERM1] =   1.0
// pdTables[EOS_METHOD_VTERM2] =   1.0
// pdTables[EOS_METHOD_VTERM3] =   1.0 
// pdTables[EOS_METHOD_VTERM4] =   1.0
//
// Tuning parameters for corresponding states viscosity model
//
// pdTables[EOS_METHOD_CSVSCMETHOD] =   0.0
// Flag for corresponding states viscosity version number
//   0 is old 2 term model  
//   1 is newer 4 term heavy oil extension model with special PVTSim Shell JIP formulation
//   2 is newer 4 term heavy oil extension model without the special JIP formulation 
// 
// EOS_METHOD_LAST_DOUBLE    
// 
// Number of elements in real data for class 
// EosPvtTable. 
// 
// For class EosApplication:
//      
// pdApplication[EOS_APPLICATION_PMIN]
// 
// Minimum pressure for flasher.  Used in class 
// EosApplication since MoReS may send negative pressures 
// (!) to the flasher.
// 
// pdApplication[EOS_APPLICATION_CHANGE]  
// 
// Minimum relative change in overall compositions or 
// pressures to perform a stability test on a single 
// phase mixture.  0.0001 is a reasonable default 
// value.  
// 
// EOS_APPLICATION_LAST_DOUBLE  
// 
// Number of elements in real data for class 
// EosApplication or EosApplication. 
// 
// Pointers to Integer Arrays
// 
// Associated with class EosApplication is an array of pointers 
// to integer arrays pIArray. This array can be 
// destroyed after the class is initialized.  The 
// following values either must be set or in some 
// cases are allocated when a class is initialized 
// with its own memory.   If the array is not to be 
// used, can be set to ( int* ) 0.
// 
// pIArray[INTEGERDATA]      
// 
// Pointer to the piApplication array.  The 
// pointer to this array must ALWAYS be specified.
//         
// EOS_LAST_IARRAY       
// EOS_METHOD_LAST_IARRAY 
// 
// Used internally
// 
// For class EosApplication:
// 
// pIArray[EOS_APPLICATION_EOSCOMPS]    
// 
// Pointer to array giving translation from internal 
// flasher numbering to external application numbering 
// of components used in the two phase flasher.  The 
// pointer to this array must ALWAYS be specified.
//
// pIArray[EOS_APPLICATION_INDIRECTION] 
// 
// Pointer to indirection array.  The pointer to this array 
// must ALWAYS be specified for flash and separator 
// calculations; not used for initialization computations.
// 
// pIArray[EOS_APPLICATION_PHASES]   
// 
// Pointer to array used to save state of system for 
// generating initial guess at next time step when
// the restore option is used.  The pointer to this array 
// must be specified when saving information from the 
// last flash.
//  
// EOS_APPLICATION_LAST_IARRAY 
// 
// Number of elements in array of pointers to integer 
// arrays in class EosApplication or EosMoReS.
// 
// Pointers to Real Arrays
// 
// Associated with class EosApplication is an array of pointers 
// to real arrays pdApplication. This array can be destroyed 
// after the class is initialized.  The following 
// values either must be set or in some cases are 
// allocated when a class is initialized with its own 
// memory.   If the array is not to be used, can be 
// set to ( double* ) 0.
// 
// pRArray[REALDATA]        
// 
// For all pointer to the pRData array
// 
// EOS_LAST_DARRAY   
// 
// Used internally
// 
// For class EosPvtTable these are used internally:
//    
// pRArray[EOS_METHOD_CRITICAL]          
// 
// Pointer to array of critical values for each 
// component.  The array is of length 5 or 6 times the 
// number of flash components.  The order of the array 
// is first all molecular weights, then all critical 
// pressures, then all critical temperatures, then all 
// critical volumes, then all parachors.  If the class 
// is initialized with its own memory, a pointer to 
// the location for memory allocated for this array is 
// returned.
//
// If any equation of state other than EOS_PVT_TABLE or 
// EOS_PVT_IDEAL is used, the array must be of length 6 times 
// the number of flash components, as the acentric factor
// is the last member of this array
//
// If EOS_PVT_TABLE or EOS_PVT_IDEAL is used as the equation 
// of state, the acentric factors are not relevant and hence 
// the length of the array is 5 times the number of flash 
// components
// 
// pRArray[EOS_METHOD_RAWABC] 
// 
// Values of raw equation of state parameters for each 
// temperature appearing in the table.  The order is, 
// for each temperature, first the temperature, then 
// the values of A, then the values of B, then the 
// values of C, then the values of the interaction 
// parameters.  Thus the number of entries is the 
// number of temperatures times [ 1 + number of flash 
// components * [ 3 + number of flash components ] ]. 
// The flasher expects these in units of volume (why?  
// Because that is what Libra did).  If the class is 
// initialized with its own memory, a pointer to the 
// location for memory allocated for this array is 
// returned.
//
// If the pvt method is EOS_PVT_IDEAL, then this data
// need not be supplied, with the exception of the 
// default temperature
//
// If any option is used except for EOS_PVT_TABLE or EOS_PVT_IDEAL,
// the number of temperature entries must be 1.  In this
// case the input is not A, B, and C but OMEGAA, OMEGAB,
// and the multiplicative factor for B for volume translation
// 
// pRArray[EOS_METHOD_ABC] 
// 
// Values of cached equation of state parameters for 
// each temperature appearing in the table.  The order 
// is slightly different as to whether one temperature 
// entry is specified or several.  If only a single 
// temperature is specified, then first the 
// temperature, then the B parameter as a volume, then 
// the C parameter as a volume, then the term with the 
// A parameters and the interaction coefficients 
// combined as pressure per volume.  Thus 1 + number 
// of flash components * [ 2 + number of flash 
// components ] ] entries are needed.  When more than 
// a single temperature is used, then data associated 
// with number of temperatures minus one entries is 
// associated.   First the temperature, then the B 
// parameter at zero temperature as a volume, then the 
// C parameter at zero temperature as a volume, then 
// the term with the A parameters and the interaction 
// coefficients combined as pressure per volume at 
// zero temperature, then the slope of the B parameter 
// at as a volume, then the slope of the C parameter 
// at as a volume, then slope of the term with the A 
// parameters and the interaction coefficients 
// combined as pressure per volume.  This is 
// sufficient information to specify the linear 
// interpolation between each temperature.  Thus the 
// number of entries is the [ number of temperatures - 
// 1 ] times [ 1 + number of flash components * [ 4 + 
// 2 * number of flash components ] ].  If the class 
// is initialized with its own memory, a pointer to 
// the location for memory allocated for this array is 
// returned.
//
// If the method is not EOS_PVT_TABLE or EOS_PVT_IDEAL, the 
// value of A and B are not cached as volumes but 
// related values for increased performance.  
//
// For the ideal gas case, A, B, and C are set to zero
//            
// pRArray[EOS_METHOD_HEATS]    
// 
// Coefficients for zero state enthalpy values for 
// each component.  The equation is specified as a 
// fifth degree polynonial of the form:
// 
// HeatI[T] = Heat1I * T + ... + 
//            Heat5I * T ^ 5
//
// The corresponding entropy is
//
// EntropyI[T] = Heat1I * ln ( T ) + 2 * Heat2I * T + 
//               3 * Heat3I * T * T / 2 + ...
//               5 * Heat5I * T ^ 4 / 4 + Heat6I
// 
// First Heat1 must be specified for each component, 
// then Heat2, etc.  The terms must be in internal 
// energy units.  Finally the entropy constant must
// be given.  Thus 6 times the number of flash 
// components entries are expected. If the class is 
// initialized with its own memory, a pointer to the 
// location for memory allocated for this array is 
// returned.
//            
// pRArray[EOS_METHOD_SALTMW] 
// 
// Molecular weights of salt components, the number of
// which is governed by EOS_METHOD_SALTS.
//            
// EOS_METHOD_LAST_DARRAY 
// 
// Number of elements in array of pointers to real 
// arrays in class EosPvtTable.
// 
// For class EosApplication:
// 
// pRArray[EOS_APPLICATION_TEMPERATURE]    
// 
// Pointer to temperature array of length number of 
// objects.  Can be set to the null pointer in an 
// isothermal situation. If the class is initialized 
// with its own memory, a pointer to the location for 
// memory allocated for this array is returned.
// 
// pRArray[EOS_APPLICATION_ACCUMULATION] 
// 
// Pointer to accumulation array (mass or moles) of 
// length number of objects times the number of 
// application components, stored in that order. This 
// is input to the flasher.  If the class is 
// initialized with its own memory, a pointer to the 
// location for memory allocated for this array is 
// returned.  In the case of compositional grading 
// calculations, this array need only be specified at 
// the reference depth.
// 
// pRArray[EOS_APPLICATION_FRACTION] 
// 
// Pointer to phase component mass or moles fraction 
// array of length number of objects times the number 
// of application components times the number of 
// phases, stored in that order. If the class is 
// EosApplication, note that enthalpy information is stored 
// as the last component in this array.  When using 
// separator or surface flashes the information is 
// returned as total mass or moles in the phase.  If 
// the class is initialized with its own memory, a 
// pointer to the location for memory allocated for 
// this array is returned.
//
// The enthalpy will be returned in the last component
// entry as an extra component if requested
// 
// pRArray[EOS_APPLICATION_DFRACTION] 
// 
// Pointer to derivatives of phase component mass or 
// moles fraction array of length number of objects 
// times the number of application components times 
// the number of derivatives times the number of 
// phases, stored in that order. Derivative 
// information is stored as first the pressure, then 
// the compositions, and finally the temperature.  If 
// pressure derivatives are not requested, note that 
// the composition derivative is in the first 
// location.  If the class is EosApplication, note that 
// enthalpy information is stored as the last 
// component in this array.  If the class is 
// initialized with its own memory, a pointer to the 
// location for memory allocated for this array is 
// returned.
//
// The enthalpy derivatives will be returned in the 
// last component entry as an extra component if requested
// 
// pRArray[EOS_APPLICATION_TOTALPHASE] 
// 
// Pointer to phase total mass or moles array of 
// length number of objects times the number of 
// phases, stored in that order. Only returned in the 
// standard flash mode, and not for volume or 
// separator modes.  If the class is initialized with 
// its own memory, a pointer to the location for 
// memory allocated for this array is returned.
// 
// pRArray[EOS_APPLICATION_DTOTALPHASE] 
// 
// Pointer to derivatives of phase total mass or moles 
// array of length number of objects times the number 
// of derivatives times the number of phases, stored 
// in that order. Derivative information is stored as 
// first the pressure, then the compositions, and 
// finally the temperature.  If pressure derivatives 
// are not requested, note that the composition 
// derivative is in the first location. Only returned 
// in the standard flash mode, and not for volume or 
// separator modes.  If the class is initialized with 
// its own memory, a pointer to the location for 
// memory allocated for this array is returned.
// 
// pRArray[EOS_APPLICATION_DENSITY] 
// 
// Pointer to phase density array of length number of 
// objects times the number of phases, stored in that 
// order. Returned as a density in the standard flash 
// mode, volume in volume or surface modes, and not 
// returned for separator mode.  If the class is 
// initialized with its own memory, a pointer to the 
// location for memory allocated for this array is 
// returned.
// 
// pRArray[EOS_APPLICATION_DDENSITY] 
// 
// Pointer to derivatives of phase density array of 
// length number of objects times the number of 
// derivatives times the number of phases, stored in 
// that order. Derivative information is stored as 
// first the pressure, then the compositions, and 
// finally the temperature.  If pressure derivatives 
// are not requested, note that the composition 
// derivative is in the first location. Returned as a 
// density in the standard flash mode, volume in 
// volume or surface modes, and not returned for 
// separator mode.  If the class is initialized with 
// its own memory, a pointer to the location for 
// memory allocated for this array is returned.
// 
// pRArray[EOS_APPLICATION_VISCOSITIES] 
// 
// Pointer to phase viscosity array of length number 
// of objects times the number of phases, stored in 
// that order. Only returned if information requested.  
// If the class is initialized with its own memory, a 
// pointer to the location for memory allocated for 
// this array is returned.
// 
// pRArray[EOS_APPLICATION_DVISCOSITIES] 
// 
// Pointer to derivatives of phase viscosity array of 
// length number of objects times the number of 
// derivatives times the number of phases, stored in 
// that order. Derivative information is stored as 
// first the pressure, then the compositions, and 
// finally the temperature.  If pressure derivatives 
// are not requested, note that the composition 
// derivative is in the first location.  Only returned 
// if requested.  If the class is initialized with its 
// own memory, a pointer to the location for memory 
// allocated for this array is returned.
// 
// pRArray[EOS_APPLICATION_TENSIONS] 
// 
// Pointer to phase tension array of length number of 
// objects times the number of phases, stored in that 
// order. Only returned if information requested.  If 
// the class is initialized with its own memory, a 
// pointer to the location for memory allocated for 
// this array is returned.
// 
// pRArray[EOS_APPLICATION_DTENSIONS] 
// 
// Pointer to derivatives of phase tension array of 
// length number of objects times the number of 
// derivatives times the number of phases, stored in 
// that order. Derivative information is stored as 
// first the pressure, then the compositions, and 
// finally the temperature.  If pressure derivatives 
// are not requested, note that the composition 
// derivative is in the first location.  Only returned 
// if requested.  If the class is initialized with its 
// own memory, a pointer to the location for memory 
// allocated for this array is returned.
// 
// pRArray[EOS_APPLICATION_DEPTH]       
// 
// Pointer to depth array of length number of objects, 
// used in initialization calculations.  Must be 
// monotonic. If the class is initialized with its own 
// memory, a pointer to the location for memory 
// allocated for this array is returned.
//   
// pRArray[EOS_APPLICATION_PRESSURE] 
// 
// Pointer to reference pressure array of length 
// number of objects.  If initialization calculations, 
// need only be specified at the reference depth. If 
// the class is initialized with its own memory, a 
// pointer to the location for memory allocated for 
// this array is returned.
// 
// pRArray[EOS_APPLICATION_SPLIT] 
// 
// Pointer to saved phase split array of length number 
// of objects.  Only used when in restore mode, i.e., the 
// results of flash calculations are saved as initial 
// values for the next iteration. 
// 
// pRArray[EOS_APPLICATION_KVALUES] 
// 
// Pointer to saved K-Value array of length number of 
// objects times the number of application components.  
// Only used in restore mode, i.e., when the results of 
// flash calculations are saved as initial values for 
// the next iteration. 
// 
// pRArray[EOS_APPLICATION_BPRESSURE] 
// 
// Pointer to bubble point pressure array of length 
// number of objects. Only returned if requested.  If 
// the class is initialized with its own memory, a 
// pointer to the location for memory allocated for 
// this array is returned.
// 
// pRArray[EOS_APPLICATION_DPRESSURE] 
// 
// Pointer to derivatives of bubble point pressure 
// array of length number of objects times the number 
// of derivatives, in that order. Derivative 
// information is stored as first the pressure, then 
// the compositions, and finally the temperature.  If 
// pressure derivatives are not requested, note that 
// the composition derivative is in the first 
// location. Only returned if requested.  If the class 
// is initialized with its own memory, a pointer to 
// the location for memory allocated for this array is 
// returned.
// 
// pRArray[EOS_APPLICATION_MW] 
// 
// Pointer to molecular weight array of length number 
// of objects times the number of phases. Only 
// returned if requested.  If the class is initialized 
// with its own memory, a pointer to the location for 
// memory allocated for this array is returned.
// 
// pRArray[EOS_APPLICATION_PHASEPRESSURE] 
// 
// Pointer to phase pressure array of length number of 
// objects times the number of phases. Only returned 
// from initialization computations.  If the class is 
// initialized with its own memory, a pointer to the 
// location for memory allocated for this array is 
// returned.
// 
// EOS_APPLICATION_LAST_DARRAY 
// 
// Number of elements in array of pointers to real 
// arrays in class EosApplication. 
*/

/*
 * This macro is a local copy of shared one in frontend
 * It exists to allow stand-alone eos flash application
 */
#ifdef FE_DEBUG
#define USE_PARAM_EOSPVT( x ) ( (void)( x ) )
#else
#define USE_PARAM_EOSPVT( x ) ( (void)( x ) )
#endif

/* Standalone mode */
#define EOS_STANDALONE 1

/* Fortran offset */
#define EOS_FORTRAN 1

/* Define an option on or off */
#define EOS_OPTION_UNKNOWN - 1
#define EOS_OPTION_OFF     0
#define EOS_OPTION_ON      1

/* Debug printing */
#define EOS_DEBUG_PRINTING 1
#define EOS_DEBUG_INPUT    1

/* Define constants for type of component */
#define EOS_HYDROCARBON 0
#define EOS_AQUEOUS     1
#define EOS_NOTUSED     - 1
#define EOS_EXPLICIT    - 2

/* Define an option on or off */
#define EOS_DEFAULT_DENSITY   1000.000
#define EOS_DEFAULT_VISCOSITY 0.001
#define EOS_DEFAULT_TENSION   0.000
#define EOS_JOSSI_MW_SPLIT    90.000

/* Spectrum for ideal K values */
#define EOS_KVALUE_SPECTRUM 5.0

/* Type of single phase */
#define EOS_SINGLE_PHASE_GAS     0
#define EOS_SINGLE_PHASE_OIL     1
#define EOS_SINGLE_PHASE_DEFAULT 2
#define EOS_SINGLE_PHASE_ZMINUSB 3
#define EOS_SINGLE_PHASE_AOVERB  4
#define EOS_WATER                5

/* Define type of expansion */
#define EOS_EXPANSION      0
#define EOS_DIFFLIB        1
#define EOS_CONSTANTVOLUME 2

/* Type of water model */
#define EOS_WATER_CORRELATIONS 0
#define EOS_WATER_LINEAR       1

/* Type of c model */
#define EOS_CONSTANT_C 0
#define EOS_LINEAR_C   1
#define EOS_ZERO_C     - 1

/* Define constants for flash states */
#define EOS_FL_1P_NCV 1
#define EOS_FL_1P_CV  2
#define EOS_FL_2P_NCV 3
#define EOS_FL_2P_CV  4
#define EOS_FL_BP_NCV 5
#define EOS_FL_BP_CV  6
#define EOS_FL_BP_CHK 7

/* Define type of flash */
#define EOS_TOF_0P - 1
#define EOS_TOF_2P - 2

/* Define type of calculations */
#define EOS_FLASH_CALCULATIONS    0
#define EOS_COMPOSITIONAL_GRADING 1
#define EOS_SEPARATOR_FLASH       2

/* Define constants for final states */
#define EOS_FL_0P 0
#define EOS_FL_1P 1
#define EOS_FL_2P 2
#define EOS_FL_BP 3
#define EOS_FL_AP - 9

/* Define constants for loads */
#define EOS_TO_APPLICATION 0
#define EOS_TO_INTERNAL    1

/* Define constants for loads */
#define EOS_NOGETK 0
#define EOS_GETK   1

/* Define constants for scaling K values */
#define EOS_NOSCALEK 0
#define EOS_SCALEK   1

/* Define constants for retrograde bubble points */
#define EOS_NORETROGRADE 0
#define EOS_RETROGRADE   1

/* Define constants for forcing bubble point calculation */
#define EOS_NOFORCEBP 0
#define EOS_FORCEBP   1

/* Define constants for flash pressure */
#define EOS_NONORMALIZE 0
#define EOS_NORMALIZE   1

/* Define constants for getting density */
#define EOS_DENSITY    0
#define EOS_VOLUME     1
#define EOS_SEPARATOR  2
#define EOS_STOCK_TANK 3

/* Define constants for phase identification */
#define EOS_GAS 0
#define EOS_OIL 1

/* Define derivative levels */
#define EOS_NODRV 0
#define EOS_DRV_N 1
#define EOS_DRV_P 2
#define EOS_DRV_T 3

/* Define whether to restore terms */
#define EOS_NORESTORE 0
#define EOS_RESTORE   1

/* Define whether to get fugacity and heats */
#define EOS_NOFUGACITY  0
#define EOS_FUGACITY    1
#define EOS_NOHEAT      0
#define EOS_HEAT        1
#define EOS_NOPOTENTIAL 0
#define EOS_POTENTIAL   1

/* Define whether to get phase identification */
#define EOS_NOPHASEID 0
#define EOS_PHASEID   1

/* Define convergence flags */
#define EOS_NOCONVERGE 0
#define EOS_CONVERGE   1

/* Terms for initialization */
#define EOS_INIT_NONE - 1
#define EOS_INIT_COMP 0

/* Directions for initialization */
#define EOS_INIT_DOWN  - 1
#define EOS_INIT_START 0
#define EOS_INIT_UP    1
#define EOS_INIT_END   2

/* Define useful integers for data transfer */
#define PVTMETHOD                       0
#define EOS_MAXITN                      1
#define EOS_MAXFLASH                    2
#define EOS_MICHELSON                   3
#define EOS_SUBSTITUTIONS               4
#define EOS_OWNMEMORY                   5
#define EOS_DEBUG                       6
#define EOS_LAST_INTEGER                7
#define EOS_METHOD                      0
#define EOS_METHOD_COMPONENTS           1
#define EOS_METHOD_PHASEID              2
#define EOS_METHOD_NUMBERABC            3
#define EOS_METHOD_HEATDATA             4
#define EOS_METHOD_OWNMEMORY            5
#define EOS_METHOD_SALTS                6
#define EOS_METHOD_WATERMODEL           7
#define EOS_METHOD_TYPE_OF_C            8
#define EOS_METHOD_TYPE_OF_BIJ          9
#define EOS_METHOD_LAST_INTEGER         10
#define APPLICATION                     0
#define EOS_APPLICATION_COMPONENTS      1
#define EOS_APPLICATION_NUMBERPHASES    2
#define EOS_APPLICATION_FLASHES         3
#define EOS_APPLICATION_THERMAL         4
#define EOS_APPLICATION_WATER           5
#define EOS_APPLICATION_LIQUID          6
#define EOS_APPLICATION_VAPOUR          7
#define EOS_APPLICATION_AQUEOUS         8
#define EOS_APPLICATION_OBJECTS         9
#define EOS_APPLICATION_RESTORE         10
#define EOS_APPLICATION_OWNMEMORY       11
#define EOS_APPLICATION_DERIVATIVES     12
#define EOS_APPLICATION_VISCOSITY       13
#define EOS_APPLICATION_TENSION         14
#define EOS_APPLICATION_HEAT            15
#define EOS_APPLICATION_VOLUMES         16
#define EOS_APPLICATION_COMPOSITIONS    17
#define EOS_APPLICATION_MOLARDENSITY    18
#define EOS_APPLICATION_MOLEFRACTIONS   19
#define EOS_APPLICATION_MOLECULARWEIGHT 20
#define EOS_APPLICATION_FLASH           21
#define EOS_APPLICATION_PROPERTIES      22
#define EOS_APPLICATION_GETBUB          23
#define EOS_APPLICATION_INITIALIZE      24
#define EOS_APPLICATION_TABLEENTRY      25
#define EOS_APPLICATION_PSEUDO          26
#define EOS_APPLICATION_BUBDEW          27
#define EOS_APPLICATION_LAST_INTEGER    28

/* For types of pvt table models */
#define EOS_PVT_TABLE  0
#define EOS_PVT_PR     1
#define EOS_PVT_PRCORR 2
#define EOS_PVT_RK     3
#define EOS_PVT_SRK    4
#define EOS_PVT_IDEAL  5

/* For type of model */
#define EOS_MORES     0
#define EOS_PVT_MODEL 0

/* Real data */
#define EOS_ENORM                   0
#define EOS_TINY                    1
#define EOS_CONVERGENCE             2
#define EOS_THERMALDIFFUSION        3
#define EOS_BUBBLEREDUCE            4
#define EOS_LAST_DOUBLE             5
#define EOS_METHOD_SALINITY         0
#define EOS_METHOD_WATERIFT         1
#define EOS_METHOD_KB1              2
#define EOS_METHOD_KB2              3
#define EOS_METHOD_PRESCONV         4
#define EOS_METHOD_TEMPCONV         5
#define EOS_METHOD_VOLCONV          6
#define EOS_METHOD_VISCCONV         7
#define EOS_METHOD_DENSCONV         8
#define EOS_METHOD_HEATCONV         9
#define EOS_METHOD_TENSCONV         10
#define EOS_METHOD_CRITZMINUSB      11
#define EOS_METHOD_CRITAOVERB       12
#define EOS_METHOD_VTUNE            13
#define EOS_METHOD_VTUNE0           14
#define EOS_METHOD_VTUNE1           15
#define EOS_METHOD_VTUNE2           16
#define EOS_METHOD_VTUNE3           17
#define EOS_METHOD_VTUNE4           18
#define EOS_METHOD_VTERM1           19
#define EOS_METHOD_VTERM2           20
#define EOS_METHOD_VTERM3           21 
#define EOS_METHOD_VTERM4           22
#define EOS_METHOD_CSVSCMETHOD      23
#define EOS_METHOD_LAST_DOUBLE      24
#define EOS_APPLICATION_CHANGE      0
#define EOS_APPLICATION_PMIN        1
#define EOS_APPLICATION_LAST_DOUBLE 2

/* Integer arrays */
#define INTEGERDATA                 0
#define EOS_LAST_IARRAY             1
#define EOS_METHOD_LAST_IARRAY      1
#define EOS_APPLICATION_EOSCOMPS    1
#define EOS_APPLICATION_INDIRECTION 2
#define EOS_APPLICATION_PHASES      3
#define EOS_APPLICATION_LAST_IARRAY 4

/* Real arrays */
#define REALDATA                      0
#define EOS_LAST_DARRAY               1
#define EOS_METHOD_CRITICAL           1
#define EOS_METHOD_RAWABC             2
#define EOS_METHOD_ABC                3
#define EOS_METHOD_HEATS              4
#define EOS_METHOD_SALTMW             5
#define EOS_METHOD_LAST_DARRAY        6
#define EOS_APPLICATION_TEMPERATURE   1
#define EOS_APPLICATION_ACCUMULATION  2
#define EOS_APPLICATION_FRACTION      3
#define EOS_APPLICATION_DFRACTION     4
#define EOS_APPLICATION_TOTALPHASE    5
#define EOS_APPLICATION_DTOTALPHASE   6
#define EOS_APPLICATION_DENSITY       7
#define EOS_APPLICATION_DDENSITY      8
#define EOS_APPLICATION_VISCOSITIES   9
#define EOS_APPLICATION_DVISCOSITIES  10
#define EOS_APPLICATION_TENSIONS      11
#define EOS_APPLICATION_DTENSIONS     12
#define EOS_APPLICATION_DEPTH         13
#define EOS_APPLICATION_PRESSURE      14
#define EOS_APPLICATION_SPLIT         15
#define EOS_APPLICATION_KVALUES       16
#define EOS_APPLICATION_BPRESSURE     17
#define EOS_APPLICATION_DPRESSURE     18
#define EOS_APPLICATION_MW            19
#define EOS_APPLICATION_PHASEPRESSURE 20
#define EOS_APPLICATION_LAST_DARRAY   21

/* Coefficients defined for the corresponding state viscosity method */
#define EOS_DIVFAC 1.0e-80
#define EOS_VCS00  0.0820561600e+0
#define EOS_VCS01  - 1.8439486666e-2
#define EOS_VCS02  1.0510162064e+0
#define EOS_VCS03  - 1.6057820303e+1
#define EOS_VCS04  8.4844027562e+2
#define EOS_VCS05  - 4.2738409106e+4
#define EOS_VCS06  7.6565285254e-4
#define EOS_VCS07  - 4.8360724197e-1
#define EOS_VCS08  8.5195473835e+1
#define EOS_VCS09  - 1.6607434721e+4
#define EOS_VCS10  - 3.7521074532e-5
#define EOS_VCS11  2.8616309259e-2
#define EOS_VCS12  - 2.8685285973e+0
#define EOS_VCS13  1.1906973942e-4
#define EOS_VCS14  - 8.5315715699e-3
#define EOS_VCS15  3.8365063841e+0
#define EOS_VCS16  2.4986828379e-5
#define EOS_VCS17  5.7974531455e-6
#define EOS_VCS18  - 7.1648329297e-3
#define EOS_VCS19  1.2577853784e-4
#define EOS_VCS20  2.2240102466e+4
#define EOS_VCS21  - 1.4800512328e+6
#define EOS_VCS22  5.0498054887e+1
#define EOS_VCS23  1.6428375992e+6
#define EOS_VCS24  2.1325387196e-1
#define EOS_VCS25  3.7791273422e+1
#define EOS_VCS26  - 1.1857016815e-5
#define EOS_VCS27  - 3.1630780767e+1
#define EOS_VCS28  - 4.1006782941e-6
#define EOS_VCS29  1.4870043284e-3
#define EOS_VCS30  3.1512261532e-9
#define EOS_VCS31  - 2.1670774745e-6
#define EOS_VCS32  2.4000551079e-5
#define EOS_VCS33  0.0096000000e+0
#define EOS_VCSCV  1.0000000000e-7
#define EOS_VCSCP  9.8692300000e-6
#define EOS_VCSMW  1.6043000000e+1
#define EOS_VCSDC  1.6248000000e+2
#define EOS_VCSPT  1.1743567500e+4
#define EOS_VCSTT  9.0680000002e+1
#define EOS_VCSRT  2.8147000000e+1
#define EOS_VCSPC  4.6001840000e+6
#define EOS_VCSTC  1.9060000000e+2
#define EOS_VCSRC  1.0127780000e+1
#define EOS_VCSSA  4.7774858000e+0
#define EOS_VCSSB  1.7606536300e+0
#define EOS_VCSSC  - 0.5678889400e+0
#define EOS_VCSSD  1.3278623100e+0
#define EOS_VCSLA  - 0.1788601650e+0
#define EOS_VCSLB  0.0483847500e+0
#define EOS_VCSLC  - 0.0184898700e+0
#define EOS_VCSLD  0.3600000000e+0
#define EOS_VCSA1  - 2.7036003000e+0
#define EOS_VCSA2  3.1661552000e+0
#define EOS_VCSA3  - 8.6573409000e+0
#define EOS_VCSA4  5.2640362000e+0
#define EOS_VCSA5  - 3.5269034000e+0
#define EOS_VCSA6  0.4600000000e+0
#define EOS_VCSB1  - 2.0909750000e+5
#define EOS_VCSB2  2.6472690000e+5
#define EOS_VCSB3  - 1.4728180000e+5
#define EOS_VCSB4  4.7167400000e+4
#define EOS_VCSB5  - 9.4918720000e+3
#define EOS_VCSB6  1.2199790000e+3
#define EOS_VCSB7  - 9.6279930000e+1
#define EOS_VCSB8  4.2741520000e+0
#define EOS_VCSB9  - 8.1415310000e-2
#define EOS_VCSC1  1.6969859270e-3
#define EOS_VCSC2  - 0.1333723460e-3
#define EOS_VCSC3  1.4000000000e+0
#define EOS_VCSC4  1.6800000000e+2
#define EOS_VCSJ1  - 1.0350605860e+1
#define EOS_VCSJ2  8.8066614295e+0
#define EOS_VCSJ3  - 1.5132806564e+3
#define EOS_VCSJ4  1.8873011594e+2
#define EOS_VCSJ5  1.3567312582e-3
#define EOS_VCSJ6  4.5944806261e+0
#define EOS_VCSJ7  1.9377431488e+2
#define EOS_VCSK1  - 9.7460200000e+0
#define EOS_VCSK2  9.0631692206e+0
#define EOS_VCSK3  - 2.0682293095e+3
#define EOS_VCSK4  4.4605500000e+1
#define EOS_VCSK5  3.0881032754e-2
#define EOS_VCSK6  2.5871668712e+0
#define EOS_VCSK7  4.9489329154e+2
#define EOS_VCSMI  100

/*
// Mathematical functions of general utility
*/
class EosUtil
{
public:
// Vector exponential function
   static void  VectorExp( int iM, double *pInput, double *pOutput );

// Vector log function
   static void  VectorLog( int iM, double *pInput, double *pOutput );

// Vector power function
   static void  VectorPow( int iM, double *pInput1, double dInput2, double *pOutput );

// Vector power function
   static void  VectorPowX( int iM, double *pInput1, double *pInput2, double *pOutput );
};


/*
// General application interface API
*/
class EosApplication
{
public:
   EosApplication( void ){ }
   virtual ~EosApplication( void ){ }

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

   /* 
   // Various terms needed for output 
   //    pDrv     derivative level
   //    pGetV    get viscosity
   //    pGetT    get tension
   //    pGetH    get enthalpies
   //    pGetM    get molecular weights
   //    pVolume  indicator for whether densities or volumes returned
   //    pMolar   output is to be molar
   //    pMolarDensity  keep molar output as molar densities, otherwise convert to mass
   */
   virtual void WriteOutputData( int *pDrv, int *pGetV, int *pGetT, int *pGetH, int *pGetM, 
      int *pVolume, int *pMolar, int *pMolarDensity )
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

   /* 
   // Write Control Data -  Get control terms for flasher
   //
   // pType
   //    Number of hydrocarbon phases allowed in flasher, either
   //       EOS_TOF_2P
   //          Two hydrocarbon phases may be present
   //       EOS_TOF_0P
   //          No hydrocarbon phases present
   //       EOS_SINGLE_PHASE_OIL
   //          Single phase oil only possible
   //       EOS_SINGLE_PHASE_GAS
   //          Single phase gas only possible
   // pSaved
   //    Indicator for whether saved K values used
   // pNobj
   //    Total number of objects
   // pFlash
   //    Do the flash
   // pProp
   //    Get properties
   // pBubble
   //    Do a bubble dew point calculation instead of a flash
   // pWater
   //    Do water phase calculations
   // pInit
   //    Do initialization or separator calculations; values are
   //       EOS_FLASH_CALCULATIONS
   //          Indicates that the flasher will be used for flash 
   //       EOS_COMPOSITIONAL_GRADING
   //          Indicates that the flasher will be used for 
   //          compositional grading calculations.
   //       EOS_SEPARATOR_FLASH
   //          Indicates that the flasher will be used for 
   //          separator calculations.
   // pBubbleDew
   //    Indicates whether bubble point tracking to be used for
   //    a single phase mixture
   // pPseudoProperties
   //    Indicates whether pseudo phase properties to be generated
   //    for Application
   //
   // 1) Access and return data
   */
   virtual void WriteControlData( int *pType, int *pSaved, int *pNobj, int *pFlash, int *pProp,
      int *pBubble, int *pWater, int *pInit, int *pBubbleDew, int *pPseudoProperties )
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


   /* 
   // WriteData - Load grid block properties into temporary vectors 
   //
   // iM ** constant **
   //    Number of elements in slice
   // i1 ** constant **
   //    Starting index between 0 and iM - 1
   // i2 ** constant **
   //    Ending index between 0 and iM - 1
   // iGetK ** constant **
   //    Indicator if estimate of K values required
   // isSalt ** constant **
   //    Indicator if we are getting hydrocarbon or water information
   // pP
   //    Pointer to pressure to be loaded
   // pT 
   //    Pointer to temperature to be loaded
   // pZ
   //    Pointer to composition to be loaded
   // pSplit
   //    Pointer to phase split to be loaded
   // pValueK
   //    Pointer to K values to be loaded
   // pPhases
   //    Pointer to phase indicator (2 phase, etc.) to be loaded
   // pIsothermal
   //    If temperature data is present then returns that 
   //    this is not isothermal.
   // pMolar
   //    Indicates that the returned data is in terms of mole fractions
   */
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


   /* 
   // Load grid block properties into temporary vectors for grading 
   //
   // iDirection ** constant **
   //    Indicator which direction we are going
   // pD ** constant **
   //    Depth of current entry
   // pP
   //    Pressure, only loaded at initial node
   // pT
   //    Temperature
   // pZ
   //    Composition, only loaded at initial node
   // pIsothermal
   //    Indicator whether the temperature array has been 
   //    specified
   // pMolar
   //    Indicator as to whether molar or mass values being returned
   */
   virtual void WriteGradingData( int iDirection, double *pD, double *pP, double *pT, 
      double *pZ, int *pIsothermal, int *pMolar )
   {
      USE_PARAM_EOSPVT( iDirection );
      USE_PARAM_EOSPVT( pD );
      USE_PARAM_EOSPVT( pP );
      USE_PARAM_EOSPVT( pT );
      USE_PARAM_EOSPVT( pZ );
      USE_PARAM_EOSPVT( pIsothermal );
      USE_PARAM_EOSPVT( pMolar );
   }


   /* 
   // Writes the gas oil contact. Returns GOC.  
   // If one is not present returns a number higher than highest entry
   */
   virtual double WriteGOC( void )  { return ( 0.0 ); }


   /* 
   // Routine to load composition of gas and density of oil
   // at previous separator
   //
   // pY
   //    Gas composition to be output
   // pZx
   //    Oil Z factor
   // pDZxda
   //    Derivative of oil z factor
   */
   virtual void WriteSeparatorInfo( double *pY, double *pZx, double *pDZxda )
   {
      USE_PARAM_EOSPVT( pY );
      USE_PARAM_EOSPVT( pZx );
      USE_PARAM_EOSPVT( pDZxda );
   }


   /* 
   // Routine to load separator derivatives
   // 
   // iPhase ** constant **
   //    Index of phase to be returned
   // pDX
   //    Pointer to derivative to be stored
   */
   virtual void WriteSeparatorDrv( int iPhase, double *pDX )
   { 
      USE_PARAM_EOSPVT( iPhase ); 
      USE_PARAM_EOSPVT( pDX ); 
   }

   /* 
   // Write separator data to application structures
   //
   // iM ** constant **
   //    Total number of feed streams
   // iFeed ** constant **
   //    Index of the feed stream
   // iInlet ** constant **
   //    Inlet for this flash
   // pFeed ** constant **
   //    The actual feeds
   */
   virtual void WriteSeparatorData( int iM, int iFeed, int *pInlet, double *pFeed )
   {
      USE_PARAM_EOSPVT( iM );
      USE_PARAM_EOSPVT( iFeed );
      USE_PARAM_EOSPVT( pInlet );
      USE_PARAM_EOSPVT( pFeed );
   }


   /* 
   // Routine to change volume control as we step through separator calculations
   //
   // iTVolume
   //    Term to set volume control to
   //       EOS_DENSITY 
   //          Density and mole/mass fractions
   //       EOS_VOLUME 
   //          Volumes only
   //       EOS_SEPARATOR
   //          Total moles/mass only
   //       EOS_STOCK_TANK
   //          Volumes and total moles/mass
   */
   virtual void ReadVolumeControl( int iTVolume )
   { 
      USE_PARAM_EOSPVT( iTVolume );
   }


   /* 
   // Routine to change minimum allowed pressure
   //
   // dMinP  Minimum allowed pressure
   */
   virtual void ReadMinimumPressure( double dMinP )
   {
      USE_PARAM_EOSPVT( dMinP ); 
   }


   /* 
   // Read separator data to Application structures
   //
   // iM ** constant **
   //    Total number of feed streams
   // iFeed ** constant **
   //    Index of the feed stream
   // iDrv ** constant **
   //    Derivative level for Application run
   // pQv
   //    Phase surface volumes
   // pDQv
   //    Derivatives of phase surface volumes
   // pQm
   //    Phase surface mass
   // pComp
   //    Phase surface compositions
   */
   virtual void ReadSeparatorData( int iM, int iFeed, int iDrv, double *pQv, double *pDQv, 
      double *pQm, double *pComp )
   {
      USE_PARAM_EOSPVT( iM );
      USE_PARAM_EOSPVT( iFeed );
      USE_PARAM_EOSPVT( iDrv );
      USE_PARAM_EOSPVT( pQv );
      USE_PARAM_EOSPVT( pDQv );
      USE_PARAM_EOSPVT( pQm );
      USE_PARAM_EOSPVT( pComp );
   }


   /* 
   // Routine to store phase properties into application permanent arrays 
   //
   // iPhaseId ** constant **
   //    Type of phase to be processed
   //       EOS_SINGLE_PHASE_OIL
   //          Phase stored in "x" arrays in flasher
   //       EOS_SINGLE_PHASE_GAS
   //          Phase stored in "y" arrays in flasher
   //       EOS_WATER
   //          Aqueous phase
   // iM ** constant **
   //    Number of objects to read
   // iNc ** constant **
   //    Number of components used in the flasher
   // pPhases ** constant **
   //    Indentifier for phases
   //       EOS_SINGLE_PHASE_OIL
   //          Phase is a liquid
   //       EOS_SINGLE_PHASE_GAS
   //          Phase is a gas
   //    Not used for an aqueous phase load
   // pX ** constant **
   //    Pointer to compositions or overall mass/moles from the flasher
   // pDX ** constant **
   //    Pointer to derivatives of pX
   // pMx ** constant **
   //    Pointer to total mass/moles from the flasher
   // pDMx ** constant **
   //    Pointer to derivatives of pMx
   // pZx ** constant **
   //    Pointer to density or volume from the flasher
   // pDZx ** constant **
   //    Pointer to derivatives of pZx
   // pMux ** constant **
   //    Pointer to viscosity from the flasher
   // pDMux ** constant **
   //    Pointer to derivatives of pMux
   // pIfx ** constant **
   //    Pointer to tension from the flasher
   // pDIfx ** constant **
   //    Pointer to derivatives of pIfx
   // pHx ** constant **
   //    Pointer to enthalpies from flasher
   // pDHx ** constant **
   //    Pointer to derivatives of pHx
   // pP ** constant **
   //    Pointer to bubble or dew point from flasher
   // pDP ** constant **
   //    Pointer to derivatives of pP
   // pMW ** constant **
   //    Pointer to molecular weight from flasher
   */
   virtual void ReadData( int iPhaseId, int iM, int iNc, int *pPhases, double *pX, 
      double *pDX, double *pMx, double *pDMx, double *pZx, double *pDZx, double *pMux, 
      double *pDMux, double *pIfx, double *pDIfx, double *pHx, double *pDHx, 
      double *pP, double *pDP, double *pMwx )
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


   /* 
   // Routine to store phase properties into Application permanent arrays 
   // when a phase is not present
   //
   // iPhaseId ** constant **
   //    Type of phase to be processed
   //       EOS_SINGLE_PHASE_OIL
   //          Phase stored in "x" arrays in flasher
   //       EOS_SINGLE_PHASE_GAS
   //          Phase stored in "y" arrays in flasher
   //       EOS_WATER
   //          Aqueous phase
   // iM ** constant **
   //    Number of objects to read
   // pPhases ** constant **
   //    Indentifier for phases
   //       EOS_SINGLE_PHASE_OIL
   //          Phase is a liquid
   //       EOS_SINGLE_PHASE_GAS
   //          Phase is a gas
   //    Not used for an aqueous phase load
   */
   virtual void ReadNull( int iPhaseId, int iM, int *pPhases )
   {
      USE_PARAM_EOSPVT( iPhaseId );
      USE_PARAM_EOSPVT( iM );
      USE_PARAM_EOSPVT( pPhases );
   }


   /* 
   // Subroutine to load flash results into permanent arrays 
   //
   // iS ** constant **
   //    Index of element to load; if iM then load all
   // iM ** constant **
   //    Total number of elements to load
   // iSet ** constant **
   //    Integer above which success is declared.  Generally when the
   //    pPhase array is of following form
   //       EOS_2P_NCV
   //          Nonconverged two phase flash
   //       EOS_2P_CV
   //          Converged two phase flash
   //       EOS_BP_NCV
   //          Nonconverged bubble point calculation
   //       EOS_BP_CV
   //          Converged bubble point calculation
   //    It is better to load partially converged blocks than
   //    to ditch them!
   // iReset ** constant **
   //    Integer controlling what happens if not successful. 
   //    Lack of success is generally indicated by the following
   //       EOS_1P_NCV
   //          Nonconverged one phase system
   //    If iReset is on then the current compositions and
   //    pressures are saved in order to be used later to see
   //    if we need to do the stability testing again.  
   // pSplit ** constant **
   //    Phase split from the flasher.  Overloaded with bubble
   //    or dew point for a bubble point calculation
   // pPhases ** constant **
   //    Current phase indicator from flasher; see above
   // pValueK ** constant **
   //    Current estimate of K values
   */
   virtual void ReadFlashResults( int iS, int iM, int iSet, int iReset, double *pSplit, 
      int *pPhases, double *pValueK )
   {
      USE_PARAM_EOSPVT( iS );
      USE_PARAM_EOSPVT( iM );
      USE_PARAM_EOSPVT( iSet );
      USE_PARAM_EOSPVT( iReset );
      USE_PARAM_EOSPVT( pSplit );
      USE_PARAM_EOSPVT( pPhases );
      USE_PARAM_EOSPVT( pValueK );
   }

   /* 
   // Load grading results into permanent arrays 
   //
   // dDepth ** constant **
   //    Current depth returned from flasher
   // dTemperature ** constant **
   //    Current temperature returned from flasher
   // dPressure ** constant **
   //    Current reference phase pressure returned from flasher
   // dPx ** constant **
   //    Current oil phase pressure returned from flasher; either
   //    reference phase pressure or dew point
   // dPy ** constant **
   //    Current gas phase pressure returned from flasher; either
   //    reference phase pressure or bubble point
   // dRox ** constant **
   //    Current oil density at oil phase pressure
   // dRoy ** constant **
   //    Current gas density at gas phase pressure
   // dHx ** constant **
   //    Current oil enthalpy at oil phase pressure
   // dHy ** constant **
   //    Current gas density at gas phase pressure
   // pZ ** constant **
   //    Composition of reference phase
   // pX ** constant **
   //    Composition of oil phase, either reference phase
   //    or at dew point
   // pY ** constant **
   //    Composition of gas phase, either reference phase
   //    or at bubble point
   */
   virtual void ReadGradingData( double dDepth, double dTemperature, double dPressure, 
      double dPx, double dPy, double dRox, double dRoy, double dHx, double dHy, 
      double *pZ, double *pX, double *pY )
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

   /* 
   // Reads the gas oil contact 
   //
   // dGOCT ** constant **
   //    Gas oil contact computed by flasher
   */
   virtual void ReadGOC( double dGOCT )
   { 
      USE_PARAM_EOSPVT( dGOCT ); 
   }

   // Print application Input Data used for debugging purposes
   virtual void PrintInputData( void ) { }


   // Print application Output Data used for debugging purposes
   virtual void PrintOutputData( void ){ }

   /* 
   // Modify bubble point indicator 
   //
   // iType ** constant **
   //    Direction in which the conversion occurs
   //       EOS_TO_INTERNAL
   //          Change from Application storage (negative split
   //          for bubble point) to flasher storage 
   //       EOS_TO_APPLICATION
   //          Change to Application storage (negative split
   //          for bubble point) from flasher storage 
   // iFrom ** constant **
   //    Phase indicator on which to perform conversion
   //       EOS_FL_1P, EOS_FL_1P_CV, or EOS_FL_1P_NCV
   //          General input on from Application
   //       EOS_FL_BP, EOS_FL_BP_CV, or EOS_FL_BP_NCV
   //          General input on to Application
   // iTo ** constant **
   //    Phase indicator on which to perform conversion
   //       EOS_FL_1P, EOS_FL_1P_CV, or EOS_FL_1P_NCV
   //          General input on to Application
   //       EOS_FL_BP, EOS_FL_BP_CV, or EOS_FL_BP_NCV
   //          General input on from Application
   */
   virtual void ModifyBubble( int iType, int iFrom, int iTo )
   {
      USE_PARAM_EOSPVT( iType );
      USE_PARAM_EOSPVT( iFrom );
      USE_PARAM_EOSPVT( iTo );
   }


   /* 
   // Routine to modify phase property indicators 
   //
   // iDoAll ** constant **
   //    Indicator as to whether to modify all phase indicators
   //    or only the current flash
   */
   virtual void ModifyPhaseIndicator( int iDoAll )
   { 
      USE_PARAM_EOSPVT( iDoAll ); 
   }

   /* 
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
   */
   virtual void ModifyPhaseIdentification( double dEnorm )
   { 
      USE_PARAM_EOSPVT( dEnorm ); 
   }


   /* 
   // Check to see which grid blocks to do stability test 
   //
   // 1) If no hydrocarbon phases then do nothing!
   // 2) Then test to see if any hydrocarbons present.  If not
   //    then the block is EOS_FL_0P
   // 3) If only one component is present then block is EOS_FL_1P_CV
   // 4) Else block is EOS_FL_1P_NCV
   */
   virtual void ModifyOnePhaseIdentification( double dEnorm )
   { 
      USE_PARAM_EOSPVT( dEnorm ); 
   }

   /* 
   // Routine to load liquids from previous separator into the
   // current and add in any additional feed
   */
   virtual void ModifySeparatorAccumulations( void ){ }


   /* 
   // Slice objects 
   // 
   // iTestPhase ** constant **
   //    Phase indicator for which to form the slice
   // iMVL ** constant **
   //    Slice length
   //
   // Returns the length of the slice
   */
   virtual int  SetSlice( int iTestPhase, int iMVL )
   { 
      USE_PARAM_EOSPVT( iTestPhase ); 
      USE_PARAM_EOSPVT( iMVL ); 
      return ( 0 ); 
   }

   /* 
   // See if any more objects present to compute
   // 
   // iM ** constant ** 
   //   Set the indirection "aan de beurt" to this value if greater than zero
   // iTestPhase ** constant ** 
   //   Phase indicator for which to check if anything more present for calculations
   //
   // Returns 1 if anything aan de beurt, 0 otherwise
   */
   virtual int  Aandebeurt( int iM, int iTestPhase )
   { 
      USE_PARAM_EOSPVT( iM ); 
      USE_PARAM_EOSPVT( iTestPhase ); 
      return ( 0 ); 
   }


   // Set trivial slice 
   virtual void SetTrivialSlice( void ){ }


   /* 
   // Routine to set current pointer to next separator 
   //
   // iDirection - direction to move
   //    EOS_INIT_START - set to first separator
   //    EOS_INIT_END   - set to stock tank
   //    EOS_INIT_UP    - go up one step
   //    EOS_INIT_DOWN  - go down one step
   //
   // Returns the index of the next separator
   */
   virtual int  SetSeparator( int iDirection )
   { 
      USE_PARAM_EOSPVT( iDirection ); 
      return ( 0 ); 
   }

   /* 
   // Reset table entry in compositional grading
   // 
   // iDirection
   //    Direction to move in the table
   //       EOS_INIT_START
   //          Go to the reference entry with data
   //       EOS_INIT_UP
   //          Increment pointer to a higher number
   //       EOS_INIT_DOWN
   //          Increment pointer to a lower number
   //
   // Returns 1 if there is another entry in table
   */
   virtual int SetTableEntry( int iDirection )
   { 
      USE_PARAM_EOSPVT( iDirection ); 
      return ( 0 );
   }

   /* 
   // Set the pointers to temporary memory
   //
   // iN
   //    Length of calculations
   // pFinal
   //    On input the end of the temporary memory.  On output
   //    the new end
   //
   // The memory will actually be allocated from within the
   // EosPvtModel class.  This routine gets two passes; the
   // first calculates the memory assuming that the pointers
   // start at null to get the length, the second after the
   // allocation to set the correct locations
   */
   virtual void SetPointers( int iN, double **pFinal )
   { 
      USE_PARAM_EOSPVT( iN ); 
      USE_PARAM_EOSPVT( pFinal ); 
   }


   // Reset the slice back to the beginning of objects
   virtual void ResetSlice( void )  { }

private:
   // Copy constructor turned off
   EosApplication( EosApplication &self );
};

/*
// Methods for the water properties
*/
class EosWater
{
public:
   // Constructor
   EosWater( void );

   //Copy constructor; shut off
   EosWater( const EosWater &self );

   // Destructor
   virtual ~EosWater( void ){ }

   // DOCUMENT ME!
   virtual void Water( int iM, int iNc, int iDrv, int iVolume, int iGetV, int iGetT, int iGetH, int iTherm, 
      double *pP, double *pT, double *pS, double *pDSda, double *pRo, double *pDRodp, double *pDRodt,
      double *pDRods, double *pDRoda, double *pMu, double *pDMudp, double *pDMudt, double *pDMuds,
      double *pDMuda, double *pIft, double *pDIftdp, double *pDIftdt, double *pDIftda, double *pHeat,
      double *pDHeatdp, double *pDHeatdt, double *pDHeatds, double *pDHeatda, double *pWork1 );

   // Return the water vapor pressure
   double WaterVaporPressure( double dT );

protected:
   // Get the water density
   void WaterDensity( int iM, int iNc, int iDrv, int iVolume, double *pP, double *pT, double *pS, double *pDSda,
      double *pRo, double *pDRodp, double *pDRodt, double *pDRods, double *pDRoda );

   // Compute water enthalpy
   void   WaterHeat( int iM, int iNc, int iDrv, int iTherm, double *pP, double *pT, 
      double *pS, double *pDSda, double *pHeat, double *pDHeatdp, double *pDHeatdt, 
      double *pDHeatds, double *pDHeatda, double *pWork1 );

   // Set the water/reference phase ift
   void WaterIft( int iM, int iNc, int iDrv, double *pIft, double *pDIftdp, double *pDIftdt, double *pDIftda );

   // Set the water viscosity
   void WaterViscosity( int iM, int iNc, int iDrv, int iTherm, double *pP, double *pT, double *pS, double *pDSda,
      double *pRo, double *pDRodp, double *pDRodt, double *pDRods, 
      double *pMu, double *pDMudp, double *pDMudt, double *pDMuds, 
      double *pDMuda, double *pWork1 );

   // Get the water volume
   void WaterVolume( int iM, int iDrv, int iTherm, double *pP, double *pT, double *pS, double *pRo, double *pDRodp,
      double *pDRodt, double *pDRods, double *pWork1 );

   // Set the salt partial volume
   void SaltVolume( int iM, int iDrv, int iTherm, double *pP, double *pT, double *pS, double *pVol, double *pDVoldp,
      double *pDVoldt, double *pDVolds );

   // Set the salt partial heat
   void SaltHeat( int iM, int iDrv, int iTherm, double *pP, double *pT, double *pS, double *pHeat, double *pDHeatdp,
      double *pDHeatdt, double *pDHeatds );

   // Get temperature terms for water enthalpy
   void WaterHeatTerms( int iM, int iDrv, double *pT, double *pWTerms, double *pDTerms );

   // Get temperature dependent terms for water viscosity
   void WaterViscosityTerms( int iM, int iDrv, double *pT, double *pWTerms, double *pDTerms );

   // Get temperature terms for water volume
   void WaterVolumeTerms( int iM, int iDrv, double *pT, double *pWTerms, double *pDTerms );

protected:

   // Pressure conversion constant from MKS units to flasher internal units.  
   // For PSI set to 1/6894.7. Controlled by pdTables[EOS_METHOD_PRESCONV] 
   double m_dConvPressure;

   // Temperature conversion constant from MKS units to flasher internal units.  
   // For R set to 1.8. Controlled by pdTables[EOS_METHOD_TEMPCONV] 
   double m_dConvTemperature;

   // Volume conversion constant from MKS units to flasher internal units.
   // For FT3 set to 35.314667. Controlled by pdTables[EOS_METHOD_VOLCONV]   
   double m_dConvVolume;

   // Viscosity conversion constant from MKS units to flasher internal units.
   // For CP set to 1000. Controlled by pdTables[EOS_METHOD_VISCCONV]              
   double m_dConvViscosity;

   // Density conversion constant from MKS units to flasher internal units.  
   // For LB set to 2.2046226. Controlled by pdTables[EOS_METHOD_DENSCONV]  
   double m_dConvDensity;

   // Heat conversion constant from MKS units to flasher internal units.  
   // For BTU set to 2.2046226. Controlled by pdTables[EOS_METHOD_HEATCONV] 
   double m_dConvHeat;

   // Water/reference phase ift pdTables[EOS_METHOD_WATERIFT] 
   double m_dWaterIft;

   // Water model constant.
   // Controlled by piTables[EOS_METHOD_WATERMODEL]
   // Options are:
   //  EOS_WATER_CORRELATIONS: use correlations
   //  EOS_WATER_LINEAR: use linear model
   int m_iWaterModel;

   //--------------------------------------------------------------------// 
   // Terms associated with the calculation of water DENSITY, viz.:
   // DENS = DENS0 * ( 1 + DENSDP * P ) * ( 1 + DENSDT * T ) + S *
   //      ( DENS0DS * ( 1 + DENSDPDS * P ) * ( 1 + DENSDTDS * T ) )
   //--------------------------------------------------------------------// 

   // Water base density constant.  
   // First element of array controlled by pRArray[EOS_METHOD_WATERTERMS]
   double m_dWaterDens;

   // Water density pressure constant term. 
   // Second element of array controlled by pRArray[EOS_METHOD_WATERTERMS]
   double m_dWaterDensDp;

   // Water density temperature constant term.
   // Third element of array controlled by pRArray[EOS_METHOD_WATERTERMS]
   double m_dWaterDensDt;

   // Water base density salinity derivative. 
   // Eigth element of array controlled by pRArray[EOS_METHOD_WATERTERMS]
   double m_dWaterDensDs;

   // Water density pressure term salinity derivative.
   // Ninth element of array controlled by pRArray[EOS_METHOD_WATERTERMS]
   double m_dWaterDensDpDs;

   // Water density temperature term salinity derivative.
   // Tenth element of array controlled by pRArray[EOS_METHOD_WATERTERMS]
   double m_dWaterDensDtDs;

   //--------------------------------------------------------------------// 
   // Terms associated with the calculation of water VISCOSITY, viz.:
   // VISC = VISC0 * ( 1 + VISCDP * P ) * ( 1 + VISCDT * T ) + S *
   //      ( VISCDS * ( 1 + VISCDPDS * P ) * ( 1 + VISCDTDS * T ) )
   //--------------------------------------------------------------------// 

   // Water base viscosity.
   // Fourth element of array controlled by pRArray[EOS_METHOD_WATERTERMS]
   double m_dWaterVisc;

   // Water viscosity pressure term. 
   // Fifth element of array controlled by pRArray[EOS_METHOD_WATERTERMS]
   double m_dWaterViscDp;

   // Water viscosity temperature term.
   // Sixth element of array controlled by pRArray[EOS_METHOD_WATERTERMS]
   double m_dWaterViscDt;

   // Water base viscosity salinity derivative. 
   // Eleventh element of array controlled by pRArray[EOS_METHOD_WATERTERMS]
   double m_dWaterViscDs;

   // Water viscosity pressure term salinity derivative.
   // Twelfth element of array controlled by pRArray[EOS_METHOD_WATERTERMS]
   double m_dWaterViscDpDs;

   // Water viscosity temperature term salinity derivative.
   // Thirteenth element of array controlled by pRArray[EOS_METHOD_WATERTERMS]
   double m_dWaterViscDtDs;

   //--------------------------------------------------------------------// 
   // Terms associated with the calculation of water ENTHALPY, viz.:
   // ENTH = ENTHDT * T + S * ENTHDTDS * T 
   //--------------------------------------------------------------------// 

   // Water enthalpy term. 
   // Seventh element of array controlled by pRArray[EOS_METHOD_WATERTERMS]
   double m_dWaterHeat;

   // Water enthalpy term salinity derivative.
   // Fourteenth element of array controlled by pRArray[EOS_METHOD_WATERTERMS]
   double m_dWaterHeatDs;
};

/* 
// Class for cached pvt tables for equation of state 
*/
class EosPvtTable : public EosWater
{
   /*
   // Routines to set up or transfer data
   */
public:
   // Constructors
   EosPvtTable( void );
   EosPvtTable( int iVersion, int *piTables, double *pdTables, double *pCritical, double *pvtData, double *pSaltMW );
   EosPvtTable( EosPvtTable *pCache, double dT );
   EosPvtTable( EosPvtTable &pCache );
   EosPvtTable( int iNcArg, double dCvpArg, double dCvtArg, double dCvvArg, 
      double dTune1, double dTune2, double dTune3, double dTune4,
      int CSVscMethod, double *pPCArg, double *pTCArg, double *pMWArg );

   // Destructor
   virtual ~EosPvtTable( void );

   // Get the number of hydrocarbons modelled by the
   // Pvt tables for use by the flasher 
   int GetNumberHydrocarbons( void );

   // Get the number of SALTS (not including water) used
   // in aqueous phase computations by the flasher
   int GetNumberSalts( void );

   // Return gravity constant and gas constant to flasher
   // for compositional grading calculations
   void GetGravity( double *pGravity, double *pGascon );

   // Print out an overview of the data
   void PrintInputData( void );

   // Write out information contained in this class to the flasher class, e.g., molecular weights
   void WritePvtInfo( int iM, int i1, int i2, int isothermal, int isSalt, int iWaterComp, int *pAbcOffSetT,
      int *pMultipleAbcT, double *pT, double *pMw );

   // Provide an initial estimate of the bubble point for the flasher
   double  InitBubblePoint( void );

   // Set the pointers to the ABC terms used in the flasher
   void SetAbcPointers( int iN, int **pAbcOffsetT, double **pFinal );

   // Set the temporary array pointers
   void SetPointers( int iN, double *pTCubic, double *pTMu, double *pTWork, double **pATable, 
      double **pSumTable, double **pFinal );

   // Writes out default temperature
   double  WriteTemperature( void );

protected:
   // Initialize the data
   void Initialize( int iVersion, int *piTables, double *pdTables, double *pCritical, double *pvtData, double *pSaltMW );

   // Copy the data
   void Copy( EosPvtTable *pCache );

   // Read all the data supplied by the application for the pvt tables
   void ReadAllData( int iVersion, int **pointI, double **pointR );

   // Read the constant terms for the pvt tables
   int ReadConstants( int iVersion, int *pITerms, double *pTerms, double *pWaterData );

   // Allocate the temporary memory
   void AllocateMemory( void );

   // Set the terms dKb2 through dKb9 for the cubic equation 
   void SetCubicTerms( double dUserKb0, double dUserKb1 );

   // Set the array pointers
   void SetArrayPointers( void );

   // Set the volume translation flag
   void SetVolumeTranslation( void );

   // Initialize calculated terms for the pvt table
   void CachePvtTables( double *pAcentric );

   // Cache the ABC data supplied from the application into
   // a more usuable form for computations
   void CacheAbcData( void );

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

   // Type of equation of state used.  Options include
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

   // Pointer to an array of length NC of molecular weights.
   // Controlled by the first NC entries of pRArray[EOS_METHOD_CRITICAL] 
   double *m_pMolecularWeight;

   // Pointer to an array of length NC of critical pressures
   // Controlled by the second NC entries of pRArray[EOS_METHOD_CRITICAL] 
   double *m_pCriticalPressure;

   // Pointer to an array of length NC of critical temperatures
   // Controlled by the third NC entries of pRArray[EOS_METHOD_CRITICAL] 
   double *m_pCriticalTemperature;

   // Pointer to an array of length NC of critical volumes
   // Controlled by the fourth NC entries of pRArray[EOS_METHOD_CRITICAL] 
   double *m_pCriticalVolume;

   // Pointer to an array of length NC of parachors
   // Controlled by the fifth NC entries of pRArray[EOS_METHOD_CRITICAL] 
   double *m_pParachor;

   // Indicator as to whether reference state heat capacity data 
   // will be available to the flasher. Controlled by
   // piTables[EOS_METHOD_HEATDATA] = EOS_OPTION_OFF || 
   //                                 EOS_OPTION_ON
   // The standard state enthalpy is calculated from
   //       H = H1 * T + H2 * T^2 + H3 * T^3 +
   //                    H4 * T^4 + H5 * T^5
   // The standard state entropy is calculated from
   //       S = H1 * ln(T) + 2 * H2 * T + 3/2 * H3 * T^2
   //           4/3 * H4 * T^3 + 5/4 * H5 * T^4 + H6
   // If heat constants are not specified the ideal
   // contribution to the enthalpy will not be calculated
   int m_iHeat;

   // Pointer to an array of length NC of heat constants
   // Controlled by the sixth NC entries of pRArray[EOS_METHOD_CRITICAL] 
   // Only loaded if iHeat is specified as EOS_OPTION_ON
   double *m_pHeat0;

   // Pointer to an array of length NC of heat constants
   // Controlled by the seventh NC entries of pRArray[EOS_METHOD_CRITICAL] 
   // Only loaded if iHeat is specified as EOS_OPTION_ON
   double *m_pHeat1;

   // Pointer to an array of length NC of heat constants
   // Controlled by the eighth NC entries of pRArray[EOS_METHOD_CRITICAL] 
   // Only loaded if iHeat is specified as EOS_OPTION_ON
   double *m_pHeat2;

   // Pointer to an array of length NC of heat constants
   // Controlled by the ninth NC entries of pRArray[EOS_METHOD_CRITICAL] 
   // Only loaded if iHeat is specified as EOS_OPTION_ON
   double *m_pHeat3;

   // Pointer to an array of length NC of heat constants
   // Controlled by the tenth NC entries of pRArray[EOS_METHOD_CRITICAL] 
   // Only loaded if iHeat is specified as EOS_OPTION_ON
   double *m_pHeat4;

   // Pointer to an array of length NC of heat constants
   // Controlled by the eleventh NC entries of pRArray[EOS_METHOD_CRITICAL] 
   // Only loaded if iHeat is specified as EOS_OPTION_ON
   double *m_pHeat5;

   // Pointer to an array of length NC of heat constants
   // Controlled by the twelfth NC entries of pRArray[EOS_METHOD_CRITICAL] 
   // Only loaded if iHeat is specified as EOS_OPTION_ON
   double *m_pHeat6;

   // Term for cubic equation of state.  
   // For Redlich-Kwong set to one; for Peng Robinson set to 1+2^0.5.
   // Controlled by pdTables[EOS_METHOD_KB0]  
   double m_dKb0;

   // Term for cubic equation of state.  
   // For Redlich-Kwong set to 0; for Peng Robinson set to 1-2^0.5. 
   // Controlled by pdTables[EOS_METHOD_KB1]  
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

   // Salinity in PPM.  
   // Used in water phase calculations when salts are not supplied.
   // Controlled by pdTables[EOS_METHOD_SALINITY] 
   double m_dSalinity;

   // Linear interpolation in temperature of C terms. 
   // Controlled by piTables[EOS_METHOD_TYPE_OF_C].
   // Options are:
   //   EOS_CONSTANT_C: constant
   //   EOS_LINEAR_C: use linear model
   int m_iTypeOfC;

   // Linear interpolation in temperature of Bij terms. 
   // Controlled by piTables[EOS_METHOD_TYPE_OF_BIJ]
   // Options are:
   //   EOS_CONSTANT_C: constant
   //   EOS_LINEAR_C: use linear model
   int m_iTypeOfBij;

   // Viscosity tuning parameter for Jossi/GSS model.
   // If set to zero use Lohrenz viscosity.  
   // Only for version 2 of flasher
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

   // Version of corresponding states model.
   //  0 is 2 parameter, 
   //  1 is 4 parameter heavy oil PVTsim-Shell JIP formulation,
   //  2 is 4 parameter heavy oil non JIP formulation, 
   int m_CSVscMethod;

   // Molecular weights of salt components, the number of
   // which is governed by EOS_METHOD_SALTS.  
   // Controlled by pRArray[EOS_METHOD_SALTMW] 
   // If this term is negative, it is interpretted as a salinity
   double *m_pSaltMW;

   // Indicator for method used for single phase naming.
   //  piTables[EOS_METHOD_PHASEID] = EOS_SINGLE_PHASE_GAS ||
   //                                 EOS_SINGLE_PHASE_OIL ||
   //                                 EOS_SINGLE_PHASE_DEFAULT || 
   //                                 EOS_SINGLE_PHASE_ZMINUSB || 
   //                                 EOS_SINGLE_PHASE_AOVERB 
   //  EOS_SINGLE_PHASE_GAS
   //     Single phase mixture to be treated as a gas
   //  EOS_SINGLE_PHASE_OIL 
   //     Single phase mixture to be treated as an oil
   //  EOS_SINGLE_PHASE_DEFAULT 
   //     Single phase mixture to be named from critical A/B
   //  EOS_SINGLE_PHASE_ZMINUSB
   //     Single phase mixture to be named from Z-B
   //  EOS_SINGLE_PHASE_AOVERB 
   //     Single phase mixture to be named from user supplied A/B
   int m_iTypeOfSingle;

   // Critical z factor minus b for naming a single phase mixture 
   // when piTables[EOS_METHOD_PHASEID] is EOS_SINGLE_PHASE_ZMINUSB.
   // 0.25 is the MoReS default.
   // Controlled by pdTables[EOS_METHOD_CRITZMINUSB]  
   double m_dCritZMinusB;

   // Critical a over b for naming a single phase mixture when 
   // piTables[EOS_METHOD_PHASEID] is EOS_SINGLE_PHASE_AOVERB.
   // 5.0 is a reasonable default value. 
   // Controlled by pdTables[EOS_METHOD_CRITAOVERB] 
   double m_dCritAOverB;

   int m_iTranslate;///< Internal indicator as to whether volume translation is being used

   // Number of temperature entries minus one in the ABC coefficient tables.  
   // For an isothermal table, enter 0, for a table with two temperatures, enter  1, etc.
   // Controlled by piTables[EOS_METHOD_NUMBERABC]
   int m_iTables;

   // Values of raw equation of state parameters for each temperature appearing in the table.
   // The order is, for each temperature, first the temperature, then the values of A, 
   // then the values of B, then the values of C, then the values of the interaction 
   // parameters.  Thus the number of entries is the number of temperatures times 
   // [ 1 + number of flash components * [ 3 + number of flash components ] ]. 
   // The flasher expects these in units of volume (why? Because that is what Libra did).
   // If the class is initialized with its own memory, a pointer to the location for 
   // memory allocated for this array is returned.  
   // Controlled by pRArray[EOS_METHOD_RAWABC] 
   double *m_pRawAbcData;

   // Array of cached temperatures for the PVT table.
   // If a single temperature, then one entry.  Otherwise iTables entries
   double *m_pTemperature;

   // Array of volumes*R*T (AiAj)^0.5*(1-bij) for the PVT table
   // at zero temperature.    If a single temperature, then NC*NC 
   // entries.  Otherwise iTables*NC*NC entries
   double *m_pAdata;

   // Array of volumes Bi for the PVT table at zero temperature. 
   // If a single temperature, then NC entries. Otherwise iTables*NC entries
   double *m_pBdata;

   // Array of volumes Ci for volume translation in the PVT table
   // at zero temperature. If a single temperature, then NC 
   // entries.  Otherwise iTables*NC entries
   double *m_pCdata;

   // Array of A values at default temperature for volumes*R*T 
   // (AiAj)^0.5*(1-bij) for the PVT table.  Always used if 
   // iMethod is not EOS_PVT_TABLE, EOS_PVT_IDEAL, or EOS_PVT_RK.
   // Otherwise not used
   double *m_pADefault;

   // Array of square root of temperature terms of volumes*R*T 
   // (AiAj)^0.5*(1-bij) for the PVT table.  Always used if 
   // iMethod is not EOS_PVT_TABLE or EOS_PVT_IDEAL. 
   // Otherwise not used
   double *m_pDAdsdata;

   // Array of linear temperature terms of volumes*R*T 
   // (AiAj)^0.5*(1-bij) for the PVT table.  Always used if 
   // iMethod is not EOS_PVT_TABLE or EOS_PVT_IDEAL. Otherwise if a 
   // single temperature no entries since constant.  Otherwise 
   // iTables*NC*NC entries Linear interpolation is then used for 
   // each temperature
   double *m_pDAdtdata;

   // Array of 3/2 power temperature terms of volumes*R*T 
   // (AiAj)^0.5*(1-bij) for the PVT table.  Only used if 
   // iMethod is not EOS_PVT_TABLE or EOS_PVT_IDEAL and temperature
   // dependent binary interaction terms used. Otherwise if a 
   // single temperature no entries since constant.  
   // iTables*NC*NC entries.
   double *m_pDAdrdata;

   // Array of 2 power temperature terms of volumes*R*T 
   // (AiAj)^0.5*(1-bij) for the PVT table.  Only used if 
   // iMethod is not EOS_PVT_TABLE or EOS_PVT_IDEAL and temperature
   // dependent binary interaction terms used. Otherwise if a 
   // single temperature no entries since constant.  
   // iTables*NC*NC entries.
   double *m_pDAdldata;

   // Array of temperature derivative of volumes Bi for the PVT table.
   // If a single temperature no entries since constant.  
   // Otherwise iTables*NC entries.
   // Linear interpolation is then used for each temperature
   double *m_pDBdtdata;

   // Array of temperature derivative of volume translation Ci
   // for the PVT table.  If a single temperature
   // no entries since constant unless the iLinearC flag is
   // on.  Otherwise iTables*NC entries
   // Linear interpolation is then used for each temperature
   double *m_pDCdtdata;

   
   double *m_pPCMod;///< Cached term for corresponding states viscosity model
   double *m_pTCMod;///< Cached term for corresponding states viscosity mode
};

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
   // Linear solver routines 
   //--------------------------------------------------------------------// 

   // Performs the back substitution on a factored matrix
   static void BackSolve( int iM, int iNc, double *pMatrix, double *pRhs );

   // Performs a modified Cholesky decomposition
   void Cholesky( int iM, int iNc, double *pMatrix );

   //--------------------------------------------------------------------// 
   // Methods to perform flash calculations
   //--------------------------------------------------------------------// 

   // Perform flash for multiple objects
   void FlashMultipleObjects( int iNc );

   // Perform flash for one object
   void FlashOneObject( int iNc );

   // Solve flash equations for mole fractions and splits
   void FlashEquations( int iM, int i1, int i2, int iNc, int iUpdate );

   // Perform on side of the Michelson stability analysis
   void OneSideStability( int iM, int iNc, double *pP, double *pT, double *pState, double *pFz );

   // Perform a Michelson stability analysis
   void Michelson( int iM, int iNc );

   // DOCUMENT ME!
   void FastInitialization( int iM, int iNc );

   // Successive substitutions for two phase K values
   void Substitution( int iM, int iNc );

   // Newton's method for two phase K values
   void NewtonFlash( int iM, int iNc, int iRestore, int iLevel );

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
