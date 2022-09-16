//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Controller for the correlations tab in the thermal wizard, plotting the correlation matrix
// of the influential parameters for the MC samples.
#pragma once

#include <QObject>

class QTableWidgetItem;

namespace casaWizard
{

class ScriptRunController;

namespace ua
{

enum class TabID;

class CorrelationTab;
class UAScenario;

class CorrelationController : public QObject
{
  Q_OBJECT

public:
  CorrelationController(CorrelationTab* correlationTab,
                        UAScenario& casaScenario,
                        ScriptRunController& scriptRunController,
                        QObject* parent);

private slots:
  void correlationSelectionItemChanged();

  void slotUpdateTabGUI(int tabID);

private:
  void refreshGUI();
  void updateCorrelationPlotLayout();
  void updateCorrelationItem(const int row);

  CorrelationTab* correlationTab_;
  UAScenario& casaScenario_;
  ScriptRunController& scriptRunController_;
};

} // namespace ua

} // namespace casaWizard
