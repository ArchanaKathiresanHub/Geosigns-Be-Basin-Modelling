// Copyright 2011, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include <math.h>

#include "EosUtils.h"

/* Use intel math library */
// #define EOS_INTEL
#ifdef EOS_INTEL
#include "C:/Program Files/Intel/MKL61/include/mkl.h"
#endif

///
/// \brief Function for vector exponentiation
///
void EosUtils::VectorExp( int iM, double *pInput, double *pOutput )
{
#ifdef EOS_INTEL
   vdExp( iM, pInput, pOutput );
#else
   for ( int i = 0; i < iM; i++ )
   {
      pOutput[i] = exp( pInput[i] );
   }
#endif
}

///
/// \brief Function for vector log
///
void EosUtils::VectorLog( int iM, double *pInput, double *pOutput )
{
#ifdef EOS_INTEL
   vdLn( iM, pInput, pOutput );
#else
   for ( int i = 0; i < iM; i++ )
   {
      pOutput[i] = log( pInput[i] );
   }
#endif
}

///
/// \brief Function for vector power
///
void EosUtils::VectorPow( int iM, double *pInput1, double dInput2, double *pOutput )
{
#ifdef EOS_INTEL
   vdPowx( iM, pInput1, dInput2, pOutput );
#else
   for ( int i = 0; i < iM; ++i )
   {
      pOutput[i] = pow( pInput1[i], dInput2 );
   }
#endif
}

///
/// \brief Function for vector power
///
/// \param[in]  iM (constant) number of objects
/// \param[in] pInput1 (constant) variable to take power of
/// \param[in] pInput2 (constant) exponent
/// \param[out] pOutput   result
///
void EosUtils::VectorPowX( int iM, double *pInput1, double *pInput2, double *pOutput )
{
#ifdef EOS_INTEL
   vdPow( iM, pInput1, pInput2, pOutput );
#else
   for ( int i = 0; i < iM; ++i )
   {
      pOutput[i] = pow( pInput1[i], pInput2[i] );
   }
#endif
}

