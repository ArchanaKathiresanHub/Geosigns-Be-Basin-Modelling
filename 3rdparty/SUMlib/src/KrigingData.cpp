// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include <cassert>
#include <cmath>

#include "BaseTypes.h"
#include "NumericUtils.h"
#include "KrigingData.h"
#include "SerializerUtils.h"

using std::vector;

namespace SUMlib {

KrigingData::KrigingData()
{
   //empty
}

KrigingData::~KrigingData()
{
   //empty
}

KrigingData::KrigingData( ParameterSet const& parSet, unsigned int nbOfOrdPars )
{
   initialise( parSet, nbOfOrdPars );
}

void KrigingData::initialise( ParameterSet const& parSet, unsigned int nbOfOrdPars )
{
   m_nbOfOrdPars = nbOfOrdPars;
   defineCorrelationLengths( m_nbOfOrdPars );
   calcDistances( parSet );
   calcInvCov();
}

void KrigingData::defineCorrelationLengths( unsigned int dim )
{
   // Global Kriging assumes a correlation length that spans the entire ordinal parameter space.
   m_globalCorLength = 2*sqrt( double( dim ) ); //maximum distance (main diagonal) in ordinal parameter space
   if ( dim == 0 ) m_globalCorLength = 1e-6; //arbitrary small positive value if ordinal parameter space is 0D

   // Local Kriging assumes a smaller correlation length that spans only a part of the ordinal parameter space.
   // The magic factor 1/6 turns out to be appropriate.
   m_localCorLength = m_globalCorLength/6.0;
}

void KrigingData::calcDistances( ParameterSet const& parSet )
{
   const unsigned int m = static_cast<unsigned int>( parSet.size() );
   m_distances.resize( m );
   for ( unsigned i = 0; i < m; ++i )
   {
      m_distances[i].assign( m, 0.0 );
   }
   m_minDistance = m_globalCorLength; //initialise minimum distance
   double dMax = 0.0; //initialise maximum distance
   m_refIndex = 0; //initialise reference index
   for ( unsigned i = 0; i < m; ++i )
   {
      for ( unsigned j = i+1; j < m; ++j )
      {
         m_distances[i][j] = KrigingDistance( parSet[i], parSet[j], m_nbOfOrdPars );
         if ( m_distances[i][j] < m_minDistance )
         {
            m_minDistance = m_distances[i][j];
         }
         if ( m_distances[i][j] > dMax )
         {
            dMax = m_distances[i][j];
            m_refIndex = i;
         }
         m_distances[j][i] = m_distances[i][j]; //distance matrix is symmetric
      }
   }
}

void KrigingData::calcInvCov()
{
   const unsigned int m = static_cast<unsigned int>( m_distances.size() );
   m_globalInvCov.resize( m );
   m_localInvCov.resize( m );
   for ( unsigned i = 0; i < m; ++i )
   {
      m_globalInvCov[i].assign( m, 1.0 );
      m_localInvCov[i].assign( m, 1.0 );
   }
   for ( unsigned i = 0; i < m; ++i )
   {
      for ( unsigned j = i+1; j < m; ++j )
      {
         //store Covariance matrix in m_invCov
         m_globalInvCov[i][j] = calcCovariance( m_distances[i][j], m_globalCorLength );
         m_globalInvCov[j][i] = m_globalInvCov[i][j]; //Covariance matrix is symmetric
         m_localInvCov[i][j] = calcCovariance( m_distances[i][j], m_localCorLength );
         m_localInvCov[j][i] = m_localInvCov[i][j]; //Covariance matrix is symmetric
      }
   }

   // actual matrix (pseudo) inversions
   if ( m > 0 )
   {
      svinv( m_globalInvCov ); //inverse is also symmetric!
      svinv( m_localInvCov ); //inverse is also symmetric!
   }
}

double KrigingData::calcCovariance( double distance, double corLength ) const
{
   double cov = 0.0;
   if ( distance < corLength )
   {
      // Covariance scaled to [0,1] and smoothly decreasing (monotone) as a funcion of distance!
      //cov = 1 + distance*distance*(2*distance/corLength - 3)/(corLength*corLength);
      // Or a linear decrease as a function of distance in order to avoid overshoot!
      cov = 1 - distance/corLength;
   }
   return cov;
}

bool KrigingData::empty() const
{
   return m_distances.empty();
}


bool KrigingData::load( IDeserializer* deserializer, unsigned int )
{
   bool  ok(true);

   ok = ok && deserialize(deserializer, m_globalCorLength);
   ok = ok && deserialize(deserializer, m_localCorLength);
   ok = ok && deserialize(deserializer, m_nbOfOrdPars);
   ok = ok && deserialize(deserializer, m_distances);
   ok = ok && deserialize(deserializer, m_minDistance);
   ok = ok && deserialize(deserializer, m_refIndex);
   ok = ok && deserialize(deserializer, m_globalInvCov);
   ok = ok && deserialize(deserializer, m_localInvCov);

   return ok;
} // KrigingData::load()


bool KrigingData::save( ISerializer* serializer, unsigned int ) const
{
   bool  ok(true);

   ok = ok && serialize(serializer, m_globalCorLength);
   ok = ok && serialize(serializer, m_localCorLength);
   ok = ok && serialize(serializer, m_nbOfOrdPars);
   ok = ok && serialize(serializer, m_distances);
   ok = ok && serialize(serializer, m_minDistance);
   ok = ok && serialize(serializer, m_refIndex);
   ok = ok && serialize(serializer, m_globalInvCov);
   ok = ok && serialize(serializer, m_localInvCov);

   return ok;
} // KrigingData::save()

} // namespace SUMlib

