// Main window for the SAC wizard
#pragma once

#include "view/mainWindow.h"

namespace casaWizard
{

namespace sac
{

class SacInputTab;

class Window : public MainWindow
{
   Q_OBJECT

public:
   Window(QWidget* parent = nullptr);
   virtual ~Window() override = default;
   MenuBar* menu() const override;

private:
   MenuBar* menuBarSAC_;
};

} // namespace sac

} // namespace casaWizard

