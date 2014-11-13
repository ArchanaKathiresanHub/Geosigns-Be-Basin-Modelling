//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

#ifndef TXT_DESERIALIZER_H
#define TXT_DESERIALIZER_H

// CASA
#include "CasaDeserializer.h"

// STD C lib
#include <stdio.h>

namespace casa
{
   /// @brief This class implements the IDeserializer interface
   class  TxtDeserializer : public CasaDeserializer
   {
   public:

      /// @brief Constructor
      /// @param fileHandle file pointer
      /// @param ver version
      TxtDeserializer( FILE * fileHandle, int ver );

      /// @brief Destructor
      virtual ~TxtDeserializer();

      /// @brief Read the description of the next object in file. Works only for class objects
      /// @param objType string representation of object type as it returned by typeid().name()
      /// @param objName object name as it was given to serializer
      /// @param ver version of object representation in file
      /// @return true on success, false on any error
      virtual bool loadObjectDescription( std::string & objType, std::string & objName, unsigned int & ver );

      /// @brief Load SUMlib serializable object
      /// @param so SUMlib object reference
      /// @param objName name of the object to be loaded
      /// @return true if on success, false otherwise
      virtual bool load( SUMlib::ISerializable & so, const std::string & objName );

      /// @brief  Load boolean value
      /// @param  val the boolean to load
      /// @return true if on success, false otherwise
      virtual bool load( bool & val, const std::string & valName );

      /// @brief  Load integer value
      /// @param  val the integer value to load
      /// @return true if on success, false otherwise
      virtual bool load( int & val, const std::string & valName );

      /// @brief  Load unsigned integer value
      /// @param  val the unsigned integer value to load
      /// @return true if on success, false otherwise
      virtual bool load( unsigned int & val, const std::string & valName );

      /// @brief  Load long long value
      /// @param  val the long long value to load
      /// @return true if on success, false otherwise
      virtual bool load( long long & val, const std::string & valName );

#ifndef _WIN32
      /// @brief  Load unsigned long long value
      /// @param  val the unsigned long long value to load
      /// @return true if on success, false otherwise
      virtual bool load( unsigned long long & val, const std::string & valName );
#endif

      /// @brief  Load ObjRefID value
      /// @param  val the ObjRefID value to load
      /// @return true if on success, false otherwise
      virtual bool load( ObjRefID & val, const std::string & valName );

      /// @brief  Load single precision float value
      /// @param  val the single precision float value to load
      /// @return true if on success, false otherwise
      virtual bool load( float & val, const std::string & valName );

      /// @brief  Load double precision float value
      /// @param  val the double precision float value to load
      /// @return true if on success, false otherwise
      virtual bool load( double & val, const std::string & valName );

      /// @brief  Load STL string
      /// @param  val the STL string value to load
      /// @return true if on success, false otherwise
      virtual bool load( std::string & val, const std::string & valName );

      /// @brief  Load vector of booleans
      /// @param  vec the vector of booleans to load
      /// @return true if on success, false otherwise
      virtual bool load( std::vector< bool > & vec, const std::string & vecName );

      /// @brief  Load vector of integer values
      /// @param  vec the vector of integer values to load
      /// @return true if on success, false otherwise
      virtual bool load( std::vector< int > & vec, const std::string & vecName );

      /// @brief  Load vector of unsigned integer values
      /// @param  vec the vector of unsigned integer values to load
      /// @return true if on success, false otherwise
      virtual bool load( std::vector< unsigned int > & vec, const std::string & vecName );

      /// @brief  Load vector of long long values
      /// @param  vec the vector of long long values to load
      /// @return true if on success, false otherwise
      virtual bool load( std::vector< long long > & vec, const std::string & vecName );

#ifndef _WIN32
      /// @brief  Load vector of unsigned long long values
      /// @param  vec the vector of unsigned long long values to load
      /// @return true if on success, false otherwise
      virtual bool load( std::vector< unsigned long long > & vec, const std::string & vecName );
#endif

     /// @brief  Load vector of long long values
      /// @param  vec the vector of long long values to load
      /// @return true if on success, false otherwise
      virtual bool load( std::vector< ObjRefID > & vec, const std::string & vecName );

      /// @brief  Load vector of single precision float values
      /// @param  vec the vector of single precision float values to load
      /// @return true if on success, false otherwise
      virtual bool load( std::vector< float > & vec, const std::string & vecName );

      /// @brief  Load vector of double precision float values
      /// @param  vec the vector of double precision float values to load
      /// @return true if on success, false otherwise
      virtual bool load( std::vector< double > & vec, const std::string & vecName );

      /// @brief  Load string vector
      /// @param  vec the string vector to load
      /// @return true if on success, false otherwise
      virtual bool load( std::vector< std::string > & vec, const std::string & vecName );

   private:
      FILE*  m_file;
      int    m_version;
      char * m_buf;

      TxtDeserializer( const TxtDeserializer & );               // copy constructor
      TxtDeserializer & operator = ( const TxtDeserializer & ); // copy operator

   };
}
#endif // TXT_DESERIALIZER_H
