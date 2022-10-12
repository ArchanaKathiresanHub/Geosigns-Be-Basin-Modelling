//
// Copyright (C) 2010-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _MIGRATION_UTILS_H
#define _MIGRATION_UTILS_H

#include <string>

namespace migration
{

   namespace utils
   {

      std::string getProjectBaseName (const std::string& projectName);

      std::string replaceSpaces (const std::string& str);

   } // namespace utils

} // namespace migration

#endif
