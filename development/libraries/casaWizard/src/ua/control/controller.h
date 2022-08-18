//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Controller for the UA thermal wizard
#pragma once

#include "control/mainController.h"
#include "model/uaScenario.h"
#include "view/window.h"

namespace casaWizard
{

namespace ua
{

class CorrelationController;
class ModelInputsController;
class ResponseSurfacesController;
class TargetController;
class UAResultsController;

class Controller : public MainController
{
  Q_OBJECT

public:
  Controller();
  MainWindow& mainWindow() override;
  CasaScenario& scenario() override;

private slots:
  void slotPopupRemoveDoeData();
  void slotPopupRemoveDoeDataAll();

private:
  Window ui_;
  UAScenario scenario_;

  ModelInputsController* m_modelInputsController;
  TargetController* m_targetController;
  ResponseSurfacesController* m_responseSurfacesController;
  UAResultsController* m_uaResultsController;
  CorrelationController* m_correlationController;
};

} // namespace ua

} // namespace casaWizard
