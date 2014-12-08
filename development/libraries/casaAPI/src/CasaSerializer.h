//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

#ifndef CASA_SERIALIZER_H
#define CASA_SERIALIZER_H

// CASA

// SUMlib
#include "ISerializer.h"

// STL
#include <map>
#include <vector>

namespace casa
{
   class CasaSerializable;
   class CasaDeserializer;

   /// @brief Base class for CASA serializer. It defines interfaces which must be
   /// implemented by serialaizer to be able to save CASA objects to output stream
   class CasaSerializer 
   {
   public:
      typedef size_t ObjRefID;

      /// @brief  Destructor
      virtual ~CasaSerializer() { ; }

      /// @brief Save CasaSerializable object
      /// @param so CasaSerializable object reference
      /// @param objName name of the object to be saved
      /// @return true if on success, false otherwise
      virtual bool save( const CasaSerializable & so, const std::string & objName ) = 0;

      /// @brief Save SUMlib serializable object
      /// @param so SUMlib object reference
      /// @param objName name of the object to be saved
      /// @return true if on success, false otherwise
      virtual bool save( const SUMlib::ISerializable & so, const std::string & objName ) = 0;

      /// @brief  Save boolean value
      /// @param  val the boolean to save
      /// @return true if on success, false otherwise
      virtual bool save( bool val, const std::string & valName ) = 0;

      /// @brief  Save integer value
      /// @param  val the integer value to save
      /// @return true if on success, false otherwise
      virtual bool save( int val, const std::string & valName ) = 0;

      /// @brief  Save unsigned integer value
      /// @param  val the unsigned integer value to save
      /// @return true if on success, false otherwise
      virtual bool save( unsigned int val, const std::string & valName ) = 0;

      /// @brief  Save long long value
      /// @param  val the long long value to save
      /// @return true if on success, false otherwise
      virtual bool save( long long val, const std::string & valName ) = 0;

#ifndef _WIN32
      /// @brief  Save unsigned long long value
      /// @param  val the long long value to save
      /// @return true if on success, false otherwise
      virtual bool save( unsigned long long val, const std::string & valName ) = 0;
#endif

      /// @brief  Save ObjRefID value
      /// @param  val the ObjRefID value to save
      /// @return true if on success, false otherwise
      virtual bool save( ObjRefID val, const std::string & valName ) = 0;

      /// @brief  Save single precision float value
      /// @param  val the single precision float value to save
      /// @return true if on success, false otherwise
      virtual bool save( float val, const std::string & valName ) = 0;

      /// @brief  Save double precision float value
      /// @param  val the double precision float value to save
      /// @return true if on success, false otherwise
      virtual bool save( double val, const std::string & valName ) = 0;

      /// @brief  Save STL string
      /// @param  val the STL string value to save
      /// @return true if on success, false otherwise
      virtual bool save( const std::string & val, const std::string & valName ) = 0;

      /// @brief  Save vector of booleans
      /// @param  vec the vector of booleans to save
      /// @return true if on success, false otherwise
      virtual bool save( const std::vector< bool > & vec, const std::string & vecName ) = 0;

      /// @brief  Save vector of integer values
      /// @param  vec the vector of integer values to save
      /// @return true if on success, false otherwise
      virtual bool save( const std::vector< int > & vec, const std::string & vecName ) = 0;

      /// @brief  Save vector of unsigned integer values
      /// @param  vec the vector of unsigned integer values to save
      /// @return true if on success, false otherwise
      virtual bool save( const std::vector< unsigned int > & vec, const std::string & vecName ) = 0;

      /// @brief  Save vector of long long values
      /// @param  vec the vector of long long values to save
      /// @return true if on success, false otherwise
      virtual bool save( const std::vector< long long > & vec, const std::string & vecName ) = 0;

#ifndef _WIN32
      /// @brief  Save vector of unsigned long long values
      /// @param  vec the vector of long long values to save
      /// @return true if on success, false otherwise
      virtual bool save( const std::vector< unsigned long long > & vec, const std::string & vecName ) = 0;
#endif

      /// @brief  Save vector of long long values
      /// @param  vec the vector of long long values to save
      /// @return true if on success, false otherwise
      virtual bool save( const std::vector< ObjRefID > & vec, const std::string & vecName ) = 0;

      /// @brief  Save vector of single precision float values
      /// @param  vec the vector of single precision float values to save
      /// @return true if on success, false otherwise
      virtual bool save( const std::vector< float > & vec, const std::string & vecName ) = 0;

      /// @brief  Save vector of double precision float values
      /// @param  vec the vector of double precision float values to save
      /// @return true if on success, false otherwise
      virtual bool save( const std::vector< double > & vec, const std::string & vecName ) = 0;

      /// @brief  Save string vector
      /// @param  vec the string vector to save
      /// @return true if on success, false otherwise
      virtual bool save( const std::vector< std::string > & vec, const std::string & vecName ) = 0;

      /// @brief Get version of serialization file
      /// @return get file version
      virtual int version() = 0;

      /// @brief Register or convert observable pointer to observable ID
      template <class T> ObjRefID ptr2id( const T * obj );


   protected:
      /// @brief  Default constructor
      CasaSerializer() { ; }

   private:
      std::vector< const void* >     m_id2ptr;
      std::map< const void*, size_t> m_ptr2id;

      CasaSerializer( const CasaSerializer & );               // copy constructor
      CasaSerializer & operator = ( const CasaSerializer & ); // copy operator
   };

   /// @brief Intermediate class to hide inheritance from SUMlib::ISerializable
   class CasaSerializable
   {
   public:
      /// @brief Default constructor
      CasaSerializable() { ; }
      virtual ~CasaSerializable() { ; }

      virtual unsigned int version() const { return 0; } // version of the object representation in file
      virtual bool save( CasaSerializer & sz, unsigned int version ) const = 0;
      virtual const char * typeName() const = 0;
   };


   template <class T> CasaSerializer::ObjRefID CasaSerializer::ptr2id( const T * obj )
   {
      // if not added yet - add as new entry in map and in vector
      if ( m_ptr2id.count( static_cast<const void*>(obj) ) < 1 )
      {
         m_ptr2id[static_cast<const void*>(obj)] = m_id2ptr.size();
         m_id2ptr.push_back( static_cast<const void*>(obj) );
      }
      return m_ptr2id[static_cast<const void*>(obj)];
   }

} // namespace casa

#endif // CASA_SERIALIZER_H
