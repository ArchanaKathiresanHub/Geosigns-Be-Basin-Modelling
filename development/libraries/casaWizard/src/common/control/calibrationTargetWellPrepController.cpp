//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//


#include "calibrationTargetWellPrepController.h"

#include "model/casaScenario.h"
#include "view/calibrationTargetTable.h"

namespace casaWizard
{

CalibrationTargetWellPrepController::CalibrationTargetWellPrepController(CalibrationTargetTable* calibTable,
                                                                         CasaScenario& casaScenario,
                                                                         QObject* parent) :
  CalibrationTargetController(calibTable, casaScenario, parent)
{  
}

CalibrationTargetManager& CalibrationTargetWellPrepController::calibrationTargetManager()
{
  return casaScenario().calibrationTargetManagerWellPrep();
}

const CalibrationTargetManager& CalibrationTargetWellPrepController::calibrationTargetManager() const
{
  return casaScenario().calibrationTargetManagerWellPrep();
}

QMap<QString, QSet<int>> CalibrationTargetWellPrepController::getPropertyNamesPerWell() const
{
  QMap<QString, QSet<int>> propertyNamesPerWell;
  for (const Well* well :  calibrationTargetManager().wells())
  {
    const int wellId = well->id();
    for ( const QString& propertyUserName : calibrationTargetManager().getPropertyUserNamesForWell(wellId))
    {
      propertyNamesPerWell[propertyUserName].insert(wellId);
    }
  }
  return propertyNamesPerWell;
}

void CalibrationTargetWellPrepController::slotDeleteSelectedWells()
{
  const QVector<int> wellSelection = calibrationTable()->getWellSelection();
  calibrationTargetManager().deleteWells(wellSelection);

  slotRefresh();
}

}  // namespace casaWizard
