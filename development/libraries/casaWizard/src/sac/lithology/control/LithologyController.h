//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Controller for the SAC wizard
#pragma once

#include "control/mainController.h"
#include "model/LithologyScenario.h"
#include "view/LithologyWindow.h"

namespace casaWizard
{

namespace sac
{

class LithologyMapsController;
class ResultsController;
class T2Zcontroller;
class WellPrepSACcontroller;

namespace lithology
{

class LithologyInputController;
class LithologyResultsController;

class LithologyController : public MainController
{
  Q_OBJECT

public:
  LithologyController();
  virtual ~LithologyController() override = default;

  MainWindow& mainWindow() final;
  LithologyScenario& scenario() final;

private:
  LithologyWindow m_ui;
  LithologyScenario m_scenario;
  WellPrepSACcontroller* m_wellPrepSACcontroller;
  LithologyInputController* m_inputController;
  LithologyMapsController* m_mapsController;
  T2Zcontroller* m_t2zController;
  LithologyResultsController* m_resultsController;
};

} // namespace lithology

} // namespace sac

} // namespace casaWizard
