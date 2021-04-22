//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Controller for the table with the lithofractions, used in the SAC wizard
#pragma once

#include <QObject>

class QTableWidgetItem;

namespace casaWizard
{

class ProjectReader;

namespace sac
{

class LithofractionManager;
class LithofractionTable;
class SACScenario;

class LithofractionController : public QObject
{
  Q_OBJECT

public:
  explicit LithofractionController(LithofractionTable* table, SACScenario& scenario, QObject* parent);

  void loadLayersFromProject();
  void updateLithofractionTable();

private slots:
  void slotFirstOptimizationChanged(int state, int row);  
  void slotTableChange(QTableWidgetItem* item);

private:
  LithofractionManager& lithofractionManager_;
  const ProjectReader& projectReader_;
  LithofractionTable* table_;

  void updateLithoPercentages(const int index);
};

} // namespace sac

} // namespace casaWizard
