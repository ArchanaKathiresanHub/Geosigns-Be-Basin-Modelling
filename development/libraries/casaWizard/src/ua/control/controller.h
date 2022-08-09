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

  ModelInputsController* m_modelInputsController;
  TargetController* m_targetController;
  ResponseSurfacesController* m_responseSurfacesController;
  MCMCController* m_mcmcController;
  CorrelationController* m_correlationController;
};

} // namespace ua

} // namespace casaWizard
