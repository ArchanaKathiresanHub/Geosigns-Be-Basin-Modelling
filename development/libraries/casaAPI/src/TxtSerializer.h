//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

#ifndef TXT_SERIALIZER_H
#define TXT_SERIALIZER_H

#include <stdio.h>

#include "CasaSerializer.h"

namespace casa
{
   /// @brief This class implements the ISerializer interface
   class TxtSerializer : public CasaSerializer
   {
   public:

      /// @brief  Constructor. Throw on null file handle and negative version number
      /// @param  fileHandle file pointer, must be not null
      /// @param  ver file version 
      TxtSerializer( FILE * fileHandle, int ver );

      /// @brief  Destructor
      virtual ~TxtSerializer() { ; }

      /// @brief Save CasaSerializable object
      /// @param so CasaSerializable object reference
      /// @param objName name of the object to be saved
      /// @return true if on success, false otherwise
      virtual bool save( const CasaSerializable & so, const std::string & objName );

      /// @brief Save SUMlib serializable object
      /// @param so SUMlib object reference
      /// @param objName name of the object to be saved
      /// @return true if on success, false otherwise
      virtual bool save( const SUMlib::ISerializable & so, const std::string & objName );

      /// @brief  Save boolean value
      /// @param  val the boolean to save
      /// @return true if on success, false otherwise
      virtual bool save( bool val, const std::string & valName );

      /// @brief  Save integer value
      /// @param  val the integer value to save
      /// @return true if on success, false otherwise
      virtual bool save( int val, const std::string & valName );

      /// @brief  Save unsigned integer value
      /// @param  val the unsigned integer value to save
      /// @return true if on success, false otherwise
      virtual bool save( unsigned int val, const std::string & valName );

      /// @brief  Save long long value
      /// @param  val the long long value to save
      /// @return true if on success, false otherwise
      virtual bool save( long long val, const std::string & valName );

 #ifndef _WIN32
      /// @brief  Save unsigned long long value
      /// @param  val the long long value to save
      /// @return true if on success, false otherwise
      virtual bool save( unsigned long long val, const std::string & valName );
#endif

     /// @brief  Save ObjRefID value
      /// @param  val the ObjRefID value to save
      /// @return true if on success, false otherwise
      virtual bool save( ObjRefID val, const std::string & valName );

      /// @brief  Save single precision float value
      /// @param  val the single precision float value to save
      /// @return true if on success, false otherwise
      virtual bool save( float val, const std::string & valName );

      /// @brief  Save double precision float value
      /// @param  val the double precision float value to save
      /// @return true if on success, false otherwise
      virtual bool save( double val, const std::string & valName );

      /// @brief  Save STL string
      /// @param  val the STL string value to save
      /// @return true if on success, false otherwise
      virtual bool save( const std::string & val, const std::string & valName );


      /// @brief  Save vector of booleans
      /// @param  vec the vector of booleans to save
      /// @return true if on success, false otherwise
      virtual bool save( const std::vector< bool > & vec, const std::string & vecName );

      /// @brief  Save vector of integer values
      /// @param  vec the vector of integer values to save
      /// @return true if on success, false otherwise
      virtual bool save( const std::vector< int > & vec, const std::string & vecName );

      /// @brief  Save vector of unsigned integer values
      /// @param  vec the vector of unsigned integer values to save
      /// @return true if on success, false otherwise
      virtual bool save( const std::vector< unsigned int > & vec, const std::string & vecName );

      /// @brief  Save vector of long long values
      /// @param  vec the vector of long long values to save
      /// @return true if on success, false otherwise
      virtual bool save( const std::vector< long long > & vec, const std::string & vecName );

 #ifndef _WIN32
      /// @brief  Save vector of unsigned long long values
      /// @param  vec the vector of long long values to save
      /// @return true if on success, false otherwise
      virtual bool save( const std::vector< unsigned long long > & vec, const std::string & vecName );
#endif

     /// @brief  Save vector of long long values
      /// @param  vec the vector of long long values to save
      /// @return true if on success, false otherwise
      virtual bool save( const std::vector< ObjRefID > & vec, const std::string & vecName );

      /// @brief  Save vector of single precision float values
      /// @param  vec the vector of single precision float values to save
      /// @return true if on success, false otherwise
      virtual bool save( const std::vector< float > & vec, const std::string & vecName );

      /// @brief  Save vector of double precision float values
      /// @param  vec the vector of double precision float values to save
      /// @return true if on success, false otherwise
      virtual bool save( const std::vector< double > & vec, const std::string & vecName );

      /// @brief  Save string vector
      /// @param  vec the string vector to save
      /// @return true if on success, false otherwise
      virtual bool save( const std::vector< std::string > & vec, const std::string & vecName );

   private:
      FILE *      m_file;
      int         m_version;

      TxtSerializer( const TxtSerializer & );               // copy constructor
      TxtSerializer & operator = (const TxtSerializer &); // copy operator
   };
}

#endif // TXT_SERIALIZER_H
