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
  connect(this, SIGNAL(signalUpdateTabGUI(int)), resultsController_, SLOT(slotUpdateTabGUI(int)));
  connect(this, SIGNAL(signalUpdateTabGUI(int)), sacController_,     SLOT(slotUpdateTabGUI(int)));
  connect(this, SIGNAL(signalProjectOpened()),   sacController_,     SLOT(slotExtractAfterOpen()));

  ui_.show();
  constructWindow(new LogDisplayController(ui_.logDisplay(), this));
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
