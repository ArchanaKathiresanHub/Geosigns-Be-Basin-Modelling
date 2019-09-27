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


#define SIMPLE_BIN_SD_DATA_TYPES 1
#include "SimpleBin.h"

using namespace std;

namespace casa
{
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

   inline bool saveValue( boost::iostreams::filtering_ostream & fp, const string & val )
   {
      int64_t len = val.size();
      fp.write( reinterpret_cast<const char*>(&len), sizeof( int64_t ) );
      fp.write( val.c_str(), len + 1 );
      return fp.good();
   }

   // Save one value as a new line in file with template - type name - name value\n
   template <typename T> inline bool saveValTo( boost::iostreams::filtering_ostream & fp
                                              , SimpleDataTypeBinID                   typeName
                                              , const string                        & name
                                              , T                                     val )
   {
      bool   ok = saveValue( fp, static_cast<unsigned char>( typeName ) );
      ok   = ok ? saveValue( fp, name ) : ok;
      return ok ? saveValue( fp, val  ) : ok;
   }

   template < class T > inline bool saveVecTo( boost::iostreams::filtering_ostream & fp
                                             , SimpleDataTypeBinID                   typeName
                                             , const string                        & name
                                             , const vector< T >                   & vec
                                             )
   {
      bool ok = saveValue( fp, static_cast<unsigned char>( typeName ) );
      ok = ok ? saveValue( fp, name ) : ok;
      ok = ok ? saveValue( fp, vec.size() ) : ok;

      for ( size_t i = 0; i < vec.size() && ok; ++i ) { ok = ok ? saveValue( fp, vec[i] ) : ok; }
      return ok;
   }

   SimpleBinSerializer::SimpleBinSerializer( const string & fileName, int ver )
   {
      m_file.push( boost::iostreams::gzip_compressor() );
      m_file.push( boost::iostreams::file_sink( fileName, ios::out | ios::trunc | ios::binary ), 65536 );

      if ( !saveObjectDescription( "BinSerializer", "Serializer", ver ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::SerializationError ) << "Can't save object description";
      }
   }

   bool SimpleBinSerializer::save( bool                               val, const string & vn ) { return saveValTo( m_file, BoolID,   vn, val ); }
   bool SimpleBinSerializer::save( int                                val, const string & vn ) { return saveValTo( m_file, IntID,    vn, val ); }
   bool SimpleBinSerializer::save( unsigned int                       val, const string & vn ) { return saveValTo( m_file, UintID,   vn, val ); }
   bool SimpleBinSerializer::save( long long                          val, const string & vn ) { return saveValTo( m_file, LlongID,  vn, val ); }
#ifndef _WIN32
   bool SimpleBinSerializer::save( unsigned long long                 val, const string & vn ) { return saveValTo( m_file, LlongID,  vn, val ); }
#endif
   bool SimpleBinSerializer::save( ObjRefID                           val, const string & vn ) { return saveValTo( m_file, RefID,    vn, val ); }
   bool SimpleBinSerializer::save( float                              val, const string & vn ) { return saveValTo( m_file, FloatID,  vn, val ); }
   bool SimpleBinSerializer::save( double                             val, const string & vn ) { return saveValTo( m_file, DoubleID, vn, val ); }
   bool SimpleBinSerializer::save( const string                     & val, const string & vn ) { return saveValTo( m_file, StringID, vn, val ); }

   bool SimpleBinSerializer::save( const vector<bool>               & vec, const string & vn ) { return saveVecTo( m_file, BoolID,   vn, vec ); }
   bool SimpleBinSerializer::save( const vector<int>                & vec, const string & vn ) { return saveVecTo( m_file, IntID,    vn, vec ); }
   bool SimpleBinSerializer::save( const vector<unsigned int>       & vec, const string & vn ) { return saveVecTo( m_file, UintID,   vn, vec ); }
#ifndef _WIN32
   bool SimpleBinSerializer::save( const vector<unsigned long long> & vec, const string & vn ) { return saveVecTo( m_file, LlongID,  vn, vec ); }
#endif
   bool SimpleBinSerializer::save( const vector<long long>          & vec, const string & vn ) { return saveVecTo( m_file, LlongID,  vn, vec ); }
   bool SimpleBinSerializer::save( const vector<ObjRefID>           & vec, const string & vn ) { return saveVecTo( m_file, RefID,    vn, vec ); }
   bool SimpleBinSerializer::save( const vector<float>              & vec, const string & vn ) { return saveVecTo( m_file, FloatID,  vn, vec ); }
   bool SimpleBinSerializer::save( const vector<double>             & vec, const string & vn ) { return saveVecTo( m_file, DoubleID, vn, vec ); }
   bool SimpleBinSerializer::save( const vector<string>             & vec, const string & vn ) { return saveVecTo( m_file, StringID, vn, vec ); }

   // Save CasaSerializable object
   bool SimpleBinSerializer::save( const CasaSerializable & so, const string & objName )
   {
      saveObjectDescription( so.typeName(), objName, so.version() );
      return m_file.good() ? so.save( *this ) : false;
   }

   bool SimpleBinSerializer::saveObjectDescription( const string & objType, const string & objName, unsigned int ver )
   {
      bool ok = saveValue( m_file, objType );
      ok = ok ? saveValue( m_file, objName ) : ok;
      ok = ok ? saveValue( m_file, ver     ) : ok;

      return ok;
   }

   // Save SUMlib serializable object
   bool SimpleBinSerializer::save( const SUMlib::ISerializable & so, const string & objName )
   {
      const SUMlib::ISerializationVersion * soVersion = dynamic_cast<const SUMlib::ISerializationVersion*>(&so);
      unsigned int version = soVersion ? soVersion->getSerializationVersion() : 0;

      bool ok = saveObjectDescription( "ISerializable", objName, version );

      SUMlibSerializer<SimpleBinSerializer> sumlibSer( *this );
      ok = ok ? so.save( &sumlibSer, version ) : ok;

      return ok;
   }
}
