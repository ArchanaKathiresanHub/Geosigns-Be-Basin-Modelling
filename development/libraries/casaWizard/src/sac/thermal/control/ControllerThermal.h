#pragma once
#include "control/SacController.h"
#include "view/Window.h"

namespace casaWizard
{

namespace sac
{

namespace thermal
{

class InputControllerThermal;

class ControllerThermal : public SacController
{
public:
   ControllerThermal();
   virtual ~ControllerThermal() override = default;
   MainWindow& mainWindow() override;

private:
   Window m_ui;
   InputControllerThermal* m_inputController;
};

} // namespace thermal

} // namespace sac

} // namespace casaWizard
