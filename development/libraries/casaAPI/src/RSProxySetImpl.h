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

#include <RSProxySet.h>

#include <map>
#include <cassert>

namespace casa
{
   class RSProxySetImpl : public RSProxySet
   {
   public:
      /// @brief Destructor
      virtual ~RSProxySetImpl();

      /// @brief Get size of the set
      /// @return size of the set
      virtual size_t size() const { return m_proxySet.size(); }

      //Access to i-th element
      virtual const RSProxy * operator[] ( size_t i ) const
      {  
         if ( m_proxySet.size() <= i ) return 0;
         std::map<std::string, RSProxy *>::const_iterator it = m_proxySet.begin();
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

   protected:
      std::map<std::string, RSProxy *> m_proxySet;
   };
}
#endif // CASA_API_RS_PROXY_SET_IMPL_H
