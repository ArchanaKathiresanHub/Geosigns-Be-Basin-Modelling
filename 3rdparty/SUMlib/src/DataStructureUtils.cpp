// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <vector>

#include "DataStructureUtils.h"
#include "Exception.h"
#include "SUMlib.h"

using std::vector;

namespace SUMlib {

std::stringstream& IntVectorToString( std::stringstream& s, intVector const& v )
{
   s << std::setprecision(10);
   s << v;
   return s;
}

std::stringstream& IntMatrixToString( std::stringstream& s, intMatrix const& m )
{
   s << std::setprecision(10);
   s << "{";
   size_t num_i = m.size();
   for ( size_t i = 0; i < num_i; ++i )
   {
      s <<  m[i];
   }
   s << "}";

   return s;
}

std::stringstream& DblVectorToString( std::stringstream& s, dblVector const& v )
{
   s << std::setprecision(10);
   s << v;

   return s;
}

std::stringstream& DblMatrixToString( std::stringstream& s, dblMatrix const& m )
{
   s << std::setprecision(10);
   s << "{";
   size_t num_i = m.size();
   for ( size_t i = 0; i < num_i; ++i )
   {
      s << m[i];
   }
   s << "}";

   return s;
}

/**
 * SelectSort is of O(n^2), suitable for small data sets (n < 50).
 * Possible future improvement: insertSort of O(n^2) is faster on average.
 * Algorithms of O(n*log(n)) like quickSort are faster for large data sets (n > 50).
 */
void SelectSort( vector<unsigned int>& index, vector<double>& values )
{
   if ( index.size() != values.size() )
   {
      THROW2( DimensionMismatch, "index and values" );
   }

   // iterators into the values vector
   vector<double>::iterator vi0 = values.begin();

   // iterators into the index vector
   vector<unsigned int>::iterator ii0 = index.begin();

   while ( vi0 != values.end() )
   {
      vector<double>::iterator vi = std::max_element( vi0, values.end() );
      vector<unsigned int>::iterator ii = ii0 + std::distance( vi0, vi );

      std::swap( *vi0, *vi );
      std::swap( *ii0, *ii );

      ++vi0;
      ++ii0;
   }
}

void GetExtremes( dblVector const& vec, double& min, double& max)
{
   // [TODO] Throw an exception?
   if ( vec.size() == 0 )
   {
      min = 0.0;
      max = 0.0;
   }
   else
   {
      min = *( std::min_element( vec.begin(), vec.end() ) );
      max = *( std::max_element( vec.begin(), vec.end() ) );
   }
}

IndexList::iterator IndexListItem( IndexList & list, unsigned int index )
{
   return std::find(list.begin(),list.end(),index);
}

bool IndexListContains( IndexList const& list, unsigned int index )
{
   return ( std::find(list.begin(),list.end(),index)  != list.end() );
}

void IndexListInsert( IndexList& list, unsigned int index )
{
   if ( ! IndexListContains( list, index ) )
   {
      list.push_back(index);
      std::sort(list.begin(),list.end());
   }
}
void IndexListRemove( IndexList& list, unsigned int index )
{
   IndexList::iterator it = IndexListItem( list, index );
   if ( it != list.end() )
   {
      list.erase( it );
   }
}

void IndexListFill( IndexList& list, unsigned int N )
{
   list.resize( N );
   for ( unsigned int k = 0; k < N; ++k )
   {
      list[k]=k;
   }
}

// explicit instantiation for unsigned int and double
template std::ostream& operator<< <unsigned int>( std::ostream& s, std::vector<unsigned int> const& v );
template std::ostream& operator<< <double>( std::ostream& s, std::vector<double> const& v );

// explicit instantiation for unsigned int and double
template std::ostream& operator<< <unsigned int>( std::ostream& s, std::vector<std::vector<unsigned int> > const& m );
template std::ostream& operator<< <double>( std::ostream& s, std::vector<std::vector<double> > const& m );

} // namespace SUMlib
