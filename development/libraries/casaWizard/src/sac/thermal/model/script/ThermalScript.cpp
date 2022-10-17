//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ThermalScript.h"
#include "model/TCHPManager.h"

#include "model/SacScenario.h"
#include "model/logger.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

namespace casaWizard
{

namespace sac
{

namespace thermal
{

ThermalScript::ThermalScript(const ThermalScenario& scenario,
                             const QString& baseDirectory,
                             const bool doOptimization) :
   SACScript(baseDirectory, doOptimization),
   m_scenario(scenario)
{}

const SacScenario& ThermalScript::scenario() const
{
   return m_scenario;
}

void ThermalScript::writeParameters(QTextStream& stream) const
{
   const double min = m_scenario.TCHPmanager().minTCHP();
   const double max = m_scenario.TCHPmanager().maxTCHP();

   QString scriptLine = "varprm \"BasementIoTbl:TopCrustHeatProd\" ";
   scriptLine.append(QString::number(min, 'g', 3));
   scriptLine.append(" ");
   scriptLine.append(QString::number(max, 'g', 3));
   scriptLine.append(" ");
   scriptLine.append("\"Normal\"\n");

   stream << scriptLine;
}


} // namespace thermal

} // namespace sac

} // namespace casaWizard
