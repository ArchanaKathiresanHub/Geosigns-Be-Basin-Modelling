// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_PROBDISTR_H
#define SUMLIB_PROBDISTR_H

#include "BaseTypes.h"
#include "SUMlib.h"

namespace SUMlib {

class INTERFACE_SUMLIB ProbDistr
{
   public:

      static const double MinInf; /* -1e300 */
      static const double CloseToZero; /* 1e-200 */

      /// Calculate the log of the prior probability for the specified parameter
      /// @param [in] p parameter to calculate the log of the prior probability for
      /// @returns the log of the prior probability
      virtual double calcLogPriorProb( Parameter const& p ) const = 0;

      /// Calculate the log of the prior probability for all parameters in the parameterset
      /// @param [in] parSet parameter set to calculate the log of the prior probability for
      /// @param [out] priorProb the log of the prior probability for each parameter
      void calcLogPriorProb( ParameterSet const& parSet, RealVector &priorProb ) const;

      /// Calculate the log of the prior interpolated weight for all discrete parameters
      /// @param [in] rel_p   relative value (between 0 and 1) of ordinal parameter p
      /// @param [in] weights the prior weights associated with ordinal parameter p
      /// @returns the log of the prior interpolated weight
      double calcLogWeight( double rel_p, RealVector const& weights ) const;
};

} // namespace SUMlib
#endif // SUMLIB_PROBDISTR_H
