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
#include "TxtDeserializer.h"

// STL
#include <sstream>

#define MAX_BUF_SIZE 128000

// Set of axillary function to be used in loading 

// elementary string to type converting functions
template <typename T> inline bool string2val( const std::string & str, T & val )
{
   std::istringstream iss( str );
   iss >> val;
   return true;
}

inline bool string2val( const std::string & str, bool & val )
{
   if (      !str.compare( "true"  ) ) val = true;
   else if ( !str.compare( "false" ) ) val = false;
   else return false;

   return true;
}

inline bool string2val( const std::string & str, std::string & v ) { v = str; return true; }

// read one line from file and split it using ' ' as separator
inline bool readAndSplitLine( FILE * fp, char * buf, std::vector<std::string> & tokens )
{
   if ( tokens.size() ) tokens.clear();

   if ( !fgets( buf, MAX_BUF_SIZE, fp ) ) return false;

   // split string to tokens
   std::string line( buf );
 
   // clean last symbol '\n'
   std::string::size_type endLinePos = line.find( "\n" );
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
inline bool loadVal( FILE * fp, T & val, const std::string & valName, const char * typeName, char * buf )
{
   std::vector<std::string> tokens;

   if ( !readAndSplitLine( fp, buf, tokens ) )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "TxtDeserializer: Can't read line from input file for " << typeName << " value: " << valName;
   }

   if ( tokens.size() != 3 )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "TxtDeserializer: Can't read " << typeName << " value: " << valName;
   }
   if ( tokens[1] != valName )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "TxtDeserializer: Wrong value name: " << tokens[0] << ", expected: " << valName;
   }

   if ( tokens[0].compare( typeName ) != 0 )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "TxtDeserializer: Mismatch value type for " << valName
         << ", expected: " << typeName << " but read: " << tokens[0];
   }
   
   return string2val( tokens[2], val );
}

// read vector of simple type variables like int/double/string
template <typename T>
inline bool loadVec( FILE * fp, std::vector< T > & val, const std::string & valName, const char * typeName, char * buf )
{
   std::vector<std::string> tokens;

   if ( !readAndSplitLine( fp, buf, tokens ) )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "TxtDeserializer: Can't read line from input file for " << typeName << " value: " << valName;
   }

   if ( tokens.size() < 3 )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "TxtDeserializer: Can't read vector of " << typeName << " values: " << valName;
   }

   if ( tokens[1] != valName )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "TxtDeserializer: Wrong value name: " << tokens[0] << ", expected: " << valName;
   }
   std::string vecTypeName = std::string( "vector[" ) + typeName + "]";

   if ( tokens[0].compare( vecTypeName ) != 0 )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "TxtDeserializer: Mismatch values type for " << valName
         << ", expected: " << vecTypeName << " but read: " << tokens[0];
   }

   size_t vecLen;
   if ( !string2val( tokens[2], vecLen ) )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "TxtDeserializer: Can't read array " << valName << " size";
   }

   if ( tokens.size() - 3 != vecLen )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "TxtDeserializer: Can't read all elements of array " << valName;
   }

   val.resize( vecLen );
   bool ok = true;
   for ( size_t i = 0; i < vecLen && ok; ++i )
   {
      T inVal;
      ok = string2val( tokens[i + 3], inVal );
      val[i] = inVal;
   }
   return ok;
}

