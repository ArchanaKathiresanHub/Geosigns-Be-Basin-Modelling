//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Genex0dDataExtractor function: extracts data from maps and updates dataMining table

#pragma once

#include "datadriller.h"

namespace Genex0d
{

namespace Genex0dDataExtractor
{

void run(const std::string& projectFileName)
{
  DataExtraction::DataDriller dataDriller(projectFileName);
  dataDriller.run(true);
  dataDriller.saveToFile(projectFileName);
}

} // namespace Genex0dDataExtractor

} // namespace genex0d
