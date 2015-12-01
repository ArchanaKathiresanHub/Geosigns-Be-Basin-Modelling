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
#include "SimpleTxtDeserializer.h"

#include <stdint.h>
#include <cstring>

namespace casa
{
   // elementary string to type converting functions
   template <typename T> inline bool string2val( const std::string & str, T & val )
   {
      std::istringstream iss( str );
      iss >> val;
      return true;
   }

   inline bool string2val( const std::string & str, bool & val )
   {
      if (      str == "true"  ) val = true;
      else if ( str == "false" ) val = false;
      else return false;

      return true;
   }

   inline bool string2val( const std::string & str, std::string & v ) { v = str; return true; }

   // read one line from file and split it using ' ' as separator
   inline bool readAndSplitLine( std::istream & fp, std::vector<std::string> & tokens )
   {
      if ( tokens.size() ) tokens.clear();

      std::string line;

      if ( !std::getline( fp, line ) ) return false;

      // clean last symbol '\n'
      std::string::size_type endLinePos = line.find( "\n" );
      if ( std::string::npos != endLinePos ) line = line.substr( 0, endLinePos );

      // clean possible last symbol '\r'
      endLinePos = line.find( "\r" );
      if ( std::string::npos != endLinePos ) line = line.substr( 0, endLinePos );


      std::istringstream iss( line );

      std::string result;

      std::string opt;

      while ( std::getline( iss, result, ' ' ) )
      {
         if ( result.empty() || (result.size() == 1 && result[0] == ' ') ) continue;

         if ( opt.size() ) // we had string in parentheses, read all between parentheses as one string
         {
            if ( *result.rbegin() == '"' )
            {
               opt += " " + result.substr( 0, result.size() - 1 );
               tokens.push_back( opt );
               opt = "";
            }
            else { opt += " " + result; }
         }
         else // new option
         {
            if ( result[0] == '"' )
            {
               if ( *result.rbegin() == '"' )
               {
                  tokens.push_back( result.substr( 1, result.size() - 2 ) );
               }
               else
               {
                  opt = result.substr( 1 );
               }
            }
            else
            {
               tokens.push_back( result );
            }
         }
      }
      if ( !opt.empty() ) tokens.push_back( opt );

      return true;
   }

   // read simple type variable like int/double/string
   template <typename T>
   inline bool loadVal( std::istream & fp, T & val, const std::string & valName, const char * typeName )
   {
      std::vector<std::string> tokens;

      if ( !readAndSplitLine( fp, tokens ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "SimpleTxtDeserializer: Can't read line from input file for " << typeName << " value: " << valName;
      }

      if ( tokens.size() != 3 )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "SimpleTxtDeserializer: Can't read " << typeName << " value: " << valName;
      }
      if ( tokens[1] != valName )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "SimpleTxtDeserializer: Wrong value name: " << tokens[0] << ", expected: " << valName;
      }

