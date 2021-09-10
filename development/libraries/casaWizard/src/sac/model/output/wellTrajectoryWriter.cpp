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
    QStringList propertyUserNames;
    const QVector<QVector<const CalibrationTarget*>> targetsInWell = manager.extractWellTargets(propertyUserNames, well->id());

    for(const QString& propertyUserName : propertyUserNames)
    {
      const int propertyIndex = propertyUserNames.indexOf(propertyUserName);
      const QString filename{folder + "/" + well->name() + "_" + propertyUserName + "_" + manager.getCauldronPropertyName(propertyUserName) + ".in"};
      QFile file{filename};

      // Don't write data for wells with a duplicate name
      if (file.exists())
      {
        continue;
      }

      if( !file.open(QIODevice::WriteOnly | QIODevice::Text))
      {
        throw std::exception();
      }
      QTextStream out{&file};

      for (const CalibrationTarget* target : targetsInWell[propertyIndex])
      {
        out << QString::number(well->x(),'f') << " "
            << QString::number(well->y(),'f') << " "
            << QString::number(target->z()) << " "
            << QString::number(target->value()) << " "
            << QString::number(target->standardDeviation()) << "\n";
      }

      file.close();
    }
  }
}

} // namespace wellTrajectoryWriter

} // namespace sac

} // namespace casaWizard
