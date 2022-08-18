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
#include "model/sacScenario.h"
#include "view/window.h"

namespace casaWizard
{

namespace sac
{

class MapsController;
class ResultsController;
class InputController;
class T2Zcontroller;
class WellPrepSACcontroller;

class Controller : public MainController
{
  Q_OBJECT

public:
  Controller();
  MainWindow& mainWindow() override;
  CasaScenario& scenario() override;

private:
  Window ui_;
  SACScenario scenario_;

  WellPrepSACcontroller* wellPrepSACcontroller_;
  InputController* inputController_;
  MapsController* mapsController_;
  T2Zcontroller* t2zController_;
  ResultsController* resultsController_;
};

} // namespace sac

} // namespace casaWizard
