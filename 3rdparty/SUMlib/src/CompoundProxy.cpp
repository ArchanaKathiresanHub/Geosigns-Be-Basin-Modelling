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

static unsigned int  g_version(1);

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
         Partition const&           partition
         )
:
   m_krigingData(krigingData),
   m_adjustedR2(-1)
{
   initialise( parSet, caseValid, krigingData, targetSet, nbOfOrdPars, order, modelSearch, targetR2, confLevel, partition );
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
         Partition const&           partition
         )
{
   if ( parSet.empty() )
   {
      THROW2( DimensionOutOfBounds, "Parameter set cannot be empty" );
   }
   m_size = parSet.front().size();
   assert( nbOfOrdPars <= size() );

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
   calculateProxyPair( m_proxyPair, m_adjustedR2, parSet, caseValid, targetSet, krigingData, nbOfOrdPars, modelSearch, targetR2, confLevel, initVars, order );
}

bool CompoundProxy::calculateProxyPair(
      ProxyPair&                 proxyPair,
      double&                    adjustedR2,
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
   estimator.setParameterSet( validParSet );
   estimator.setTargetSet( targetSet );

   // Compute Cubic proxy
   ProxyCandidate candidate;
   bool cubicOK = estimator.autoEstimate( candidate, nbOfOrdPars, order, initVars, modelSearch, targetR2, confLevel );

   // If a proxy was calculated, use it even if the estimator did not converge
   if ( candidate.proxy )
   {
      proxyPair.first.reset( candidate.proxy );
      proxyPair.second.reset( new KrigingProxy( candidate.proxy, krigingData, parSet, caseValid, targetSet, nbOfOrdPars ) );
      adjustedR2 = candidate.adjustedR2;
   }

   // Return whether the proxy calculation converged
   return cubicOK;
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
   double value = cubicProxy() ? cubicProxy()->getProxyValue ( p ) : 0.0;
   if ( krigingType != NoKriging && krigingProxy() )
   {
      value += krigingProxy()->getProxyValue ( p, krigingType );
   }

   return value;
}

double CompoundProxy::getProxyValue( KrigingWeights const& krigingWeights, Parameter const& p, KrigingType krigingType ) const
{
   // call the appropriate proxies as above
   double value = cubicProxy() ? cubicProxy()->getProxyValue ( p ) : 0.0;
   if ( krigingType != NoKriging && krigingProxy() )
   {
      value += krigingProxy()->getProxyValue( krigingWeights, p, krigingType ); //now with the already calculated (relevant) weights!
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

   return ok;
} // CompoundProxy::save()

unsigned int CompoundProxy::getSerializationVersion() const
{
   return g_version;
}

} // namespace SUMlib
