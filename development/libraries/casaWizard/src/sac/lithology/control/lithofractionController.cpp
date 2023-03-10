//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "lithofractionController.h"

#include "view/assets/lithofractionTable.h"
#include "model/LithologyScenario.h"

#include "model/input/projectReader.h"

#include <QTableWidgetItem>

namespace casaWizard
{

namespace sac
{

namespace lithology
{

LithofractionController::LithofractionController(LithofractionTable* table, LithologyScenario& scenario, QObject* parent) :
  QObject(parent),
  lithofractionManager_{scenario.lithofractionManager()},
  projectReader_{scenario.projectReader()},
  table_{table}
{
  connect(table_, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(slotTableChange(QTableWidgetItem*)));
  connect(table_, SIGNAL(firstOptimizationChanged(int, int)), this, SLOT(slotFirstOptimizationChanged(int, int)));  
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
    case 6:
      lithofractionManager_.setLithofractionSecondMinFraction(index, item->data(0).toDouble());
      updateLithoPercentages(index);
      break;
    case 7:
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

void LithofractionController::loadLayersFromProject()
{
  const QStringList layerNames = projectReader_.layerNames();

  for (const QString& name : layerNames )
  {
    const QStringList lithoTypes = projectReader_.lithologyTypesForLayer(projectReader_.getLayerID(name.toStdString()));
    const bool doFirstOptimization = lithoTypes[1] != "";
    const bool doSecondOptimization = lithoTypes[2] != "";
    lithofractionManager_.addLithofraction(name, doFirstOptimization, doSecondOptimization);
  }
}

void LithofractionController::updateLithofractionTable()
{
  table_->updateTable(lithofractionManager_.lithofractions(), projectReader_);
}

} // namespace lithology

} // namespace sac

} // namespace casaWizard
