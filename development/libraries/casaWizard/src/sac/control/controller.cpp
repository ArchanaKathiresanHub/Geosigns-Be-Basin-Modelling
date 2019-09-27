#include "controller.h"

#include "control/logDisplayController.h"
#include "control/sacController.h"
#include "control/resultsController.h"
#include "control/t2zController.h"

#include "model/input/cmbProjectReader.h"

namespace casaWizard
{

namespace sac
{

Controller::Controller() :
  MainController(),
  ui_{},
  scenario_{std::unique_ptr<ProjectReader>(new CMBProjectReader())},
  sacController_{new SACcontroller{ui_.sacTab(), scenario_, scriptRunController(), this}},
  //t2zController_{new T2Zcontroller{ui_.t2zTab(), scenario_, scriptRunController(), this}},
  resultsController_{new ResultsController{ui_.resultsTab(), scenario_, scriptRunController(), this}}
{
  connect(this, SIGNAL(signalRefresh()), sacController_, SLOT(slotRefresh()));
  connect(this, SIGNAL(signalRefreshAfterOpen()), sacController_, SLOT(extractAfterOpen()));
  connect(this, SIGNAL(signalRefresh()), resultsController_, SLOT(slotRefresh()));

  constructWindow(new LogDisplayController(ui_.logDisplay(), this));
  ui_.show();
}

MainWindow& Controller::mainWindow()
{
  return ui_;
}

CasaScenario& Controller::scenario()
{
  return scenario_;
}

} // namespace sac

} // namespace casaWizard
