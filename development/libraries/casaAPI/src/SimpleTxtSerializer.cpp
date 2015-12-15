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

using namespace std;

namespace casa
{
   // functions to save values to txt file
   template <typename T> inline bool saveValue( ofstream & fp, T val ) { fp << val; return fp.good(); }

   inline bool saveValue( ofstream & fp, double         val ) { fp << scientific <<        val;           return fp.good(); }
   inline bool saveValue( ofstream & fp, float          val ) { fp << scientific <<        val;           return fp.good(); }
   inline bool saveValue( ofstream & fp, bool           val ) { fp << (val ? "true" : "false");           return fp.good(); }
   inline bool saveValue( ofstream & fp, const string & val ) { fp << "\"" <<              val   << "\""; return fp.good(); }
   inline bool saveValue( ofstream & fp, const char   * val ) { fp << "\"" <<      string( val ) << "\""; return fp.good(); }

   // Save one value as a new line in file with template - type name - name value\n
   template <typename T> inline bool saveValTo( ofstream & fp, const char * typeName, const string & name, T val )
   {
      fp << typeName << " " << name << " ";
      
      bool ok = fp.good();
      ok = ok ? saveValue( fp, val ) : ok;
      
      if ( ok ) { fp << "\n"; }
      ok = ok ? fp.good() : ok;

      return ok;
   }

   template < class T > inline bool saveVecTo( ofstream & fp, const char * typeName, const string & name, const vector< T > & vec )
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

   SimpleTxtSerializer::SimpleTxtSerializer( const string & fileName, int ver ) : m_version( ver )
   {
      m_file.open( fileName.c_str(), ios::out | ios::trunc );
      if ( !m_file.good() ) throw ErrorHandler::Exception( ErrorHandler::SerializationError ) << "Can not open file: " << fileName << " for writing";

      if ( !saveObjectDescription( "TxtSerializer", "Serializer", ver ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::SerializationError ) << "Can't save object description";
      }
   }

   bool SimpleTxtSerializer::save( bool                               val, const string & vn ) { return saveValTo( m_file, "bool",   vn, val ); }
   bool SimpleTxtSerializer::save( int                                val, const string & vn ) { return saveValTo( m_file, "int",    vn, val ); }
   bool SimpleTxtSerializer::save( unsigned int                       val, const string & vn ) { return saveValTo( m_file, "uint",   vn, val ); }
   bool SimpleTxtSerializer::save( long long                          val, const string & vn ) { return saveValTo( m_file, "llong",  vn, val ); }
#ifndef _WIN32
   bool SimpleTxtSerializer::save( unsigned long long                 val, const string & vn ) { return saveValTo( m_file, "llong",  vn, val ); }
#endif
   bool SimpleTxtSerializer::save( ObjRefID                           val, const string & vn ) { return saveValTo( m_file, "refID",  vn, val ); }
   bool SimpleTxtSerializer::save( float                              val, const string & vn ) { return saveValTo( m_file, "float",  vn, val ); }
   bool SimpleTxtSerializer::save( double                             val, const string & vn ) { return saveValTo( m_file, "double", vn, val ); }
   bool SimpleTxtSerializer::save( const string                     & val, const string & vn ) { return saveValTo( m_file, "string", vn, val ); }

   bool SimpleTxtSerializer::save( const vector<bool>               & vec, const string & vn ) { return saveVecTo( m_file, "bool",   vn, vec ); }
   bool SimpleTxtSerializer::save( const vector<int>                & vec, const string & vn ) { return saveVecTo( m_file, "int",    vn, vec ); }
   bool SimpleTxtSerializer::save( const vector<unsigned int>       & vec, const string & vn ) { return saveVecTo( m_file, "uint",   vn, vec ); }
#ifndef _WIN32
   bool SimpleTxtSerializer::save( const vector<unsigned long long> & vec, const string & vn ) { return saveVecTo( m_file, "llong",  vn, vec ); }
#endif
   bool SimpleTxtSerializer::save( const vector<long long>          & vec, const string & vn ) { return saveVecTo( m_file, "llong",  vn, vec ); }
   bool SimpleTxtSerializer::save( const vector<ObjRefID>           & vec, const string & vn ) { return saveVecTo( m_file, "refID",  vn, vec ); }
   bool SimpleTxtSerializer::save( const vector<float>              & vec, const string & vn ) { return saveVecTo( m_file, "float",  vn, vec ); }
   bool SimpleTxtSerializer::save( const vector<double>             & vec, const string & vn ) { return saveVecTo( m_file, "double", vn, vec ); }
   bool SimpleTxtSerializer::save( const vector<string>             & vec, const string & vn ) { return saveVecTo( m_file, "string", vn, vec ); }

   // Save CasaSerializable object
   bool SimpleTxtSerializer::save( const CasaSerializable & so, const string & objName )
   {
      saveObjectDescription( so.typeName(), objName, so.version() );
      bool ok = m_file.good();
      return ok ? so.save( *this, m_version ) : ok;
   }

   bool SimpleTxtSerializer::saveObjectDescription( const string & objType, const string & objName, unsigned int ver )
   {
      m_file << objType << " " << objName << " " << ver << "\n";
      return m_file.good();
   }

   // Save SUMlib serializable object
   bool SimpleTxtSerializer::save( const SUMlib::ISerializable & so, const string & objName )
   {
      const SUMlib::ISerializationVersion * soVersion = dynamic_cast<const SUMlib::ISerializationVersion*>(&so);
      unsigned int version = soVersion ? soVersion->getSerializationVersion() : 0;

      bool ok = saveObjectDescription( "ISerializable", objName, version );
      
      SUMlibSerializer<SimpleTxtSerializer> sumlibSer( *this );
      ok = ok ? so.save( &sumlibSer, version ) : ok;
      
      return ok;
   }
}
