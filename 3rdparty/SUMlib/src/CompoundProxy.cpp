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
#include "EliminationCriteria.h"
#include "Exception.h"
#include "KrigingData.h"
#include "KrigingProxy.h"
#include "NumericUtils.h"
#include "ProxyEstimator.h"
#include "ProxyCases.h"
#include "SerializerUtils.h"

using std::vector;

namespace SUMlib {

static unsigned int  g_version(3);

void CompoundProxy::setValidityAndTragets( std::vector<bool> const& caseValid, TargetSet const& targets )
{
   m_caseValid = caseValid;
   m_targetSet = targets;
}

CompoundProxy::CompoundProxy(KrigingData *krigingData, ParameterSet const& parSet)
:
   m_krigingData(krigingData),
   m_adjustedR2(-1),
   m_parSet(&parSet)
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
         ParameterTransforms::ptr   parTransforms,
         EliminationCriterion&      criterion
         )
:
   m_krigingData( krigingData ),
   m_adjustedR2( -1 )
{
   initialise( parSet, caseValid, krigingData, targetSet, nbOfOrdPars, order, modelSearch, targetR2, confLevel, partition, parTransforms, criterion );
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
   NoElimination noElim;
   initialise( parSet, caseValid, krigingData, targetSet, nbOfOrdPars, order, modelSearch, targetR2, confLevel, partition, parTransforms, noElim );
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
   NoElimination noElim;
   initialise( parSet, caseValid, krigingData, targetSet, nbOfOrdPars, order, modelSearch, targetR2, confLevel, partition, parTransforms, noElim );
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
         ParameterTransforms::ptr   parTransforms,
         EliminationCriterion&      criterion
         )
{
   if ( parSet.empty() )
   {
      THROW2( DimensionOutOfBounds, "Parameter set cannot be empty" );
   }

   m_parSet = &parSet;

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
   calculateProxyPair( *m_parSet, caseValid, targetSet, krigingData, nbOfOrdPars, modelSearch, targetR2, confLevel, initVars, order, criterion );
}

void CompoundProxy::update( EliminationCriterion& criterion )
{
   ParameterSet validParSet;
   for ( unsigned int i = 0; i < m_caseValid.size(); ++i )
   {
      if ( m_caseValid[i] )
      {
         validParSet.push_back( (*m_parSet)[i] );
      }
   }
   // eliminate
   ProxyCases proxycases( validParSet, m_targetSet );
   assert( validParSet.size() == m_targetSet.size() );
   proxycases.createProxyBuilder( m_proxyPair.first->variables() );

   if ( proxycases.eliminate( criterion ) )
   {
      ProxyCandidate candidate;
      candidate.setProxy( proxycases );

      // Remove the terms that contain a constant-transformed parameter. See the documentation of
      // CubicProxy::removeConstTransformedParameters().
      const IndexList& indices = m_parTransforms->getConstTransformedParameters();
      candidate.proxy->removeConstTransformedParameters( indices );

      m_proxyPair.first.reset( candidate.proxy );
      m_proxyPair.second->calcProxyError( *candidate.proxy );
      m_adjustedR2 = candidate.adjustedR2;
      m_leverages = candidate.leverages;
   }
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
      unsigned int               order
      )
{
   NoElimination noElim;
   calculateProxyPair( parSet, caseValid, targetSet, krigingData, nbOfOrdPars, modelSearch, targetR2, confLevel, initVars, order, noElim );
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
      unsigned int               order,
      EliminationCriterion&      criterion
      )
{
   m_targetSet = targetSet;
   m_caseValid = caseValid;

   ParameterSet validParSet;
   for ( unsigned int i = 0; i < m_caseValid.size(); ++i )
   {
      if ( m_caseValid[i] )
      {
         validParSet.push_back( parSet[i] );
      }
   }
   assert( validParSet.size() == m_targetSet.size() );

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

   estimator.setTargetSet( m_targetSet );

   // Compute Cubic proxy
   ProxyCandidate candidate;
   estimator.autoEstimate( candidate, nbOfOrdPars, order, initVars, modelSearch, targetR2, confLevel, criterion );

   // If a proxy was calculated, use it even if the estimator did not converge
   if ( candidate.proxy )
   {
      // Remove the terms that contain a constant-transformed parameter. See the documentation of
      // CubicProxy::removeConstTransformedParameters().
      const IndexList& indices = m_parTransforms->getConstTransformedParameters();
      candidate.proxy->removeConstTransformedParameters( indices );

      m_proxyPair.first.reset( candidate.proxy );
      m_proxyPair.second.reset( new KrigingProxy( candidate.proxy, *m_parTransforms, krigingData, parSet, m_caseValid, m_targetSet, nbOfOrdPars ) );
      m_adjustedR2 = candidate.adjustedR2;
      m_leverages = candidate.leverages;
   }
}

unsigned int CompoundProxy::size( ) const
{
   return static_cast<unsigned int>( m_parSet->front().size() );
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

unsigned int CompoundProxy::getDesignMatrixRank() const
{
   assert( cubicProxy() );
   return cubicProxy()->getDesignMatrixRank();
}

bool CompoundProxy::isRegressionIllPosed() const
{
   assert( cubicProxy() );
   return cubicProxy()->isRegressionIllPosed();
}

bool CompoundProxy::load( IDeserializer* deserializer, unsigned int version )
{
   m_parTransforms.reset( new ParameterTransforms() );
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
      m_proxyPair.second.reset(new KrigingProxy(*m_parSet, m_krigingData, m_caseValid, m_targetSet, *m_parTransforms));
      ok = ok && deserialize( deserializer, *(m_proxyPair.second) );
   }

   if ( version < 3 )
   {
      unsigned int size;
      ok = ok && deserialize( deserializer, size);
   }

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
      ok = ok && deserialize( deserializer, *m_parTransforms );
      ok = ok && deserialize( deserializer, m_leverages );
   }
   else
   {
      m_leverages.clear();
   }

   if ( version >= 3 )
   {
      ok = ok && deserialize( deserializer, m_targetSet );
      ok = ok && deserializer->load(m_caseValid);
   }
   else
   {
      m_targetSet.clear();
      m_caseValid.clear();
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

   ok = ok && serialize( serializer, m_adjustedR2);

   ok = ok && serialize( serializer, *m_parTransforms );

   ok = ok && serialize( serializer, m_leverages );

   ok = ok && serialize( serializer, m_targetSet );

   ok = ok && serializer->save(m_caseValid);

   return ok;
} // CompoundProxy::save()

unsigned int CompoundProxy::getSerializationVersion() const
{
   return g_version;
}

} // namespace SUMlib
