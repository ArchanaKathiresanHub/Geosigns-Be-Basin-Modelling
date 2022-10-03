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

void SACLithologyScript::writeScriptContents(QFile& file) const
{
   QTextStream out(&file);

   out << writeApp(1, scenario().applicationName() + " \"-allproperties\" \"-onlyat 0\"");

   // Project filename should not contain path information
   out << writeBaseProject(scenario().project3dFilename());

   const LithofractionManager& manager{scenario().lithofractionManager()};
   for (const Lithofraction& lithofraction : manager.lithofractions())
   {
      out << writeLithofraction(lithofraction);
   }

   const CalibrationTargetManager& ctManager = scenario().calibrationTargetManager();
   const QVector<const Well*>& wells = ctManager.wells();
   for (const Well* well : wells)
   {
      if ( well->isActive())
      {
         for (const QString& propertyUserName : ctManager.getPropertyUserNamesForWell(well->id()))
         {
            if (scenario().propertyIsActive(propertyUserName))
            {
               out << writeWellTrajectory(well->name(), well->id(), propertyUserName);
            }
         }
      }
   }

   out << QString("generateMulti1D \"Default\" \"none\" 0.01\n");
   out << writeLocation(scenario().runLocation(), false, !doOptimization(), true);
   out << writeRun(scenario().clusterName());
   out << writeSaveState(scenario().stateFileNameSAC());
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
