// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_I_SERIALIZER_H
#define SUMLIB_I_SERIALIZER_H

#include <string>

#include "BaseTypes.h"
#include "SUMlib.h"

namespace SUMlib
{
struct ISerializable;

////////////////////////////////////////////////////////////////////////////////
/// @brief  IDeserializer for loading data
///
/// @return return true if load was successful
////////////////////////////////////////////////////////////////////////////////
struct IDeserializer
{
   virtual bool load( bool& ) = 0;
   virtual bool load( int& ) = 0;
   virtual bool load( unsigned int& ) = 0;
   virtual bool load( long long& ) = 0;
   virtual bool load( unsigned long long& ) = 0;
   virtual bool load( float& ) = 0;
   virtual bool load( double& ) = 0;
   virtual bool load( std::string& ) = 0;
   
   virtual bool load( std::vector< bool >& ) = 0;
   virtual bool load( std::vector< int >& ) = 0;
   virtual bool load( std::vector< unsigned int >& ) = 0;
   virtual bool load( std::vector< long long >& ) = 0;
   virtual bool load( std::vector< unsigned long long >& ) = 0;
   virtual bool load( std::vector< float >& ) = 0;
   virtual bool load( std::vector< double >& ) = 0;
   virtual bool load( std::vector< std::string >& ) = 0;

   virtual bool load( SUMlib::ISerializable& ) = 0;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief  ISerializer for saving data
///
/// @return return true if save was successful
////////////////////////////////////////////////////////////////////////////////
struct ISerializer
{
   virtual bool save( bool ) = 0;
   virtual bool save( int ) = 0;
   virtual bool save( unsigned int ) = 0;
   virtual bool save( long long ) = 0;
   virtual bool save( unsigned long long ) = 0;
   virtual bool save( float ) = 0;
   virtual bool save( double ) = 0;
   virtual bool save( const std::string& ) = 0;

   virtual bool save( const std::vector< bool >& ) = 0;
   virtual bool save( const std::vector< int >& ) = 0;
   virtual bool save( const std::vector< unsigned int >& ) = 0;
   virtual bool save( const std::vector< long long >& ) = 0;
   virtual bool save( const std::vector< unsigned long long >& ) = 0;
   virtual bool save( const std::vector< float >& ) = 0;
   virtual bool save( const std::vector< double >& ) = 0;
   virtual bool save( const std::vector< std::string >& ) = 0;

   virtual bool save( const SUMlib::ISerializable& ) = 0;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief  ISerializable for serializing data
///
/// @return return true if serialization was successful
////////////////////////////////////////////////////////////////////////////////
struct ISerializable
{
   // returns true if ok
   virtual bool load( IDeserializer*, unsigned int version ) = 0;
   virtual bool save( ISerializer*, unsigned int version ) const = 0;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief  ISerializableVersionInfo for version info of the ISerializable version
///
/// @return return unsigned int the version number
////////////////////////////////////////////////////////////////////////////////
struct ISerializationVersion
{
   virtual unsigned int  getSerializationVersion() const = 0;
};

} // namespace SUMlib

#endif // SUMLIB_I_SERIALIZER_H
