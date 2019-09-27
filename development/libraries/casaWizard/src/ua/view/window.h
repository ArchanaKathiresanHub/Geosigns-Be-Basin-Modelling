// Main window for the Thermal Wizard UA
#pragma once

#include "view/mainWindow.h"

namespace casaWizard
{

namespace ua
{

class CorrelationTab;
class DoeTab;
class MenuBarUA;
class QCTab;
class TargetTab;
class MCMCTab;

class Window : public MainWindow
{
  Q_OBJECT

public:
  Window(QWidget* parent = 0);

  DoeTab* doeTab() const;
  TargetTab* targetTab() const;
  QCTab* qcTab() const;
  MCMCTab* mcmcTab() const;
  CorrelationTab* correlationsTab() const;
  MenuBar* menu() const override;
  MenuBarUA* menuUA() const;

private:
  DoeTab* doeTab_;
  TargetTab* targetTab_;
  QCTab* qcTab_;
  MCMCTab* mcmcTab_;
  CorrelationTab* correlationsTab_;
  MenuBarUA* menuBarUA_;
};

} // namespace ua

} // namespace casaWizard

