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

#if BOOST_VERSION < 103400
   return bp.leaf();
#else
   return bp.filename().string();
#endif
}

// Cut from the path filen name with extension and return back
std::string FilePath::filePath() const
{
   boost::filesystem::path bp( m_path );

#if BOOST_VERSION < 103400
   return bp.branch_path().string();
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

}

