// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_DATASTRUCTUREUTILS_IMPL_H
#define SUMLIB_DATASTRUCTUREUTILS_IMPL_H

#include <ostream>
#include <vector>

namespace SUMlib {
template<typename T>
std::ostream& operator<< ( std::ostream&s, std::vector<T> const& v )
{
   s << "[";
   if ( v.size() )
   {
      unsigned int i;
      for ( i = 0; i < v.size() - 1; ++i )
      {
         s<<v[i]<<";";
      }
      s<<v[i];
   }
   s<<"]";
   return s;
}

template<typename T>
std::ostream& operator<<( std::ostream&s, std::vector< std::vector<T> > const& m )
{
   s<<"{";
   unsigned int i;
   for ( i = 0;i < m.size(); ++i )
   {
      s<<m[i];
   }
   s<<"}";
   return s;
}

template<typename T>
void MapActive( std::vector<T> const& src, std::vector<bool> const& active, std::vector<T> &dst )
{
   const size_t n(src.size());
   dst.clear();
   dst.reserve(n);
   for ( size_t i=0; i < n; ++i )
   {
      if ( active[i] )
      {
         dst.push_back( src[i] );
      }
   }
}

template<typename T>
void MapActive( std::vector<T> const& src, IndexList const& active, std::vector<T> &dst )
{
   const size_t n(active.size());
   dst.resize(n);
   for ( size_t i=0; i < n; ++i )
   {
      dst[i] = src[active[i]];
   }
}

} // namespace SUMlib

#endif // SUMLIB_DATASTRUCTUREUTILS_IMPL_H
