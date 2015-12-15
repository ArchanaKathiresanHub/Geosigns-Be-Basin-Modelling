// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_COMPOUNDPROXYCOLLECTION_H
#define SUMLIB_COMPOUNDPROXYCOLLECTION_H

#include "BaseTypes.h"
#include "CompoundProxy.h"
#include "Case.h"
#include "ParameterSpace.h"
#include "ParameterTransforms.h"

namespace SUMlib {

class KrigingData;


/// A container for the CompoundProxy instances calculated for a
/// TargetCollection and a case set.
///
/// A raw case set is assumed and frozen parameters are allowed.
/// During construction the case set is prepared,
/// after which the kriging data is calculated.
///
/// The calculate method can then be used to calculate the proxies for the
/// specified collection of target sets, with optional settings for the
/// proxy calculation.
///
/// The function calculate(...) receives a few arguments (settings) that control the
/// polynomial proxy building:
/// order of polynomial (0, 1, 2, or 9); 9 is conventional: linear + pure quadratic.
/// modelSearch (bool) to search for more optimal polynomial proxies that honor 'order'.
/// targetR2 is the adjusted R^2 value between 0 and 1 above which we accept the model.
/// confLevel ranging between 0.5 (50% confidence level) and 0.999 (99.9% confidence level),
/// 0.5 has no effect whereas higher values gradually result in more significant monomials.

/// The combination modelSearch = true and order = 0 does not make sense (no search needed).
/// This combination (default) is therefore used to search for terms up to 3rd order.

/// So-called binary dummy parameters indicate which categorical value (if any) applies.
/// Dummy parameters can be part of a polynomial term but they don't increase its order.
/// For example, dummy_par * x * y is a second order term that applies to a categorical state.
class INTERFACE_SUMLIB CompoundProxyCollection : public ISerializable
{
   public:
      void setValidityAndTargets( std::vector< std::pair< std::vector<bool>, std::vector<double> > > validityAndTargets );

      typedef std::vector<CompoundProxy *> CompoundProxyList;

      /// Default constructor.
      CompoundProxyCollection();

      /// Constructor.
      /// The case set is prepared, frozen parameters are removed.
      /// @param [in] caseSet       case set
      /// @param [in] origLow       original lower bounds as specified by the user
      /// @param [in] origHigh      original upper bounds as specified by the user
      /// @param [in] tr            set of parameter transformations
      CompoundProxyCollection( std::vector<Case> const& caseSet,
         Case const& origLow, Case const& origHigh,
         ParameterSpace::TransformationSet const& tr = ParameterSpace::TransformationSet() );

      /// Destructor.
      virtual ~CompoundProxyCollection();

      /// Calculate the proxies for a collection of specified targets with optional
      /// proxy calculation settings.
      /// @param [in] targets          collection of targets, of same size as parameter set
      /// @param [in] case2Obs2Valid   indicates (in)valid "case(row)-obs(column)" combinations
      /// @param [in] order            maximum order of monomials to include in the proxy
      /// @param [in] modelSearch      flag indicating whether to search for optimal proxies
      /// @param [in] targetR2         adjusted R^2 value above which we accept the model
      /// @param [in] confLevel        needed for significance test of model increments
      /// @param [in] partition        list of flags, one for each parameter, to indicate
      ///                              whether the element should be included.
      /// @param [in] criterion        used to eliminate statistically insignificant coefficients
      /// @param [in] parTransformsDef definition of observable dependent parameter transforms.
      ///                              first index is observable index, second index is the SUMlib
      ///                              parameter index. Vector can be empty, in that case no
      ///                              transforms will be applied. Transforms are only applied on the
      ///                              continuous, unfrozen, scalar parameters. @see SUMlib::ParameterTransforms.
      void calculate(
            TargetCollection const&                              targets,
            std::vector<std::vector<bool> > const&               case2Obs2Valid,
            unsigned int                                         order,
            bool                                                 modelSearch,
            double                                               targetR2,
            double                                               confLevel,
            Partition const&                                     partition,
            EliminationCriterion&                                criterion,
            const std::vector< ParameterTransformTypeVector >&   parTransformsDef = std::vector< ParameterTransformTypeVector >()
            );

