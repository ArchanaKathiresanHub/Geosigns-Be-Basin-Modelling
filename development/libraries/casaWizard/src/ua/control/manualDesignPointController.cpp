// Controller for the manual design points
#include "manualDesignPointController.h"

#include "model/logger.h"
#include "model/manualDesignPointManager.h"
#include "view/manualDesignPointTable.h"

#include <QPushButton>
#include <QTableWidgetItem>

namespace casaWizard
{

namespace ua
{

ManualDesignPointController::ManualDesignPointController(ManualDesignPointTable* table,
                                                         ManualDesignPointManager& manager,
                                                         QObject* parent) :
  QObject(parent),
  table_{table},
  manager_{manager},
  headerNames_{}
{
  connect(parent, SIGNAL(signalRefreshChildWidgets()), this, SLOT(slotRefresh()));

  connect(table_->buttonAdd(),    SIGNAL(clicked()), this, SLOT(slotAddButtonClicked()));
  connect(table_->buttonDelete(), SIGNAL(clicked()), this, SLOT(slotDeleteButtonClicked()));
  connect(table_->table(), SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(slotTableItemChanged(QTableWidgetItem*)));
}

void ManualDesignPointController::updateInfluentialParameters(const int numberNew, const QStringList names)
{
  const int numberOld = manager_.numberOfParameters();
  if (numberNew > numberOld)
  {
    manager_.addInfluentialParameter(numberNew - numberOld);
  }
  headerNames_ = names;
  slotRefresh();
}

void ManualDesignPointController::removeParameter(const int index)
{
  manager_.removeInfluentialParameter(index);
  slotRefresh();
}

void ManualDesignPointController::slotAddButtonClicked()
{
  manager_.addDesignPoint();
  emit designPointsChanged();
  slotRefresh();
}

void ManualDesignPointController::slotDeleteButtonClicked()
{
  int row = table_->table()->currentRow();
  if (row < 0)
  {
    row = manager_.numberOfPoints() - 1;
  }
  if (row < 0)
  {
    return;
  }

  QVector<bool> completed = manager_.completed();
  if (completed[row])
  {
    Logger::log() << "Design point " << (row + 1) << " cannot be removed because the case has already been run" << Logger::endl();
    return;
  }
  manager_.removeDesignPoint(row);
  emit designPointsChanged();
  slotRefresh();
}

void ManualDesignPointController::slotTableItemChanged(QTableWidgetItem* item)
{
  manager_.setParameterValue(item->row(), item->column(), item->data(0).toDouble());
}

void ManualDesignPointController::slotRefresh()
{
  QSignalBlocker blocker(table_);
  table_->updateTable(manager_.parameters(), manager_.completed(), headerNames_);
}

} // namespace ua

} // namespace casaWizard
