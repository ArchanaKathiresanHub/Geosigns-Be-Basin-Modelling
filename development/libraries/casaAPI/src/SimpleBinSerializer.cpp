//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

// CASA API
#include "SimpleBinSerializer.h"

// CMB API
#include "ErrorHandler.h"

// C lib
#include <stdint.h>
#include <string.h>

// Boost compression/stream  libraries
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/device/file.hpp>

namespace casa
{
   // to reduce binary file size, change simple data types to IDs
   // this enum is duplicated in SimpleDeserializer.C
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

   static SimpleDataTypeBinID typeName2DataTypeID( const char * typeName )
   {
      if (      !strcmp( typeName, "bool"   ) ) return BoolID;
      else if ( !strcmp( typeName, "int"    ) ) return IntID;
      else if ( !strcmp( typeName, "uint"   ) ) return UintID;
      else if ( !strcmp( typeName, "llong"  ) ) return LlongID;
      else if ( !strcmp( typeName, "refID"  ) ) return RefID;
      else if ( !strcmp( typeName, "float"  ) ) return FloatID;
      else if ( !strcmp( typeName, "double" ) ) return DoubleID;
      else if ( !strcmp( typeName, "string" ) ) return StringID;
      return UnknownID;
   }

   // functions to save values to binary file
   template <typename T> inline bool saveValue( boost::iostreams::filtering_ostream & fp, T val )
   {
      fp.write( reinterpret_cast<const char *>(&val), sizeof(T) );
      return fp.good();
   }

   inline bool saveValue( boost::iostreams::filtering_ostream & fp, bool val ) { return saveValue( fp, static_cast<int>(val ? 1 : 0) ); }

   inline bool saveValue( boost::iostreams::filtering_ostream & fp, const char * val )
   {
      int64_t len = strlen( val );
      fp.write( reinterpret_cast<const char*>(&len), sizeof( int64_t ) );
      fp.write( val, len + 1 );
      return fp.good();
   }

   inline bool saveValue( boost::iostreams::filtering_ostream & fp, const std::string & val )
   {
      int64_t len = val.size();
      fp.write( reinterpret_cast<const char*>(&len), sizeof( int64_t ) );
      fp.write( val.c_str(), len + 1 );
      return fp.good();
   }

   // Save one value as a new line in file with template - type name - name value\n
   template <typename T> inline bool saveValTo( boost::iostreams::filtering_ostream & fp, const char * typeName, const std::string & name, T val )
   {
      bool   ok = saveValue( fp, static_cast<unsigned char>( typeName2DataTypeID( typeName ) ) );
      ok   = ok ? saveValue( fp, name ) : ok;
      return ok ? saveValue( fp, val  ) : ok;
   }

   template < class T > inline bool saveVecTo( boost::iostreams::filtering_ostream & fp
                                             , const char                          * typeName
                                             , const std::string                   & name
                                             , const std::vector< T >              & vec
                                             )
   {
      bool ok = saveValue( fp, static_cast<unsigned char>( typeName2DataTypeID( typeName ) ) );
      ok = ok ? saveValue( fp, name ) : ok;
      ok = ok ? saveValue( fp, vec.size() ) : ok;

      for ( size_t i = 0; i < vec.size() && ok; ++i ) { ok = ok ? saveValue( fp, vec[i] ) : ok; }
      return ok;
   }

   SimpleBinSerializer::SimpleBinSerializer( const std::string & fileName, int ver ) : m_version( ver )
   {
      m_file.push( boost::iostreams::gzip_compressor() );
      m_file.push( boost::iostreams::file_sink( fileName, std::ios::out | std::ios::trunc | std::ios::binary ), 65536 );

      if ( !saveObjectDescription( "BinSerializer", "Serializer", ver ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::SerializationError ) << "Can't save object description";
      }
   }

   bool SimpleBinSerializer::save( bool                val, const std::string & vn ) { return saveValTo( m_file, "bool",   vn, val ); }
   bool SimpleBinSerializer::save( int                 val, const std::string & vn ) { return saveValTo( m_file, "int",    vn, val ); }
   bool SimpleBinSerializer::save( unsigned int        val, const std::string & vn ) { return saveValTo( m_file, "uint",   vn, val ); }
   bool SimpleBinSerializer::save( long long           val, const std::string & vn ) { return saveValTo( m_file, "llong",  vn, val ); }
#ifndef _WIN32
   bool SimpleBinSerializer::save( unsigned long long  val, const std::string & vn ) { return saveValTo( m_file, "llong",  vn, val ); }
#endif
   bool SimpleBinSerializer::save( ObjRefID            val, const std::string & vn ) { return saveValTo( m_file, "refID",  vn, val ); }
   bool SimpleBinSerializer::save( float               val, const std::string & vn ) { return saveValTo( m_file, "float",  vn, val ); }
   bool SimpleBinSerializer::save( double              val, const std::string & vn ) { return saveValTo( m_file, "double", vn, val ); }
   bool SimpleBinSerializer::save( const std::string & val, const std::string & vn ) { return saveValTo( m_file, "string", vn, val ); }

