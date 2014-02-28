// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_PROXY_H
#define SUMLIB_PROXY_H

#include <vector>

#include "BaseTypes.h"
#include "SUMlib.h"

#include "ISerializer.h"

namespace SUMlib {

/// @typedef KrigingType   defines the Kriging type to use
///
/// Use GlobalKriging for highest accuracy
/// LocalKriging (fast) compromises between GlobalKriging and None
/// Recommended: GlobalKriging
typedef enum {
   NoKriging,
   LocalKriging,
   GlobalKriging
} KrigingType;

static const KrigingType DefaultKriging = GlobalKriging;

class KrigingWeights;

class INTERFACE_SUMLIB Proxy : public ISerializable
{
   public:

      virtual ~Proxy()
      {}

      /// Calculate singular value decomposition (SVD) of original matrix a
      /// The SVD calculation changes matrix a but not its dimensions: m rows and n columns
      /// Requirement: m >= n
      /// @param [in,out] a   matrix of proxy model monomials (in) -> orthonormal matrix (out)
      /// @param [out] w      n x 1 vector of singular values
      /// @param [out] v      n x n orthonormal matrix
      /// @returns the status of the SVD operation (0 = success)
      static int calculateSVD( std::vector<std::vector<double> >& a, std::vector<double>& w,
      std::vector<std::vector<double> >& v );

      /// Calculate coefficients for a proxy model based on SVD data (a, w, and v) and target values
      /// @param [in] stat    integer status of SVD (0 = success)
      /// @param [in] a       m x n orthonormal matrix
      /// @param [in] w       n x 1 vector of singular values
      /// @param [in] v       n x n orthonormal matrix
      /// @param [in] b       m x 1 vector of target values
      /// @param [out] coef   n x 1 vector of coefficients corresponding to the model monomials
      static void calculateCoefficients( int stat, std::vector<std::vector<double> > const& a, std::vector<double> const& w, std::vector<std::vector<double> > const& v, std::vector<double> const& b, std::vector<double> & coef );

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
