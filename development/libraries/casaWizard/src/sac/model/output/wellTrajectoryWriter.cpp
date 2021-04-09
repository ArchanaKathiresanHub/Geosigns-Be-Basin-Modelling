//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "wellTrajectoryWriter.h"

#include "model/sacScenario.h"

#include <QVector>
#include <QDir>
#include <QTextStream>

namespace casaWizard
{

namespace sac
{

namespace wellTrajectoryWriter
{

void writeTrajectories(SACScenario& scenario)
{
  const CalibrationTargetManager& manager = scenario.calibrationTargetManager();
  const QVector<const Well*> wells = manager.wells();

  if( wells.isEmpty() )
  {
    return;
  }

  const QString folder{scenario.workingDirectory() + "/wells"};
  QDir().mkdir(folder);

  for (const Well* well : wells)
  {
    QStringList properties;
    const QVector<QVector<CalibrationTarget>> targetsInWell = manager.extractWellTargets(properties, well->id());

    for(const QString& property : properties)
    {
      const int propertyIndex = properties.indexOf(property);
      const QString filename{folder + "/" + well->name() + "_" + property + ".in"};
      QFile file{filename};
      if( !file.open(QIODevice::WriteOnly | QIODevice::Text))
      {
        throw std::exception();
      }
      QTextStream out{&file};

      for (const CalibrationTarget& target : targetsInWell[propertyIndex])
      {
        out << QString::number(well->x(),'f') << " "
            << QString::number(well->y(),'f') << " "
            << QString::number(target.z()) << " "
            << QString::number(target.value()) << " "
            << QString::number(target.standardDeviation()) << "\n";
      }

      file.close();
    }
  }

}

} // namespace wellTrajectoryWriter

} // namespace sac

} // namespace casaWizard
