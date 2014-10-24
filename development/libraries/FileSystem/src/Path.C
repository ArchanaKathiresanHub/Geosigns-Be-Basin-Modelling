//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file Path.C
/// @brief This file keeps definition of methods for the Path class 

#include "Path.h"

#include <sys/types.h>
#include <sys/stat.h>

namespace ibs {

// Check if given path is exist
bool Path::exists() const
{
   bool ret = false;
   if ( !m_path.empty() )
   {
      struct stat File_Stats;
      ret = stat( m_path.c_str(), &File_Stats ) == 0 ? true : false;
   }
   return ret;
}

// Strip last element from the path (postfix operator)
Path & Path::cutLast()
{
   if ( !m_path.empty() )
   {
      m_path = boost::filesystem::path( m_path ).branch_path().string();
   }
   return *this;
}

// Split path by path separator and return the number of elements in path
size_t Path::size()
{
   int sz = 0;
   boost::filesystem::path thePath( m_path );

   for ( boost::filesystem::path::iterator it = thePath.begin(); it != thePath.end(); ++it )
   {
      if ( !(*it).empty() ) ++sz;
   }
   return sz;
}

// Path element accessor
std::string Path::operator [] ( size_t i )
{
   boost::filesystem::path thePath( m_path );

   boost::filesystem::path::iterator it = thePath.begin();

   while( i > 0 && it != thePath.end() ) { ++it; --i; }

   if (it != thePath.end())
      return it->string();
   else
      return "";
}

}
