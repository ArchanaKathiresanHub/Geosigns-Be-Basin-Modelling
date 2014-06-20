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

#include "boost/filesystem/operations.hpp"

namespace ibs {


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
   catch ( const boost::filesystem::filesystem_error & ex )
   {
      if ( ex.code() == boost::system::errc::permission_denied )
      {
         throw PathException() << "Permission is denied for one of the sub folders of files in the path " << m_path <<
            ", the error message is " << ex.code().message();
      }
      else
      {
         throw PathException() << "Folder::create( " << m_path << " ) failed with " << ex.code().message();
      }
   }   
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
   catch ( const boost::filesystem::filesystem_error & ex )
   {
      if ( ex.code() == boost::system::errc::permission_denied )
      {
         throw PathException() << "Permission is denied for one of the sub folders of files in the path " << m_path <<
            ", the error message is " << ex.code().message();
      }
      else
      {
         throw PathException() << "Folder::remove( " << m_path << " ) failed with " << ex.code().message();
      }
   }
}

// Check if folder is empty
bool FolderPath::empty( ) const
{
   try
   {
      return exists() ? boost::filesystem::is_empty( m_path ) : true;
   }
   catch ( const boost::filesystem::filesystem_error & ex )
   {
      if ( ex.code() == boost::system::errc::permission_denied )
      {
         throw PathException() << "Permission is denied for one of the sub folders of files in the path " << m_path <<
            ", the error message is " << ex.code().message();
      }
      else
      {
         throw PathException() << "Folder::is_empty( " << m_path << " ) failed with " << ex.code().message();
      }
   }
   return true;
}

// Delete all folder contents
void FolderPath::clean( )
{
   if ( exists() )
   {
      try
      {
         for ( boost::filesystem::directory_iterator it( m_path ); it != boost::filesystem::directory_iterator(); ++it )
         {
            if ( boost::filesystem::is_directory( it->path() ) )
            {
               boost::filesystem::remove_all( it->path() );
            }
            else boost::filesystem::remove( it->path() );
         }
      }
      catch ( const boost::filesystem::filesystem_error & ex )
      {
         if ( ex.code() == boost::system::errc::permission_denied )
         {
            throw PathException() << "Permission is denied for one of the sub folders of files in the path " << m_path <<
                                     ", the error message is " << ex.code().message();
         }
         else
         {
            throw PathException() << "Folder::clean( " << m_path << " ) failed with " << ex.code().message();
         }
      }
   }
}

} // ibs
