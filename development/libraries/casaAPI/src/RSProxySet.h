//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file RSProxySet.h
/// @brief This file keeps interface class declaration to manage the set of response proxy objects.

#ifndef CASA_API_RS_PROXY_SET_H
#define CASA_API_RS_PROXY_SET_H

#include <string>
#include <vector>

namespace casa
{
   class RSProxy;

   /// @brief RSProxySet class keep list of casa::RSProxy objects and allows to manage them.
   ///        Also it can filter objects in set depending on experiment name.
   class RSProxySet
   {
   public:
      /// @brief Destructor
      virtual ~RSProxySet( ) {;}

      /// @brief Get size of the set
      /// @return size of the set
      virtual size_t size() const = 0;

      /// @brief Access to i-th element
      /// @param i position element in the list
      /// @return pointer to RSProxy object on success, or null pointer otherwise. User should not delete this object
      virtual const RSProxy * operator[] ( size_t i ) const = 0;

      /// @brief Another way to access to i-th element. C# doesn't support operator[] through swig
      /// @param i position of the element in the list
      /// @return pointer to RSProxy object on success, or null pointer otherwise. User should not delete this object
      const RSProxy * rsProxy( size_t i ) const { return (*this)[ i ]; }

      /// @brief Search proxy by name
      /// @param name proxy name
      /// @return pointer to RSProxy object on success, or null pointer otherwise. User should not delete this object
      virtual RSProxy * rsProxy( const std::string & name ) const = 0;

      /// @brief Get all rs proxy names set as an array
      /// @return list of proxy names as an array
      virtual std::vector< std::string > names() const = 0;

      /// @brief Is set empty
      /// @return true if set is empty, false otherwise
      virtual bool empty() const = 0;

   protected:
      RSProxySet() { ; }
   };

}

#endif // CASA_API_RS_PROXY_SET_H
