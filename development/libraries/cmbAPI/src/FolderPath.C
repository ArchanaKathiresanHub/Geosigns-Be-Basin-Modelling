//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file Folder.C
/// @brief This file keeps definition of methods for the Folder class 

#include "FolderPath.h"

#include "boost/version.hpp"

#include "boost/filesystem/operations.hpp"

#if BOOST_VERSION < 103400

#include "boost/filesystem/convenience.hpp"
#include "boost/filesystem/exception.hpp"
#include "boost/filesystem/path.hpp"

#endif

namespace ibs {

static void convertBoostException( const boost::filesystem::filesystem_error & ex, const std::string & fnName, const std::string & pth )
{
#if BOOST_VERSION < 103400
   int errCode = ex.error();
   std::string msg = ex.what();

   if ( errCode == boost::filesystem::security_error )
#else         
   std::string msg = ex.code().message();
   if ( ex.code() == boost::system::errc::permission_denied )
#endif
   {
      throw PathException() << "Permission is denied for one of the sub folders of files in the path " << pth <<
                               ", the error message is " << msg;
   }
   else
   {
      throw PathException() << "FolderPath::" << fnName << "( " << pth << " ) failed with " << msg;
   }
}

// Create folder
void FolderPath::create()
{
   try
   {
      if ( !exists() )
      {
         boost::filesystem::create_directories( m_path );
      }
   }
   catch ( const boost::filesystem::filesystem_error & ex ) { convertBoostException( ex, "create", m_path ); }   
}


// delete folder and it contents
void FolderPath::remove( )
{
   try
   {
      if ( exists() )
      {
         boost::filesystem::remove_all( m_path );
      }
   }
   catch ( const boost::filesystem::filesystem_error & ex ) { convertBoostException( ex, "remove", m_path  ); }   
}

// Check if folder is empty
bool FolderPath::empty( ) const
{
   try
   {
      return exists() ? boost::filesystem::is_empty( m_path ) : true;
   }
   catch ( const boost::filesystem::filesystem_error & ex ) { convertBoostException( ex, "is_empty", m_path  ); }   

   return true;
}

// Delete all folder contents
void FolderPath::clean()
{
   if ( exists() )
   {
      try
      {
         for ( boost::filesystem::directory_iterator it( m_path ); it != boost::filesystem::directory_iterator(); ++it )
         {
#if BOOST_VERSION < 103400
            std::string pth = (*it).string();
#else
            std::string pth = it->path().string();
#endif
            if ( boost::filesystem::is_directory( pth ) )
            {
               boost::filesystem::remove_all( pth );
            }
            else boost::filesystem::remove( pth );
         }
      }
      catch ( const boost::filesystem::filesystem_error & ex ) { convertBoostException( ex, "clean", m_path  ); }   
   }
}

} // ibs
