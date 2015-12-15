// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include <assert.h>

#include "CompoundProxyCollection.h"
#include "EliminationCriteria.h"
#include "Exception.h"
#include "KrigingData.h"
#include "SerializerUtils.h"

namespace SUMlib
{

namespace
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Intialises the parameter transforms. Performs the following tasks:
/// * Filter out the frozen parameters in @param parameterSpace.
/// * Create the transforms (default, trivial transforms are created when parTransformsDef is empty).
/// * Check if all transforms are valid, throws a SUMlib::InvalidTransforms exception when this is not the case.
///
/// Callers owns the ParameterTransforms (i.e. return vector elements) and is responsible for clean-up or ownership transfer.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector< ParameterTransforms* > initialiseParameterTransforms( std::vector< ParameterTransformTypeVector > parTransformsDef,
                                                                   const ParameterSpace& parameterSpace,
                                                                   const size_t numObservables )
{
   if ( parTransformsDef.size() == 0 )
   {
      parTransformsDef.assign( numObservables, ParameterTransformTypeVector( parameterSpace.size(), ParameterTransforms::transformNone ) );
   }
   assert( parTransformsDef.size() == numObservables );

   /// Remember the invalid transforms during the prepare below.
   std::vector< size_t >      invalidObsTransforms;
   std::vector< size_t >      invalidParTransforms;
   std::vector< std::string > invalidTransformReasons;
   bool                       allTransformsAreValid = true;

   /// Prepare transformations.
   std::vector< ParameterTransforms* > obsDependentParTransforms( numObservables );
   for ( size_t iObs = 0; iObs < numObservables; ++iObs )
   {
      assert( parTransformsDef[ iObs ].size() == parameterSpace.size() );

      std::vector< size_t > invalidParTransformsForThisObs;
      std::vector< std::string > invalidTransformReasonsForThisObs;

      /// Create new ParameterTransforms.
      ParameterTransforms* pt = new ParameterTransforms( parTransformsDef[ iObs ], parameterSpace );
      obsDependentParTransforms[ iObs ] = pt;

      /// Handle invalid transforms.
      allTransformsAreValid &= pt->isValid( invalidParTransformsForThisObs, invalidTransformReasonsForThisObs );

      /// Append the invalid indices and reasons.
      invalidParTransforms.insert( invalidParTransforms.end(), invalidParTransformsForThisObs.begin(), invalidParTransformsForThisObs.end() );
      invalidTransformReasons.insert( invalidTransformReasons.end(), invalidTransformReasonsForThisObs.begin(), invalidTransformReasonsForThisObs.end() );

      /// This appends the current value of iObs for each invalid parameter transform found for this observable.
      invalidObsTransforms.resize( invalidParTransforms.size(), iObs );
   }

   /// Throw exception if there are invalid transforms.
   if ( !allTransformsAreValid )
   {
      /// Need to clean up the parameter transforms before throwing the exception.
      for ( size_t i = 0; i < obsDependentParTransforms.size(); ++i )
      {
         delete obsDependentParTransforms[ i ];
      }
      throw InvalidTransforms( invalidObsTransforms, invalidParTransforms, invalidTransformReasons );
   }

   return obsDependentParTransforms;
}

} /// Anonymous namespace.

void CompoundProxyCollection::setValidityAndTargets( std::vector< std::pair< std::vector<bool>, std::vector<double> > > validityAndTargets )
{
   assert( m_proxies.size() == validityAndTargets.size() );

   for ( size_t i = 0; i < m_proxies.size(); ++i )
   {
      m_proxies[i]->setValidityAndTragets( validityAndTargets[i].first, validityAndTargets[i].second );
   }
}

CompoundProxyCollection::CompoundProxyCollection()
{
   m_krigingData.reset( new KrigingData );
}

