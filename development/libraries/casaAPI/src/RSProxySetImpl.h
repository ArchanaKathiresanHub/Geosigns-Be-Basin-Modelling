//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file RSProxySetImpl.h
/// @brief This file keeps API implementation declarations to manage set of proxy objects

#ifndef CASA_API_RS_PROXY_SET_IMPL_H
#define CASA_API_RS_PROXY_SET_IMPL_H

// CASA
#include "RSProxySet.h"

// STL
#include <map>
#include <cassert>

namespace casa
{
   class RSProxySetImpl : public RSProxySet
   {
   public:
      typedef std::map < std::string, RSProxy * > ProxySet;

      // Default constructor
      RSProxySetImpl() { ; }

      // Destructor
      virtual ~RSProxySetImpl();

      // Get size of the set
      virtual size_t size() const { return m_proxySet.size(); }

      // Access to i-th element
      virtual const RSProxy * operator[] ( size_t i ) const
      {  
         if ( m_proxySet.size() <= i ) return 0;
         ProxySet::const_iterator it = m_proxySet.begin();
         for ( size_t j = 0; j <= i; ++j ) ++it;
         return it->second;
      }

      // Another way to access to i-th element. C# doesn't support operator[] through swig
      const RSProxy * rsProxy( size_t i ) const { return (*this)[ i ]; }

      // Search for proxy by name
       RSProxy * rsProxy( const std::string & name ) const { return m_proxySet.count( name ) ? m_proxySet.find( name )->second : 0; }

      // Get all rs proxy names set as an array
      virtual std::vector< std::string > names() const;

      // Check if set is empty
      virtual bool empty() const { return m_proxySet.empty(); }

      // Add new proxy to the set
      void addNewRSProxy( RSProxy * proxy, const std::string & name ) { assert( m_proxySet.count( name ) == 0 ); m_proxySet[name] = proxy; }

      // Serialization / Deserialization

      // version of serialized object representation
      virtual unsigned int version() const { return 0; }

      // Serialize object to the given stream
      virtual bool save( CasaSerializer & sz, unsigned int version ) const;

      // Get type name of the serialaizable object, used in deserialization to create object with correct type
      virtual const char * typeName() const { return "RSProxySetImpl"; }
 
      // Create a new instance and deserialize it from the given stream
      RSProxySetImpl( CasaDeserializer & dz, const char * objName );

   protected:
      ProxySet m_proxySet;

   private:
      // disable copy constructor and copy operator
      RSProxySetImpl( const RSProxySetImpl & );
      RSProxySetImpl & operator = (const RSProxySetImpl &);

   };
}
#endif // CASA_API_RS_PROXY_SET_IMPL_H
