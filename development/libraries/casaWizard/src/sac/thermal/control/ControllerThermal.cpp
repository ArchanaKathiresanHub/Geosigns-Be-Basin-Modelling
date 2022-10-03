#include "ControllerThermal.h"
#include "view/Window.h"

#include "control/logDisplayController.h"
#include "InputControllerThermal.h"

namespace casaWizard
{

namespace sac
{

namespace thermal
{

ControllerThermal::ControllerThermal() :
   SacController(),
   m_ui{},
   m_inputController{new InputControllerThermal{m_ui.inputTab(), this->scenario(), scriptRunController(), this}}
{
   m_ui.show();
   constructWindow(new LogDisplayController(m_ui.logDisplay(), this));
}

MainWindow& ControllerThermal::mainWindow()
{
  return m_ui;
}

}

}

}
