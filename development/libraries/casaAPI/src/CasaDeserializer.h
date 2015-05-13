//
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef CASA_DESERIALIZER_H
#define CASA_DESERIALIZER_H

// CMB
#include "ErrorHandler.h"

// SUMlib
#include "ISerializer.h"

// STL
#include <map>
#include <vector>
#include <typeinfo>

namespace casa
{
   // Classes which are referenced by pointers in other classes
   class Observable;
   class VarParameter;

   /// @brief Intermediate class to hide inheritance from SUMlib::ISerializer
   class CasaDeserializer
   {
   public:
      typedef size_t ObjRefID;

      /// @brief  Destructor
      virtual ~CasaDeserializer() { ; }

      /// @brief Read the description of the next object from file and compare with given data. Works only for CasaSerializable objects
      /// @param objType string representation of object type as it returned by CasaSerializable::typeName() virtual method
      /// @param objName object name
      /// @param [in,out] ver expected version of object, if version of the object in file is newer it will throw an exception.
      ///                     On return it keeps object version from the file.
      /// @return true on success, false on any error
      virtual bool checkObjectDescription( const char * objType, const std::string & objName, unsigned int & ver ) = 0;

      /// @brief Read the description of the next object in file. Works only for class objects
      /// @param objType string representation of object type as it returned by CasaSerializable::typeName() virtual method
      /// @param objName object name as it was given to serializer
      /// @param ver version of object representation in file
      /// @return true on success, false on any error
      virtual bool loadObjectDescription( std::string & objType, std::string & objName, unsigned int & ver ) = 0;

      /// @brief Load SUMlib serializable object
      /// @param so SUMlib object reference
      /// @param objName name of the object to be loaded
      /// @return true if on success, false otherwise
      virtual bool load( SUMlib::ISerializable & so, const std::string & objName ) = 0;

      /// @brief  Load boolean value
      /// @param  val the boolean to load
      /// @param  valName name of the value to be loaded
      /// @return true if on success, false otherwise
      virtual bool load( bool & val, const std::string & valName ) = 0;

      /// @brief  Load integer value
      /// @param  val the integer value to load
      /// @param  valName name of the value to be loaded
      /// @return true if on success, false otherwise
      virtual bool load( int & val, const std::string & valName ) = 0;

      /// @brief  Load unsigned integer value
      /// @param  val the unsigned integer value to load
      /// @param  valName name of the value to be loaded
      /// @return true if on success, false otherwise
      virtual bool load( unsigned int & val, const std::string & valName) = 0;

      /// @brief  Load long long value
      /// @param  val the long long value to load
      /// @param  valName name of the value to be loaded
      /// @return true if on success, false otherwise
      virtual bool load( long long & val, const std::string & valName ) = 0;

#ifndef _WIN32
      /// @brief  Load unsigned long long value
      /// @param  val the unsigned long long value to load
      /// @param  valName name of the value to be loaded
      /// @return true if on success, false otherwise
      virtual bool load( unsigned long long & val, const std::string & valName ) = 0;
#endif

      /// @brief  Load ObjRefID value
      /// @param  val the ObjRefID value to load
      /// @param  valName name of the value to be loaded
      /// @return true if on success, false otherwise
      virtual bool load( ObjRefID & val, const std::string & valName ) = 0;

      /// @brief  Load single precision float value
      /// @param  val the single precision float value to load
      /// @param  valName name of the value to be loaded
      /// @return true if on success, false otherwise
      virtual bool load( float & val, const std::string & valName ) = 0;

      /// @brief  Load double precision float value
      /// @param  val the double precision float value to load
      /// @param  valName name of the value to be loaded
      /// @return true if on success, false otherwise
      virtual bool load( double & val, const std::string & valName ) = 0;

      /// @brief  Load STL string
      /// @param  val the STL string value to load
      /// @param  valName name of the value to be loaded
      /// @return true if on success, false otherwise
      virtual bool load( std::string & val, const std::string & valName ) = 0;

      /// @brief  Load vector of booleans
      /// @param  vec the vector of booleans to load
      /// @param  vecName name of the vector of values to be loaded
      /// @return true if on success, false otherwise
      virtual bool load( std::vector< bool > & vec, const std::string & vecName ) = 0;

