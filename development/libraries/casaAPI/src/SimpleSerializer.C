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
#include "SimpleSerializer.h"

// CMB API
#include "ErrorHandler.h"

// C lib
#include <stdint.h>
#include <string.h>

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
   template <typename T> inline bool saveBinValue( std::ofstream & fp, T val )
   {
      fp.write( reinterpret_cast<const char *>(&val), sizeof( T ) );
      return fp.good();
   }

   inline bool saveBinValue( std::ofstream & fp, bool val )
   {
      return val ? saveBinValue( fp, static_cast<int>(1) ) : saveBinValue( fp, static_cast<int>(0) );
   }

   inline bool saveBinValue( std::ofstream & fp, const char * val )
   {
      int16_t len = strlen( val );
      fp.write( reinterpret_cast<const char*>(&len), sizeof( int16_t ) );
      fp.write( val, len + 1 );
      return fp.good();
   }

   inline bool saveBinValue( std::ofstream & fp, const std::string & val )
   {
      int16_t len = val.size();
      fp.write( reinterpret_cast<const char*>(&len), sizeof( int16_t ) );
      fp.write( val.c_str(), len + 1 );
      return fp.good();
   }

   // functions to save values to txt file
   template <typename T> inline bool saveValue( std::ofstream & fp, T val )
   {
      fp << val;
      return fp.good();
   }

   inline bool saveValue( std::ofstream & fp, double val )
   {
      fp << std::scientific << val;
      return fp.good();
   }

   inline bool saveValue( std::ofstream & fp, float val )
   {
      fp << std::scientific << val;
      return fp.good();
   }

   inline bool saveValue( std::ofstream & fp, bool val )
   {
      fp << (val ? "true" : "false");
      return fp.good();
   }

   inline bool saveValue( std::ofstream & fp, const std::string & val )
   {
      fp << "\"" << val << "\"";
      return fp.good();
   }

   inline bool saveValue( std::ofstream & fp, const char * val )
   {
      fp << "\"" << std::string( val ) << "\"";
      return fp.good();
   }

   // Save one value as a new line in file with template - type name - name value\n
   template <typename T> inline bool saveValTo( std::ofstream & fp, bool isBin, const char * typeName, const std::string & name, T val )
   {
      bool ok = true;
      if ( isBin )
      {
         ok = ok ? saveBinValue( fp, static_cast<unsigned char>( typeName2DataTypeID( typeName ) ) ) : ok;
         ok = ok ? saveBinValue( fp, name     ) : ok;
         ok = ok ? saveBinValue( fp, val      ) : ok;
      }
      else
      {
         fp << typeName << " " << name << " ";
         ok = fp.good();
         ok = ok ? saveValue( fp, val ) : ok;
         if ( ok ) { fp << "\n"; }
         ok = ok ? fp.good() : ok;
      }
      return ok;
   }

   template < class T > inline bool saveVecTo( std::ofstream & fp, bool isBin, const char * typeName, const std::string & name, const std::vector< T > & vec )
   {
      bool ok = true;
      if ( isBin )
      {
         ok = ok ? saveBinValue( fp, "vector"   ) : ok;
         ok = ok ? saveBinValue( fp, static_cast<unsigned char>( typeName2DataTypeID( typeName ) ) ) : ok;
         ok = ok ? saveBinValue( fp, name ) : ok;
         ok = ok ? saveBinValue( fp, vec.size() ) : ok;

         for ( size_t i = 0; i < vec.size() && ok; ++i )
         {
            ok = ok ? saveBinValue( fp, vec[i] ) : ok;
         }
      }
      else
      {
         fp << "vector[" << typeName << "] " << name << " " << vec.size();
         bool ok = fp.good();

         for ( size_t i = 0; i < vec.size() && ok; ++i )
         {
            fp << " ";
            ok = fp.good();
            ok = ok ? saveValue( fp, vec[i] ) : ok;
         }
         if ( ok )
         { 
            fp << "\n";
            ok = fp.good();
         }
      }
      return ok;
   }

   SimpleSerializer::SimpleSerializer( const std::string & fileName, const std::string & fileType, int ver )
      : m_version( ver )
      , m_isBinary( false )
   {

      if ( fileType == "bin" )
      {
         m_file.open( fileName.c_str(), std::ios::out | std::ios::trunc | std::ios::binary );
         m_isBinary = true;
      }
      else if ( fileType == "txt" ) { m_file.open( fileName.c_str(), std::ios::out | std::ios::trunc ); }
      else
      {
         throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Unknown type of output file for saving ScenarioAnalysis object: " << fileType;
      }

      if ( !m_file.good() ) throw ErrorHandler::Exception( ErrorHandler::SerializationError ) << "Can not open file: " << fileName << " for writing";

      if ( !saveObjectDescription( (m_isBinary ? "BinSerializer" : "TxtSerializer"), "Serializer", ver ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::SerializationError ) << "Can't save object description";
      }
   }

   bool SimpleSerializer::save( bool                val, const std::string & vn ) { return saveValTo( m_file, m_isBinary, "bool",   vn, val ); }
   bool SimpleSerializer::save( int                 val, const std::string & vn ) { return saveValTo( m_file, m_isBinary, "int",    vn, val ); }
   bool SimpleSerializer::save( unsigned int        val, const std::string & vn ) { return saveValTo( m_file, m_isBinary, "uint",   vn, val ); }
   bool SimpleSerializer::save( long long           val, const std::string & vn ) { return saveValTo( m_file, m_isBinary, "llong",  vn, val ); }
