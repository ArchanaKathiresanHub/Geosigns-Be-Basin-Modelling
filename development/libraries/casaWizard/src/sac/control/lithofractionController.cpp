#include "lithofractionController.h"

#include "view/lithofractionTable.h"
#include "model/sacScenario.h"

#include "model/input/projectReader.h"

#include <QPushButton>
#include <QTableWidgetItem>
#include <QMessageBox>

namespace casaWizard
{

namespace sac
{

LithofractionController::LithofractionController(LithofractionTable* table, SACScenario& scenario, QObject* parent) :
  QObject(parent),
  projectReader_{scenario.projectReader()},
  lithofractionManager_{scenario.lithofractionManager()},
  table_{table}
{
  connect(table_->addRow(),     SIGNAL(clicked()), this, SLOT(slotAddRow()));
  connect(table_->delRow(),     SIGNAL(clicked()), this, SLOT(slotDelRow()));
  connect(table_->fillLayers(), SIGNAL(clicked()), this, SLOT(slotLayersFromProject()));
  connect(table_->table(), SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(slotTableChange(QTableWidgetItem*)));
  updateLithofractionTable();
}

void LithofractionController::slotAddRow()
{
  lithofractionManager_.addLithofraction("New layer");
  updateLithofractionTable();
}

void LithofractionController::slotDelRow()
{
  int row = table_->table()->currentRow();
  if (row < 0)
  {
    row = table_->table()->rowCount()-1;
  }
  if (row >= 0)
  {
    lithofractionManager_.removeLithofraction(row);
    updateLithofractionTable();
  }
}

void LithofractionController::slotTableChange(QTableWidgetItem* item)
{
  const int index = item->row();
  switch (item->column())
  {
    case 0:
      lithofractionManager_.setLithofractionLayerName(index, item->data(0).toString());
      break;
    case 1:
      lithofractionManager_.setLithofractionFirstComponent(index, item->data(0).toInt());
      break;
    case 2:
      lithofractionManager_.setLithofractionFirstMinPercentage(index, item->data(0).toDouble());
      updateLithofractionTable();
      break;
    case 3:
      lithofractionManager_.setLithofractionFirstMaxPercentage(index, item->data(0).toDouble());
      updateLithofractionTable();
      break;
    case 4:
      lithofractionManager_.setLithofractionSecondComponent(index, item->data(0).toInt());
      break;
    case 5:
      lithofractionManager_.setLithofractionSecondMinFraction(index, item->data(0).toDouble());
      updateLithofractionTable();
      break;
    case 6:
      lithofractionManager_.setLithofractionSecondMaxFraction(index, item->data(0).toDouble());
      updateLithofractionTable();
      break;
  }
}

void LithofractionController::slotLayersFromProject()
{
  loadLayersFromProject();
}

void LithofractionController::loadLayersFromProject()
{
  if (!lithofractionManager_.lithofractions().empty())
  {
    QMessageBox willOverwrite(QMessageBox::Icon::Warning,
                             "Warning",
                             "Your changes in lithology will be overwritten by the defaults as defined in the project3d file. Continue anyway?",
                             QMessageBox::Yes | QMessageBox::No);
    if (willOverwrite.exec() == QMessageBox::No)
    {
      return;
    }

    lithofractionManager_.clear();
  }

  const QStringList layerNames = projectReader_.layerNames();
  for (const QString& name : layerNames )
  {
    lithofractionManager_.addLithofraction(name);
  }
  updateLithofractionTable();
}

void LithofractionController::updateLithofractionTable()
{
  table_->updateTable(lithofractionManager_.lithofractions(), projectReader_);
}

} // namespace sac

} // namespace casaWizard
