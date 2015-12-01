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
#include "SimpleTxtSerializer.h"

// CMB API
#include "ErrorHandler.h"

// C lib
#include <stdint.h>
#include <string.h>

namespace casa
{
   // functions to save values to txt file
   template <typename T> inline bool saveValue( std::ofstream & fp, T val ) { fp << val; return fp.good(); }

   inline bool saveValue( std::ofstream & fp, double              val ) { fp << std::scientific << val;             return fp.good(); }
   inline bool saveValue( std::ofstream & fp, float               val ) { fp << std::scientific << val;             return fp.good(); }
   inline bool saveValue( std::ofstream & fp, bool                val ) { fp << (val ? "true" : "false");           return fp.good(); }
   inline bool saveValue( std::ofstream & fp, const std::string & val ) { fp << "\"" <<              val   << "\""; return fp.good(); }
   inline bool saveValue( std::ofstream & fp, const char        * val ) { fp << "\"" << std::string( val ) << "\""; return fp.good(); }

   // Save one value as a new line in file with template - type name - name value\n
   template <typename T> inline bool saveValTo( std::ofstream & fp, const char * typeName, const std::string & name, T val )
   {
      fp << typeName << " " << name << " ";
      
      bool ok = fp.good();
      ok = ok ? saveValue( fp, val ) : ok;
      
      if ( ok ) { fp << "\n"; }
      ok = ok ? fp.good() : ok;

      return ok;
   }

   template < class T > inline bool saveVecTo( std::ofstream & fp, const char * typeName, const std::string & name, const std::vector< T > & vec )
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
      return ok;
   }

   SimpleTxtSerializer::SimpleTxtSerializer( const std::string & fileName, int ver ) : m_version( ver )
   {
      m_file.open( fileName.c_str(), std::ios::out | std::ios::trunc );
      if ( !m_file.good() ) throw ErrorHandler::Exception( ErrorHandler::SerializationError ) << "Can not open file: " << fileName << " for writing";

      if ( !saveObjectDescription( "TxtSerializer", "Serializer", ver ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::SerializationError ) << "Can't save object description";
      }
   }

   bool SimpleTxtSerializer::save( bool                val, const std::string & vn ) { return saveValTo( m_file, "bool",   vn, val ); }
   bool SimpleTxtSerializer::save( int                 val, const std::string & vn ) { return saveValTo( m_file, "int",    vn, val ); }
   bool SimpleTxtSerializer::save( unsigned int        val, const std::string & vn ) { return saveValTo( m_file, "uint",   vn, val ); }
   bool SimpleTxtSerializer::save( long long           val, const std::string & vn ) { return saveValTo( m_file, "llong",  vn, val ); }
#ifndef _WIN32
   bool SimpleTxtSerializer::save( unsigned long long  val, const std::string & vn ) { return saveValTo( m_file, "llong",  vn, val ); }
#endif
   bool SimpleTxtSerializer::save( ObjRefID            val, const std::string & vn ) { return saveValTo( m_file, "refID",  vn, val ); }
   bool SimpleTxtSerializer::save( float               val, const std::string & vn ) { return saveValTo( m_file, "float",  vn, val ); }
   bool SimpleTxtSerializer::save( double              val, const std::string & vn ) { return saveValTo( m_file, "double", vn, val ); }
   bool SimpleTxtSerializer::save( const std::string & val, const std::string & vn ) { return saveValTo( m_file, "string", vn, val ); }

   bool SimpleTxtSerializer::save( const std::vector< bool >         & vec, const std::string & vn ) { return saveVecTo( m_file, "bool", vn, vec ); }
   bool SimpleTxtSerializer::save( const std::vector< int >          & vec, const std::string & vn ) { return saveVecTo( m_file, "int",  vn, vec ); }
   bool SimpleTxtSerializer::save( const std::vector< unsigned int > & vec, const std::string & vn ) { return saveVecTo( m_file, "uint", vn, vec ); }
#ifndef _WIN32
   bool SimpleTxtSerializer::save( const std::vector< unsigned long long > & vec, const std::string & vn ) { return saveVecTo( m_file, "llong",  vn, vec ); }
#endif
   bool SimpleTxtSerializer::save( const std::vector< long long >    & vec, const std::string & vn ) { return saveVecTo( m_file, "llong",  vn, vec ); }
   bool SimpleTxtSerializer::save( const std::vector< ObjRefID >     & vec, const std::string & vn ) { return saveVecTo( m_file, "refID",  vn, vec ); }
   bool SimpleTxtSerializer::save( const std::vector< float >        & vec, const std::string & vn ) { return saveVecTo( m_file, "float",  vn, vec ); }
   bool SimpleTxtSerializer::save( const std::vector< double >       & vec, const std::string & vn ) { return saveVecTo( m_file, "double", vn, vec ); }
   bool SimpleTxtSerializer::save( const std::vector< std::string >  & vec, const std::string & vn ) { return saveVecTo( m_file, "string", vn, vec ); }

   // Save CasaSerializable object
   bool SimpleTxtSerializer::save( const CasaSerializable & so, const std::string & objName )
   {
      saveObjectDescription( so.typeName(), objName, so.version() );
      bool ok = m_file.good();
      return ok ? so.save( *this, m_version ) : ok;
   }

   bool SimpleTxtSerializer::saveObjectDescription( const std::string & objType, const std::string & objName, unsigned int ver )
   {
      m_file << objType << " " << objName << " " << ver << "\n";
      return m_file.good();
   }

   // Wrapper to save SUMlib serialazable objects
   class SUMlibSerializer : public SUMlib::ISerializer
   {
   public:
      SUMlibSerializer( SimpleTxtSerializer & os ) : m_oStream( os ) { ; }
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
         const SUMlib::ISerializationVersion* soVersion = dynamic_cast<const SUMlib::ISerializationVersion*>( &so );
         unsigned int version = soVersion ? soVersion->getSerializationVersion() : 0;
         save( version );
         return so.save( this, version );
      }

   private:
      SimpleTxtSerializer & m_oStream;
   };

   // Save SUMlib serializable object
   bool SimpleTxtSerializer::save( const SUMlib::ISerializable & so, const std::string & objName )
   {
      const SUMlib::ISerializationVersion * soVersion = dynamic_cast<const SUMlib::ISerializationVersion*>(&so);
      unsigned int version = soVersion ? soVersion->getSerializationVersion() : 0;

      bool ok = saveObjectDescription( "ISerializable", objName, version );
      
      SUMlibSerializer sumlibSer( *this );
      ok = ok ? so.save( &sumlibSer, version ) : ok;
      
      return ok;
   }
}
