// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.


#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <numeric>
#include <string>
#include <vector>
#include <list>

#include "Exception.h"
#include "NumericUtils.h"
#include "ParameterBounds.h"

#include "SerializerUtils.h"

using std::vector;

namespace SUMlib {

ParameterBounds::ParameterBounds()
{
}

ParameterBounds::ParameterBounds( Case const& low, Case const& high,
                                  vector<IndexList> const& catValues )
{
   initialise( low, high, catValues );
}

ParameterBounds::ParameterBounds( vector<Case> const& caseSet )
{
   if ( ! caseSet.empty() )
   {
      unsigned int nbOfPars = caseSet[0].size(); //total number of parameters
      unsigned int nbOfCatPars = caseSet[0].sizeCat(); //number of categorical parameters
      unsigned int nbOfCases = static_cast<unsigned int>( caseSet.size() ); //total number of cases

      // Initialise bounds to the values of the first case in caseSet.
      m_low = caseSet[0];
      m_high = m_low;

      // Find the bounds for each parameter from the remaining cases in caseSet.
      for ( unsigned int i = 0; i < nbOfPars; ++i )
      {
         for ( unsigned int j = 1; j < nbOfCases; ++j )
         {
            if ( caseSet[j].isSmallerThan( i, m_low ) )
            {
               m_low.copyFrom( i, caseSet[j] );
            }
            else if ( m_high.isSmallerThan( i, caseSet[j] ) )
            {
               m_high.copyFrom( i, caseSet[j] );
            }
         }
      }

      // Find the values for each categorical parameter from the cases in caseSet.
      m_catValues.resize( nbOfCatPars );
      for ( unsigned int i = 0; i < nbOfCatPars; ++i )
      {
         std::list<unsigned int> catVal;
         for ( unsigned int j = 0; j < nbOfCases; ++j )
         {
            catVal.push_back( caseSet[j].categoricalPar( i ) );
         }
         catVal.sort(); //sort list from low to high
         catVal.unique(); //remove duplicates
         size_t nbOfCatValues = catVal.size();
         m_catValues[i].resize( nbOfCatValues );
         for ( unsigned int j = 0; j < nbOfCatValues; ++j )
         {
            m_catValues[i][j] = catVal.front();
            catVal.pop_front();
         }
      }
   }
}

ParameterBounds::~ParameterBounds()
{
   // empty
}

void ParameterBounds::initialise( Case const& low, Case const& high,
                                  vector<IndexList> const& catValues )
{
   m_low = low;
   m_high = high;
   checkLowHigh();
   if ( validCatValues( catValues ) )
   {
      m_catValues = catValues;
   }
   else
   {
      setDefaultCatValues();
   }
}

void ParameterBounds::checkLowHigh() const
{
   if ( ! low().isComparableTo( high() ) )
   {
      THROW2( DimensionMismatch, "parameter bounds have different size" );
   }
   for ( unsigned int i = 0; i < size(); ++i )
   {
      if ( high().isSmallerThan( i, low() ) )
      {
         THROW2( InvalidValue, "parameter bounds minimum value larger than maximum value" );
      }
   }
}

bool ParameterBounds::validCatValues( vector<IndexList> const& catValues ) const
{
   if ( catValues.size() != sizeCat() ) return false;
   for ( unsigned int i = 0; i < sizeCat(); ++i )
   {
      if ( catValues[i].empty() ) return false; //a parameter must have a value
      if ( catValues[i][0] < low().categoricalPar( i ) ) return false;
      unsigned int lastIdx = static_cast<unsigned int>( catValues[i].size() ) - 1;
      if ( catValues[i][lastIdx] > high().categoricalPar( i ) ) return false;
      for ( unsigned int j = 1; j <= lastIdx; ++j )
      {
         if ( catValues[i][j] <= catValues[i][j - 1] ) return false; //strictly ordered list!
      }
   }
   return true;
}

void ParameterBounds::setDefaultCatValues()
{
   m_catValues.resize( sizeCat() );
   for ( unsigned int i = 0; i < sizeCat(); ++i )
   {
      unsigned int lowVal = low().categoricalPar(i);
      unsigned int nbOfCatValues = 1 + high().categoricalPar(i) - lowVal;
      m_catValues[i].resize( nbOfCatValues );
      for ( unsigned int j = 0; j < nbOfCatValues; ++j )
      {
         m_catValues[i][j] = j + lowVal;
      }
   }
}


bool ParameterBounds::load( IDeserializer* deserializer, unsigned int )
{
   bool           ok(true);

   ok = ok && deserialize( deserializer, m_low );
   ok = ok && deserialize( deserializer, m_high );

   unsigned int   nrOfIndexLists(0);
   ok = ok && deserialize( deserializer, nrOfIndexLists);

   if (ok)
   {
      m_catValues.resize(nrOfIndexLists);

      // for all indexLists
      for (size_t i(0); ok && i < nrOfIndexLists; ++i)
      {
         ok = ok && deserialize( deserializer, m_catValues[i]);
      } // for all indexLists
   }

   return ok;
} // ParameterBounds::load()


bool ParameterBounds::save( ISerializer* serializer, unsigned int ) const
{
   bool  ok(true);

   ok = ok && serialize( serializer, m_low );
   ok = ok && serialize( serializer, m_high );
   ok = ok && serialize( serializer, (unsigned int)m_catValues.size());

   // for all indexLists
   for (size_t i(0); ok && i < m_catValues.size(); ++i)
   {
      ok = ok && serialize( serializer, m_catValues[i]);
   } // for all indexLists

   return ok;
} // ParameterBounds::save()

} // namespace SUMlib
