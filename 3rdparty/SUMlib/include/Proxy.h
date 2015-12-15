// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_PROXY_H
#define SUMLIB_PROXY_H

#include <vector>

#include "BaseTypes.h"
#include "KrigingProxy.h"
#include "SUMlib.h"

#include "ISerializer.h"

namespace SUMlib {

class KrigingWeights;

class INTERFACE_SUMLIB Proxy : public ISerializable
{
   public:

      virtual ~Proxy()
      {}

      /// Get the allowed size of parameter vectors
      /// @returns the size of parameter vectors for which this proxy model is calculated
      virtual unsigned int size() const = 0;

      /// Get the proxy model value for the specified parameter
      /// @param [in] parameter           the scaled parameter to get the model value for
      /// @param [in] krigingType         kriging type to calculate weights for
      /// @returns the calculated model value
      virtual double getProxyValue(
            Parameter const&        parameter,
            KrigingType             krigingType = DefaultKriging ) const = 0;

      /// As above but now with additional arguments to speed up proxy calls
      ///
      /// The default implementation ignores the kriging weights.
      /// @param [in] krigingWeights      kriging weights datastructure
      /// @param [in] p                   scaled parameter
      /// @param [in] krigingType         kriging type to calculate weights for
      /// @returns the proxy value
      virtual double getProxyValue(
            KrigingWeights const&   krigingWeights,
            Parameter const&        p,
            KrigingType             krigingType = DefaultKriging ) const;

      /// Convenience function. Calls getProxyValue for all Parameters in the Parameterset.
      /// @param [in]  ParameterSet       the parameters to get the model value for
      /// @param [out] TargetSet the      response values for all parameters
      void getProxyValues( ParameterSet const& parameter, TargetSet &targetSet ) const;

      /// Calculates Kriging weights (and indices of the relevant ones) for a parameter p and the sum of weights.
      ///
      /// The default implementation sets the sum of weights to zero
      /// @param [in]  p                  a parameter vector
      /// @param [in]  krigingType        kriging type to calculate weights for
      /// @param [out] krigingWeights     kriging weights datastructure
      virtual void calcKrigingWeights(
            Parameter const&        p,
            KrigingType             krigingType,
            KrigingWeights&         krigingWeights ) const;

}; // class Proxy

} // namespace SUMlib

#endif // SUMLIB_PROXY_H
