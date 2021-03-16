//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

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
  lithofractionManager_{scenario.lithofractionManager()},
  projectReader_{scenario.projectReader()},
  table_{table}
{
  connect(table_, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(slotTableChange(QTableWidgetItem*)));
  connect(table_, SIGNAL(firstOptimizationChanged(int, int)), this, SLOT(slotFirstOptimizationChanged(int, int)));
  connect(table_, SIGNAL(secondOptimizationChanged(int, int)), this, SLOT(slotSecondOptimizationChanged(int, int)));

  updateLithofractionTable();
}

void LithofractionController::slotTableChange(QTableWidgetItem* item)
{
  const int index = item->row();
  switch (item->column())
  {
    case 3:
      lithofractionManager_.setLithofractionFirstMinPercentage(index, item->data(0).toDouble());
      updateLithoPercentages(index);
      break;
    case 4:
      lithofractionManager_.setLithofractionFirstMaxPercentage(index, item->data(0).toDouble());
      updateLithoPercentages(index);
      break;
    case 7:
      lithofractionManager_.setLithofractionSecondMinFraction(index, item->data(0).toDouble());
      updateLithoPercentages(index);
      break;
    case 8:
      lithofractionManager_.setLithofractionSecondMaxFraction(index, item->data(0).toDouble());
      updateLithoPercentages(index);
      break;
  }
}

void LithofractionController::updateLithoPercentages(const int index)
{
  table_->updateLithoPercentagesAndFractions(lithofractionManager_.lithofractions(), index);
}

void LithofractionController::slotFirstOptimizationChanged(int state, int row)
{
  lithofractionManager_.setLithoFractionDoFirstOptimization(row, state == Qt::CheckState::Checked);
  table_->disableProhibitedComponents(row);
}

void LithofractionController::slotSecondOptimizationChanged(int state, int row)
{
  lithofractionManager_.setLithoFractionDoSecondOptimization(row, state == Qt::CheckState::Checked);
  table_->disableProhibitedComponents(row);
}

void LithofractionController::loadLayersFromProject()
{
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
