#include "lithosphereParameterController.h"

#include "model/ctcScenario.h"
#include "view/lithosphereParameterTable.h"
#include "view/ctcTab.h"

#include <QComboBox>
#include <QPushButton>
#include <QString>
#include <QTableWidgetItem>

namespace ctcWizard
{

LithosphereParameterController::LithosphereParameterController(LithosphereParameterTable* table,
                                                               CtcScenario& ctcScenario,
                                                               QObject* parent) :
  QObject(parent),
  table_(table),
  ctcScenario_(ctcScenario)
{
  connect(table_->tableWidgetLithosphereParameter(), SIGNAL(itemChanged(QTableWidgetItem*)),
          this,                                      SLOT(slotTableWidgetLithosphereParameterItemChanged(QTableWidgetItem*)));

  updateLithosphereParameterTable();

}


void LithosphereParameterController::slotTableWidgetLithosphereParameterItemChanged(QTableWidgetItem* item)
{
  ctcScenario_.setLithosphereParameter(item->row(), item->column(), item->data(0).toString());
}

void LithosphereParameterController::updateLithosphereParameterTable()
{
  table_->updateTable(ctcScenario_.lithosphereParameters());
}

} // namespace ctcWizard
