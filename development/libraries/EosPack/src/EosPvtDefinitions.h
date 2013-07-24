// Copyright 2011, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef EOS_PVT_DEFINITIONS_H
#define EOS_PVT_DEFINITIONS_H

#define USE_PARAM_EOSPVT( x ) ( (void)( x ) ) /*!< to supress warning about unused parameters */

/* Fortran offset */
#define EOS_FORTRAN 1            /*!< (0 || 1) Controls whether FORTRAN indicing used in indirection */

/* Define an option on or off */
#define EOS_OPTION_UNKNOWN  (-1)  /*!< Only used internally */
#define EOS_OPTION_OFF        0   /*!< Turn an option off.   */
#define EOS_OPTION_ON         1   /*!< Turn and option on.   */
#define EOS_OPTION_INITIALISE 2   /*!< Turn on solver initalisation. */

/* Debug printing */
#define EOS_DEBUG_PRINTING   1  /*!< (0 || 1) Controls whether some debug printing will be generated. */
#define EOS_DEBUG_INPUT      1  /*!< (0 || 1) Controls whether some debug output will be generated    */

/* Define constants for type of component */
#define EOS_HYDROCARBON      0  /*!< Indicator whether a component is a hydrocarbon */
#define EOS_AQUEOUS          1  /*!< Indicator whether a component is water or a salt */
#define EOS_NOTUSED        (-1) /*!< Indicator for a component which is not used in PVT model */
#define EOS_EXPLICIT       (-2) /*!< Indicator for a passive component which does not have any room in the derivative arrays */

/* Default values if phase not present */
#define EOS_DEFAULT_DENSITY   1000.000 /*!< Default density in flasher internal units if phase not present       */
#define EOS_DEFAULT_VISCOSITY 0.001    /*!< Default viscosity in flasher internal units if phase not present     */
#define EOS_DEFAULT_TENSION   0.000    /*!< Default parachor term in flasher internal units if phase not present */
#define EOS_JOSSI_MW_SPLIT    90.000   /*!< Molecular weight to divide heavy and light components                */

/* Spectrum for ideal K values */
#define EOS_KVALUE_SPECTRUM 5.0    /*!< Minimum factor of difference in ideal K values used to generate initial estimates */
                                   /*!< for the flasher. If the ideal K values generated are too close together the methods may not converge */

/// \brief Methods for labeling a single phase
enum
{
   EOS_SINGLE_PHASE_GAS     = 0, /*!< All single phase hydrocarbon mixtures will be considered gas. Not used if only one hydrocarbon phase present */
   EOS_SINGLE_PHASE_OIL     = 1, /*!< All single phase hydrocarbon mixtures will be considered oil. Not used if only one hydrocarbon phase present */
   EOS_SINGLE_PHASE_DEFAULT = 2, /*!< Single phase hydrocarbon mixtures will be assigned a phase name based upon the lumped A and B. 
                                      This is rigorous for a single component and will result in the same phase name being used
                                      as pressure varies, but can be in error in detecting whether the fluid has a bubble point or dew point.
                                      Not used if only one hydrocarbon phase present */
   EOS_SINGLE_PHASE_ZMINUSB = 3, /*!< Single phase hydrocarbon mixtures will be assigned a phase name based upon a user
                                      supplied value for the difference of Z and B. This is not rigorous for a single component and might
                                      result in different phase names being used as pressure varies, and can be in error in detecting
                                      whether the fluid has a bubble point or dew point. This is the Application default as of this time.
                                      Not used if only one hydrocarbon phase present */
   EOS_SINGLE_PHASE_AOVERB = 4, /*!< Indicates that single phase hydrocarbon mixtures will be assigned a phase name based upon a user
                                     supplied value for the quotient of the lumped A and B. This will result in the same phase name being 
                                     used as pressure varies and if properly specified by the user can be made consistent with whether the 
                                     fluid has a bubble point or dew point. Not used if only one hydrocarbon phase present */
   EOS_WATER               = 5  /*!< TODO Document value */
};

/// \brief Define type of expansion
enum
{
   EOS_EXPANSION      = 0,
   EOS_DIFFLIB        = 1,
   EOS_CONSTANTVOLUME = 2
};

/// \brief Method for labeling of water model
enum
{
   EOS_WATER_CORRELATIONS = 0, /*! Indicator that water correlations to be used */
   EOS_WATER_LINEAR       = 1  /*! Indicator that linear water model to be used */
};

/// \brief Define type of c model
enum 
{
   EOS_CONSTANT_C =  0, /*!< Indicator that constant c value is used */
   EOS_LINEAR_C   =  1, /*!< Indicator that linear c function to be used */
   EOS_ZERO_C     = -1  /*!< Indicator that linear c function to be used but zero data supplied */
};

/// \brief Define type of calculations
enum
{
   EOS_FLASH_CALCULATIONS    = 0, /*!< Indicates that the flasher will be used for flash */
   EOS_COMPOSITIONAL_GRADING = 1, /*!< Indicates that the flasher will be used for compositional grading calculations */
   EOS_SEPARATOR_FLASH       = 2  /*!< Indicates that the flasher will be used for separator calculations */
};

/// \brief Define derivative levels to be returned by the flasher
enum
{
   EOS_NODRV = 0, /*!< No derivatives will be returned by the flasher */
   EOS_DRV_N = 1, /*!< Only composition derivatives will be returned by the flasher */
   EOS_DRV_P = 2, /*!< Composition and pressure derivatives will be returned by the flasher */
   EOS_DRV_T = 3  /*!< Composition, pressure, and temperature derivatives will be returned by the flasher */
};


// Use generic memory management 
#include <stdlib.h>
#define CNEW( type, count ) (type *)malloc( ( count ) * sizeof( type ) )
#define CDELETE( pObject )  free( pObject )

#endif