      if ( tokens[0].compare( typeName ) != 0 )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "SimpleTxtDeserializer: Mismatch value type for " << valName
            << ", expected: " << typeName << " but read: " << tokens[0];
      }

      return string2val( tokens[2], val );
   }

   // read vector of simple type variables like int/double/string
   template <typename T>
   inline bool loadVec( std::istream & fp, std::vector< T > & val, const std::string & valName, const char * typeName )
   {
      std::vector<std::string> tokens;

      if ( !readAndSplitLine( fp, tokens ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "SimpleTxtDeserializer: Can't read line from input file for " << typeName << " value: " << valName;
      }

      if ( tokens.size() < 3 )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "SimpleTxtDeserializer: Can't read vector of " << typeName << " values: " << valName;
      }

      if ( tokens[1] != valName )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "SimpleTxtDeserializer: Wrong value name: " << tokens[0] << ", expected: " << valName;
      }
      std::string vecTypeName = std::string( "vector[" ) + typeName + "]";

      if ( tokens[0].compare( vecTypeName ) != 0 )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "SimpleTxtDeserializer: Mismatch values type for " << valName
            << ", expected: " << vecTypeName << " but read: " << tokens[0];
      }

      size_t vecLen;
      if ( !string2val( tokens[2], vecLen ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError ) << "SimpleTxtDeserializer: Can't read array " << valName << " size";
      }

      if ( tokens.size() - 3 != vecLen )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError ) << "SimpleTxtDeserializer: Can't read all elements of array " << valName;
      }

      bool ok = true;

      val.resize( vecLen );
      for ( size_t i = 0; i < vecLen && ok; ++i )
      {
         T inVal;
         ok = string2val( tokens[i + 3], inVal );
         val[i] = inVal;
      }
      
      return ok;
   }


   bool SimpleTxtDeserializer::checkSignature( std::istream & ifs )
   {
      char buf[15];
      ifs.read( buf, 15 );
      buf[13] = 0;
      ifs.seekg( 0, std::ios::beg );
      
      return !strcmp( buf, "TxtSerializer" ) ? true : false;
   }

   SimpleTxtDeserializer::SimpleTxtDeserializer( std::istream & fileHandle, unsigned int ver ) : m_file( fileHandle )
   {
      std::string objType;
      std::string objName;
      unsigned int objVer;

      bool ok = loadObjectDescription( objType, objName, objVer );

      // read info from file about serializer      
      if ( !ok )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "SimpleTxtDeserializer: Can't read serializer signature from input file";
      }

      if ( objType != "TxtSerializer" )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError ) <<
            "Deserialization error. Expected object type is: SimpleTxtSerializer, but in stream: " << objType;
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

   SimpleTxtDeserializer::~SimpleTxtDeserializer()
   {
   }

   // Read the description of the next object in file. Works only for CasaSerializable objects
   bool SimpleTxtDeserializer::checkObjectDescription( const char * objType, const std::string & objName, unsigned int & ver )
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
   bool SimpleTxtDeserializer::loadObjectDescription( std::string & objType, std::string & objName, unsigned int & ver )
   {
      bool ok = true;

      std::vector<std::string> tokens;
      if ( !readAndSplitLine( m_file, tokens ) || tokens.size() != 3 )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "SimpleTxtDeserializer: Can't read next object description from input file";
      }
      objType = tokens[0];
      objName = tokens[1];

      unsigned objVer;
      ok = string2val( tokens.back(), objVer );
      ver = objVer;
      
      return ok;
   }

   bool SimpleTxtDeserializer::load( bool               & v, const std::string & n ) { return loadVal( m_file, v, n, "bool" ); }
   bool SimpleTxtDeserializer::load( int                & v, const std::string & n ) { return loadVal( m_file, v, n, "int" ); }
   bool SimpleTxtDeserializer::load( unsigned int       & v, const std::string & n ) { return loadVal( m_file, v, n, "uint" ); }
   bool SimpleTxtDeserializer::load( long long          & v, const std::string & n ) { return loadVal( m_file, v, n, "llong" ); }
#ifndef _WIN32
   bool SimpleTxtDeserializer::load( unsigned long long & v, const std::string & n ) { return loadVal( m_file, v, n, "llong"  ); }
#endif
   bool SimpleTxtDeserializer::load( ObjRefID           & v, const std::string & n ) { return loadVal( m_file, v, n, "refID" ); }
   bool SimpleTxtDeserializer::load( float              & v, const std::string & n ) { return loadVal( m_file, v, n, "float" ); }
   bool SimpleTxtDeserializer::load( double             & v, const std::string & n ) { return loadVal( m_file, v, n, "double" ); }
   bool SimpleTxtDeserializer::load( std::string        & v, const std::string & n ) { return loadVal( m_file, v, n, "string" ); }

   bool SimpleTxtDeserializer::load( std::vector< bool >         & v, const std::string & n ) { return loadVec( m_file, v, n, "bool" ); }
   bool SimpleTxtDeserializer::load( std::vector< int >          & v, const std::string & n ) { return loadVec( m_file, v, n, "int" ); }
   bool SimpleTxtDeserializer::load( std::vector< unsigned int > & v, const std::string & n ) { return loadVec( m_file, v, n, "uint" ); }
   bool SimpleTxtDeserializer::load( std::vector< long long >    & v, const std::string & n ) { return loadVec( m_file, v, n, "llong" ); }
#ifndef _WIN32
   bool SimpleTxtDeserializer::load( std::vector< unsigned long long > & v, const std::string & n ) { return loadVec( m_file, v, n, "llong" ); }
#endif
   bool SimpleTxtDeserializer::load( std::vector< ObjRefID >     & v, const std::string & n ) { return loadVec( m_file, v, n, "refID" ); }
   bool SimpleTxtDeserializer::load( std::vector< float >        & v, const std::string & n ) { return loadVec( m_file, v, n, "float" ); }
   bool SimpleTxtDeserializer::load( std::vector< double >       & v, const std::string & n ) { return loadVec( m_file, v, n, "double" ); }
   bool SimpleTxtDeserializer::load( std::vector< std::string >  & v, const std::string & n ) { return loadVec( m_file, v, n, "string" ); }


   // Wrapper to save SUMlib serializable objects
   class SUMlibDeserializer : public SUMlib::IDeserializer
   {
   public:
      SUMlibDeserializer( SimpleTxtDeserializer & os ) : m_iStream( os ) { ; }
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
      SimpleTxtDeserializer & m_iStream;
   };

   // Load SUMlib serializable object
   bool SimpleTxtDeserializer::load( SUMlib::ISerializable & so, const std::string & objName )
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
