// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_COMPOUNDPROXY_H
#define SUMLIB_COMPOUNDPROXY_H

#include <memory>
#include <vector>

#include "BaseTypes.h"
#include "ParameterTransforms.h"
#include "Proxy.h"
#include "SUMlib.h"

namespace SUMlib
{
class CubicProxy;
class EliminationCriterion;
class KrigingData;
class KrigingProxy;
class KrigingWeights;

/// @class CompoundProxy represents a proxy model for parameters assumed to be
/// scaled to [-1:1] with a cubic polynomial model and kriging.
///
/// The polynomial proxy model is determined by ProxyEstimator::autoEstimate.
/// Optionally kriging may be applied.
class INTERFACE_SUMLIB CompoundProxy : public Proxy, public ISerializationVersion
{
   public:
      /// Specifies which of the cases are valid and for the valid cases specifies the target value.
      /// @param [in] caseValid      A vector indicating for each case if it is valid or not.
      /// @param [in] targets        For each valid case, specifies the target value.
      ///
      /// Requirement: The size of caseValid must be equal to the number of cases in the ParameterSet.
      /// Requirement: The size of targets has to be equal to the number of "true" values in caseValid
      void setValidityAndTragets( std::vector<bool> const& caseValid, TargetSet const& targets );

      /// Defines the default polynomial order to determine the initial list of monomials (vars).
      /// This list is input for ProxyEstimator::autoEstimate().
      // set to 0 (intercept only)
      static const unsigned int defaultOrder = 0;

      /// Default constructor
      CompoundProxy(
            KrigingData *krigingData,
            ParameterSet const& parSet
           );

      /// Destructor
      virtual ~CompoundProxy();

      /// Constructor. Initialises cubic proxy and Kriging proxy based on the specified
      /// parameter set.
      ///
      /// The parameter elements are assumed to be scaled to [-1:1].
      /// @param [in]  parSet         Case set
      /// @param [in]  caseValid      Case validity indicator
      /// @param [in]  krigingData    Kriging data
      /// @param [in]  targetSet      Target to calculate proxy for
      /// @param [in]  nbOfOrdPars    Number of prepared ordinal parameters
      /// @param [in]  order          Order of polynomial (0, 1, 2, or 9); 9 is conventional: linear + pure quadratic.
      /// @param [in]  modelSearch    Cubic polynomial model search
      /// @param [in]  targetR2       adjusted R^2 value above which we accept the model
      /// @param [in]  confLevel      needed for significance test of model increments
      /// @param [in]  partition      Parameters to include in initial polynomial
      /// @param [in]  parTransforms  Transforms for parameters (@see ParameterTransforms). Zero pointer is not accepted.
      ///                             Instantiate a default, trivial, transform instead.
      /// @param [in]  criterion      used to eliminate statistically insignificant coefficients
      CompoundProxy(
            ParameterSet const&        parSet,
            std::vector<bool> const&   caseValid,
            KrigingData *              krigingData,
            TargetSet const&           targetSet,
            unsigned int               nbOfOrdPars,
            unsigned int               order,
            bool                       modelSearch,
            double                     targetR2,
            double                     confLevel,
            Partition const&           partition,
            ParameterTransforms::ptr&  parTransforms,
            EliminationCriterion&      criterion
           );

      /// Constructor. Initialises cubic proxy and Kriging proxy based on the specified
      /// parameter set.
      ///
      /// The parameter elements are assumed to be scaled to [-1:1].
      /// @param [in]  parSet         Case set
      /// @param [in]  caseValid      Case validity indicator
      /// @param [in]  krigingData    Kriging data
      /// @param [in]  targetSet      Target to calculate proxy for
      /// @param [in]  nbOfOrdPars    Number of prepared ordinal parameters
      /// @param [in]  order          Order of polynomial (0, 1, 2, or 9); 9 is conventional: linear + pure quadratic.
      /// @param [in]  modelSearch    Cubic polynomial model search
      /// @param [in]  targetR2       adjusted R^2 value above which we accept the model
      /// @param [in]  confLevel      needed for significance test of model increments
      /// @param [in]  partition      Parameters to include in initial polynomial
      /// @param [in]  parTransforms  Transforms for parameters (@see ParameterTransforms). Zero pointer is not accepted.
      ///                             Instantiate a default, trivial, transform instead.
      CompoundProxy(
            ParameterSet const&        parSet,
            std::vector<bool> const&   caseValid,
            KrigingData *              krigingData,
            TargetSet const&           targetSet,
            unsigned int               nbOfOrdPars,
            unsigned int               order,
            bool                       modelSearch,
            double                     targetR2,
            double                     confLevel,
            Partition const&           partition,
            ParameterTransforms::ptr&  parTransforms
           );

