//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file FolderPath.h 
/// @brief This file keeps FolderPath class declaration 

#ifndef IBS_FOLDER_H
#define IBS_FOLDER_H

#include "Path.h"

namespace ibs
{

   /// @class FolderPath FolderPath.h "FolderPath.h"
   /// @brief Class FolderPath is a wrapper for OS independent typical folder operations like exist/empty/delete
   class FolderPath : public Path
   {
   public:
      /// @brief Constructor
      FolderPath( const std::string & folderPath ) : Path( folderPath ) { ; }

      /// @brief Copy constructor
      FolderPath( const FolderPath & fld ) : Path( fld.m_path ) { ; }

      /// @brief Destructor
      virtual ~FolderPath( ) { ; }

      /// @brief Create folder 
      void create();

      /// @brief Delete folder and it contents
      void remove();

      /// @brief Check if the folder has noting in it
      /// @return true if the folder is empty, false otherwise
      bool empty() const;

      /// @brief Delete all folder contents and make it clean
      void clean(); 

      /// @brief copy operator
      /// @param fld folder to copy
      /// @return Folder object itself
      FolderPath & operator = ( const FolderPath & fld ) { m_path = fld.m_path; return *this; }

   private:
   };

} // ibs

#endif // IBS_FOLDER_H