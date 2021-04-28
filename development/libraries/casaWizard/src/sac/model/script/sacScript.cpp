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
  const WellTrajectoryManager& mgr = scenario_.wellTrajectoryManager();
  for (const WellTrajectory& wellTrajectory: mgr.trajectoriesType(TrajectoryType::Original1D))
  {
    const Well* well = wells[wellTrajectory.wellIndex()];
    if ( well->isActive() )
    {
      out << writeWellTrajectory(wellTrajectory, well->name());
    }
  }

  out << QString("generateMulti1D \"Default\" \"none\" 0.01\n");
  out << writeLocation(scenario_.runLocation(), false, !doOptimization_, true);
  out << writeRun(scenario_.clusterName());
  out << writeSaveState(scenario_.stateFileNameSAC());
}

QString SACScript::writeWellTrajectory(const WellTrajectory& trajectory, const QString& wellName) const
{
  const QString folder{scenario_.workingDirectory() + "/wells"};
  const QString filename{folder + "/" + wellName + "_" + trajectory.property() + ".in"};
  return QString("target \"" + trajectory.name() + "\" WellTraj \"" + filename + "\" \"" + trajectory.property() + "\" 0 0.0 1.0 1.0\n");
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
