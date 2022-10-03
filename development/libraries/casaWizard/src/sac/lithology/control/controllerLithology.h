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
#include "view/Window.h"

namespace casaWizard
{

namespace sac
{

class MapsController;
class ResultsController;
class T2Zcontroller;
class WellPrepSACcontroller;

namespace lithology
{

class InputControllerLithology;

class ControllerLithology : public SacController
{
  Q_OBJECT

public:
  ControllerLithology();
  virtual ~ControllerLithology() override = default;
  MainWindow& mainWindow() override;
  SacScenario& scenario() override;

private:
  Window ui_;
  SacLithologyScenario scenario_;

  WellPrepSACcontroller* wellPrepSACcontroller_;
  InputControllerLithology* inputController_;
  MapsController* mapsController_;
  T2Zcontroller* t2zController_;
  ResultsController* resultsController_;
};

} // namespace lithology

} // namespace sac

} // namespace casaWizard
