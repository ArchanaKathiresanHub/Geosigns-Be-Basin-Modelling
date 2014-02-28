// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_KRIGINGPROXY_H
#define SUMLIB_KRIGINGPROXY_H

#include "BaseTypes.h"
#include "Proxy.h"
#include "SUMlib.h"

#include "ISerializer.h"

namespace SUMlib {

class KrigingData;
class KrigingWeights;

class INTERFACE_SUMLIB KrigingProxy : public ISerializable
{
   public:
      KrigingProxy(KrigingData *kr = 0);

      /// Constructor.
      /// @param [in] *proxyModel   pointer to polynomial proxy model
      /// @param [in] *kr           pointer to Kriging data model
      /// @param [in] parSet        the prepared cases
      /// @param [in] caseValid     case validity indicator
      /// @param [in] target        the target values corresponding to the cases
      /// @param [in] nbOfOrdPars   number of prepared ordinal parameters
      KrigingProxy(
            Proxy *                    proxyModel,
            KrigingData *              kr,
            ParameterSet const&        parSet,
            std::vector<bool> const&   caseValid,
            TargetSet const&           target,
            unsigned int               nbOfOrdPars );

      /// Initialises all internal data structures from parSet, target, proxyModel and kr.
      /// @param [in] *proxyModel   pointer to polynomial proxy model
      /// @param [in] *kr           pointer to Kriging data model
      /// @param [in] parSet        the scaled cases
      /// @param [in] caseValid     case validity indicator
      /// @param [in] target        the target values corresponding to the cases
      /// @param [in] nbOfOrdPars   number of prepared ordinal parameters
      void initialise(
            Proxy *                    proxyModel,
            KrigingData *              kr,
            ParameterSet const&        parSet,
            std::vector<bool> const&   caseValid,
            TargetSet const&           target,
            unsigned int               nbOfOrdPars );

      /// @returns the number of parameter elements
      /// Implements Proxy
      virtual unsigned int size() const;

      /// Interpolates between the predefined polynomial proxy errors that are
      /// given by the known target values minus the known polynomial proxy values
      /// corresponding to the parameter vectors in the fixed parameter set.
      /// @param [in] p           a parameter vector
      /// @param [in] krigingType Kriging type (default set by Proxy)
      /// @returns an estimation of the polynomial proxy error for parameter p
      /// Implements Proxy
      virtual double getProxyValue(
            Parameter const&           p,
            KrigingType                kriging = DefaultKriging ) const;

      /// As above but now with Kriging weights as input to speed up proxy calls.
      /// The parameter p has been used to determine the Kriging weights.
      /// @param [in] krigingWeights       Kriging weights datastructure
      /// @param [in] p                    the parameter vector that is no longer necessary here
      /// @param [in] krigingType          Kriging type (default set by Proxy)
      /// @returns an estimation of the polynomial proxy error for the parameter p from calcKrigingWeights
      /// Overrides Proxy default implementation
      virtual double getProxyValue(
            KrigingWeights const&      krigingWeights,
            Parameter const&           p,
            KrigingType                krigingType = DefaultKriging ) const;

      /// Calculates the Kriging weights as a function of a parameter vector.
      /// @param [in] p                    a parameter vector
      /// @param [in] krigingType          Kriging type (none, local or global)
      /// @param [out] krigingWeights      the kriging weights data structure
      /// Overrides Proxy default implementation
      virtual void calcKrigingWeights(
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

      /// the parameter set is also needed for a function call (getProxyValue)
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
