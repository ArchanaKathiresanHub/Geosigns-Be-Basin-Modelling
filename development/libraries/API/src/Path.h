//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file Path.h 
/// @brief This file keeps Path class declaration 

#ifndef IBS_PATH_H
#define IBS_PATH_H

#include "formattingexception.h"
#include "boost/filesystem/path.hpp"

#include <string>
#include <sstream>

namespace ibs
{  
   /// @brief Path, File & Folder can throw PathException which will keep message about any error which cause an exception
   struct PathException : formattingexception::BaseException<PathException> {};

   template <typename T> std::string to_string( const T & prm )
   {
      std::ostringstream oss;
      oss << prm;
      return oss.str();
   }

   /// @brief Parent class for ibs::File & ibs::Folder which keeps common functionality
   class Path
   {
   public:
      Path( const std::string & aPath ) : m_path( aPath ) { ; }
      virtual ~Path() { ; }

      /// @brief Check if given path is exist
      bool exists() const;

      /// @brief Get path as a string
      /// @return path
      std::string path() const { return m_path; }

      /// @brief Get path as a string
      /// @return path
      const char * cpath() const { return m_path.c_str(); }

      /// @brief Strip last element from the path (postfix operator)
      /// @return Path object itself
      Path & cutLast();

      /// @brief Split path by path separator and return the number of elements in path
      /// @return number path elements
      size_t size();

      /// @brief Path element accessor
      /// @param i access to i-th path element
      /// @return i-th path element as a string or empty string if i is out of bounds
      std::string operator [] ( size_t i );

      /// @brief Add to path new element divided by path separator
      /// @param pathEl the new element of the path to be added
      /// @return Path object itself
      template <typename T> Path & operator << ( const T & pathEl )
      {
         m_path = ( boost::filesystem::path( m_path ) / to_string( pathEl ) ).string();
         return *this;
      }

   protected:
      std::string m_path;
   };
} // ibs

#endif // IBS_PATH_H
