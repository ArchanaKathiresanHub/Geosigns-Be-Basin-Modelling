//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file File.C 
/// @brief This file keeps definition of methods for the File class 

#include "FilePath.h"

#include <stdio.h>

#include "boost/filesystem/path.hpp"
#include "boost/version.hpp"
#include "boost/filesystem.hpp"

#ifndef _WIN32
//#include <libgen.h>
#include <unistd.h>
#else
#include <windows.h>
#endif

#include <iostream>

namespace ibs
{

FilePath::FilePath( const std::string & filePath ) : Path( filePath )
{
}

FilePath::~FilePath()
{
}

void FilePath::remove( )
{
   if ( exists() )
   {
      ::remove( m_path.c_str() );
   }
}

// Extract from the path file name
std::string FilePath::fileName() const
{
   boost::filesystem::path bp( m_path );

// Boost Filesystem interface version 1
#if BOOST_VERSION < 103400
   return bp.leaf();
// Boost Filesystem interface version 2
#elif BOOST_VERSION <= 104900
   return bp.filename();
// Boost Filesystem interface version 3
#else
   return bp.filename().string();
#endif
}

// Cut from the path filen name with extension and return back
std::string FilePath::filePath() const
{
   boost::filesystem::path bp( m_path );

// Boost Filesystem interface version 1 
#if BOOST_VERSION < 103400
   return bp.branch_path().string();
// Boost Filesystem interface version 2 & 3
#else
   return bp.parent_path().string();
#endif
}
      
// Extract from the path file name without extension
std::string FilePath::fileNameNoExtension() const
{
   std::string fnne = fileName();

   if ( !fnne.empty() )
   {
      size_t pp = fnne.rfind( '.' );
      if ( pp != std::string::npos )
      {
         fnne = fnne.substr( 0, pp );
      }
   }

   return fnne;
}

// Extract from the path file name extension
std::string FilePath::fileNameExtension() const
{
   std::string fnne = fileName();

   if ( !fnne.empty() )
   {
      size_t pp = fnne.rfind( '.' );
      if ( pp != std::string::npos )
      {
         fnne = fnne.substr( pp+1 );
      }
      else fnne.clear();
   }

   return fnne;
}


bool FilePath::copyFile( const Path & destPath )
{
   if ( destPath.exists() ) return false;

   try
   {
      const boost::filesystem::path fromPath( m_path );
      const boost::filesystem::perms oldPermission = boost::filesystem::status( fromPath ).permissions();
      boost::filesystem::copy_file( boost::filesystem::absolute( fromPath ), boost::filesystem::path( destPath.path() ) );
      boost::filesystem::permissions( boost::filesystem::path( destPath.path() ), oldPermission | boost::filesystem::owner_all );
   }
   catch ( ... )
   {
      return false;
   }
   return true;
}

bool FilePath::linkFile( const Path & destPath )
{
   if ( destPath.exists() ) return false;

#ifndef _MSC_VER
   const boost::filesystem::path & curPath = boost::filesystem::absolute( boost::filesystem::path( m_path ) );
   try
   { 
      boost::filesystem::create_symlink( curPath, boost::filesystem::path( destPath.path() ) );
   }
   catch ( ... )
   {
      return false;
   }
   return true;
#else
   // On Windows, to create symbolic links Administrator rights are ofter needed
   return false;
#endif
}

}

