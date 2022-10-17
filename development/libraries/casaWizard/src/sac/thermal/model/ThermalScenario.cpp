//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ThermalScenario.h"

#include "model/input/projectReader.h"
#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"

static const int s_ThermalScenarioVersion = 0;

namespace casaWizard
{

namespace sac
{

namespace thermal
{

ThermalScenario::ThermalScenario(ProjectReader* projectReader) :
   SacScenario{projectReader},
   m_TCHPManager{}
{
}

TCHPManager& ThermalScenario::TCHPmanager()
{
   return m_TCHPManager;
}

const TCHPManager& ThermalScenario::TCHPmanager() const
{
   return m_TCHPManager;
}

void ThermalScenario::writeToFile(ScenarioWriter& writer) const
{
   SacScenario::writeToFile(writer);
   writer.writeValue("ThermalScenarioVersion", s_ThermalScenarioVersion);

   m_TCHPManager.writeToFile(writer);
}

void ThermalScenario::readFromFile(const ScenarioReader& reader)
{
   SacScenario::readFromFile(reader);
   m_TCHPManager.readFromFile(reader);
}

void ThermalScenario::clear()
{
   SacScenario::clear();
   m_TCHPManager.clear();
}

} // namespace thermal

} // namespace sac

} // namespace casaWizard
