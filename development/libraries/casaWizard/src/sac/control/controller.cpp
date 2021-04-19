#include "controller.h"

#include "control/logDisplayController.h"
#include "control/mapsController.h"
#include "control/resultsController.h"
#include "control/sacController.h"
#include "control/t2zController.h"

#include "model/input/cmbProjectReader.h"

namespace casaWizard
{

namespace sac
{

Controller::Controller() :
  MainController(),
  ui_{},
  scenario_{new CMBProjectReader()},
  sacController_{new SACcontroller{ui_.sacTab(), scenario_, scriptRunController(), this}},  
  mapsController_{new MapsController{ui_.mapsTab(), scenario_, scriptRunController(), this}},
  t2zController_{new T2Zcontroller{ui_.t2zTab(), scenario_, scriptRunController(), this}},
  resultsController_{new ResultsController{ui_.resultsTab(), scenario_, scriptRunController(), this}}
{
  connect(this, SIGNAL(signalUpdateTabGUI(int)), mapsController_,        SLOT(slotUpdateTabGUI(int)));
  connect(this, SIGNAL(signalUpdateTabGUI(int)), resultsController_,     SLOT(slotUpdateTabGUI(int)));
  connect(this, SIGNAL(signalUpdateTabGUI(int)), sacController_,         SLOT(slotUpdateTabGUI(int)));
  connect(this, SIGNAL(signalUpdateTabGUI(int)), t2zController_,         SLOT(slotUpdateTabGUI(int)));

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