CompoundProxyCollection::CompoundProxyCollection(
               std::vector<Case> const& caseSet,
               Case const& origLow, Case const& origHigh,
               ParameterSpace::TransformationSet const&  tr
               ) :
   m_parameterSpace( origLow, origHigh, tr )
{
   m_parameterSpace.setBounds( ParameterBounds( caseSet ) );
   m_parameterSpace.prepare( caseSet, m_preparedCaseSet );
   unsigned int nbOfPreparedOrdinals = m_parameterSpace.nbOfNonFixedOrdinalPars();
   m_krigingData.reset( new KrigingData( m_preparedCaseSet, nbOfPreparedOrdinals ) );
}

CompoundProxyCollection::~CompoundProxyCollection()
{
   deleteProxies();
}

void CompoundProxyCollection::calculate(
               TargetCollection const&                               targets,
               std::vector< std::vector< bool > > const&             case2Obs2Valid,
               unsigned int                                          order,
               bool                                                  modelSearch,
               double                                                targetR2,
               double                                                confLevel,
               Partition const&                                      partition,
               const std::vector< ParameterTransformTypeVector >&    parTransformsDef
               )
{
   NoElimination noElim;
   calculate( targets, case2Obs2Valid, order, modelSearch, targetR2, confLevel, partition, noElim, parTransformsDef );
}

void CompoundProxyCollection::calculate(
               TargetCollection const&                               targets,
               std::vector< std::vector< bool > > const&             case2Obs2Valid,
               unsigned int                                          order,
               bool                                                  modelSearch,
               double                                                targetR2,
               double                                                confLevel,
               Partition const&                                      partition,
               EliminationCriterion&                                 criterion,
               const std::vector< ParameterTransformTypeVector >&    parTransformsDef
               )
{
   assert( case2Obs2Valid.size() == m_preparedCaseSet.size() );
   for ( unsigned int i = 0; i < m_preparedCaseSet.size(); ++i )
   {
      assert( case2Obs2Valid[i].size() == targets.size() );
   }

   deleteProxies();

   Partition part( partition );
   m_parameterSpace.prepare( part );
   unsigned int n = m_parameterSpace.nbOfNonFixedOrdinalPars();

   /// Ownership is transferred to the individual sub-proxies in the loop below.
   std::vector< ParameterTransforms* > obsDependentParTransforms = initialiseParameterTransforms( parTransformsDef, m_parameterSpace, targets.size() );

   /// Create all sub-proxies.
   for ( size_t iObs = 0; iObs < targets.size(); ++iObs )
   {
      std::vector<bool> caseValid;
      provideCaseValidity( case2Obs2Valid, static_cast<unsigned int>(targets[iObs].size()), static_cast<unsigned int>(iObs), caseValid);

      ParameterTransforms::ptr parTransforms( obsDependentParTransforms[ iObs ] );

      m_proxies.push_back( new CompoundProxy( m_preparedCaseSet, caseValid, m_krigingData.get(), targets[ iObs ], n, order, modelSearch, targetR2, confLevel, part,  parTransforms, criterion ) );
   }
}

void CompoundProxyCollection::recalculate( EliminationCriterion& criterion )
{
   const CompoundProxyList::iterator end = m_proxies.end();
   for ( CompoundProxyList::iterator proxy = m_proxies.begin(); proxy != end; ++proxy )
   {
      (*proxy)->update( criterion );
   }
}

CompoundProxyCollection::CompoundProxyList const& CompoundProxyCollection::getProxyList() const
{
   return m_proxies;
}

ParameterSpace const& CompoundProxyCollection::getParameterSpace() const
{
   return m_parameterSpace;
}

ProxyValueList CompoundProxyCollection::getProxyValueList( Case const& c, KrigingType krigingType ) const
{
   RealVector preparedCase;
   m_parameterSpace.prepare( c, preparedCase );

   ProxyValueList values;
   for ( CompoundProxyList::const_iterator pp = m_proxies.begin(); pp != m_proxies.end(); ++pp )
   {
      values.push_back( (*pp)->getProxyValue( preparedCase, krigingType ) );
   }
   return values;
}

