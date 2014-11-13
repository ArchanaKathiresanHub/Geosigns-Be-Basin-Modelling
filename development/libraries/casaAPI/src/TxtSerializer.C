//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

// CASA
#include "TxtSerializer.h"

// STL
#include <sstream>
#include <typeinfo>

namespace casa
{

   template <typename T> inline bool saveValue( FILE * fp, T val )
   {
      std::ostringstream oss;
      oss << val;
      return fprintf( fp, oss.str().c_str() ) > 0;
   }

   inline bool saveValue( FILE * fp, bool val )
   {
      std::ostringstream oss; oss << (val ? "true" : "false");
      return fprintf( fp, oss.str().c_str() ) > 0;
   }

   inline bool saveValue( FILE * fp, const std::string & val )
   {
      std::ostringstream oss; oss << "\"" << val << "\"";
      return fprintf( fp, oss.str().c_str() ) > 0;
   }

   inline bool saveValue( FILE * fp, const char * val )
   {
      std::ostringstream oss; oss << "\"" << std::string( val ) << "\"";
      return fprintf( fp, oss.str().c_str() ) > 0;
   }

   // Save one value as a new line in file with template - typeid name value\n
   template <typename T> inline bool saveValTo( FILE * fp, const char * typeName, const std::string & name, T val )
   {
      bool ok = fprintf( fp, "%s %s ", typeName, name.c_str() ) > 0;
      ok = ok ? saveValue( fp, val ) : ok;
      ok = ok ? fprintf( fp, "\n" ) > 0 : ok;
      return ok;
   }

   template < class T > inline bool saveVecTo( FILE * fp, const char * typeName, const std::string & name, const std::vector< T > & vec )
   {
      bool ok = fprintf( fp, "vector[%s] %s %lu", typeName, name.c_str(), vec.size() ) > 0;

      for ( size_t i = 0; i < vec.size() && ok; ++i )
      {
         ok = fprintf( fp, " " ) > 0;
         ok = ok ? saveValue( fp, vec[i] ) : ok;
      }
      return ok ? fprintf( fp, "\n" ) > 0 : ok;
   }

   TxtSerializer::TxtSerializer( FILE * fileHandle, int ver )
      : m_file( fileHandle )
      , m_version( ver )
   {
      fprintf( m_file, "%s Serializer %d\n", typeid(*this).name(), ver );
   }

   bool TxtSerializer::save( bool                val, const std::string & vn ) { return saveValTo( m_file, "bool",   vn, val ); }
   bool TxtSerializer::save( int                 val, const std::string & vn ) { return saveValTo( m_file, "int",    vn, val ); }
   bool TxtSerializer::save( unsigned int        val, const std::string & vn ) { return saveValTo( m_file, "uint",   vn, val ); }
   bool TxtSerializer::save( long long           val, const std::string & vn ) { return saveValTo( m_file, "llong",  vn, val ); }
#ifndef _WIN32
   bool TxtSerializer::save( unsigned long long  val, const std::string & vn ) { return saveValTo( m_file, "llong",  vn, val ); }
#endif
   bool TxtSerializer::save( ObjRefID            val, const std::string & vn ) { return saveValTo( m_file, "refID",  vn, val ); }
   bool TxtSerializer::save( float               val, const std::string & vn ) { return saveValTo( m_file, "float",  vn, val ); }
   bool TxtSerializer::save( double              val, const std::string & vn ) { return saveValTo( m_file, "double", vn, val ); }
   bool TxtSerializer::save( const std::string & val, const std::string & vn ) { return saveValTo( m_file, "string", vn, val ); }

   bool TxtSerializer::save( const std::vector< bool >         & vec, const std::string & vn ) { return saveVecTo( m_file, "bool",   vn, vec ); }
   bool TxtSerializer::save( const std::vector< int >          & vec, const std::string & vn ) { return saveVecTo( m_file, "int",    vn, vec ); }
   bool TxtSerializer::save( const std::vector< unsigned int > & vec, const std::string & vn ) { return saveVecTo( m_file, "uint",   vn, vec ); }
#ifndef _WIN32
   bool TxtSerializer::save( const std::vector< unsigned long long > & vec, const std::string & vn ) { return saveVecTo( m_file, "llong",  vn, vec ); }
#endif
   bool TxtSerializer::save( const std::vector< long long >    & vec, const std::string & vn ) { return saveVecTo( m_file, "llong",  vn, vec ); }
   bool TxtSerializer::save( const std::vector< ObjRefID >     & vec, const std::string & vn ) { return saveVecTo( m_file, "refID",  vn, vec ); }
   bool TxtSerializer::save( const std::vector< float >        & vec, const std::string & vn ) { return saveVecTo( m_file, "float",  vn, vec ); }
   bool TxtSerializer::save( const std::vector< double >       & vec, const std::string & vn ) { return saveVecTo( m_file, "double", vn, vec ); }
   bool TxtSerializer::save( const std::vector< std::string >  & vec, const std::string & vn ) { return saveVecTo( m_file, "string", vn, vec ); }

   // Save CasaSerializable object
   bool TxtSerializer::save( const CasaSerializable & so, const std::string & objName )
   {
      bool ok = fprintf( m_file, "%s %s %d\n", typeid(so).name(), objName.c_str(), so.version() ) > 0;
      return ok ? so.save( *this, m_version ) : ok;
   }

   // Wrapper to save SUMlib serialazable objects
   class SUMlibSerializer : public SUMlib::ISerializer
   {
   public:
      SUMlibSerializer( TxtSerializer & os ) : m_oStream( os ) { ; }
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
      TxtSerializer & m_oStream;
   };

   // Save SUMlib serializable object
   bool TxtSerializer::save( const SUMlib::ISerializable & so, const std::string & objName )
   {
      const SUMlib::ISerializationVersion * soVersion = dynamic_cast<const SUMlib::ISerializationVersion*>(&so);
      unsigned int version = soVersion ? soVersion->getSerializationVersion() : 0;

      bool ok = fprintf( m_file, "%s %s %d\n", typeid(so).name(), objName.c_str(), version ) > 0;
      
      SUMlibSerializer sumlibSer( *this );
      ok = ok ? so.save( &sumlibSer, version ) : ok;
      
      return ok;
   }
}
