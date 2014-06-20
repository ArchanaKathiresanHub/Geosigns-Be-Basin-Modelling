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
   return bp.filename().string();
}

}
