// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_KRIGINGWEIGHTS_H
#define SUMLIB_KRIGINGWEIGHTS_H

#include "BaseTypes.h"

namespace SUMlib {

class KrigingData;

/// Datastructure to capture the kriging weights calculation results
class KrigingWeights
{
   public:
      KrigingWeights();
      // default, non-virtual destructor: this is not a base class

      void               calcLocalWeights( ParameterSet const&, KrigingData const&, Parameter const& );
      void               calcGlobalWeights( ParameterSet const&, KrigingData const&, Parameter const& p );
      void               zeroWeights( unsigned int );

      double             sumOfWeights() const { return m_sumOfWeights; }
      RealVector const&  weights() const { return m_weights; }
      IndexList const&   indexes() const { return m_indexes; }

   private:
      void               sumPositiveWeights();
      static void        economicMatrixVectorProduct( RealMatrix const& M, RealVector const& v, IndexList const& i_v, RealVector& w );

      RealVector  m_weights;
      IndexList   m_indexes;
      double      m_sumOfWeights;
};

} // namespace SUMlib

#endif // SUMLIB_KRIGINGWEIGHTS_H
