// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include <algorithm>
#include <cassert>
#include <cmath>
#include <vector>

#include "CompoundProxy.h"
#include "CubicProxy.h"
#include "Exception.h"
#include "KrigingData.h"
#include "KrigingProxy.h"
#include "NumericUtils.h"
#include "ProxyEstimator.h"
#include "SerializerUtils.h"

using std::vector;

namespace SUMlib {

static unsigned int  g_version(2);

CompoundProxy::CompoundProxy(
   KrigingData *krigingData)
:
   m_krigingData(krigingData)
{
   // empty
}

CompoundProxy::CompoundProxy(
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
         ParameterTransforms::ptr   parTransforms
         )
:
   m_krigingData(krigingData),
   m_adjustedR2(-1)
{
   initialise( parSet, caseValid, krigingData, targetSet, nbOfOrdPars, order, modelSearch, targetR2, confLevel, partition, parTransforms );
}

CompoundProxy::~CompoundProxy()
{
   // destructor implemented where CubicProxy and KrigingProxy classes are defined, to avoid std::auto_ptr related memory leak
}

void CompoundProxy::initialise(
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
         ParameterTransforms::ptr   parTransforms
         )
{
   if ( parSet.empty() )
   {
      THROW2( DimensionOutOfBounds, "Parameter set cannot be empty" );
   }
   m_size = parSet.front().size();
   assert( nbOfOrdPars <= size() );

   m_parTransforms = parTransforms;
   assert( m_parTransforms.get() );

   Partition part( partition );
   if ( part.empty() )
   {
      part.assign( size(), true );
   }
   IndexList initVars; //determined by polynomial order and parameter partition

   if ( nbOfOrdPars == size() )
   {
      initVars = CubicProxy::initialVarList( size(), order, part ); //ordinals only
   }
   else
   {
      initVars = CubicProxy::initialVarList( size(), nbOfOrdPars, order, part ); //categorical parameter(s) present
   }
   calculateProxyPair( parSet, caseValid, targetSet, krigingData, nbOfOrdPars, modelSearch, targetR2, confLevel, initVars, order );
}

void CompoundProxy::calculateProxyPair(
      ParameterSet const&        parSet,
      std::vector<bool> const&   caseValid,
      TargetSet const&           targetSet,
      KrigingData *              krigingData,
      unsigned int               nbOfOrdPars,
      bool                       modelSearch,
      double                     targetR2,
      double                     confLevel,
      IndexList const&           initVars,
      unsigned int               order )
{
   ParameterSet validParSet;
   for ( unsigned int i = 0; i < caseValid.size(); ++i )
   {
      if ( caseValid[i] )
      {
         validParSet.push_back( parSet[i] );
      }
   }
   assert( validParSet.size() == targetSet.size() );

   ProxyEstimator estimator;

   if ( m_parTransforms->isTrivial() )
   {
      estimator.setParameterSet( validParSet );
   }
   else
   {
      ParameterSet transformedParSet( validParSet.size() );
      for ( size_t i = 0; i < validParSet.size(); ++i )
      {
         transformedParSet[ i ] = m_parTransforms->apply( validParSet[ i ] );
      }
      estimator.setParameterSet( transformedParSet );
   }

   estimator.setTargetSet( targetSet );

   // Compute Cubic proxy
   ProxyCandidate candidate;
   estimator.autoEstimate( candidate, nbOfOrdPars, order, initVars, modelSearch, targetR2, confLevel );

   // If a proxy was calculated, use it even if the estimator did not converge
   if ( candidate.proxy )
   {
      m_proxyPair.first.reset( candidate.proxy );
      m_proxyPair.second.reset( new KrigingProxy( candidate.proxy, *m_parTransforms, krigingData, parSet, caseValid, targetSet, nbOfOrdPars ) );
      m_adjustedR2 = candidate.adjustedR2;
      m_leverages = candidate.leverages;
   }
}

unsigned int CompoundProxy::size( ) const
{
   return m_size;
}

void CompoundProxy::calcKrigingWeights( Parameter const& p, KrigingType krigingType, KrigingWeights& krigingWeights ) const
{
   if ( krigingProxy() )
   {
      krigingProxy()->calcKrigingWeights ( p, krigingType, krigingWeights );
   }
}

double CompoundProxy::getProxyValue( Parameter const& p, KrigingType krigingType ) const
{
   // call the appropriate proxies with the real-valued part of the parameter
   double value = 0;
   if ( cubicProxy() )
   {
      if ( m_parTransforms->isTrivial() )
      {
         value = cubicProxy()->getValue( p );
      }
      else
      {
         const Parameter& pTransformed = m_parTransforms->apply( p );
         value = cubicProxy()->getValue( pTransformed );
      }
   }
   if ( krigingType != NoKriging && krigingProxy() )
   {
      value += krigingProxy()->getValue( p, krigingType );
   }

   return value;
}

double CompoundProxy::getProxyValue( KrigingWeights const& krigingWeights, Parameter const& p, KrigingType krigingType ) const
{
   // call the appropriate proxies as above
   double value = 0;
   if ( cubicProxy() )
   {
      if ( m_parTransforms->isTrivial() )
      {
         value = cubicProxy()->getValue( p );
      }
      else
      {
         const Parameter& pTransformed = m_parTransforms->apply( p );
         value = cubicProxy()->getValue( pTransformed );
      }
   }
   if ( krigingType != NoKriging && krigingProxy() )
   {
      value += krigingProxy()->getValue( krigingWeights, p, krigingType ); //now with the already calculated (relevant) weights!
   }
   return value;
}

void CompoundProxy::getCoefficientsMap( CubicProxy::CoefficientsMap& map ) const
{
   if ( cubicProxy() )
   {
      cubicProxy()->getCoefficientsMap( map );
   }
}


bool CompoundProxy::load( IDeserializer* deserializer, unsigned int version )
{
   bool  ok(true);

   bool  hasCubicProxy(false);
   ok = ok && deserialize(deserializer, hasCubicProxy);

   bool  hasKrigingProxy(false);
   ok = ok && deserialize(deserializer, hasKrigingProxy);

   if (hasCubicProxy)
   {
      m_proxyPair.first.reset(new CubicProxy);
      ok = ok && deserialize(deserializer, *(m_proxyPair.first) );
   }

   if (hasKrigingProxy)
   {
      m_proxyPair.second.reset(new KrigingProxy(m_krigingData));
      ok = ok && deserialize( deserializer, *(m_proxyPair.second) );
   }

   ok = ok && deserialize( deserializer, m_size);

   if (version >= 1)
   {
      ok = ok && deserialize(deserializer, m_adjustedR2);
   }
   else
   {
      // Use -1 to indicate absence.
      m_adjustedR2 = -1;
   }

   if ( version >= 2 )
   {
      m_parTransforms.reset( new ParameterTransforms() );
      ok = ok && deserialize( deserializer, *m_parTransforms );

      ok = ok && deserialize( deserializer, m_leverages );
   }
   else
   {
      m_parTransforms.reset( new ParameterTransforms() );

      m_leverages.clear();
   }

   return ok;
} // CompoundProxy::load()


bool CompoundProxy::save( ISerializer* serializer, unsigned int version ) const
{
   assert( version == getSerializationVersion() );
   bool  ok(true);

   bool  hasCubicProxy(m_proxyPair.first.get() != 0);
   ok = ok && serialize(serializer, hasCubicProxy);

   bool  hasKrigingProxy(m_proxyPair.second.get() != 0);
   ok = ok && serialize(serializer, hasKrigingProxy);

   if (hasCubicProxy)
   {
      ok = ok && serialize( serializer, *(m_proxyPair.first) );
   }

   if (hasKrigingProxy)
   {
      ok = ok && serialize( serializer, *(m_proxyPair.second) );
   }

   ok = ok && serialize( serializer, m_size);

   ok = ok && serialize( serializer, m_adjustedR2);

   ok = ok && serialize( serializer, *m_parTransforms );

   ok = ok && serialize( serializer, m_leverages );

   return ok;
} // CompoundProxy::save()

unsigned int CompoundProxy::getSerializationVersion() const
{
   return g_version;
}

} // namespace SUMlib
