//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file ObsValueDoubleScalar.C

#include "Observable.h"
#include "ObsValueDoubleScalar.h"

#include <cassert>

namespace casa
{         
   double ObsValueDoubleScalar::MSE() const
   {
      double rmse = 0.0;

      if ( m_parent && m_parent->hasReferenceValue() )
      {
         ObsValueDoubleScalar * refVal = dynamic_cast<ObsValueDoubleScalar*>( m_parent->referenceValue() );
 
         if ( refVal )
         {
            const std::vector<double> & rv = refVal->doubleValue();
            rmse = ( rv[0] - value() ) / ( m_parent->stdDeviationForRefValue() > 0.0 ? m_parent->stdDeviationForRefValue() : (0.1 * rv[0]) );
         }
      }
      return rmse * rmse;
   }
}
