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
   m_table{table},
   m_manager{manager},
   m_headerNames{}
{
   connect(parent, SIGNAL(signalRefreshChildWidgets()), this, SLOT(slotRefresh()));

   connect(m_table->buttonAdd(),    SIGNAL(clicked()), this, SLOT(slotAddButtonClicked()));
   connect(m_table->buttonDelete(), SIGNAL(clicked()), this, SLOT(slotDeleteButtonClicked()));
   connect(m_table->table(), SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(slotTableItemChanged(QTableWidgetItem*)));
}

void ManualDesignPointController::updateInfluentialParameters(const int numberNew, const QStringList names)
{
   const int numberOld = m_manager.numberOfParameters();
   if (numberNew > numberOld)
   {
      m_manager.addInfluentialParameter(numberNew - numberOld);
   }
   m_headerNames = names;
   slotRefresh();
}

void ManualDesignPointController::removeParameter(const int index)
{
   m_manager.removeInfluentialParameter(index);
   slotRefresh();
}

void ManualDesignPointController::slotAddButtonClicked()
{
   if (m_manager.numberOfParameters() == 0)
   {
      return;
   }

   m_manager.addDesignPoint();
   emit designPointsChanged();
   slotRefresh();
}

void ManualDesignPointController::slotDeleteButtonClicked()
{
   if (m_manager.numberOfParameters() == 0)
   {
      return;
   }

   int row = m_table->table()->currentRow();
   if (row < 0)
   {
      row = m_manager.numberOfVisiblePoints() - 1;
   }
   if (row < 0)
   {
      return;
   }

   if (m_manager.isCompleted(row))
   {
      Logger::log() << "Design point " << (row + 1) << " cannot be removed because the case has already been run" << Logger::endl();
      return;
   }
   m_manager.removeDesignPoint(row);
   emit designPointsChanged();
   slotRefresh();
}

void ManualDesignPointController::slotTableItemChanged(QTableWidgetItem* item)
{
   m_manager.setParameterValue(item->row(), item->column(), item->data(0).toDouble());
}

void ManualDesignPointController::slotRefresh()
{
   QSignalBlocker blocker(m_table);
   m_table->updateTable(m_manager.parameters(), m_manager.completed(), m_headerNames);
}

} // namespace ua

} // namespace casaWizard
