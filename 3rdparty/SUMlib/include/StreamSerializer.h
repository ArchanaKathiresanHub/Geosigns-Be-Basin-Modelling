// Copyright 2014, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_STREAMSERIALIZER_H
#define SUMLIB_STREAMSERIALIZER_H

///////////////////////////////////////////////////////////////////////////////
//
// This pair of classes implements the ISerializer and IDeserializer interfaces
// The implementation converts the data to byte objects
// A subclass must be implemented that sends/receives the byte objects
//
///////////////////////////////////////////////////////////////////////////////

#include <cstdio>

#include "SUMlib.h"
#include "ISerializer.h"

namespace SUMlib {

// This class implements the IDeserializer interface.
// The load functions return true on success.
class INTERFACE_SUMLIB StreamDeserializer
   : public IDeserializer
{
public:
   virtual ~StreamDeserializer();

   // Method to be implemented by subclass 
   virtual bool loadBytes( void*, unsigned int ) = 0;

   // IDeserializer

   ////////////////////////////////////////////////////////////////////////////////
   /// @brief  load SUMlib::ISerializable
   ///
   /// @param  SUMlib::ISerializable
   /// @return true if ok
   ////////////////////////////////////////////////////////////////////////////////
   virtual bool load( ISerializable& serializable );

   ////////////////////////////////////////////////////////////////////////////////
   /// @brief  load boolean
   ///
   /// @param  p_bool the boolean
   /// @return true if ok
   ////////////////////////////////////////////////////////////////////////////////
   virtual bool load( bool& p_bool );

   ////////////////////////////////////////////////////////////////////////////////
   /// @brief  load int
   ///
   /// @param  p_int the int
   /// @return true if ok
   ////////////////////////////////////////////////////////////////////////////////
   virtual bool load( int& p_int );

   ////////////////////////////////////////////////////////////////////////////////
   /// @brief  load unsigned int
   ///
   /// @param  p_uInt unsigned int
   /// @return true if ok
   ////////////////////////////////////////////////////////////////////////////////
   virtual bool load( unsigned int& p_uint );

   ////////////////////////////////////////////////////////////////////////////////
   /// @brief  load long long
   ///
   /// @param  p_longlong the long long
   /// @return true if ok
   ////////////////////////////////////////////////////////////////////////////////
   virtual bool load( long long& p_longlong );

   ////////////////////////////////////////////////////////////////////////////////
   /// @brief  load unsigned long long
   ///
   /// @param  p_ulonglong the unsigned long long
   /// @return true if ok
   ////////////////////////////////////////////////////////////////////////////////
   virtual bool load( unsigned long long& p_ulonglong );

   ////////////////////////////////////////////////////////////////////////////////
   /// @brief  load float
   ///
   /// @param  p_float the float
   /// @return true if ok
   ////////////////////////////////////////////////////////////////////////////////
   virtual bool load( float& p_float );

   ////////////////////////////////////////////////////////////////////////////////
   /// @brief  load double
   ///
   /// @param  p_double the double
   /// @return true if ok
   ////////////////////////////////////////////////////////////////////////////////
   virtual bool load( double& p_double );

   ////////////////////////////////////////////////////////////////////////////////
   /// @brief  load string
   ///
   /// @param  p_string the string
   /// @return true if ok
   ////////////////////////////////////////////////////////////////////////////////
   virtual bool load( std::string& p_string );

   ////////////////////////////////////////////////////////////////////////////////
   /// @brief  load bool vector
   ///
   /// @param  p_boolVector the bool vector
   /// @return true if ok
   ////////////////////////////////////////////////////////////////////////////////
   virtual bool load( std::vector< bool >& p_boolVector );

   ////////////////////////////////////////////////////////////////////////////////
   /// @brief  load int vector
   ///
   /// @param  p_intVector the int vector
   /// @return true if ok
   ////////////////////////////////////////////////////////////////////////////////
   virtual bool load( std::vector< int >& p_intVector );

   ////////////////////////////////////////////////////////////////////////////////
   /// @brief  load unsigned int vector
   ///
   /// @param  p_intVector the int vector
   /// @return true if ok
   ////////////////////////////////////////////////////////////////////////////////
   virtual bool load( std::vector< unsigned int >& p_uintVector );

   ////////////////////////////////////////////////////////////////////////////////
   /// @brief  load long long vector
   ///
   /// @param  p_longlongVector the long long vector
   /// @return true if ok
   ////////////////////////////////////////////////////////////////////////////////
   virtual bool load( std::vector< long long >& p_longlongVector );

   ////////////////////////////////////////////////////////////////////////////////
   /// @brief  load unsigned long long vector
   ///
   /// @param  p_ulonglongVector the unsigned long long vector
   /// @return true if ok
   ////////////////////////////////////////////////////////////////////////////////
   virtual bool load( std::vector< unsigned long long >& p_ulonglongVector );

   ////////////////////////////////////////////////////////////////////////////////
   /// @brief  load float vector
   ///
   /// @param  p_floatVector the float vector
   /// @return true if ok
   ////////////////////////////////////////////////////////////////////////////////
   virtual bool load( std::vector< float >& p_floatVector );

   ////////////////////////////////////////////////////////////////////////////////
   /// @brief  load real vector
   ///
   /// @param  p_realVector the real vector
   /// @return true if ok
   ////////////////////////////////////////////////////////////////////////////////
   virtual bool load( std::vector< double >& p_realVector );

   ////////////////////////////////////////////////////////////////////////////////
   /// @brief  load StringVector
   ///
   /// @param  p_stringVec the StringVector
   /// @return true if ok
   ////////////////////////////////////////////////////////////////////////////////
   virtual bool load( std::vector< std::string >& p_stringVec );

private:
   /// Load (an array of) T
   template< typename T >
   bool loadArray( T& t, unsigned int nElem=1 )
   {
         return loadBytes( &t, sizeof( t ) * nElem );
   }

   /// Load a vector of Ts
   template< typename T >
   bool loadVec( std::vector< T >& t )
   {
      bool ok = true;
      unsigned int vecSize = 0;
      ok = ok && loadArray( vecSize );

      if ( ok )
      {
         t.clear();
         t.resize( vecSize );
         if ( vecSize > 0 )
         {
            ok = loadBytes( &t[0], sizeof( t[0] ) * vecSize );
         }
      }
      return ok;
   }

   /// Load a vector of bools (specialisation)
   bool loadVec( std::vector< bool >& t )
   {
      bool ok = true;
      unsigned int vecSize = 0;
      ok = ok && loadArray( vecSize );

      if ( ok )
      {
         t.clear();
         t.resize( vecSize );
         for ( size_t i = 0; i < vecSize && ok; ++i )
         {
            int val;
            ok = load( val );
            t[i] = val ? true : false;
         }
      }
      return ok;
   }
}; // class StreamDeserializer

// This class implements the ISerializer interface.
// The save functions return true on success.
class INTERFACE_SUMLIB StreamSerializer
   : public ISerializer
{
public:
   virtual ~StreamSerializer();

   // Method to be implemented by subclass 
   virtual bool saveBytes( const void*, unsigned int ) = 0;

   // ISerializer
   ////////////////////////////////////////////////////////////////////////////////
   /// @brief  save SUMlib::ISerializable
   ///
   /// @param  SUMlib::ISerializable
   /// @return true if ok
   ////////////////////////////////////////////////////////////////////////////////
   virtual bool save( const ISerializable& );

   ////////////////////////////////////////////////////////////////////////////////                                                                              /// @brief  save boolean                                                       ///                                                                            /// @param  p_bool the boolean                                                 /// @return true if ok                                                         ////////////////////////////////////////////////////////////////////////////////
   virtual bool save( bool );

   ////////////////////////////////////////////////////////////////////////////////                                                                              /// @brief  save int                                                           ///                                                                            /// @param  p_int the int                                                      /// @return true if ok                                                         ////////////////////////////////////////////////////////////////////////////////
   virtual bool save( int );

   ////////////////////////////////////////////////////////////////////////////////
   /// @brief  save unsigned int
   ///
   /// @param  p_uInt unsigned int
   /// @return true if ok
   ////////////////////////////////////////////////////////////////////////////////
   virtual bool save( unsigned int );

   ////////////////////////////////////////////////////////////////////////////////
   /// @brief  save long long
   ///
   /// @param  p_longlong the long long
   /// @return true if ok
   ////////////////////////////////////////////////////////////////////////////////
   virtual bool save( long long );

   ////////////////////////////////////////////////////////////////////////////////
   /// @brief  save unsigned long long
   ///
   /// @param  p_ulonglong the unsigned long long
   /// @return true if ok
   ////////////////////////////////////////////////////////////////////////////////
   virtual bool save( unsigned long long );

   ////////////////////////////////////////////////////////////////////////////////
   /// @brief  save float
   ///
   /// @param  p_float the float
   /// @return true if ok
   ////////////////////////////////////////////////////////////////////////////////
   virtual bool save( float );

   ////////////////////////////////////////////////////////////////////////////////
   /// @brief  save double
   ///
   /// @param  p_double the double
   /// @return true if ok
   ////////////////////////////////////////////////////////////////////////////////
   virtual bool save( double );

   ////////////////////////////////////////////////////////////////////////////////
   /// @brief  save string
   ///
   /// @param  p_string the string
   /// @return true if ok
   ////////////////////////////////////////////////////////////////////////////////
   virtual bool save( const std::string& );

   ////////////////////////////////////////////////////////////////////////////////
   /// @brief  save bool vector
   ///
   /// @param  p_boolVector the bool vector
   /// @return true if ok
   ////////////////////////////////////////////////////////////////////////////////
   virtual bool save( const std::vector< bool >& );

   ////////////////////////////////////////////////////////////////////////////////
   /// @brief  save int vector
   ///
   /// @param  p_intVector the int vector
   /// @return true if ok
   ////////////////////////////////////////////////////////////////////////////////
   virtual bool save( const std::vector< int >& );

   ////////////////////////////////////////////////////////////////////////////////
   /// @brief  save int vector
   ///
   /// @param  p_uintVector the int vector
   /// @return true if ok
   ////////////////////////////////////////////////////////////////////////////////
   virtual bool save( const std::vector< unsigned int >& );

   ////////////////////////////////////////////////////////////////////////////////
   /// @brief  save long long vector
   ///
   /// @param  p_longlongVector the long long vector
   /// @return true if ok
   ////////////////////////////////////////////////////////////////////////////////
   virtual bool save( const std::vector< long long >& );

   ////////////////////////////////////////////////////////////////////////////////
   /// @brief  save unsigned long long vector
   ///
   /// @param  p_ulonglongVector the unsigned long long vector
   /// @return true if ok
   ////////////////////////////////////////////////////////////////////////////////
   virtual bool save( const std::vector< unsigned long long >& );

   ////////////////////////////////////////////////////////////////////////////////
   /// @brief  save float vector
   ///
   /// @param  p_floatVector the float vector
   /// @return true if ok
   ////////////////////////////////////////////////////////////////////////////////
   virtual bool save( const std::vector< float >& );

   ////////////////////////////////////////////////////////////////////////////////
   /// @brief  save real vector
   ///
   /// @param  p_realVector the real vector
   /// @return true if ok
   ////////////////////////////////////////////////////////////////////////////////
   virtual bool save( const std::vector< double >& );

   ////////////////////////////////////////////////////////////////////////////////
   /// @brief  save string vector
   ///
   /// @param  p_stringVec the string vector
   /// @return true if ok
   ////////////////////////////////////////////////////////////////////////////////
   virtual bool save( const std::vector< std::string >& );

private:
   /// Save (an array of) T
   template< typename T >
   bool saveArray( const T& t, size_t nElem=1 )
   {
      return saveBytes( &t, sizeof( t ) * static_cast<unsigned int>( nElem ) );
   }

   /// Save a vector of T
   template< typename T >
   bool saveVec( const std::vector< T >& t )
   {
      bool ok = true;
      unsigned int vecSize = static_cast<unsigned int>( t.size() );
      ok = ok && saveArray( vecSize );

      if ( ok && vecSize > 0 )
      {
         ok = saveBytes( &t[0], sizeof( t[0] ) * vecSize );
      }
      return ok;
   }

   /// Save a vector of type bool (specialisation of template)
   bool saveVec( const std::vector< bool >& t )
   {
      bool ok = true;
      unsigned int vecSize = static_cast<unsigned int>( t.size() );
      ok = ok && saveArray( vecSize );

      if ( ok )
      {
         for ( size_t i = 0; i < vecSize && ok; ++i )
         {
            int val = t[i];
            ok = save( val );
         }
      }
      return ok;
   }
}; // class StreamSerialiser

}

#endif // SUMLIB_STREAMSERIALIZER_H