      /// Initialiser for the CompoundProxy.
      /// @param [in]  parSet         Case set
      /// @param [in]  caseValid      Case validity indicator
      /// @param [in]  krigingData    Kriging data
      /// @param [in]  targetSet      Target to calculate proxy for
      /// @param [in]  nbOfOrdPars    Number of prepared ordinal parameters
      /// @param [in]  order          Order of polynomial (0, 1, 2, or 9); 9 is conventional: linear + pure quadratic.
      /// @param [in]  modelSearch    Cubic polynomial model search
      /// @param [in]  targetR2       adjusted R^2 value above which we accept the model
      /// @param [in]  confLevel      needed for significance test of model increments
      /// @param [in]  partition      Parameters to include in initial polynomial
      /// @param [in]  parTransforms  Transforms for parameters (@see ParameterTransforms). Zero pointer is not accepted.
      ///                             Instantiate a default, trivial, transform instead.
      /// @param [in]  criterion      used to eliminate statistically insignificant coefficients
      void initialise(
            ParameterSet const&        parSet,
            std::vector<bool> const&   caseValid,
            KrigingData *              krigingData,
            TargetSet const&           targetSet,
            unsigned int               nbOfOrdPars,
            unsigned int               order,
            bool                       modelSearch,
            double                     targetR2,
            double                     confLevel,
            Partition const&           partition,
            ParameterTransforms::ptr&  parTransforms,
            EliminationCriterion&      criterion
            );

      /// Initialiser for the CompoundProxy.
      /// @param [in]  parSet         Case set
      /// @param [in]  caseValid      Case validity indicator
      /// @param [in]  krigingData    Kriging data
      /// @param [in]  targetSet      Target to calculate proxy for
      /// @param [in]  nbOfOrdPars    Number of prepared ordinal parameters
      /// @param [in]  order          Order of polynomial (0, 1, 2, or 9); 9 is conventional: linear + pure quadratic.
      /// @param [in]  modelSearch    Cubic polynomial model search
      /// @param [in]  targetR2       adjusted R^2 value above which we accept the model
      /// @param [in]  confLevel      needed for significance test of model increments
      /// @param [in]  partition      Parameters to include in initial polynomial
      /// @param [in]  parTransforms  Transforms for parameters (@see ParameterTransforms). Zero pointer is not accepted.
      ///                             Instantiate a default, trivial, transform instead.
      void initialise(
            ParameterSet const&        parSet,
            std::vector<bool> const&   caseValid,
            KrigingData *              krigingData,
            TargetSet const&           targetSet,
            unsigned int               nbOfOrdPars,
            unsigned int               order,
            bool                       modelSearch,
            double                     targetR2,
            double                     confLevel,
            Partition const&           partition,
            ParameterTransforms::ptr&  parTransforms
            );

      void update( EliminationCriterion& criterion );

      /// Getter for the number of parameters that this proxy expects.
      ///
      /// Implements the Proxy interface
      virtual unsigned int size( void ) const;

      /// Calculates a proxy response value for a parameter
      ///
      /// Assumes parameter p to be scaled between [-1:1] and then does
      /// CubicProxy and/or KrigingProxy function calls to calculate the response
      ///
      /// Implements the Proxy interface
      /// @param [in] p scaled parameter
      /// @param [in] krigingType the kriging type to use. Default set by Proxy
      /// @returns the proxy value
      virtual double getProxyValue(
            Parameter const&           p,
            KrigingType                krigingType = DefaultKriging ) const;

      /// As above but now with additional arguments to speed up proxy calls
      /// @param [in] krigingWeights     Kriging weights datastructure
      /// @param [in] p                  scaled parameter
      /// @param [in] krigingType the kriging type to use. Default set by Proxy
      /// @returns the proxy value
      virtual double getProxyValue(
            KrigingWeights const&      krigingWeights,
            Parameter const&           p,
            KrigingType                krigingType = DefaultKriging ) const;

