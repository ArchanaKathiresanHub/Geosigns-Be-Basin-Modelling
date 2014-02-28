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
#include "SerializerUtils.h"

using std::vector;

namespace SUMlib {


KrigingProxy::KrigingProxy( KrigingData *kr )
:
   m_krigingData(kr),
   m_parSize(0)
{
}

KrigingProxy::KrigingProxy( Proxy *proxyModel, KrigingData *kr, ParameterSet const& parSet,
                            std::vector<bool> const& caseValid, TargetSet const& target, unsigned int nbOfOrdPars )
{
   initialise( proxyModel, kr, parSet, caseValid, target, nbOfOrdPars );
}

void KrigingProxy::initialise( Proxy *proxyModel, KrigingData *kr, ParameterSet const& parSet,
                               std::vector<bool> const& caseValid, TargetSet const& target, unsigned int nbOfOrdPars )
{
   m_parSize = parSet.front().size();
   m_parSet = parSet;
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

   // function calls to calculate proxy model response values
   m_proxyError.reserve( parSet.size() );
   for ( ParameterSet::const_iterator it = parSet.begin(); it != parSet.end(); ++it )
   {
      m_proxyError.push_back( proxyModel->getProxyValue( *it ) );
   }
   assert( m_proxyError.size() == caseValid.size() );

   unsigned int iObs = 0;
   for ( unsigned i = 0; i < caseValid.size(); ++i )
   {
      if ( caseValid[i] )
      {
         m_proxyError[i] = target[iObs] - m_proxyError[i];
         iObs++;
      }
      else
      {
         m_proxyError[i] = 0.0;
      }
   }
   assert( iObs == target.size() );
}

unsigned int KrigingProxy::size() const
{
   return m_parSize;
}

double KrigingProxy::getProxyValue( Parameter const& p, KrigingType krigingType ) const
{
   KrigingWeights krigingWeights;
   calcKrigingWeights( p, krigingType, krigingWeights );
   return getProxyValue( krigingWeights, p, krigingType );
}

double KrigingProxy::getProxyValue( KrigingWeights const& krigingWeights, Parameter const& p, KrigingType ) const
{
   assert( size() == p.size() );
   assert( m_parSet.size() == krigingWeights.weights().size() );
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
         krigingWeights.calcGlobalWeights( m_parSet, *m_krigingData, p );
         break;
      case LocalKriging:
         krigingWeights.calcLocalWeights( m_parSet, *m_krigingData, p );
         break;
      default:
         krigingWeights.zeroWeights( m_parSet.size() );
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


bool KrigingProxy::load( IDeserializer* deserializer, unsigned int )
{
   bool  ok(true);

   ok = ok && deserialize(deserializer, m_parSet);
   ok = ok && deserialize(deserializer, m_proxyError);
   ok = ok && deserialize(deserializer, m_parSize);

   return ok;
} // KrigingProxy::load()


bool KrigingProxy::save( ISerializer* serializer, unsigned int ) const
{
   bool  ok(true);

   ok = ok && serialize(serializer, m_parSet);
   ok = ok && serialize(serializer, m_proxyError);
   ok = ok && serialize(serializer, m_parSize);

   return ok;
} // KrigingProxy::save()

} // namespace SUMlib

