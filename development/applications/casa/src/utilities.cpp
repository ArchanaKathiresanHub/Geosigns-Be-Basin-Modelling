// Copyright (C) 2012-2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "utilities.h"

#include "CauldronEnvConfig.h"
#include "Path.h"

namespace casaAppUtils
{

void checkCauldronVersion(std::string& currentVersionPath)
{
  if ( currentVersionPath.empty() || currentVersionPath == "Default" )
  {
     currentVersionPath = ibs::Path::applicationFullPath().path();
     std::size_t index = currentVersionPath.find(IBS_INSTALL_PATH);
     if (index != std::string::npos)
     {
       currentVersionPath = currentVersionPath.substr(index);
     }
  }
}

}