#ifndef _WIN32
   bool SimpleSerializer::save( unsigned long long  val, const std::string & vn ) { return saveValTo( m_file, m_isBinary, "llong",  vn, val ); }
#endif
   bool SimpleSerializer::save( ObjRefID            val, const std::string & vn ) { return saveValTo( m_file, m_isBinary, "refID",  vn, val ); }
   bool SimpleSerializer::save( float               val, const std::string & vn ) { return saveValTo( m_file, m_isBinary, "float",  vn, val ); }
   bool SimpleSerializer::save( double              val, const std::string & vn ) { return saveValTo( m_file, m_isBinary, "double", vn, val ); }
   bool SimpleSerializer::save( const std::string & val, const std::string & vn ) { return saveValTo( m_file, m_isBinary, "string", vn, val ); }

   bool SimpleSerializer::save( const std::vector< bool >         & vec, const std::string & vn ) { return saveVecTo( m_file, m_isBinary, "bool", vn, vec ); }
   bool SimpleSerializer::save( const std::vector< int >          & vec, const std::string & vn ) { return saveVecTo( m_file, m_isBinary, "int",  vn, vec ); }
   bool SimpleSerializer::save( const std::vector< unsigned int > & vec, const std::string & vn ) { return saveVecTo( m_file, m_isBinary, "uint", vn, vec ); }
#ifndef _WIN32
   bool SimpleSerializer::save( const std::vector< unsigned long long > & vec, const std::string & vn ) { return saveVecTo( m_file, m_isBinary, "llong",  vn, vec ); }
#endif
   bool SimpleSerializer::save( const std::vector< long long >    & vec, const std::string & vn ) { return saveVecTo( m_file, m_isBinary, "llong",  vn, vec ); }
   bool SimpleSerializer::save( const std::vector< ObjRefID >     & vec, const std::string & vn ) { return saveVecTo( m_file, m_isBinary, "refID",  vn, vec ); }
   bool SimpleSerializer::save( const std::vector< float >        & vec, const std::string & vn ) { return saveVecTo( m_file, m_isBinary, "float",  vn, vec ); }
   bool SimpleSerializer::save( const std::vector< double >       & vec, const std::string & vn ) { return saveVecTo( m_file, m_isBinary, "double", vn, vec ); }
   bool SimpleSerializer::save( const std::vector< std::string >  & vec, const std::string & vn ) { return saveVecTo( m_file, m_isBinary, "string", vn, vec ); }

   // Save CasaSerializable object
   bool SimpleSerializer::save( const CasaSerializable & so, const std::string & objName )
   {
      saveObjectDescription( so.typeName(), objName, so.version() );
      bool ok = m_file.good();
      return ok ? so.save( *this, m_version ) : ok;
   }

   bool SimpleSerializer::saveObjectDescription( const std::string & objType, const std::string & objName, unsigned int ver )
   {
      bool ok = true;
      if ( m_isBinary )
      {
         ok = ok ? saveBinValue( m_file, objType ) : ok;
         ok = ok ? saveBinValue( m_file, objName ) : ok;
         ok = ok ? saveBinValue( m_file, ver     ) : ok;
      }
      else
      {
         m_file << objType << " " << objName << " " << ver << "\n";
         ok = m_file.good();
      }
      return ok;
   }

   // Wrapper to save SUMlib serialazable objects
   class SUMlibSerializer : public SUMlib::ISerializer
   {
   public:
      SUMlibSerializer( SimpleSerializer & os ) : m_oStream( os ) { ; }
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
      SimpleSerializer & m_oStream;
   };

   // Save SUMlib serializable object
   bool SimpleSerializer::save( const SUMlib::ISerializable & so, const std::string & objName )
   {
      const SUMlib::ISerializationVersion * soVersion = dynamic_cast<const SUMlib::ISerializationVersion*>(&so);
      unsigned int version = soVersion ? soVersion->getSerializationVersion() : 0;

      bool ok = saveObjectDescription( "ISerializable", objName, version );
      
      SUMlibSerializer sumlibSer( *this );
      ok = ok ? so.save( &sumlibSer, version ) : ok;
      
      return ok;
   }
}
