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
class SACcontroller;
class T2Zcontroller;

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

  SACcontroller* sacController_;  
  MapsController* mapsController_;
  T2Zcontroller* t2zController_;
  ResultsController* resultsController_;
};

} // namespace sac

} // namespace casaWizard
