//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "model/output/infoGenerator.h"

namespace casaWizard
{

class StubInfoGenerator : public InfoGenerator
{
public:

  void generateInfoTextFile() override
  {
    addSectionSeparator();
    addHeader("Header");
    addOption("Option 1", "Setting A");
    addOption("Option 2", "Setting B");
    addSectionSeparator();

    writeTextToFile();
  }

  void loadProjectReader(const std::string &projectFileName) override
  {
  }
};

}
