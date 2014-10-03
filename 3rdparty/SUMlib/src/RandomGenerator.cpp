// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include <cassert>
#include <cmath>
#include <cstdlib>

#include "RandomGenerator.h"

namespace SUMlib {

   static const int RG_MZ = 0;
   static const int RG_MBIG = 1000000000;
   static const int RG_MSEED = 161803398;
   static const double RG_FAC = 1.0 / RG_MBIG;

/**
 *  The portable random number generator (Knuth).
 *  =============================================
 *
 */

RandomGenerator::RandomGenerator( int iseed )
   : m_inext(0),
     m_inextp(0),
     m_iset(0),
     m_gset(0)
{
   initialise( iseed );
}

RandomGenerator::~RandomGenerator()
{
}


/**
 *  (Re)initialize the random number generator with a specified
 *  seed.
 */

void RandomGenerator::initialise( int idum )
{
   // Use seed idum
   int mj = abs(RG_MSEED - abs(idum));
   mj = mj % RG_MBIG;
   m_ma[55] = mj;
   int mk = 1;

   // Initialise the rest of the table
   for ( int i = 1; i <= 54; ++i )
   {
      // Slight random order index
      int ii = (21 * i) % 55;

      // Not really random number
      m_ma[ii] = mk;
      mk = mj-mk;
      if (mk < RG_MZ) mk += RG_MBIG;
      mj = m_ma[ii];
   }

   // Warm up the generator by randomizing
   for ( int k=1; k <= 4; k++ )
   {
      for (int i = 1; i <= 55; ++i )
      {
         m_ma[i] -= m_ma[ 1 + (i+30) % 55 ];
         if ( m_ma[i] < RG_MZ ) m_ma[i] += RG_MBIG;
      }
   }

   // Indices for first generated number
   m_inext = 0;
   m_inextp = 31;
}

/**
 *  Generate uniform random number in [0,1) range.
 */

double RandomGenerator::uniformRandom()
{
   m_inext = m_inext + 1;
   if ( m_inext == 56 ) m_inext = 1;
   m_inextp = m_inextp + 1;
   if ( m_inextp == 56 ) m_inextp = 1;

   int mj = m_ma[m_inext] - m_ma[m_inextp];
   if ( mj < RG_MZ ) mj = mj + RG_MBIG;
   m_ma[m_inext] = mj;

   double r = mj * RG_FAC;
   assert( 0 <= r && r < 1 );
   return r;
}

/**
 *  Generate a normal random number with mean 0 and standard
 *  deviation 1.
 */

double RandomGenerator::normalRandom()
{
   if ( m_iset == 0 )
   {
      // No saved normal present, generate a new pair
      double r, v1, v2;
      do
      {
         // Two uniform random number in [-1,1]
         v1 = 2 * uniformRandom() - 1;
         v2 = 2 * uniformRandom() - 1;

         // For checking if within unit circle
         r = v1*v1+v2*v2;
      } while ( r == 0 || r >= 1 );

      double fac = sqrt( -2 * log(r)/r );

      // save gset for next call
      m_gset = v1 * fac;
      m_iset = 1;
      return v2 * fac;
   }

   // Generate new normal next call
   m_iset = 0;

   // Return saved normal
   return m_gset;
}

} // namespace SUMlib
