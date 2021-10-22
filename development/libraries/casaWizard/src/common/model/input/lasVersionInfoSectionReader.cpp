//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "lasVersionInfoSectionReader.h"

namespace casaWizard
{

LASVersionInfoSectionReader::LASVersionInfoSectionReader(const std::vector<std::string>& section, WellData& welldata, ImportOptions& importOptions) :
  LASSectionReader{section, welldata, importOptions}
{

}

void LASVersionInfoSectionReader::readSection()
{

}

} // namespace casaWizard
