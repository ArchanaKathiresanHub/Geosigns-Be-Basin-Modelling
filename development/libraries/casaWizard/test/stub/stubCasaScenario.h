//
// Copyright (C) 2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once
#include "model/casaScenario.h"
#include "stubProjectReader.h"

#include <memory>
namespace casaWizard
{

class StubCasaScenario : public CasaScenario
{
public:
  explicit StubCasaScenario() :
    CasaScenario(new StubProjectReader())
  {
  }

   QString amsterdamDirPath() const override {return m_amsterdamDirPath;}
   QString houstonDirPath() const override {return m_houstonDirPath;}

   QString m_amsterdamDirPath;
   QString m_houstonDirPath;
};

} // casaWizard
