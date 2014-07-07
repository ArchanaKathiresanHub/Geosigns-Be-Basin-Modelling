// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_KRIGINGPROXY_H
#define SUMLIB_KRIGINGPROXY_H

#include "BaseTypes.h"
#include "CubicProxy.h"
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

class KrigingData;
class KrigingWeights;
class ParameterTransforms;

class KrigingProxy : public ISerializable
{
   public:
      KrigingProxy(KrigingData *kr = 0);

      /// Constructor.
      /// @param [in] *proxyModel         pointer to polynomial proxy model
      /// @param [in] parameterTransforms parameter transforms to be applied on the cubic proxy.
      /// @param [in] *kr                 pointer to Kriging data model
      /// @param [in] parSet              the prepared cases
      /// @param [in] caseValid           case validity indicator
      /// @param [in] target              the target values corresponding to the cases
      /// @param [in] nbOfOrdPars         number of prepared ordinal parameters
      KrigingProxy(
            CubicProxy *               proxyModel,
            const ParameterTransforms& parameterTransforms,
            KrigingData *              kr,
            ParameterSet const&        parSet,
            std::vector<bool> const&   caseValid,
            TargetSet const&           target,
            unsigned int               nbOfOrdPars );

      /// Initialises all internal data structures from parSet, target, proxyModel and kr.
      /// @param [in] *proxyModel         pointer to polynomial proxy model
      /// @param [in] parameterTransforms parameter transforms to be applied on the cubic proxy.
      /// @param [in] *kr                 pointer to Kriging data model
      /// @param [in] parSet              the prepared cases
      /// @param [in] caseValid           case validity indicator
      /// @param [in] target              the target values corresponding to the cases
      /// @param [in] nbOfOrdPars         number of prepared ordinal parameters
      void initialise(
            CubicProxy *               proxyModel,
            const ParameterTransforms& parameterTransforms,
            KrigingData *              kr,
            ParameterSet const&        parSet,
            std::vector<bool> const&   caseValid,
            TargetSet const&           target,
            unsigned int               nbOfOrdPars );

      /// @returns the number of parameter elements
      /// Implements Proxy
      unsigned int size() const;

      /// Interpolates between the predefined polynomial proxy errors that are
      /// given by the known target values minus the known polynomial proxy values
      /// corresponding to the parameter vectors in the fixed parameter set.
      /// @param [in] p           a parameter vector
      /// @param [in] krigingType Kriging type (default set by Proxy)
      /// @returns an estimation of the polynomial proxy error for parameter p
      /// Implements Proxy
      double getValue(
            Parameter const&           p,
            KrigingType                kriging = DefaultKriging ) const;

      /// As above but now with Kriging weights as input to speed up proxy calls.
      /// The parameter p has been used to determine the Kriging weights.
      /// @param [in] krigingWeights       Kriging weights datastructure
      /// @param [in] p                    the parameter vector that is no longer necessary here
      /// @param [in] krigingType          Kriging type (default set by Proxy)
      /// @returns an estimation of the polynomial proxy error for the parameter p from calcKrigingWeights
      /// Overrides Proxy default implementation
      double getValue(
            KrigingWeights const&      krigingWeights,
            Parameter const&           p,
            KrigingType                krigingType = DefaultKriging ) const;

      /// Calculates the Kriging weights as a function of a parameter vector.
      /// @param [in] p                    a parameter vector
      /// @param [in] krigingType          Kriging type (none, local or global)
      /// @param [out] krigingWeights      the kriging weights data structure
      /// Overrides Proxy default implementation
      void calcKrigingWeights(
            Parameter const&           p,
            KrigingType                kriging,
            KrigingWeights&            krigingWeights ) const;

      // made deliberately private so that calling load/save directly on this class is more difficult
      // the preferred way is to call save/load on the ISerializer.
   private:
      // ISerializable
      virtual bool load( IDeserializer*, unsigned int version );
      virtual bool save( ISerializer*, unsigned int version ) const;

   private:

      /// Calculates the interpolated error.
      /// @param [in] KrigingWeights
      /// @returns the interpolated error
      double calcKrigingError( KrigingWeights const& ) const;

      /// the parameter set is also needed for a function call (getValue)
      ParameterSet m_parSet;

      /// Contains all Kriging specific data like inverses of covariance matrices
      /// and correlation lengths, all a function of the scaled parameter set only.
      KrigingData *m_krigingData;

      /// predefined polynomial proxy errors
      std::vector<double> m_proxyError;

      /// the number of parameter elements
      unsigned int   m_parSize;
};

} // namespace SUMlib

#endif // SUMLIB_KRIGINGPROXY_H
