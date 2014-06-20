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
      
      /// @brief Destructor
      virtual ~FilePath();   

      /// @brief delete file
      void remove();

      /// @brief extract from the path file name
      /// @return file name which is usually the last part of the path
      std::string fileName() const;

   private:
   };
} // ibs

#endif // IBS_FILE_PATH_H