      /// @brief  Load vector of integer values
      /// @param  vec the vector of integer values to load
      /// @param  vecName name of the vector of values to be loaded
      /// @return true if on success, false otherwise
      virtual bool load( std::vector< int > & vec, const std::string & vecName ) = 0;

      /// @brief  Load vector of unsigned integer values
      /// @param  vec the vector of unsigned integer values to load
      /// @param  vecName name of the vector of values to be loaded
      /// @return true if on success, false otherwise
      virtual bool load( std::vector< unsigned int > & vec, const std::string & vecName ) = 0;

      /// @brief  Load vector of long long values
      /// @param  vec the vector of long long values to load
      /// @param  vecName name of the vector of values to be loaded
      /// @return true if on success, false otherwise
      virtual bool load( std::vector< long long > & vec, const std::string & vecName ) = 0;

#ifndef _WIN32
      /// @brief  Load vector of unsigned long long values
      /// @param  vec the vector of unsigned long long values to load
      /// @param  vecName name of the vector of values to be loaded
      /// @return true if on success, false otherwise
      virtual bool load( std::vector< unsigned long long > & vec, const std::string & vecName ) = 0;
#endif

      /// @brief  Load vector of long long values
      /// @param  vec the vector of long long values to load
      /// @param  vecName name of the vector of values to be loaded
      /// @return true if on success, false otherwise
      virtual bool load( std::vector< ObjRefID > & vec, const std::string & vecName ) = 0;

      /// @brief  Load vector of single precision float values
      /// @param  vec the vector of single precision float values to load
      /// @param  vecName name of the vector of values to be loaded
      /// @return true if on success, false otherwise
      virtual bool load( std::vector< float > & vec, const std::string & vecName ) = 0;

      /// @brief  Load vector of double precision float values
      /// @param  vec the vector of double precision float values to load
      /// @param  vecName name of the vector of values to be loaded
      /// @return true if on success, false otherwise
      virtual bool load( std::vector< double > & vec, const std::string & vecName ) = 0;

      /// @brief  Load string vector
      /// @param  vec the string vector to load
      /// @param  vecName name of the vector of values to be loaded
      /// @return true if on success, false otherwise
      virtual bool load( std::vector< std::string > & vec, const std::string & vecName ) = 0;

      /// @brief Get version of derialized file
      /// @return get file version
      virtual int version() = 0;

      template <class T> bool registerObjPtrUnderID( const T * obj, ObjRefID id );
      template <class T> const T * id2ptr( ObjRefID id ) const;

   protected:
      /// @brief  Default constructor
      CasaDeserializer() { ; }

   private:
      std::vector< const void *>          m_id2ptr;
      std::map<    const void *, size_t > m_ptr2id;

      CasaDeserializer( const CasaDeserializer & ); // copy constructor
      CasaDeserializer & operator = (const CasaDeserializer &); // copy operator
   };


   template <class T> bool CasaDeserializer::registerObjPtrUnderID( const T * obj, ObjRefID id )
   {
      const void * vobj = static_cast<const T *>(obj);

      // if not added yet - add as new entry in map and in vector
      if ( m_ptr2id.count( vobj ) < 1 )
      {
         if ( m_id2ptr.size() <= id ) m_id2ptr.resize( id + 1, 0 );

         if ( m_id2ptr[id] ) // check are any other ptr with the same id
         {
            if ( m_id2ptr[id] != vobj ) // yes, registering another pointer under existed ID - error
            {
               throw ErrorHandler::Exception( ErrorHandler::DeserializationError ) <<
                  "Duplicated ID during registration of object";
            }
            else
            {
               throw ErrorHandler::Exception( ErrorHandler::DeserializationError ) <<
                  "Inconsistency in ptr->ID table for registration of objects";
            }
         }
         else // register it at least
         {
            m_id2ptr[id] = vobj;
            m_ptr2id[obj] = id;
         }
      }
      return true;
   }

   template <class T> const T * CasaDeserializer::id2ptr( ObjRefID id ) const
   {
      if ( id >= m_id2ptr.size() )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError ) << "Wrong ID for object";
      }
      return static_cast<const T*>( m_id2ptr[id] );
   }
}

#endif // CASA_DESERIALIZER_H
