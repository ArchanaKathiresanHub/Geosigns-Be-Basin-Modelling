// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include <cassert>

#include "Case.h"
#include "NumericUtils.h"
#include "SerializerUtils.h"

namespace SUMlib {

Case::Case()
{
}

Case::Case( vector<double> const& vCon )
{
   m_continuous = vCon;
}

Case::Case( vector<double> const& vCon, vector<int> const& vDis, vector<unsigned int> const& vCat )
{
   m_continuous = vCon;
   m_discrete = vDis;
   m_categorical = vCat;
}

Case::Case( unsigned int nCon, unsigned int nDis, unsigned int nCat, double conVal, int disVal, unsigned int catVal )
{
   m_continuous.assign( nCon, conVal );
   m_discrete.assign( nDis, disVal );
   m_categorical.assign( nCat, catVal );
}

Case::~Case()
{
   // empty
}

vector<double> Case::ordinalPart() const
{
   unsigned int nCon = sizeCon();
   vector<double> ordinals = m_continuous;
   unsigned int nOrd = sizeOrd();
   ordinals.resize( nOrd );
   for ( unsigned int i = nCon; i < nOrd; ++i )
   {
      ordinals[i] = double( m_discrete[i-nCon] );
   }
   return ordinals;
}

double Case::ordinalPar( unsigned int i ) const
{
   assert( i < sizeOrd() );
   unsigned int nCon = sizeCon();
   if ( i < nCon )
   {
      return m_continuous[i];
   }
   else
   {
      return double( m_discrete[i-nCon] );
   }
}

double Case::par( unsigned int i ) const
{
   assert( i < size() );
   unsigned int nCon = sizeCon();
   unsigned int nOrd = sizeOrd();
   if ( i < nCon )
   {
      return m_continuous[i];
   }
   else if ( i < nOrd )
   {
      return double( m_discrete[i-nCon] );
   }
   else
   {
      return double( m_categorical[i-nOrd] );
   }
}

bool Case::isComparableTo( Case const& c ) const
{
   return ( ( sizeCon() == c.sizeCon() ) && ( sizeDis() == c.sizeDis() ) && ( sizeCat() == c.sizeCat() ) );
}

void Case::copyFrom( unsigned int i, Case const& c )
{
   assert( isComparableTo( c ) );
   unsigned int nCon = sizeCon();
   unsigned int nOrd = sizeOrd();
   if ( i < nCon )
   {
      m_continuous[i] = c.continuousPar( i );
   }
   else if ( i < nOrd )
   {
      m_discrete[i-nCon] = c.discretePar( i-nCon );
   }
   else
   {
      m_categorical[i-nOrd] = c.categoricalPar( i-nOrd );
   }
}

bool Case::isEqualTo( unsigned int i, Case const& c ) const
{
   assert( isComparableTo( c ) );
   return IsEqualTo( par( i ), c.par( i ) );
}

bool Case::isEqualTo( Case const& c ) const
{
   unsigned int k = size(); //number of parameters
   bool equal = true;
   while ( equal && k > 0 )
   {
      equal = isEqualTo( k-1, c );
      k--;
   }
   return equal;
}

bool Case::isSmallerThan( unsigned int i, Case const& c ) const
{
   assert( isComparableTo( c ) );
   if ( isEqualTo( i, c ) )
   {
      return false;
   }
   else
   {
      return ( par( i ) < c.par( i ) );
   }
}


bool Case::load( IDeserializer* deserializer, unsigned int )
{
   bool  ok(true);
   ok = ok && deserialize(deserializer, m_continuous);
   ok = ok && deserialize(deserializer, m_discrete);
   ok = ok && deserialize(deserializer, m_categorical);

   return ok;
} // Case::load()


bool Case::save( ISerializer* serializer, unsigned int ) const
{
   bool  ok(true);

   ok = ok && serialize(serializer, m_continuous);
   ok = ok && serialize(serializer, m_discrete);
   ok = ok && serialize(serializer, m_categorical);

   return ok;
} // Case::save()

} // namespace SUMlib
