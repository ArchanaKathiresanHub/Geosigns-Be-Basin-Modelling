// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include <assert.h>

#include "CompoundProxyCollection.h"
#include "KrigingData.h"
#include "SerializerUtils.h"

namespace SUMlib {

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
               TargetCollection const&                                                 targets,
               std::vector< std::vector< bool > > const&                               case2Obs2Valid,
               unsigned int                                                            order,
               bool                                                                    modelSearch,
               double                                                                  targetR2,
               double                                                                  confLevel,
               Partition const&                                                        partition,
               const std::vector< std::vector< ParameterTransforms::TransformType > >& parTransformsDef
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
   unsigned int iObs = 0;

   const size_t nContinuousPar = m_parameterSpace.nbOfNonFixedContinuousPars();

   /// Copy needed in order to ensure that the parameter definitions are initialised.
   std::vector< std::vector< ParameterTransforms::TransformType > > parTransformsDefCopy( parTransformsDef );

   if ( parTransformsDefCopy.size() == 0 )
   {
      parTransformsDefCopy.assign( targets.size(), std::vector< ParameterTransforms::TransformType >( nContinuousPar, ParameterTransforms::transformNone ) );
   }

   assert( parTransformsDefCopy.size() == targets.size() );

   for ( TargetCollection::const_iterator t = targets.begin(); t != targets.end(); ++t )
   {
      std::vector<bool> caseValid;
      provideCaseValidity( case2Obs2Valid, (*t).size(), iObs, caseValid );

      assert( parTransformsDefCopy[ iObs ].size() == nContinuousPar );
      ParameterTransforms::ptr parTransforms( new ParameterTransforms( parTransformsDefCopy[ iObs ], m_parameterSpace ) );

      m_proxies.push_back( new CompoundProxy( m_preparedCaseSet, caseValid, m_krigingData.get(), *t, n, order, modelSearch, targetR2, confLevel, part,  parTransforms ) );

      iObs++;
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

void CompoundProxyCollection::getCoefficientsMapList( std::vector<MonomialCoefficientsMap>& coef ) const
{
   coef.resize( m_proxies.size() );
   for ( unsigned int i = 0; i < m_proxies.size(); ++i )
   {
      // Get the coefficients map for the proxy
      MonomialCoefficientsMap map;
      m_proxies[i]->getCoefficientsMap( map );

      // Insert unprepared indexlists as keys, and coefficients as values
      for ( MonomialCoefficientsMap::const_iterator m = map.begin(); m != map.end(); ++m )
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
         CompoundProxy* compoundProxy(new CompoundProxy(m_krigingData.get()));
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


} // namespace SUMlib
