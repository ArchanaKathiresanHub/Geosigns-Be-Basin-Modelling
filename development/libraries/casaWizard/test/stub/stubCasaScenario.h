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

   QString amsterdamDirPath() const override
   {
      return m_amsterdamDirPath == "" ? CasaScenario::amsterdamDirPath() : m_amsterdamDirPath;
   }
   QString houstonDirPath() const override
   {
      return m_houstonDirPath == "" ? CasaScenario::houstonDirPath() : m_houstonDirPath;
   }
   QString bpa2ToolsPath() const override
   {
      return m_bpa2ToolsPath == "" ? CasaScenario::bpa2ToolsPath() : m_bpa2ToolsPath;
   }

   QString m_amsterdamDirPath;
   QString m_houstonDirPath;
   QString m_bpa2ToolsPath;
};

} // casaWizard
