//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "LithologyScript.h"
#include "model/lithofractionManager.h"

#include "model/LithologyScenario.h"
#include "model/logger.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

namespace casaWizard
{

namespace sac
{

namespace lithology
{

LithologyScript::LithologyScript(const LithologyScenario& scenario, const QString& baseDirectory, const bool doOptimization) :
   SACScript(baseDirectory, doOptimization),
   m_scenario(scenario)
{}

const LithologyScenario& LithologyScript::scenario() const
{
   return m_scenario;
}

void LithologyScript::writeParameters(QTextStream& stream) const
{
   const LithofractionManager& manager{m_scenario.lithofractionManager()};
   for (const Lithofraction& lithofraction : manager.lithofractions())
   {
      stream << writeLithofraction(lithofraction);
   }
}

QString LithologyScript::writeLithofraction(const Lithofraction& lithofraction) const
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

} // namespace lithology

} // sac

} // namespace casaWizard