ProxyValue CompoundProxyCollection::getProxyValue( size_t index, Case const& c, KrigingType krigingType ) const
{
   assert( index < m_proxies.size() );

   RealVector preparedCase;
   m_parameterSpace.prepare( c, preparedCase );

   CompoundProxy const* px( m_proxies[index] );
   return px->getProxyValue( preparedCase, krigingType );
}

void CompoundProxyCollection::getCoefficientsMapList( std::vector<CubicProxy::CoefficientsMap>& coef ) const
{
   coef.resize( m_proxies.size() );
   for ( unsigned int i = 0; i < m_proxies.size(); ++i )
   {
      // Get the coefficients map for the proxy
      CubicProxy::CoefficientsMap map;
      m_proxies[i]->getCoefficientsMap( map );

      // Insert unprepared indexlists as keys, and coefficients and standard errors as values
      for ( CubicProxy::CoefficientsMap::const_iterator m = map.begin(); m != map.end(); ++m )
      {
         IndexList key( ( *m ).first );
         m_parameterSpace.convert2origProxyIdx( key );
         coef[i][ key ] = ( *m ).second;
      }
   }
}

size_t CompoundProxyCollection::getNumSubProxies() const
{
   return m_proxies.size();
}

void CompoundProxyCollection::provideCaseValidity(
            std::vector<std::vector<bool> > const& case2Obs2Valid,
            unsigned int                           nbOfObsValues,
            unsigned int                           obsIndex,
            std::vector<bool>&                     caseValid ) const
{
   caseValid.resize( m_preparedCaseSet.size() );
   unsigned int nbOfValidCases = 0;
   for ( unsigned int caseIndex = 0; caseIndex < caseValid.size(); ++caseIndex )
   {
      caseValid[caseIndex] = case2Obs2Valid[caseIndex][obsIndex];
      if ( caseValid[caseIndex] ) nbOfValidCases++;
   }
   assert( nbOfObsValues == nbOfValidCases );
}

void CompoundProxyCollection::deleteProxies()
{
   for ( CompoundProxyList::iterator pp = m_proxies.begin(); pp != m_proxies.end(); ++pp )
   {
      delete( *pp );
   }
   m_proxies.clear();
}

bool CompoundProxyCollection::load( IDeserializer* deserializer, unsigned int )
{
   bool ok = true;
   ok = ok && deserialize( deserializer, *m_krigingData );

   unsigned int   nrOfProxies(0);
   ok = ok && deserialize( deserializer, nrOfProxies);

   if (ok)
   {
      deleteProxies();
      m_proxies.resize(nrOfProxies);

      // for all proxies
      for (size_t i(0); ok && i < m_proxies.size(); ++i)
      {
         CompoundProxy* compoundProxy(new CompoundProxy(m_krigingData.get(), m_preparedCaseSet));
         ok = ok && deserialize( deserializer, *compoundProxy );
         m_proxies[i] = compoundProxy;
      } // for all proxies
   }

   ok = ok && deserialize( deserializer, m_parameterSpace );
   ok = ok && deserialize( deserializer, m_preparedCaseSet);

   return ok;
} // CompoundProxyCollection::load()

bool CompoundProxyCollection::save( ISerializer* serializer, unsigned int ) const
{
   bool ok = true;

   ok = ok && serialize( serializer, *m_krigingData );

   ok = ok && serialize( serializer, (unsigned int)m_proxies.size());

   // for all proxies
   for (size_t i(0); ok && i < m_proxies.size(); ++i)
   {
      const CompoundProxy* compoundProxy(m_proxies[i]);
      ok = ok && serialize( serializer, *compoundProxy );
   } // for all proxies

   ok = ok && serialize( serializer, m_parameterSpace );
   ok = ok && serialize( serializer, m_preparedCaseSet);

   return ok;
} // CompoundProxyCollection::save()

/// Fetch the krigingData
KrigingData const& CompoundProxyCollection::getKrigingData() const
{
   return *m_krigingData;
}

ParameterSet const& CompoundProxyCollection::getPreparedCaseSet() const
{
   return m_preparedCaseSet;
}

} // namespace SUMlib
