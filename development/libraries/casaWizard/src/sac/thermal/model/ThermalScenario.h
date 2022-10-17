//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Data class for the SAC Thermal Wizard
#pragma once

#include "model/SacScenario.h"
#include "model/TCHPManager.h"

namespace casaWizard
{

namespace sac
{

namespace thermal
{

class ThermalScenario : public SacScenario
{
public:
  ThermalScenario(ProjectReader* projectReader);

  TCHPManager& TCHPmanager();
  const TCHPManager& TCHPmanager() const;

  void writeToFile(ScenarioWriter& writer) const override;
  void readFromFile(const ScenarioReader& reader) override;
  void clear() override;

private:
  TCHPManager m_TCHPManager;
};

} // namespace thermal

} // namespace sac

} // namespace casaWizard