      /// Calculates the Kriging weights as a function of a parameter vector
      /// @param [in] p                 a parameter vector
      /// @param [in] krigingType       the kriging type to use.
      /// @param [out] krigingWeights   Kriging weights data structure
      /// @returns the sum of weights
      virtual void calcKrigingWeights(
            Parameter const&           p,
            KrigingType                krigingType,
            KrigingWeights&            krigingWeights ) const;

      /// Get the map of cubic proxy monomials and coefficients and standard errors.
      /// @param [out]  the coefficients map
      void getCoefficientsMap( CubicProxy::CoefficientsMap & ) const;

      /// Returns the adjusted R^2 of the CubicProxy
      double adjustedR2() const { return m_adjustedR2; }

      /// Returns the rank of the design matrix.
      unsigned int getDesignMatrixRank() const;

      /// Returns true iff the regression is ill-posed.
      bool isRegressionIllPosed() const;

      /// Returns the leverage scores (i.e. diagonal of hat matrix)
      std::vector<double> const& leverages() const { return m_leverages; }

      // made deliberately private so that calling load/save directly on this class is more difficult
      // the preferred way is to call save/load on the ISerializer.
   private:
      // ISerializable
      virtual bool load( IDeserializer*, unsigned int version );
      virtual bool save( ISerializer*, unsigned int version ) const;
      // ISerializationVersion
      // when in the future implementing this interface also in base class
      // add the version of this class to the version of the base class:  Proxy::getSerializationVersion() + g_version
      // this way if the base changes all subclasses will get an increase of version
      virtual unsigned int getSerializationVersion() const;

   private: // noncopyable
      CompoundProxy( const CompoundProxy& );
      CompoundProxy& operator=( const CompoundProxy& );

   private: // methods

      /// A pair of polynomial and kriging proxies
      typedef std::pair< std::unique_ptr<CubicProxy>,std::unique_ptr<KrigingProxy> > ProxyPair;

      /// Compute the proxies associated with a parameterSet and TargetSet
      /// @param [in]  parSet       scaled parameter set
      /// @param [in]  caseValid    case validity indicator
      /// @param [in]  targetSet    unscaled target set
      /// @param [in]  krigingData  pointer to a single Kriging data model
      /// @param [in]  nbOfOrdPars  number of prepared ordinal parameters
      /// @param [in]  modelSearch  cubic polynomial model search
      /// @param [in]  targetR2     adjusted R^2 value above which we accept the model
      /// @param [in]  confLevel    needed for significance test of model increments
      /// @param [in]  initVars     monomials/variables of the initial polynomial
      /// @param [in]  order        Order of polynomial (0, 1, or 2)
      /// @param [in]  criterion    used to eliminate statistically insignificant coefficients
      void calculateProxyPair(
            ParameterSet const&        parSet,
            std::vector<bool> const&   caseValid,
            TargetSet const&           targetSet,
            KrigingData *              krigingData,
            unsigned int               nbOfOrdPars,
            bool                       modelSearch,
            double                     targetR2,
            double                     confLevel,
            IndexList const&           initVars,
            unsigned int               order,
            EliminationCriterion&      criterion );

      void calculateProxyPair(
            ParameterSet const&        parSet,
            std::vector<bool> const&   caseValid,
            TargetSet const&           targetSet,
            KrigingData *              krigingData,
            unsigned int               nbOfOrdPars,
            bool                       modelSearch,
            double                     targetR2,
            double                     confLevel,
            IndexList const&           initVars,
            unsigned int               order );

      /// Inline method to access the cubic proxy
      CubicProxy const * cubicProxy() const { return m_proxyPair.first.get(); }

      /// Inline method to access the kriging proxy
      KrigingProxy const * krigingProxy() const { return m_proxyPair.second.get(); }

   private: // data

      /// Contains all Kriging specific data like inverses of covariance matrices
      /// and correlation lengths, all a function of the scaled parameter set only.
      KrigingData *m_krigingData;

      /// Cubic + Kriging proxy
      ProxyPair         m_proxyPair;

      /// Adjusted R^2 of the CubicProxy
      double            m_adjustedR2;

      ParameterTransforms::ptr m_parTransforms;

      // Leverage scores (i.e. diagonal of hat matrix)
      std::vector<double> m_leverages;

      /// case validity indicator
      std::vector<bool> m_caseValid;

      TargetSet         m_targetSet;

      ParameterSet const* m_parSet;
};

} // namespace SUMlib

#endif // SUMLIB_COMPOUNDPROXY_H