      /// Calculate the proxies for a collection of specified targets with optional
      /// proxy calculation settings.
      /// @param [in] targets          collection of targets, of same size as parameter set
      /// @param [in] case2Obs2Valid   indicates (in)valid "case(row)-obs(column)" combinations
      /// @param [in] order            maximum order of monomials to include in the proxy
      /// @param [in] modelSearch      flag indicating whether to search for optimal proxies
      /// @param [in] targetR2         adjusted R^2 value above which we accept the model
      /// @param [in] confLevel        needed for significance test of model increments
      /// @param [in] partition        list of flags, one for each parameter, to indicate
      ///                              whether the element should be included.
      /// @param [in] parTransformsDef definition of observable dependent parameter transforms.
      ///                              first index is observable index, second index is the SUMlib
      ///                              parameter index. Vector can be empty, in that case no
      ///                              transforms will be applied. Transforms are only applied on the
      ///                              continuous, unfrozen, scalar parameters. @see SUMlib::ParameterTransforms.
      void calculate(
            TargetCollection const&                              targets,
            std::vector<std::vector<bool> > const&               case2Obs2Valid,
            unsigned int                                         order,
            bool                                                 modelSearch,
            double                                               targetR2,
            double                                               confLevel,
            Partition const&                                     partition,
            const std::vector< ParameterTransformTypeVector >&   parTransformsDef = std::vector< ParameterTransformTypeVector >()
            );

      void recalculate( EliminationCriterion& criterion );

      /// Getter for the list of proxies
      /// @returns a list of pointers to the calculated proxies
      CompoundProxyList const&   getProxyList() const;

      /// Getter for the kriging data
      /// @returns a kriging data object
      KrigingData const&  getKrigingData() const;

      ParameterSet const& getPreparedCaseSet() const;

      /// Getter for the ParameterSpace calculated from the ParameterSet
      /// @returns the parameter space
      ParameterSpace const&      getParameterSpace() const;

      /// Proxy function call for a specific proxy in the collection
      /// @param [in] index         the index number of the proxy in the collection
      /// @param [in] c             the case for which the proxy must be evaluated
      /// @param [in] krigingType   the kriging type to apply
      /// @returns the proxy value
      ProxyValue                 getProxyValue(
            size_t                              index,
            Case const&                         c,
            KrigingType                         krigingType = DefaultKriging ) const;

      /// Proxy function calls for all proxies in the collection
      /// @param [in] c             the case for which the proxies must be evaluated
      /// @param [in] krigingType   the kriging type to apply
      /// @returns a list of proxy values
      ProxyValueList             getProxyValueList(
            Case const&                         c,
            KrigingType                         krigingType = DefaultKriging ) const;


      /// Return the cubic proxy coefficients and their standard errors for each of the proxies in the
      /// collection.
      ///
      /// @param [out] a vector of coefficient maps. The keys are index lists
      ///              referring to parameter elements in the unprepared
      ///              parameter set.
      void getCoefficientsMapList( std::vector<CubicProxy::CoefficientsMap>& ) const;

      /// Returns the number of sub-proxies in this CompoundProxyCollection
      /// This is equal to the dimension of the ProxyValueList
      size_t getNumSubProxies() const;

      // made deliberately private so that calling load/save directly on this class is more difficult
      // the preferred way is to call save/load on the ISerializer.
   private:
      // ISerializable
      virtual bool load( IDeserializer*, unsigned int version );
      virtual bool save( ISerializer*, unsigned int version ) const;

   private:

      // methods
      void provideCaseValidity(
            std::vector<std::vector<bool> > const& case2Obs2Valid,
            unsigned int                           nbOfObsValues,
            unsigned int                           obsIndex,
            std::vector<bool>&                     caseValid ) const;

      void deleteProxies();

      // data
      std::auto_ptr<KrigingData>    m_krigingData;
      CompoundProxyList             m_proxies;
      ParameterSpace                m_parameterSpace;
      RealMatrix                    m_preparedCaseSet;

};

} // namespace SUMlib

#endif // SUMLIB_COMPOUNDPROXYCOLLECTION_H

