// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include <cassert>
#include <cmath>

#include "Exception.h"
#include "NumericUtils.h"
#include "KrigingData.h"
#include "KrigingProxy.h"
#include "KrigingWeights.h"
#include "ParameterTransforms.h"
#include "SerializerUtils.h"

using std::vector;

namespace SUMlib {

namespace {
   unsigned int g_version(1);
}

KrigingProxy::KrigingProxy( ParameterSet const& par, KrigingData *kr, std::vector<bool> const& caseValid, TargetSet const& tar, ParameterTransforms const& trans )
:
   m_parSet( &par ),
   m_krigingData( kr ),
   m_caseValid( &caseValid ),
   m_targetSet( &tar ),
   m_parTrans( &trans )
{
}

KrigingProxy::KrigingProxy( CubicProxy *proxyModel, const ParameterTransforms& parameterTransforms, KrigingData *kr, ParameterSet const& parSet,
                            std::vector<bool> const& caseValid, TargetSet const& target, unsigned int nbOfOrdPars )
{
   initialise( proxyModel, parameterTransforms, kr, parSet, caseValid, target, nbOfOrdPars );
}

void KrigingProxy::initialise( CubicProxy *proxyModel, const ParameterTransforms& parameterTransforms, KrigingData *kr, ParameterSet const& parSet,
                               std::vector<bool> const& caseValid, TargetSet const& target, unsigned int nbOfOrdPars )
{
   m_parSet = &parSet;
   m_caseValid = &caseValid;
   m_targetSet = &target;
   m_parTrans = &parameterTransforms;

   if ( kr == NULL )
   {
      THROW2( InvalidState, "Pointer to Kriging data model is NULL" );
   }

   // retrieve Kriging data
   m_krigingData = kr;
   if ( m_krigingData->empty() )
   {
      m_krigingData->initialise( parSet, nbOfOrdPars );
   }

   calcProxyError( *proxyModel );
}

void KrigingProxy::calcProxyError( CubicProxy const& proxy )
{
   m_proxyError.clear();
   m_proxyError.reserve( m_parSet->size() );
   for ( ParameterSet::const_iterator it = m_parSet->begin(); it != m_parSet->end(); ++it )
   {
      double proxyValue;
      if ( m_parTrans->isTrivial() )
      {
         proxyValue = proxy.getValue( *it );
      }
      else
      {
         const Parameter& p = m_parTrans->apply( *it );
         proxyValue = proxy.getValue( p );
      }
      m_proxyError.push_back( proxyValue );
   }
   assert( m_proxyError.size() == m_caseValid->size() );

   unsigned int iObs = 0;
   for ( unsigned i = 0; i < m_caseValid->size(); ++i )
   {
      if ( (*m_caseValid)[i] )
      {
         m_proxyError[i] = (*m_targetSet)[iObs] - m_proxyError[i];
         iObs++;
      }
      else
      {
         m_proxyError[i] = 0.0;
      }
   }
   assert( iObs == m_targetSet->size() );
}

unsigned int KrigingProxy::size() const
{
   return static_cast<unsigned int>( m_parSet->front().size() );
}

double KrigingProxy::getValue( Parameter const& p, KrigingType krigingType ) const
{
   KrigingWeights krigingWeights;
   calcKrigingWeights( p, krigingType, krigingWeights );
   return getValue( krigingWeights, p, krigingType );
}

double KrigingProxy::getValue( KrigingWeights const& krigingWeights, Parameter const& p, KrigingType ) const
{
   assert( size() == p.size() );
   assert( m_parSet->size() == krigingWeights.weights().size() );
   const double interpolatedError = calcKrigingError( krigingWeights );

   // Be conservative: sum of weights should not exceed 1.
   return ( krigingWeights.sumOfWeights() > 1 ) ? interpolatedError/krigingWeights.sumOfWeights() : interpolatedError;
}

void KrigingProxy::calcKrigingWeights( Parameter const& p, KrigingType kriging, KrigingWeights& krigingWeights ) const
{
   assert( size() == p.size() );

   switch( kriging )
   {
      case GlobalKriging:
         krigingWeights.calcGlobalWeights( *m_parSet, *m_krigingData, p );
         break;
      case LocalKriging:
         krigingWeights.calcLocalWeights( *m_parSet, *m_krigingData, p );
         break;
      default:
         krigingWeights.zeroWeights( static_cast<unsigned int>( m_parSet->size() ) );
         break;
   }
}

double KrigingProxy::calcKrigingError( KrigingWeights const& weights ) const
{
   double error( 0.0 );
   IndexList const& i_w( weights.indexes() );
   RealVector const& w( weights.weights() );

   for ( unsigned i = 0; i < i_w.size(); ++i )
   {
      error += w[ i_w[i] ] * m_proxyError[ i_w[i] ];
   }
   return error;
}


bool KrigingProxy::load( IDeserializer* deserializer, unsigned int version )
{
   bool  ok(true);

   if ( version < 1 )
   {
      ParameterSet parSet;
      ok = ok && deserialize(deserializer, parSet);
   }
   ok = ok && deserialize(deserializer, m_proxyError);
   if ( version < 1 )
   {
      unsigned int parSize;
      ok = ok && deserialize(deserializer, parSize);
   }

   return ok;
} // KrigingProxy::load()


bool KrigingProxy::save( ISerializer* serializer, unsigned int version ) const
{
   bool  ok(true);
   assert( version == getSerializationVersion() );

   ok = ok && serialize(serializer, m_proxyError);

   return ok;
} // KrigingProxy::save()

unsigned int KrigingProxy::getSerializationVersion() const
{
   return g_version;
}

} // namespace SUMlib

