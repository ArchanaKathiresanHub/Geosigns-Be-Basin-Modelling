//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "SacLithologyScript.h"
#include "model/lithofractionManager.h"

#include "model/sacLithologyScenario.h"
#include "model/logger.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

namespace casaWizard
{

namespace sac
{

SACLithologyScript::SACLithologyScript(const SacLithologyScenario& scenario, const QString& baseDirectory, const bool doOptimization) :
   SACScript(baseDirectory, doOptimization),
   m_scenario(scenario)
{}

const SacLithologyScenario& SACLithologyScript::scenario() const
{
   return m_scenario;
}

void SACLithologyScript::writeParameters(QTextStream& stream) const
{
   const LithofractionManager& manager{scenario().lithofractionManager()};
   for (const Lithofraction& lithofraction : manager.lithofractions())
   {
      stream << writeLithofraction(lithofraction);
   }
}

QString SACLithologyScript::writeLithofraction(const Lithofraction& lithofraction) const
{
   auto doubleToQString = [](double d){return QString::number(d, 'g',3); };

   if (!lithofraction.doFirstOptimization())
   {
      return {};
   }

   QString scriptLine = "varprm ";
   scriptLine += "\"StratIoTbl:" + Lithofraction::percentNames[lithofraction.firstComponent()] + "\" "
         + "\"" + lithofraction.layerName() + "\" "
         + doubleToQString(lithofraction.minPercentageFirstComponent()) + " "
         + doubleToQString(lithofraction.maxPercentageFirstComponent()) + " "
         + "\"Normal\"";

   if (lithofraction.doSecondOptimization())
   {
      scriptLine += " \"StratIoTbl:" + Lithofraction::percentNames[lithofraction.secondComponent()] + "\" "
            + doubleToQString(lithofraction.minFractionSecondComponent()) + " "
            + doubleToQString(lithofraction.maxFractionSecondComponent());
   }

   scriptLine += "\n";

   return scriptLine;
}

} // sac

} // namespace casaWizard
