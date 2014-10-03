//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file ObsValueDoubleArray.C

#include "Observable.h"
#include "ObsValueDoubleArray.h"

#include <cassert>

namespace casa
{
   double ObsValueDoubleArray::MSE() const
   {
      double rmse = 0.0;

      if ( m_value.size() && m_parent && m_parent->hasReferenceValue() )
      {
         const ObsValueDoubleArray * refVal = dynamic_cast<const ObsValueDoubleArray*>( m_parent->referenceValue() );
   
         if ( refVal )
         {
            const std::vector<double> & rv = refVal->doubleValue();
            assert( rv.size() == m_value.size() );

            for ( size_t i = 0; i < rv.size(); ++i )
            {
               double sigma = ( m_parent->stdDeviationForRefValue() > 0.0 ? m_parent->stdDeviationForRefValue() : (0.1 * rv[i]) );

               double v = ( rv[i] - m_value[i] ) / sigma;
               rmse += v * v;
            }
            rmse /= rv.size();
         }
      }
      return rmse;
   }

}

