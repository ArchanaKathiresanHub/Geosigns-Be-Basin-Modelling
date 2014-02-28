// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include <algorithm>
#include <cassert>
#include <cmath>
#include <vector>

#include "BaseTypes.h"
#include "Exception.h"
#include "NumericUtils.h"
#include "ParameterSpace.h"
#include "SerializerUtils.h"

using std::vector;

namespace SUMlib {

// overwrite of template in SerializerUtils
static bool serialize( ISerializer*, const ParameterSpace::TransformationSet& p_enumVector );
static bool deserialize( IDeserializer*, ParameterSpace::TransformationSet& );


ParameterSpace::ParameterSpace( Case const& origLow, Case const& origHigh,
                                TransformationSet const& tr ) :
   m_bounds(),
   m_tr_bounds()
{
   initialise( origLow, origHigh, tr );
}

ParameterSpace::~ParameterSpace()
{
   // empty
}

void ParameterSpace::initialise( Case const& origLow, Case const& origHigh,
                                 TransformationSet const& tr )
{
   // Set number of original dummy parameters for each categorical parameter
   unsigned int nbOfCatPars = origLow.sizeCat();
   m_nbOfOrigDummyPars.resize( nbOfCatPars );
   for ( unsigned int i = 0; i < nbOfCatPars; ++i )
   {
      assert( origLow.categoricalPar( i ) == 0 );
      m_nbOfOrigDummyPars[i] = origHigh.categoricalPar( i );
   }

   // Fill a vector with inverse transformations
   m_tr = tr;
   m_tr_inv.assign( m_tr.size(), ParameterSpace::NoTransform );
   for ( unsigned int k = 0; k < m_tr.size(); ++k )
   {
      switch( m_tr[k] )
      {
         case ParameterSpace::Log10Transform:
            m_tr_inv[k] = ParameterSpace::Pwr10Transform;
            break;
         case ParameterSpace::Pwr10Transform:
            m_tr_inv[k] = ParameterSpace::Log10Transform;
            break;
         case ParameterSpace::SqrtTransform:
            m_tr_inv[k] = ParameterSpace::SqrTransform;
            break;
         case ParameterSpace::SqrTransform:
            m_tr_inv[k] = ParameterSpace::SqrtTransform;
            break;
         case ParameterSpace::NoTransform: // fall-through intentional
         default:
            break;
      }
   }
}

void ParameterSpace::setBounds( ParameterBounds const& bounds )
{
   if ( ( ! m_tr.empty() ) && ( m_tr.size() != bounds.sizeCon() ) )
   {
      THROW2( DimensionMismatch, "Number of specified parameter transformations does not match parameter size" );
   }
   if ( bounds.sizeCat() != m_nbOfOrigDummyPars.size() )
   {
      THROW2( DimensionMismatch, "Number of specified categorical parameters does not match original size" );
   }

   m_bounds = bounds;

   // Calculate the transformed lower bounds of the continuous parameters.
   Case lowCase = low();
   vector<double> conLow = lowCase.continuousPart();
   transform( m_tr, conLow );
   lowCase.setContinuousPart( conLow );

   // Calculate the transformed upper bounds of the continuous parameters.
   Case highCase = high();
   vector<double> conHigh = highCase.continuousPart();
   transform( m_tr, conHigh );
   highCase.setContinuousPart( conHigh );

   // Combine the transformed bounds into m_tr_bounds.
   m_tr_bounds.initialise( lowCase, highCase, vector<vector<unsigned int> >() );

   listPreparedParIdx();
}

void ParameterSpace::scale( ParameterBounds const& bounds, Case &c )
{
   assert( c.sizeOrd() == bounds.sizeOrd() );
   vector<double> scaledConPars( c.sizeCon(), -1.0 );
   for ( unsigned int k = 0; k < c.sizeCon(); ++k )
   {
      if ( ! bounds.areEqual( k ) )
      {
         double shiftedConPar = c.continuousPar( k ) - bounds.low().ordinalPar( k );
         scaledConPars[k] += 2 * shiftedConPar / bounds.rangeOrd( k );
      }
   }
   c.setContinuousPart( scaledConPars );
}

void ParameterSpace::removeFixed( Case &c ) const
{
   unsigned int nOrd = sizeOrd();
   unsigned int nCat = sizeCat();
   // Check whether the discrete parameters of c have been converted to continuous parameters.
   assert( c.sizeDis() == 0 );
   assert( c.sizeOrd() == nOrd );
   assert( c.sizeCat() == nCat );

   // Store the values of only the non-fixed parameters of c.
   vector<double> conPars;
   conPars.reserve( nOrd );
   for ( unsigned int k = 0; k < nOrd; ++k )
   {
      if ( ! isFixed( k ) )
      {
         conPars.push_back( c.continuousPar( k ) );
      }
   }
   vector<unsigned int> catPars;
   catPars.reserve( nCat );
   for ( unsigned int k = 0; k < nCat; ++k )
   {
      if ( ! isFixed( k + nOrd ) )
      {
         catPars.push_back( c.categoricalPar( k ) );
      }
   }

   // Finally reset c.
   c.setContinuousPart( conPars );
   c.setCategoricalPart( catPars );
   assert( c.sizeDis() == 0 );
   assert( c.size() == m_preparedParIdx.size() );
}

void ParameterSpace::addFixed( Case &c ) const
{
   unsigned int nOrd = sizeOrd();
   unsigned int nCat = sizeCat();
   // Check whether the discrete parameters of c have been converted to continuous parameters.
   assert( c.sizeDis() == 0 );
   assert( c.size() == m_preparedParIdx.size() );

   // Ordinals have been scaled to [-1, 1]; initialise them with scaled lower bounds (= -1.0).
   // Categoricals are never scaled, so they can be initialised with the lower bounds.
   Case cNew( vector<double>( nOrd, -1.0 ), vector<int>(), low().categoricalPart() );

   // Insert the values of the non-fixed parameters of c into cNew
   unsigned int j = 0;
   for ( unsigned int k = 0; k < nOrd; ++k )
   {
      if ( ! isFixed( k ) )
      {
         cNew.setContinuousPar( k, c.continuousPar( j++ ) );
      }
   }
   j = 0;
   for ( unsigned int k = 0; k < nCat; ++k )
   {
      if ( ! isFixed( k + nOrd ) )
      {
         cNew.setCategoricalPar( k, c.categoricalPar( j++ ) );
      }
   }

   // Finally copy cNew to c.
   c = cNew;
   assert( c.sizeDis() == 0 );
   assert( c.sizeOrd() == nOrd );
   assert( c.sizeCat() == nCat );
}

void ParameterSpace::prepare( Case const& cRaw, RealVector &v ) const
{
   Case c = cRaw;
   assert( c.isComparableTo( low() ) );

   // Apply transformations for the continuous parameters.
   vector<double> conPars = c.continuousPart();
   transform( m_tr, conPars );

   // Convert discrete parameters to continuous parameters.
   conPars.resize( sizeOrd() );
   for ( unsigned int k = sizeCon(); k < sizeOrd(); ++k )
   {
      conPars[k] = c.ordinalPar( k );
   }
   c.setContinuousPart( conPars ); //copy extended continuous parameters to c
   c.setDiscretePart( vector<int>() ); //strip all discrete parameters

   // Scale ordinal (now continuous only) parameters to [-1:1].
   scale( m_tr_bounds, c );

   // Remove the values corresponding to parameters with equal bounds (low = high).
   removeFixed( c );

   // Convert each categorical parameter value to binary dummy parameters,
   // and add those to the end of the real vector (being the prepared case).
   v = c.continuousPart();
   addBinaryValues( c, v );
   assert( v.size() == m_preparedProxyParIdx.size() );
}

void ParameterSpace::unprepare( RealVector const& v, Case &c ) const
{
   // Strip dummy parameter values to get the ordinals back, and convert
   // the binary dummy values to the original categorical parameter values.
   vector<double> vOrd = v;
   vOrd.resize( v.size() - nbOfDummyPars() );
   c.setContinuousPart( vOrd );
   c.setDiscretePart( vector<int>() );
   c.setCategoricalPart( getCatParValues( v ) ); //from binaries to categoricals

   // Add the values corresponding to parameters with equal bounds (low = high).
   addFixed( c );

   // Revert scaling of continuous parameters (i.e. of all ordinals at this stage).
   vector<double> conPars = c.continuousPart();
   for ( unsigned int k = 0; k < m_tr_bounds.sizeOrd(); ++k )
   {
      conPars[k] = m_tr_bounds.low().ordinalPar( k );
      conPars[k] += ( c.continuousPar( k ) + 1.0 ) * m_tr_bounds.rangeOrd( k ) / 2.0;
   }
   c.setContinuousPart( conPars );

   // Get back the discrete parameters with the corresponding discrete values.
   vector<int> disPars( sizeDis() );
   for ( unsigned int k = 0; k < sizeDis(); ++k )
   {
      double d = conPars[k + sizeCon()];
      disPars[k] = int( d > 0.0 ? d + 0.5 : d - 0.5 ); //round off to nearest discrete value
   }
   c.setDiscretePart( disPars ); //copy the converted values to the discrete part of c
   conPars.resize( sizeCon() ); //strip continuous parameters that have just been converted

   // Apply inverse transformations for the continuous parameters.
   transform( m_tr_inv, conPars );
   c.setContinuousPart( conPars );
   assert( c.isComparableTo( low() ) );
}

void ParameterSpace::prepare( std::vector<Case> const& caseSet, RealMatrix &vSet ) const
{
   vSet.resize( caseSet.size() );
   for ( unsigned int i = 0; i < caseSet.size(); ++i )
   {
      prepare( caseSet[i], vSet[i] );
   }
}

void ParameterSpace::unprepare( RealMatrix const& vSet, std::vector<Case> &caseSet ) const
{
   caseSet.resize( vSet.size() );
   for ( unsigned int i = 0; i < vSet.size(); ++i )
   {
      unprepare( vSet[i], caseSet[i] );
   }
}

namespace {
struct doTransform
{
   double operator()( double p, ParameterSpace::TransformType const& tr )
   {
      switch ( tr )
      {
         case ParameterSpace::NoTransform:
            return p;
         case ParameterSpace::Log10Transform:
            if ( p <= 0 )
            {
               THROW2(InvalidValue, "Cannot take log of non-positive value" );
            }
            return log10( p );
            break;
         case ParameterSpace::Pwr10Transform: //inverse transform of Log10Transform
            return pow( 10.0, p );
            break;
         case ParameterSpace::SqrtTransform:
            if ( p < 0 )
            {
               THROW2(InvalidValue, "Cannot take square root of negative value" );
            }
            return sqrt( p );
            break;
         case ParameterSpace::SqrTransform: //inverse transform of SqrtTransform
            return p * p ;
            break;
         default:
            THROW2(InvalidValue, "Unknown transformation type" );
      }
   }
};
} // anon. namespace

void ParameterSpace::transform( TransformationSet const& tr, vector<double> &conPars )
{
   if ( tr.empty() )
   {
      return;
   }
   if ( tr.size() != conPars.size() )
   {
      THROW2( DimensionMismatch, "Only continuous parameters can be transformed" );
   }
   std::transform( conPars.begin(), conPars.end(), tr.begin(), conPars.begin(), doTransform() );
}

void ParameterSpace::listPreparedParIdx()
{
   // Create a list of indices corresponding to the prepared parameters.
   m_preparedParIdx.clear();
   m_preparedParIdx.reserve( size() );
   for ( unsigned int i = 0; i < size(); ++i )
   {
      if ( ! isFixed( i ) )
      {
         m_preparedParIdx.push_back( i );
      }
   }

   // Determine the maximum number (totalSize) of parameters that can show up in a polynomial.
   unsigned int totNbOfOrigDummyPars = 0;
   for ( unsigned int i = 0; i < sizeCat(); ++i )
   {
      totNbOfOrigDummyPars += m_nbOfOrigDummyPars[i];
   }
   const unsigned int totalSize = sizeOrd() + totNbOfOrigDummyPars;

   // Create a list of indices corresponding to the prepared parameters, including dummy parameters.
   m_preparedProxyParIdx.clear();
   m_preparedProxyParIdx.reserve( totalSize );
   unsigned int k;
   for ( k = 0; k < sizeOrd(); ++k )
   {
      if ( ! isFixed( k ) )
      {
         m_preparedProxyParIdx.push_back( k );
      }
   }
   assert( k == sizeOrd() );

   for ( unsigned int i = 0; i < sizeCat(); ++i )
   {
      unsigned int kNext = k + m_nbOfOrigDummyPars[i];
      if ( ! isFixed( sizeOrd() + i ) )
      {
         IndexList catVals = catValues( i );
         k += ( catVals[1] - 1 ); //catVals[0] is reference value -> no dummy par.
         m_preparedProxyParIdx.push_back( k++ );
         for ( unsigned j = 2; j < catVals.size(); ++j )
         {
            k += ( catVals[j] - catVals[j - 1] - 1 );
            m_preparedProxyParIdx.push_back( k++ );
         }
      }
      assert( k <= kNext );
      k = kNext;
   }
   assert( k == totalSize );
}

unsigned int ParameterSpace::nbOfNonFixedOrdinalPars() const
{
   unsigned int nbOfPreparedOrdinals = 0;
   for ( unsigned int i = 0; i < sizeOrd(); ++i )
   {
      if ( ! isFixed( i ) )
      {
         nbOfPreparedOrdinals++;
      }
   }
   return nbOfPreparedOrdinals;
}

void ParameterSpace::convert2origProxyIdx( IndexList& indexes ) const
{
   // Shift the supplied local indices to the right places
   for ( unsigned int i = 0; i < indexes.size(); ++i )
   {
      indexes[i] = m_preparedProxyParIdx[ indexes[i] ];
   }
}

void ParameterSpace::unprepare( IndexList& indexes ) const
{
   for ( unsigned int i = 0; i < indexes.size(); ++i )
   {
      indexes[i] = m_preparedParIdx[ indexes[i] ];
   }
}

void ParameterSpace::prepare( IndexList& indexes ) const
{
   unsigned int k = 0;
   for ( unsigned int i = 0; i < indexes.size(); ++i )
   {
      unsigned int idx = indexes[i];
      if ( ! isFixed( idx ) )
      {
         unsigned int m;
         for ( m = 0; m < m_preparedParIdx.size(); ++m )
         {
            if ( m_preparedParIdx[m] == idx ) break;
            }
         assert( m_preparedParIdx[m] == idx );
         assert( i >= k );
         indexes[ k++ ] = m;
      }
   }
   indexes.resize( k );
}

void ParameterSpace::prepare( Partition& partition ) const
{
   if ( partition.empty() )
   {
      return;
   }
   assert( partition.size() == size() );
   Partition part( m_preparedProxyParIdx.size() );

   unsigned int k = 0;
   for ( unsigned int i = 0; i < sizeOrd(); ++i )
   {
      if ( ! isFixed( i ) )
      {
         part[k++] = partition[i];
      }
   }
   for ( unsigned int i = 0; i < sizeCat(); ++i )
   {
      unsigned int parIdx = sizeOrd() + i;
      unsigned int nbOfDummyPars = catValues( i ).size() - 1;
      if ( ! isFixed( parIdx ) )
      {
         assert( nbOfDummyPars > 0 );
         for ( unsigned int j = 0; j < nbOfDummyPars; ++j )
         {
            part[k++] = partition[parIdx];
         }
      }
   }
   assert( k == m_preparedProxyParIdx.size() );
   partition = part;
}

void ParameterSpace::addBinaryValues( Case const& c, RealVector &v ) const
{
   unsigned int vIdx = v.size();
   unsigned int newSize = vIdx + nbOfDummyPars();
   v.resize( newSize, 0.0 );
   unsigned int catParIdx = 0;
   for ( unsigned int i = 0; i < sizeCat(); ++i )
   {
      IndexList catVals = catValues( i );
      if ( catVals.size() > 1 ) //otherwise cat. par is frozen
      {
         // Precondition: frozen parameters are already removed from c!
         unsigned int val = c.categoricalPar( catParIdx++ );

         unsigned int valIdx;
         for ( valIdx = 0; valIdx < catVals.size(); ++valIdx )
         {
            if ( catVals[valIdx] == val ) break;
         }
         assert( valIdx < catVals.size() );
         if ( valIdx > 0 ) //otherwise all binary values must remain zero
         {
            v[vIdx + valIdx - 1] = 1.0;
         }
         vIdx += ( catVals.size() - 1 );
      }
   }
   assert( vIdx == newSize );
}

RealVector ParameterSpace::getCatParValuesFromPreparedCase( RealVector const& v ) const
{
   vector<unsigned int> catValues = getCatParValues( v );
   RealVector convertedCatValues( catValues.size() );
   for ( unsigned int i = 0; i < catValues.size(); ++i )
   {
      convertedCatValues[i] = double( catValues[i] );
   }
   return convertedCatValues;
}

vector<unsigned int> ParameterSpace::getCatParValues( RealVector const& v ) const
{
   vector<unsigned int> catPars;
   catPars.reserve( sizeCat() );
   unsigned int vIdx = v.size() - nbOfDummyPars();
   for ( unsigned int i = 0; i < sizeCat(); ++i )
   {
      IndexList catVals = catValues( i );
      if ( catVals.size() > 1 ) //otherwise cat. par is frozen
      {
         unsigned int dummyIdx;
         for ( dummyIdx = 0; dummyIdx < catVals.size() - 1; ++dummyIdx )
         {
            if ( v[vIdx + dummyIdx] > 0.5 ) break;
         }
         if ( dummyIdx == catVals.size() - 1 ) //all binary values are zero!
         {
            catPars.push_back( catVals[0] );
         }
         else //binary value of 1.0 found!
         {
            catPars.push_back( catVals[dummyIdx + 1] );
         }
         vIdx += ( catVals.size() - 1 );
      }
   }
   assert( vIdx == v.size() );
   return catPars;
}

unsigned int ParameterSpace::nbOfDummyPars() const
{
   unsigned int nDummyPars = 0;
   for ( unsigned int i = 0; i < sizeCat(); ++i )
   {
      nDummyPars += ( catValues(i).size() - 1 );
   }
   return nDummyPars;
}


bool ParameterSpace::load( IDeserializer* deserializer, unsigned int )
{
   bool  ok(true);

   ok = ok && deserialize(deserializer, m_tr);
   ok = ok && deserialize(deserializer, m_tr_inv);
   ok = ok && deserialize(deserializer, m_bounds );
   ok = ok && deserialize(deserializer, m_tr_bounds );
   ok = ok && deserialize(deserializer, m_preparedParIdx);
   ok = ok && deserialize(deserializer, m_preparedProxyParIdx);
   ok = ok && deserialize(deserializer, m_nbOfOrigDummyPars);

   return ok;
} // ParameterSpace::load()


bool ParameterSpace::save( ISerializer* serializer, unsigned int ) const
{
   bool  ok(true);

   ok = ok && serialize(serializer, m_tr);
   ok = ok && serialize(serializer, m_tr_inv);
   ok = ok && serialize(serializer, m_bounds );
   ok = ok && serialize(serializer, m_tr_bounds );
   ok = ok && serialize(serializer, m_preparedParIdx);
   ok = ok && serialize(serializer, m_preparedProxyParIdx);
   ok = ok && serialize(serializer, m_nbOfOrigDummyPars);

   return ok;
} // ParameterSpace::save()

// This function is still needed because of backwords compatibility with runfile
bool serialize( ISerializer* p_serializer, const ParameterSpace::TransformationSet& p_enumVector )
{
   std::vector<int>  uVector(p_enumVector.size());

   for (size_t i(0); i < p_enumVector.size(); ++i)
   {
      uVector[i] = static_cast<int>(p_enumVector[i]);
   }

   return p_serializer->save(uVector);
}

// This function is still needed because of backwords compatibility with runfile
// To be used for vector of unsigned int (IndexList), bool and Transformation (TransformationSet).
bool deserialize( IDeserializer* p_deserializer, ParameterSpace::TransformationSet& p_enumVector )
{
   std::vector<int>  uVector;
   bool              ok(p_deserializer->load(uVector));

   if (ok)
   {
      p_enumVector.resize(uVector.size());

      for (size_t i(0); i < uVector.size(); ++i)
      {
         p_enumVector[i] = static_cast<ParameterSpace::TransformType>(uVector[i]);
      }
   }

   return ok;
}

} // namespace SUMlib

