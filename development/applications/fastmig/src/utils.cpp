//
// Copyright (C) 2010-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "utils.h"

namespace migration
{
   namespace utils
   {
      std::string getProjectBaseName (const std::string& projectName)
      {
         std::string result = projectName;
         std::string::size_type dotPos = result.rfind (".project");
         if (dotPos != std::string::npos)
            result.erase (dotPos, std::string::npos);
         return result;
      }

      std::string replaceSpaces (const std::string& str)
      {
         std::string result = str;
         for (unsigned int i = 0; i < result.length (); ++i)
            if (result[i] == ' ') result[i] = '_';
         return result;
      }

   } // namespace utils

} // namespace migration
