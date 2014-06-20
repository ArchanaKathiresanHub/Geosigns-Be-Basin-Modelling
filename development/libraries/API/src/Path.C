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

Path & Path::operator -- ( int unused )
{
   if ( !m_path.empty() )
   {
      m_path = boost::filesystem::path( m_path ).branch_path().string();
   }
   return *this;
}

}
