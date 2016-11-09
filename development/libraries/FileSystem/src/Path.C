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

#include "boost/filesystem.hpp"

#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

namespace ibs {

#define MAX_PATH_LEN 4096
Path Path::applicationFullPath()
{
   std::string myPath;

   char * pathBuf = new char[MAX_PATH_LEN];
   int pathBufSize = MAX_PATH_LEN;

   do
   {
#ifdef _WIN32
      int     bytes = GetModuleFileName( NULL, pathBuf, pathBufSize );
#else
      ssize_t bytes = readlink( "/proc/self/exe", pathBuf, pathBufSize  );
#endif
      if ( !bytes ) break; // something wrong, can't get path

      if ( bytes < pathBufSize ) // call successful, copy result to string
      {
         pathBuf[bytes] = '\0';
         myPath = std::string( pathBuf );
         break;
      }
      // too small buffer - increase it
      delete [] pathBuf;
      pathBufSize *= 2;
      if ( pathBufSize >= 32768U ) break; // too long buffer

      pathBuf = new char[pathBufSize];

   } while( true );
   
   delete [] pathBuf;

   return !myPath.empty() ? Path( myPath ).cutLast() : Path( "." ).fullPath(); 
}

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
size_t Path::size() const
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
std::string Path::operator [] ( size_t i ) const
{
   boost::filesystem::path thePath( m_path );

   boost::filesystem::path::iterator it = thePath.begin();

   while( i > 0 && it != thePath.end() ) { ++it; --i; }

   if (it != thePath.end())
      return it->string();
   else
      return "";
}

Path Path::fullPath() const
{
   return Path( boost::filesystem::absolute( boost::filesystem::path( m_path ) ).string() );
}

bool Path::setPath( )
{
   bool status = false;
#ifdef _WIN32
   if ( SetCurrentDirectory( fullPath().cpath() ) ) status = true;
#else
   if ( chdir( fullPath( ).cpath( ) ) == 0 ) status = true;
#endif
   return status;
}
}
