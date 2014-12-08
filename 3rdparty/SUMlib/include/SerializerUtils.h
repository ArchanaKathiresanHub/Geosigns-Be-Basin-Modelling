// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_SERIALIZER_UTILS_H
#define SUMLIB_SERIALIZER_UTILS_H


#include "BaseTypes.h"
#include "SUMlib.h"
#include "ISerializer.h"

#include <cstddef>

namespace SUMlib
{
///////////////////////////////////////////////////////////////////////////
// SERIALIZE
///////////////////////////////////////////////////////////////////////////

// template for serialize vector<T>
template < typename T >
inline bool serialize( ISerializer* p_serializer, const std::vector<T>& p_vec )
{
   unsigned int nrOfRows = p_vec.size();
   bool         ok(p_serializer->save(nrOfRows));

   // for all rows
   for (size_t i(0); ok && i < nrOfRows; ++i)
   {
      ok = serialize( p_serializer, p_vec[i] );
   }

   return ok;
}

// default template for serialize
template < class T >
bool serialize( ISerializer* p_serializer, const T& t )
{
   return p_serializer->save( t );
}

// template for serialize RealMatrix
inline bool serialize( ISerializer* p_serializer, const RealMatrix& p_realMatrix )
{
   unsigned int nrOfRows = p_realMatrix.size();
   bool         ok(p_serializer->save(nrOfRows));

   if (ok)
   {
      // for all rows
      for (size_t i(0); ok && i < nrOfRows; ++i)
      {
         const RealVector& realVector(p_realMatrix[i]);
         ok = p_serializer->save(realVector);
      }
   }

   return ok;
}

// template for serialize RealVector
inline bool serialize( ISerializer* p_serializer, const RealVector& p_realVector )
{
   return p_serializer->save(p_realVector);
}

// This function is still needed because of backwards compatibility with runfile
// To be used for vector of unsigned int (IndexList), bool and Transformation (TransformationSet).
inline bool serialize( ISerializer* p_serializer, const std::vector<unsigned int>& p_enumVector )
{
   std::vector<int>  uVector(p_enumVector.size());

   for (size_t i(0); i < p_enumVector.size(); ++i)
   {
      uVector[i] = static_cast<int>(p_enumVector[i]);
   }

   return p_serializer->save(uVector);
}

///////////////////////////////////////////////////////////////////////////
// DESERIALIZE
///////////////////////////////////////////////////////////////////////////

// template for deserialize vector<T>
template < typename T >
inline bool deserialize( IDeserializer* p_deserializer, std::vector<T>& p_vec )
{
   unsigned int nrOfRows(0);
   bool         ok(p_deserializer->load(nrOfRows));

   if (ok)
   {
      p_vec.resize(nrOfRows);

      // for all rows
      for (size_t i(0); ok && i < nrOfRows; ++i)
      {
         ok = deserialize( p_deserializer, p_vec[i] );
      }
   }

   return ok;
}

// template for deserialize
template < class T >
bool deserialize( IDeserializer* p_deserializer, T& t )
{
   return p_deserializer->load( t );
}

// template for deserialize Matrix
inline bool deserialize( IDeserializer* p_deserializer, RealMatrix& p_realMatrix )
{
   unsigned int nrOfRows(0);
   bool         ok(p_deserializer->load(nrOfRows));

   if (ok)
   {
      p_realMatrix.resize(nrOfRows);

      // for all rows
      for (size_t i(0); ok && i < nrOfRows; ++i)
      {
         RealVector& realVector(p_realMatrix[i]);
         ok = p_deserializer->load(realVector);
      }
   }

   return ok;
}

// template for deserialize RealVector
inline bool deserialize( IDeserializer* p_deserializer, RealVector& p_realVector )
{
   p_realVector.clear();
   return p_deserializer->load(p_realVector);
}

// This function is still needed because of backwards compatibility with runfile
// To be used for vector of unsigned int (IndexList), bool and Transformation (TransformationSet).
inline bool deserialize( IDeserializer* p_deserializer, std::vector<unsigned int>& p_enumVector )
{
   std::vector<int>  uVector;
   bool              ok(p_deserializer->load(uVector));

   if (ok)
   {
      p_enumVector.resize(uVector.size());

      for (size_t i(0); i < uVector.size(); ++i)
      {
         p_enumVector[i] = static_cast<unsigned int>(uVector[i]);
      }
   }

   return ok;
}



} // namespace SUMlib

#endif // SUMLIB_SERIALIZER_UTILS_H





