//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "activeWellsController.h"

#include "model/casaScenario.h"
#include "view/assets/activeWellsTable.h"

#include <QCheckBox>
#include <QTableWidgetItem>

namespace casaWizard
{

namespace sac
{

ActiveWellsController::ActiveWellsController(ActiveWellsTable* activeWellsTable,
                                             CasaScenario& casaScenario,
                                             QObject* parent) :
   QObject{parent},
   activeWellsTable_{activeWellsTable},
   casaScenario_{casaScenario}
{
   connect(activeWellsTable_, SIGNAL(checkBoxChanged(int, int)),
           this, SLOT(slotWellCheckBoxStateChanged(int, int)));

   connect(parent, SIGNAL(signalRefreshChildWidgets()), this, SLOT(slotRefresh()));
}

void ActiveWellsController::slotClearWellListHighlightSelection()
{
   activeWellsTable_->clearSelection();
}

void ActiveWellsController::slotRefresh()
{
   const CalibrationTargetManager& calibrationTargetManager = casaScenario_.calibrationTargetManager();
   activeWellsTable_->updateTable( calibrationTargetManager.activeWells());
}

void ActiveWellsController::slotWellCheckBoxStateChanged(int state, int wellIndex)
{
   CalibrationTargetManager& calibrationTargetManager = casaScenario_.calibrationTargetManager();
   calibrationTargetManager.setWellIsExcluded(state == Qt::Unchecked, wellIndex);
}

void ActiveWellsController::slotSelectAllWells()
{
   setAllsWellsExcludedState(false);
   slotRefresh();
}

void ActiveWellsController::slotDeselectAllWells()
{
   setAllsWellsExcludedState(true);
   slotRefresh();
}

void ActiveWellsController::setAllsWellsExcludedState(bool excludedState)
{
   CalibrationTargetManager& calibrationTargetManager = casaScenario_.calibrationTargetManager();
   int numWells = calibrationTargetManager.wells().size();
   for (int i = 0; i < numWells; i++)
   {
      calibrationTargetManager.setWellIsExcluded(excludedState, i);
   }
}

}  // namespace sac

}  // namespace casaWizard
