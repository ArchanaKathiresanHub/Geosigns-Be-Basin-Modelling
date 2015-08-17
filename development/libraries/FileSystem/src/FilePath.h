//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file FilePath.h 
/// @brief This file keeps FilePath class declaration 

#ifndef IBS_FILE_PATH_H
#define IBS_FILE_PATH_H

#include "Path.h"

namespace ibs {

   /// @class File FilePath.h "FilePath.h"
   /// @brief Class FilePath is a wrapper for os independent typical file operations like exist/size/delete
   class FilePath : public Path
   {
   public:
      /// @brief Constructor
      FilePath( const std::string & filePath );
 
      /// @brief Copy constructor
      FilePath( const Path & aPath ) : Path( aPath ) { ; }
     
      /// @brief Destructor
      virtual ~FilePath();   

      /// @brief delete file
      void remove();

      /// @brief Extract from the path file name
      /// @return file name which is usually the last part of the path
      std::string fileName() const;
   
      /// @brief Cut from the path a file name with extension and return back the rest
      /// @return path to the file without file name
      std::string filePath() const;
      
      /// @brief Extract from the path file name without extension
      /// @return file name without extension
      std::string fileNameNoExtension() const;

      /// @brief Extract from the path file name extension
      /// @return file name extension. If file has no extension, return empty string
      std::string fileNameExtension() const;

      /// @brief Copy file to the given path
      /// @param destPath destination path for the file
      /// @return true on success, false on any error
      bool copyFile( const Path & destPath ); 

      /// @brief Create symbolic link to the given path
      /// @param destPath destination path for the link
      /// @return true on success, false on any error
      bool linkFile( const Path & destPath );

   private:
   };
} // ibs

#endif // IBS_FILE_PATH_H