namespace casa
{
   TxtDeserializer::TxtDeserializer( FILE * fileHandle, int ver )
      : m_file( fileHandle )
      , m_version( ver )
   {
      m_buf = new char[MAX_BUF_SIZE];

      // read info from file about serializer
      std::string  srType;
      std::string  srName;
      unsigned int srVersion;
      
      if ( !loadObjectDescription( srType, srName, srVersion ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "TxtDeserializer: Can't read serializer signature from input file";
      }

      if ( srType.compare( "class casa::TxtSerializer" ) ||  srName.compare( "Serializer" ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "TxtDeserializer: Wrong input file signature, can't run deserialization";
      }

      if ( srVersion != m_version )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "TxtDeserializer: Mismatch file format versions of Serializer/Deserializer";
      }
   }

   TxtDeserializer::~TxtDeserializer()
   {
      delete[] m_buf;
   }

   // Read the description of the next object in file. Works only for class objects
   bool TxtDeserializer::loadObjectDescription( std::string & objType, std::string & objName, unsigned int & ver )
   {
      std::vector<std::string> tokens;
      if ( !readAndSplitLine( m_file, m_buf, tokens ) || tokens.size() != 4 )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "TxtDeserializer: Can't read next object description from input file";
      }

      if ( tokens[0].compare( "class" ) != 0 )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError ) << "TxtDeserialaizer: wrong object description: "
            << tokens[0] << " " << tokens[1] << " " << tokens[2];
      }
      objType = tokens[0] + " " + tokens[1];
      objName = tokens[2];

      std::string strVal = tokens[3];
      bool ok = string2val( strVal, ver );
      return ok;
   }

   bool TxtDeserializer::load( bool               & v, const std::string & n ) { return loadVal( m_file, v, n, "bool",   m_buf ); }
   bool TxtDeserializer::load( int                & v, const std::string & n ) { return loadVal( m_file, v, n, "int",    m_buf ); }
   bool TxtDeserializer::load( unsigned int       & v, const std::string & n ) { return loadVal( m_file, v, n, "uint",   m_buf ); }
   bool TxtDeserializer::load( long long          & v, const std::string & n ) { return loadVal( m_file, v, n, "llong",  m_buf ); }
#ifndef _WIN32
   bool TxtDeserializer::load( unsigned long long & v, const std::string & n ) { return loadVal( m_file, v, n, "llong",  m_buf ); }
#endif
   bool TxtDeserializer::load( ObjRefID           & v, const std::string & n ) { return loadVal( m_file, v, n, "refID",  m_buf ); }
   bool TxtDeserializer::load( float              & v, const std::string & n ) { return loadVal( m_file, v, n, "float",  m_buf ); }
   bool TxtDeserializer::load( double             & v, const std::string & n ) { return loadVal( m_file, v, n, "double", m_buf ); }
   bool TxtDeserializer::load( std::string        & v, const std::string & n ) { return loadVal( m_file, v, n, "string", m_buf ); }

   bool TxtDeserializer::load( std::vector< bool >         & v, const std::string & n ) { return loadVec( m_file, v, n, "bool",   m_buf ); }
   bool TxtDeserializer::load( std::vector< int >          & v, const std::string & n ) { return loadVec( m_file, v, n, "int",    m_buf ); }
   bool TxtDeserializer::load( std::vector< unsigned int > & v, const std::string & n ) { return loadVec( m_file, v, n, "uint",   m_buf ); }
   bool TxtDeserializer::load( std::vector< long long >    & v, const std::string & n ) { return loadVec( m_file, v, n, "llong",  m_buf ); }
#ifndef _WIN32
   bool TxtDeserializer::load( std::vector< unsigned long long > & v, const std::string & n ) { return loadVec( m_file, v, n, "llong",  m_buf ); }
#endif
   bool TxtDeserializer::load( std::vector< ObjRefID >     & v, const std::string & n ) { return loadVec( m_file, v, n, "refID",  m_buf ); }
   bool TxtDeserializer::load( std::vector< float >        & v, const std::string & n ) { return loadVec( m_file, v, n, "float",  m_buf ); }
   bool TxtDeserializer::load( std::vector< double >       & v, const std::string & n ) { return loadVec( m_file, v, n, "double", m_buf ); }
   bool TxtDeserializer::load( std::vector< std::string >  & v, const std::string & n ) { return loadVec( m_file, v, n, "string", m_buf ); }


   // Wrapper to save SUMlib serialazable objects
   class SUMlibDeserializer : public SUMlib::IDeserializer
   {
   public:
      SUMlibDeserializer( TxtDeserializer & os ) : m_iStream( os ) { ; }
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
      TxtDeserializer & m_iStream;
   };

   // Load SUMlib serializable object
   bool TxtDeserializer::load( SUMlib::ISerializable & so, const std::string & objName )
   {
      // read from file object name and version
      std::string  objNameInFile;
      std::string  objType;
      unsigned int objVer;

      bool ok = loadObjectDescription( objType, objNameInFile, objVer );
      if ( objType.compare( typeid(so).name() ) || objNameInFile.compare( objName ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "Deserialization error. Can not load SUMlib object: " << objName;
      }
      SUMlibDeserializer sumlibDsr( *this );
      ok = ok ? so.load( &sumlibDsr, objVer ) : ok;

      return ok;
   }

}
