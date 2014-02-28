// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_RANDOMGENERATOR_H
#define SUMLIB_RANDOMGENERATOR_H

#include "SUMlib.h"

namespace SUMlib {

/**
*  @brief
*  Portable random number generator class from Knuth. Used
*  to generate uniform and normal distributed random numbers.
*/

class INTERFACE_SUMLIB RandomGenerator
{
   int m_ma[55];
   int m_inext;
   int m_inextp;

   /**
   *  Flag indicating if a normal random number is in cache.
   */
   int m_iset;

   /**
   * The next random normal distributed random number when cached.
   */
   double m_gset;

public:
   /**
   *  Create generator with optional seed value.
   */
   explicit RandomGenerator( int iseed );

   // Not virtual!
   ~RandomGenerator();

   /**
   *  (Re)initialise the random generator with the given seed.
   *  @param[in] seed The random generator seed value, determining
   *     the sequence of random numbers generated.
   */
   void initialise( int seed );

   /**
   *  Generate uniform random number in [0,1) range.
   *  @result A uniform [0,1) psuedo random number.
   */
   double uniformRandom();

   /**
   *  Generate normal random number with mean 0 and stddev 1
   *  using the transformation method.
   *  @result A normal distributed N(0,1) pseudo random number.
   */
   double normalRandom();
};

} // namespace SUMlib

#endif // SUMLIB_RANDOMGENERATOR_H