   bool SimpleBinSerializer::save( const std::vector< bool >         & vec, const std::string & vn ) { return saveVecTo( m_file, "bool", vn, vec ); }
   bool SimpleBinSerializer::save( const std::vector< int >          & vec, const std::string & vn ) { return saveVecTo( m_file, "int",  vn, vec ); }
   bool SimpleBinSerializer::save( const std::vector< unsigned int > & vec, const std::string & vn ) { return saveVecTo( m_file, "uint", vn, vec ); }
#ifndef _WIN32
   bool SimpleBinSerializer::save( const std::vector< unsigned long long > & vec, const std::string & vn ) { return saveVecTo( m_file, "llong",  vn, vec ); }
#endif
   bool SimpleBinSerializer::save( const std::vector< long long >    & vec, const std::string & vn ) { return saveVecTo( m_file, "llong",  vn, vec ); }
   bool SimpleBinSerializer::save( const std::vector< ObjRefID >     & vec, const std::string & vn ) { return saveVecTo( m_file, "refID",  vn, vec ); }
   bool SimpleBinSerializer::save( const std::vector< float >        & vec, const std::string & vn ) { return saveVecTo( m_file, "float",  vn, vec ); }
   bool SimpleBinSerializer::save( const std::vector< double >       & vec, const std::string & vn ) { return saveVecTo( m_file, "double", vn, vec ); }
   bool SimpleBinSerializer::save( const std::vector< std::string >  & vec, const std::string & vn ) { return saveVecTo( m_file, "string", vn, vec ); }

   // Save CasaSerializable object
   bool SimpleBinSerializer::save( const CasaSerializable & so, const std::string & objName )
   {
      saveObjectDescription( so.typeName(), objName, so.version() );
      return m_file.good() ? so.save( *this, m_version ) : false;
   }

   bool SimpleBinSerializer::saveObjectDescription( const std::string & objType, const std::string & objName, unsigned int ver )
   {
      bool ok = saveValue( m_file, objType );
      ok = ok ? saveValue( m_file, objName ) : ok;
      ok = ok ? saveValue( m_file, ver     ) : ok;

      return ok;
   }

   // Wrapper to save SUMlib serialazable objects
   class SUMlibSerializer : public SUMlib::ISerializer
   {
   public:
      SUMlibSerializer( SimpleBinSerializer & os ) : m_oStream( os ) { ; }
      virtual ~SUMlibSerializer() { ; }

      virtual bool save( bool                v ) { return m_oStream.save( v, "sumlib" ); }
      virtual bool save( int                 v ) { return m_oStream.save( v, "sumlib" ); }
      virtual bool save( unsigned int        v ) { return m_oStream.save( v, "sumlib" ); }
      virtual bool save( long long           v ) { return m_oStream.save( v, "sumlib" ); }
      virtual bool save( unsigned long long  v ) { return m_oStream.save( v, "sumlib" ); }
      virtual bool save( float               v ) { return m_oStream.save( v, "sumlib" ); }
      virtual bool save( double              v ) { return m_oStream.save( v, "sumlib" ); }
      virtual bool save( const std::string & v ) { return m_oStream.save( v, "sumlib" ); }

      virtual bool save( const std::vector< bool >               & vec ) { return m_oStream.save( vec, "sumlib" ); }
      virtual bool save( const std::vector< int >                & vec ) { return m_oStream.save( vec, "sumlib" ); }
      virtual bool save( const std::vector< unsigned int >       & vec ) { return m_oStream.save( vec, "sumlib" ); }
      virtual bool save( const std::vector< long long >          & vec ) { return m_oStream.save( vec, "sumlib" ); }
      virtual bool save( const std::vector< unsigned long long > & vec ) { return m_oStream.save( vec, "sumlib" ); }
      virtual bool save( const std::vector< float >              & vec ) { return m_oStream.save( vec, "sumlib" ); }
      virtual bool save( const std::vector< double >             & vec ) { return m_oStream.save( vec, "sumlib" ); }
      virtual bool save( const std::vector< std::string >        & vec ) { return m_oStream.save( vec, "sumlib" ); }

      virtual bool save( const SUMlib::ISerializable & so )
      {
         const SUMlib::ISerializationVersion* soVersion = dynamic_cast<const SUMlib::ISerializationVersion*>(&so);
         unsigned int version = soVersion ? soVersion->getSerializationVersion() : 0;
         save( version );
         return so.save( this, version );
      }

   private:
      SimpleBinSerializer & m_oStream;
   };

   // Save SUMlib serializable object
   bool SimpleBinSerializer::save( const SUMlib::ISerializable & so, const std::string & objName )
   {
      const SUMlib::ISerializationVersion * soVersion = dynamic_cast<const SUMlib::ISerializationVersion*>(&so);
      unsigned int version = soVersion ? soVersion->getSerializationVersion() : 0;

      bool ok = saveObjectDescription( "ISerializable", objName, version );
      
      SUMlibSerializer sumlibSer( *this );
      ok = ok ? so.save( &sumlibSer, version ) : ok;
      
      return ok;
   }
}
