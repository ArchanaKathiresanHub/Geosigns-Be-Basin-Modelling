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
   setAllsWellsSelectionState(true);
}

void ActiveWellsController::slotDeselectAllWells()
{
   setAllsWellsSelectionState(false);
}

void ActiveWellsController::setAllsWellsSelectionState(bool selectionState)
{
   Qt::CheckState state;
   if (selectionState)
   {
      state = Qt::CheckState::Checked;
   }
   else
   {
      state = Qt::CheckState::Unchecked;
   }

   for (int i = 0; i < activeWellsTable_->rowCount(); i++)
   {
      QTableWidgetItem* item = activeWellsTable_->item(i, 1);
      Qt::ItemFlags flags = item->flags();
      if (flags & Qt::ItemIsEnabled)
      {
         QCheckBox* itemCheckBox = dynamic_cast<QCheckBox*>(activeWellsTable_->cellWidget(i, 0)->children()[1]);
         itemCheckBox->setCheckState(state);
      }
   }
}

}  // namespace sac

}  // namespace casaWizard
