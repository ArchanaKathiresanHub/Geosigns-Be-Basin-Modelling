//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.

// CASA
#include "ErrorHandler.h"
#include "SimpleBinDeserializer.h"

// Boost compression/stream  libraries
#include <boost/iostreams/filter/gzip.hpp>

#include <stdint.h>

#define SIMPLE_BIN_SD_DATA_TYPES 1
#include "SimpleBin.h"

using namespace std;

namespace casa
{
   static const char * dataTypeID2TypeName( SimpleDataTypeBinID typeID )
   {
      switch ( typeID )
      {
      case BoolID:   return "bool";
      case IntID:    return "int";
      case UintID:   return "uint";
      case LlongID:  return "llong";
      case RefID:    return "refID";
      case FloatID:  return "float";
      case DoubleID: return "double";
      case StringID: return "string";
      default:       return "unknown";
      }
      return "Unknown";
   }

   // functions to read values from binary stream
   template <typename T> inline bool loadValue( boost::iostreams::filtering_istream & fp, T & val )
   {
      fp.read( reinterpret_cast<char *>(&val), sizeof( T ) );
      return fp.good();
   }

   inline bool loadValue( boost::iostreams::filtering_istream & fp, bool & val )
   {
      int ival;
      bool ok = loadValue( fp, ival );
      if ( ok ) { val = ival == 0 ? false : true; }
      return ok;
   }

   inline bool loadValue( boost::iostreams::filtering_istream & fp, string & val )
   {
      uint64_t len;
      fp.read( reinterpret_cast<char*>(&len), sizeof( int64_t ) );

      if ( len < 1000 )
      {
         char buf[1000];
         fp.read( buf, len + 1 );
         val = string( buf );
      }
      else
      {
         char * buf = new char[len + 1];
         fp.read( buf, len + 1 );
         val = string( buf );
         delete[] buf;
      }
      return fp.good();
   }

   // read simple type influential like int/double/string
   template <typename T>
   inline bool loadVal( boost::iostreams::filtering_istream & fp, T & val, const string & valName, SimpleDataTypeBinID typeName )
   {
      string readValName;

      unsigned char tpID;
      bool ok = loadValue( fp, tpID );
      SimpleDataTypeBinID readTypeName = static_cast<SimpleDataTypeBinID>( tpID );
      
      if ( ok && readTypeName != typeName )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "SimpleBinDeserializer: Mismatch value type for " << valName
            << ", expected: " << dataTypeID2TypeName( typeName ) << " but read: " << dataTypeID2TypeName( readTypeName );
      }

