#include "controller.h"

#include "control/logDisplayController.h"
#include "control/mapsController.h"
#include "control/resultsController.h"
#include "control/inputController.h"
#include "control/t2zController.h"
#include "control/wellPrepSACcontroller.h"

#include "model/input/cmbProjectReader.h"

namespace casaWizard
{

namespace sac
{

Controller::Controller() :
  MainController(),
  ui_{},
  scenario_{new CMBProjectReader()},
  wellPrepSACcontroller_{new WellPrepSACcontroller{ui_.wellPrepTab(), scenario_, scriptRunController(), this}},
  inputController_{new InputController{ui_.inputTab(), scenario_, scriptRunController(), this}},
  mapsController_{new MapsController{ui_.mapsTab(), scenario_, scriptRunController(), this}},
  t2zController_{new T2Zcontroller{ui_.t2zTab(), scenario_, scriptRunController(), this}},
  resultsController_{new ResultsController{ui_.resultsTab(), scenario_, scriptRunController(), this}}
{
  connect(this, SIGNAL(signalUpdateTabGUI(int)), wellPrepSACcontroller_, SLOT(slotUpdateTabGUI(int)));
  connect(this, SIGNAL(signalUpdateTabGUI(int)), mapsController_,        SLOT(slotUpdateTabGUI(int)));
  connect(this, SIGNAL(signalUpdateTabGUI(int)), resultsController_,     SLOT(slotUpdateTabGUI(int)));
  connect(this, SIGNAL(signalUpdateTabGUI(int)), inputController_,         SLOT(slotUpdateTabGUI(int)));
  connect(this, SIGNAL(signalUpdateTabGUI(int)), t2zController_,         SLOT(slotUpdateTabGUI(int)));

  connect(wellPrepSACcontroller_, SIGNAL(switchToTab(int)), this, SLOT(slotSwitchToTab(int)));

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
