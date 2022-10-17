//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Controller for the SAC wizard
#pragma once

#include "control/SacController.h"
#include "model/sacLithologyScenario.h"
#include "view/LithologyWindow.h"

namespace casaWizard
{

namespace sac
{

class MapsController;
class T2Zcontroller;
class WellPrepSACcontroller;

namespace lithology
{

class LithologyInputController;
class LithologyResultsController;

class LithologyController : public SacController
{
  Q_OBJECT

public:
  LithologyController();
  virtual ~LithologyController() override = default;
  MainWindow& mainWindow() final;
  SacLithologyScenario& scenario() final;

private:
  LithologyWindow m_ui;
  SacLithologyScenario m_scenario;
  WellPrepSACcontroller* m_wellPrepSACcontroller;
  LithologyInputController* m_inputController;
  MapsController* m_mapsController;
  T2Zcontroller* m_t2zController;
  LithologyResultsController* m_resultsController;
};

} // namespace lithology

} // namespace sac

} // namespace casaWizard