      ok = ok ? loadValue( fp, readValName ) : ok;
      if ( ok && readValName.compare( valName ) != 0 )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "SimpleBinDeserializer: Wrong value name: " << readValName << ", expected: " << valName;
      }
      ok = ok ? loadValue( fp, val ) : ok;
      return ok;
   }

   // read vector of simple type variables like int/double/string
   template <typename T>
   inline bool loadVec( boost::iostreams::filtering_istream & fp, vector< T > & val, const string & valName, SimpleDataTypeBinID typeName )
   {
      string readValName;
      string vectorName;

      unsigned char tpID;
      bool ok = loadValue( fp, tpID );
      SimpleDataTypeBinID readTypeName = static_cast<SimpleDataTypeBinID>(tpID);

      if ( ok && readTypeName != typeName )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "SimpleBinDeserializer: Mismatch value type for " << valName
            << ", expected: " << dataTypeID2TypeName( typeName ) << " but read: " << dataTypeID2TypeName( readTypeName );
      }

      ok = ok ? loadValue( fp, readValName ) : ok;
      if ( ok && readValName.compare( valName ) != 0 )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "SimpleBinDeserializer: Wrong value name: " << readValName << ", expected: " << valName;
      }

      size_t vecSize = 0;
      ok = ok ? loadValue( fp, vecSize ) : ok;
      if ( ok )
      {
         val.clear();
         for ( size_t i = 0; i < vecSize && ok; ++i )
         {
            T vecVal;
            ok = ok ? loadValue( fp, vecVal ) : ok;
            if ( ok ) val.push_back( vecVal );
         }
      }

      return ok;
   }

   bool SimpleBinDeserializer::checkSignature( istream & ifs )
   {
      string sig;
      
      boost::iostreams::filtering_istream cfp;
      cfp.push( boost::iostreams::gzip_decompressor() );
      cfp.push( ifs );

      bool ok = loadValue( cfp, sig );
      cfp.pop(); // prevent ifs stream from closing on destruction of cfp

      ifs.seekg( 0, ios::beg );

      return ok && sig == "BinSerializer" ? true : false;
   }

   SimpleBinDeserializer::SimpleBinDeserializer( istream & fileHandle, unsigned int ver ) : m_cfile( fileHandle )
   {
      m_file.push( boost::iostreams::gzip_decompressor() );
      m_file.push( m_cfile, 65536 );

      string objType;
      string objName;
      unsigned int objVer;

      bool ok = loadObjectDescription( objType, objName, objVer );

      // read info from file about serializer      
      if ( !ok )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "SimpleBinDeserializer: Can't read serializer signature from input file";
      }

      if ( objType != "BinSerializer" )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError ) <<
            "Deserialization error. Expected object type is: BinSerializer, but in stream: " << objType;
      }

      if ( objName != "Serializer" )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError ) <<
            "Deserialization error. Expected object name: Serializer, but in stream: " << objName;
      }

      if ( ver < objVer )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "Version of object in file is newer. No forward compatibility!";
      }

      m_version = objVer;
   }

   SimpleBinDeserializer::~SimpleBinDeserializer()
   {
   }

   // Read the description of the next object in file. Works only for CasaSerializable objects
   bool SimpleBinDeserializer::checkObjectDescription( const char * objType, const string & objName, unsigned int & ver )
   {
      string readObjType;
      string readObjName;
      unsigned int readObjVer;

      bool ok = loadObjectDescription( readObjType, readObjName, readObjVer );

      if ( readObjType.compare( objType ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "Deserialization error. Expected object type is: " << objType << ", but in stream: " << readObjType;
      }

      if ( objName.compare( readObjName ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "Deserialization error. Expected object name: " << objName << ", but in stream: " << readObjName;
      }

      if ( ver < readObjVer )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "Version of object in file is newer. No forward compatibility!";
      }
      ver = readObjVer;

      return ok;
   }

   // Read the description of the next object in file. Works only for class objects
   bool SimpleBinDeserializer::loadObjectDescription( string & objType, string & objName, unsigned int & ver )
   {
      bool ok = loadValue( m_file, objType );
      ok = ok ? loadValue( m_file, objName ) : ok;
      ok = ok ? loadValue( m_file, ver     ) : ok;
     
      return ok;
   }

   bool SimpleBinDeserializer::load( bool                         & v, const string & n ) { return loadVal( m_file, v, n, BoolID   ); }
   bool SimpleBinDeserializer::load( int                          & v, const string & n ) { return loadVal( m_file, v, n, IntID    ); }
   bool SimpleBinDeserializer::load( unsigned int                 & v, const string & n ) { return loadVal( m_file, v, n, UintID   ); }
   bool SimpleBinDeserializer::load( long long                    & v, const string & n ) { return loadVal( m_file, v, n, LlongID  ); }
#ifndef _WIN32
   bool SimpleBinDeserializer::load( unsigned long long           & v, const string & n ) { return loadVal( m_file, v, n, LlongID  ); }
#endif
   bool SimpleBinDeserializer::load( ObjRefID                     & v, const string & n ) { return loadVal( m_file, v, n, RefID    ); }
   bool SimpleBinDeserializer::load( float                        & v, const string & n ) { return loadVal( m_file, v, n, FloatID  ); }
   bool SimpleBinDeserializer::load( double                       & v, const string & n ) { return loadVal( m_file, v, n, DoubleID ); }
   bool SimpleBinDeserializer::load( string                       & v, const string & n ) { return loadVal( m_file, v, n, StringID ); }

   bool SimpleBinDeserializer::load( vector< bool >               & v, const string & n ) { return loadVec( m_file, v, n, BoolID   ); }
   bool SimpleBinDeserializer::load( vector< int >                & v, const string & n ) { return loadVec( m_file, v, n, IntID    ); }
   bool SimpleBinDeserializer::load( vector< unsigned int >       & v, const string & n ) { return loadVec( m_file, v, n, UintID   ); }
   bool SimpleBinDeserializer::load( vector< long long >          & v, const string & n ) { return loadVec( m_file, v, n, LlongID  ); }
#ifndef _WIN32
   bool SimpleBinDeserializer::load( vector< unsigned long long > & v, const string & n ) { return loadVec( m_file, v, n, LlongID  ); }
#endif
   bool SimpleBinDeserializer::load( vector< ObjRefID >           & v, const string & n ) { return loadVec( m_file, v, n, RefID    ); }
   bool SimpleBinDeserializer::load( vector< float >              & v, const string & n ) { return loadVec( m_file, v, n, FloatID  ); }
   bool SimpleBinDeserializer::load( vector< double >             & v, const string & n ) { return loadVec( m_file, v, n, DoubleID ); }
   bool SimpleBinDeserializer::load( vector< string >             & v, const string & n ) { return loadVec( m_file, v, n, StringID ); }

   // Load SUMlib serializable object
   bool SimpleBinDeserializer::load( SUMlib::ISerializable & so, const string & objName )
   {
      // read from file object name and version
      string  objNameInFile;
      string  objType;
      unsigned int objVer;

      bool ok = loadObjectDescription( objType, objNameInFile, objVer );
      if ( objType.compare( "ISerializable" ) || objNameInFile.compare( objName ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "Deserialization error. Can not load SUMlib object: " << objName;
      }
      SUMlibDeserializer<SimpleBinDeserializer> sumlibDsr( *this );
      ok = ok ? so.load( &sumlibDsr, objVer ) : ok;

      return ok;
   }
}
