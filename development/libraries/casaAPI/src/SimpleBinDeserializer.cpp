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

namespace casa
{
   // Set of axillary function to be used in loading 

   // to reduce binary file size, change simple data types to IDs
   // this enum is duplicated in SimpleSerializer.C
   typedef enum
   {
      BoolID = 0,
      IntID,
      UintID,
      LlongID,
      RefID,
      FloatID,
      DoubleID,
      StringID,
      UnknownID
   } SimpleDataTypeBinID;

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
      }
      return "";
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

   inline bool loadValue( boost::iostreams::filtering_istream & fp, std::string & val )
   {
      uint64_t len;
      fp.read( reinterpret_cast<char*>(&len), sizeof( int64_t ) );

      if ( len < 1000 )
      {
         char buf[1000];
         fp.read( buf, len + 1 );
         val = std::string( buf );
      }
      else
      {
         char * buf = new char[len + 4];
         fp.read( buf, len + 1 );
         val = std::string( buf );
         delete[] buf;
      }
      return fp.good();
   }

   // read simple type variable like int/double/string
   template <typename T>
   inline bool loadVal( boost::iostreams::filtering_istream & fp, T & val, const std::string & valName, const char * typeName )
   {
      std::string readValName;
      std::string readTypeName;

      char tpID;
      bool ok = loadValue( fp, tpID );
      readTypeName = dataTypeID2TypeName( static_cast<SimpleDataTypeBinID>( tpID ) );
      
      if ( ok && readTypeName.compare( typeName ) != 0 )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "SimpleBinDeserializer: Mismatch value type for " << valName
            << ", expected: " << typeName << " but read: " << readTypeName;
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
   inline bool loadVec( boost::iostreams::filtering_istream & fp, std::vector< T > & val, const std::string & valName, const char * typeName )
   {
      std::string readValName;
      std::string readTypeName;
      std::string vectorName;

      char tpID;
      bool ok = loadValue( fp, tpID );
      readTypeName = dataTypeID2TypeName( static_cast<SimpleDataTypeBinID>(tpID) );

      if ( ok && readTypeName.compare( typeName ) != 0 )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "SimpleBinDeserializer: Mismatch value type for " << valName
            << ", expected: " << typeName << " but read: " << readTypeName;
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

   bool SimpleBinDeserializer::checkSignature( std::istream & ifs )
   {
      std::string sig;
      
      boost::iostreams::filtering_istream cfp;
      cfp.push( boost::iostreams::gzip_decompressor() );
      cfp.push( ifs );

      bool ok = loadValue( cfp, sig );
      cfp.pop(); // prevent ifs stream from closing on destruction of cfp

      ifs.seekg( 0, std::ios::beg );

      return ok && sig == "BinSerializer" ? true : false;
   }

   SimpleBinDeserializer::SimpleBinDeserializer( std::istream & fileHandle, unsigned int ver ) : m_cfile( fileHandle )
   {
      m_file.push( boost::iostreams::gzip_decompressor() );
      m_file.push( m_cfile, 65536 );

      std::string objType;
      std::string objName;
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
   bool SimpleBinDeserializer::checkObjectDescription( const char * objType, const std::string & objName, unsigned int & ver )
   {
      std::string readObjType;
      std::string readObjName;
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
   bool SimpleBinDeserializer::loadObjectDescription( std::string & objType, std::string & objName, unsigned int & ver )
   {
      bool ok = loadValue( m_file, objType );
      ok = ok ? loadValue( m_file, objName ) : ok;
      ok = ok ? loadValue( m_file, ver     ) : ok;
     
      return ok;
   }

   bool SimpleBinDeserializer::load( bool                        & v, const std::string & n ) { return loadVal( m_file, v, n, "bool"   ); }
   bool SimpleBinDeserializer::load( int                         & v, const std::string & n ) { return loadVal( m_file, v, n, "int"    ); }
   bool SimpleBinDeserializer::load( unsigned int                & v, const std::string & n ) { return loadVal( m_file, v, n, "uint"   ); }
   bool SimpleBinDeserializer::load( long long                   & v, const std::string & n ) { return loadVal( m_file, v, n, "llong"  ); }
#ifndef _WIN32
   bool SimpleBinDeserializer::load( unsigned long long          & v, const std::string & n ) { return loadVal( m_file, v, n, "llong"  ); }
#endif
   bool SimpleBinDeserializer::load( ObjRefID                    & v, const std::string & n ) { return loadVal( m_file, v, n, "refID"  ); }
   bool SimpleBinDeserializer::load( float                       & v, const std::string & n ) { return loadVal( m_file, v, n, "float"  ); }
   bool SimpleBinDeserializer::load( double                      & v, const std::string & n ) { return loadVal( m_file, v, n, "double" ); }
   bool SimpleBinDeserializer::load( std::string                 & v, const std::string & n ) { return loadVal( m_file, v, n, "string" ); }

   bool SimpleBinDeserializer::load( std::vector< bool >         & v, const std::string & n ) { return loadVec( m_file, v, n, "bool"   ); }
   bool SimpleBinDeserializer::load( std::vector< int >          & v, const std::string & n ) { return loadVec( m_file, v, n, "int"    ); }
   bool SimpleBinDeserializer::load( std::vector< unsigned int > & v, const std::string & n ) { return loadVec( m_file, v, n, "uint"   ); }
   bool SimpleBinDeserializer::load( std::vector< long long >    & v, const std::string & n ) { return loadVec( m_file, v, n, "llong"  ); }
#ifndef _WIN32
   bool SimpleBinDeserializer::load( std::vector< unsigned long long > & v, const std::string & n ) { return loadVec( m_file, v, n, "llong" ); }
#endif
   bool SimpleBinDeserializer::load( std::vector< ObjRefID >     & v, const std::string & n ) { return loadVec( m_file, v, n, "refID"  ); }
   bool SimpleBinDeserializer::load( std::vector< float >        & v, const std::string & n ) { return loadVec( m_file, v, n, "float"  ); }
   bool SimpleBinDeserializer::load( std::vector< double >       & v, const std::string & n ) { return loadVec( m_file, v, n, "double" ); }
   bool SimpleBinDeserializer::load( std::vector< std::string >  & v, const std::string & n ) { return loadVec( m_file, v, n, "string" ); }


   // Wrapper to save SUMlib serializable objects
   class SUMlibDeserializer : public SUMlib::IDeserializer
   {
   public:
      SUMlibDeserializer( SimpleBinDeserializer & os ) : m_iStream( os ) { ; }
      virtual ~SUMlibDeserializer() { ; }

      virtual bool load( bool               & v ) { return m_iStream.load( v, "sumlib" ); }
      virtual bool load( int                & v ) { return m_iStream.load( v, "sumlib" ); }
      virtual bool load( unsigned int       & v ) { return m_iStream.load( v, "sumlib" ); }
      virtual bool load( long long          & v ) { return m_iStream.load( v, "sumlib" ); }
      virtual bool load( unsigned long long & v ) { return m_iStream.load( v, "sumlib" ); }
      virtual bool load( float              & v ) { return m_iStream.load( v, "sumlib" ); }
      virtual bool load( double             & v ) { return m_iStream.load( v, "sumlib" ); }
      virtual bool load( std::string        & v ) { return m_iStream.load( v, "sumlib" ); }

      virtual bool load( std::vector< bool >               & vec ) { return m_iStream.load( vec, "sumlib" ); }
      virtual bool load( std::vector< int >                & vec ) { return m_iStream.load( vec, "sumlib" ); }
      virtual bool load( std::vector< unsigned int >       & vec ) { return m_iStream.load( vec, "sumlib" ); }
      virtual bool load( std::vector< long long >          & vec ) { return m_iStream.load( vec, "sumlib" ); }
      virtual bool load( std::vector< unsigned long long > & vec ) { return m_iStream.load( vec, "sumlib" ); }
      virtual bool load( std::vector< float >              & vec ) { return m_iStream.load( vec, "sumlib" ); }
      virtual bool load( std::vector< double >             & vec ) { return m_iStream.load( vec, "sumlib" ); }
      virtual bool load( std::vector< std::string >        & vec ) { return m_iStream.load( vec, "sumlib" ); }

      virtual bool load( SUMlib::ISerializable & so )
      {
         unsigned int version = 0;
         load( version );
         return so.load( this, version );
      }

   private:
      SimpleBinDeserializer & m_iStream;
   };

   // Load SUMlib serializable object
   bool SimpleBinDeserializer::load( SUMlib::ISerializable & so, const std::string & objName )
   {
      // read from file object name and version
      std::string  objNameInFile;
      std::string  objType;
      unsigned int objVer;

      bool ok = loadObjectDescription( objType, objNameInFile, objVer );
      if ( objType.compare( "ISerializable" ) || objNameInFile.compare( objName ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "Deserialization error. Can not load SUMlib object: " << objName;
      }
      SUMlibDeserializer sumlibDsr( *this );
      ok = ok ? so.load( &sumlibDsr, objVer ) : ok;

      return ok;
   }
}
