//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "sacScript.h"

#include "model/sacScenario.h"

#include <QFile>
#include <QFileInfo>
#include <QTextStream>

namespace casaWizard
{

namespace sac
{

SACScript::SACScript(const SACScenario& scenario, const QString& baseDirectory, const bool doOptimization) :
  CasaScript{baseDirectory},
  scenario_{scenario},
  doOptimization_{doOptimization}
{
}

const CasaScenario& SACScript::scenario() const
{
  return scenario_;
}

QString SACScript::scriptFilename() const
{
  return QString("sacScript.casa");
}

QString SACScript::workingDirectory() const
{
  return QString(scenario_.workingDirectory());
}

bool SACScript::prepareKill() const
{
  return createStopExecFile();
}

bool SACScript::validateScenario() const
{
  return true;
}

void SACScript::writeScriptContents(QFile& file) const
{
  QTextStream out(&file);

  out << writeApp(1, scenario_.applicationName() + " \"-allproperties\" \"-onlyat 0\"");

  // Project filename should not contain path information
  out << writeBaseProject(scenario_.project3dFilename());

  const LithofractionManager& manager{scenario_.lithofractionManager()};
  for (const Lithofraction& lithofraction : manager.lithofractions())
  {
    out << writeLithofraction(lithofraction);
  }

  const CalibrationTargetManager& ctManager = scenario_.calibrationTargetManager();
  const QVector<const Well*>& wells = ctManager.wells();
  for (const Well* well : wells)
  {
    if ( well->isActive())
    {
      for (const QString& propertyUserName : ctManager.getPropertyUserNamesForWell(well->id()))
      {
        if (ctManager.propertyIsActive(propertyUserName))
        {
          out << writeWellTrajectory(well->name(), well->id(), propertyUserName);
        }
      }
    }
  }

  out << QString("generateMulti1D \"Default\" \"none\" 0.01\n");
  out << writeLocation(scenario_.runLocation(), false, !doOptimization_, true);
  out << writeRun(scenario_.clusterName());
  out << writeSaveState(scenario_.stateFileNameSAC());
}

QString SACScript::writeWellTrajectory(const QString& wellName, const int wellIndex, const QString& propertyUserName) const
{
  const QString& propertyCauldronName = scenario_.calibrationTargetManager().getCauldronPropertyName(propertyUserName);
  const QString folder{scenario_.workingDirectory() + "/wells"};
  const QString filename{folder + "/" + wellName + "_" + propertyUserName + "_" + propertyCauldronName + ".in"};
  return QString("target \"" + QString::number(wellIndex) + "_" + propertyCauldronName + "\" WellTraj \"" + filename + "\" \"" + propertyCauldronName + "\" 0 0.0 1.0 1.0\n");
}

QString SACScript::writeLithofraction(const Lithofraction& lithofraction) const
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
