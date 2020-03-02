#include "objectiveFunctionController.h"

#include "model/calibrationTargetManager.h"
#include "view/objectiveFunctionTable.h"

#include <QTableWidgetItem>

namespace casaWizard
{

ObjectiveFunctionController::ObjectiveFunctionController(ObjectiveFunctionTable* objectiveFunctionTable,
                                                         CalibrationTargetManager& calibrationTargetManager,
                                                         QObject* parent) :
  QObject{parent},
  objectiveFunctionTable_{objectiveFunctionTable},
  calibrationTargetManager_{calibrationTargetManager}
{
  connect(objectiveFunctionTable_, SIGNAL(itemChanged(QTableWidgetItem*)),
          this,                    SLOT(slotTableObjectiveFunctionChanged(QTableWidgetItem*)));

  connect(parent, SIGNAL(signalRefreshChildWidgets()), this, SLOT(slotRefresh()));
}

void ObjectiveFunctionController::slotRefresh()
{
  objectiveFunctionTable_->updateTable(calibrationTargetManager_.objectiveFunctionManager());
}

void ObjectiveFunctionController::slotTableObjectiveFunctionChanged(QTableWidgetItem* item)
{
  calibrationTargetManager_.setObjectiveFunction(item->row(), item->column(), item->data(0).toDouble());
}

}  // namespace casaWizard
