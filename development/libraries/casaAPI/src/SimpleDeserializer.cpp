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
#include "SimpleDeserializer.h"

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
   template <typename T> inline bool loadBinValue( std::istream & fp, T & val )
   {
      fp.read( reinterpret_cast<char *>(&val), sizeof( T ) );
      return fp.good();
   }

   inline bool loadBinValue( std::istream & fp, bool & val )
   {
      int ival;
      bool ok = loadBinValue( fp, ival );
      if ( ok ) { val = ival == 0 ? false : true; }
      return ok;
   }

   inline bool loadBinValue( std::istream & fp, std::string & val )
   {
      uint16_t len;
      fp.read( reinterpret_cast<char*>(&len), sizeof( int16_t ) );

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


   // elementary string to type converting functions
   template <typename T> inline bool string2val( const std::string & str, T & val )
   {
      std::istringstream iss( str );
      iss >> val;
      return true;
   }

   inline bool string2val( const std::string & str, bool & val )
   {
      if ( str.find( "true" ) == 0 ) val = true;
      else if ( str.find( "false" ) == 0 ) val = false;
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
   inline bool loadVal( std::istream & fp, bool isBin, T & val, const std::string & valName, const char * typeName )
   {
      bool ok = true;
      if ( isBin )
      {
         std::string readValName;
         std::string readTypeName;

         char tpID;
         ok = ok ? loadBinValue( fp, tpID ) : ok;
         readTypeName = dataTypeID2TypeName( static_cast<SimpleDataTypeBinID>( tpID ) );
         
         if ( ok && readTypeName.compare( typeName ) != 0 )
         {
            throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
               << "SimpleDeserializer: Mismatch value type for " << valName
               << ", expected: " << typeName << " but read: " << readTypeName;
         }

         ok = ok ? loadBinValue( fp, readValName ) : ok;
         if ( ok && readValName.compare( valName ) != 0 )
         {
            throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
               << "SimpleDeserializer: Wrong value name: " << readValName << ", expected: " << valName;
         }
         ok = ok ? loadBinValue( fp, val ) : ok;
      }
      else
      {
         std::vector<std::string> tokens;

         if ( !readAndSplitLine( fp, tokens ) )
         {
            throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
               << "SimpleDeserializer: Can't read line from input file for " << typeName << " value: " << valName;
         }

         if ( tokens.size() != 3 )
         {
            throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
               << "SimpleDeserializer: Can't read " << typeName << " value: " << valName;
         }
         if ( tokens[1] != valName )
         {
            throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
               << "SimpleDeserializer: Wrong value name: " << tokens[0] << ", expected: " << valName;
         }

         if ( tokens[0].compare( typeName ) != 0 )
         {
            throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
               << "SimpleDeserializer: Mismatch value type for " << valName
               << ", expected: " << typeName << " but read: " << tokens[0];
         }

         ok = string2val( tokens[2], val );
      }
      return ok;
   }

   // read vector of simple type variables like int/double/string
   template <typename T>
   inline bool loadVec( std::istream & fp, bool isBin, std::vector< T > & val, const std::string & valName, const char * typeName )
   {
      bool ok = true;
      if ( isBin )
      {
         std::string readValName;
         std::string readTypeName;
         std::string vectorName;

         ok = ok ? loadBinValue( fp, vectorName ) : ok;
         if ( ok && vectorName.compare( "vector" ) != 0 )
         {
            throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
               << "SimpleDeserializer: Can't read vector of " << typeName << " values: " << valName;
         }

         char tpID;
         ok = ok ? loadBinValue( fp, tpID ) : ok;
         readTypeName = dataTypeID2TypeName( static_cast<SimpleDataTypeBinID>(tpID) );

         if ( ok && readTypeName.compare( typeName ) != 0 )
         {
            throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
               << "SimpleDeserializer: Mismatch value type for " << valName
               << ", expected: " << typeName << " but read: " << readTypeName;
         }

         ok = ok ? loadBinValue( fp, readValName ) : ok;
         if ( ok && readValName.compare( valName ) != 0 )
         {
            throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
               << "SimpleDeserializer: Wrong value name: " << readValName << ", expected: " << valName;
         }

         size_t vecSize = 0;
         ok = ok ? loadBinValue( fp, vecSize ) : ok;
         if ( ok )
         {
            val.clear();
            for ( size_t i = 0; i < vecSize && ok; ++i )
            {
               T vecVal;
               ok = ok ? loadBinValue( fp, vecVal ) : ok;
               if ( ok ) val.push_back( vecVal );
            }
         }
      }
      else
      {
         std::vector<std::string> tokens;

         if ( !readAndSplitLine( fp, tokens ) )
         {
            throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
               << "SimpleDeserializer: Can't read line from input file for " << typeName << " value: " << valName;
         }

         if ( tokens.size() < 3 )
         {
            throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
               << "SimpleDeserializer: Can't read vector of " << typeName << " values: " << valName;
         }

         if ( tokens[1] != valName )
         {
            throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
               << "SimpleDeserializer: Wrong value name: " << tokens[0] << ", expected: " << valName;
         }
         std::string vecTypeName = std::string( "vector[" ) + typeName + "]";

         if ( tokens[0].compare( vecTypeName ) != 0 )
         {
            throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
               << "SimpleDeserializer: Mismatch values type for " << valName
               << ", expected: " << vecTypeName << " but read: " << tokens[0];
         }

         size_t vecLen;
         if ( !string2val( tokens[2], vecLen ) )
         {
            throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
               << "SimpleDeserializer: Can't read array " << valName << " size";
         }

         if ( tokens.size() - 3 != vecLen )
         {
            throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
               << "SimpleDeserializer: Can't read all elements of array " << valName;
         }

         val.resize( vecLen );
         for ( size_t i = 0; i < vecLen && ok; ++i )
         {
            T inVal;
            ok = string2val( tokens[i + 3], inVal );
            val[i] = inVal;
         }
      }
      return ok;
   }

   SimpleDeserializer::SimpleDeserializer( std::istream & fileHandle, unsigned int ver, bool isBinary )
      : m_file( fileHandle )
      , m_isBinary( isBinary )
   {
      std::string objType;
      std::string objName;
      unsigned int objVer;

      bool ok = loadObjectDescription( objType, objName, objVer );

      // read info from file about serializer      
      if ( !ok )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "SimpleDeserializer: Can't read serializer signature from input file";
      }

      if ( !m_isBinary && objType != "TxtSerializer" || m_isBinary && objType != "BinSerializer" )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError ) <<
            "Deserialization error. Expected object type is: SimpleSerializer, but in stream: " << objType;
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

   SimpleDeserializer::~SimpleDeserializer()
   {
   }

   // Read the description of the next object in file. Works only for CasaSerializable objects
   bool SimpleDeserializer::checkObjectDescription( const char * objType, const std::string & objName, unsigned int & ver )
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
   bool SimpleDeserializer::loadObjectDescription( std::string & objType, std::string & objName, unsigned int & ver )
   {
      bool ok = true;
      if ( m_isBinary )
      {
         ok = ok ? loadBinValue( m_file, objType ) : ok;
         ok = ok ? loadBinValue( m_file, objName ) : ok;
         ok = ok ? loadBinValue( m_file, ver ) : ok;
      }
      else
      {
         std::vector<std::string> tokens;
         if ( !readAndSplitLine( m_file, tokens ) || tokens.size() != 3 )
         {
            throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
               << "SimpleDeserializer: Can't read next object description from input file";
         }
         objType = tokens[0];
         objName = tokens[1];

         unsigned objVer;
         ok = string2val( tokens.back(), objVer );
         ver = objVer;
      }
      return ok;
   }

   bool SimpleDeserializer::load( bool               & v, const std::string & n ) { return loadVal( m_file, m_isBinary, v, n, "bool" ); }
   bool SimpleDeserializer::load( int                & v, const std::string & n ) { return loadVal( m_file, m_isBinary, v, n, "int" ); }
   bool SimpleDeserializer::load( unsigned int       & v, const std::string & n ) { return loadVal( m_file, m_isBinary, v, n, "uint" ); }
   bool SimpleDeserializer::load( long long          & v, const std::string & n ) { return loadVal( m_file, m_isBinary, v, n, "llong" ); }
#ifndef _WIN32
   bool SimpleDeserializer::load( unsigned long long & v, const std::string & n ) { return loadVal( m_file, m_isBinary, v, n, "llong"  ); }
#endif
   bool SimpleDeserializer::load( ObjRefID           & v, const std::string & n ) { return loadVal( m_file, m_isBinary, v, n, "refID" ); }
   bool SimpleDeserializer::load( float              & v, const std::string & n ) { return loadVal( m_file, m_isBinary, v, n, "float" ); }
   bool SimpleDeserializer::load( double             & v, const std::string & n ) { return loadVal( m_file, m_isBinary, v, n, "double" ); }
   bool SimpleDeserializer::load( std::string        & v, const std::string & n ) { return loadVal( m_file, m_isBinary, v, n, "string" ); }

   bool SimpleDeserializer::load( std::vector< bool >         & v, const std::string & n ) { return loadVec( m_file, m_isBinary, v, n, "bool" ); }
   bool SimpleDeserializer::load( std::vector< int >          & v, const std::string & n ) { return loadVec( m_file, m_isBinary, v, n, "int" ); }
   bool SimpleDeserializer::load( std::vector< unsigned int > & v, const std::string & n ) { return loadVec( m_file, m_isBinary, v, n, "uint" ); }
   bool SimpleDeserializer::load( std::vector< long long >    & v, const std::string & n ) { return loadVec( m_file, m_isBinary, v, n, "llong" ); }
#ifndef _WIN32
   bool SimpleDeserializer::load( std::vector< unsigned long long > & v, const std::string & n ) { return loadVec( m_file, m_isBinary, v, n, "llong" ); }
#endif
   bool SimpleDeserializer::load( std::vector< ObjRefID >     & v, const std::string & n ) { return loadVec( m_file, m_isBinary, v, n, "refID" ); }
   bool SimpleDeserializer::load( std::vector< float >        & v, const std::string & n ) { return loadVec( m_file, m_isBinary, v, n, "float" ); }
   bool SimpleDeserializer::load( std::vector< double >       & v, const std::string & n ) { return loadVec( m_file, m_isBinary, v, n, "double" ); }
   bool SimpleDeserializer::load( std::vector< std::string >  & v, const std::string & n ) { return loadVec( m_file, m_isBinary, v, n, "string" ); }


   // Wrapper to save SUMlib serializable objects
   class SUMlibDeserializer : public SUMlib::IDeserializer
   {
   public:
      SUMlibDeserializer( SimpleDeserializer & os ) : m_iStream( os ) { ; }
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
      SimpleDeserializer & m_iStream;
   };

   // Load SUMlib serializable object
   bool SimpleDeserializer::load( SUMlib::ISerializable & so, const std::string & objName )
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
