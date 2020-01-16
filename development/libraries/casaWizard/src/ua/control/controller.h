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
class DoEcontroller;
class QCController;
class TargetController;
class MCMCController;

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

  DoEcontroller* doeController_;
  TargetController* targetController_;
  QCController* qcController_;
  MCMCController* mcmcController_;
  CorrelationController* correlationController_;
};

} // namespace ua

} // namespace casaWizard
