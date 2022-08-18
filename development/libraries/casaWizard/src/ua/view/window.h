// Main window for the Thermal Wizard UA
#pragma once

#include "view/mainWindow.h"

namespace casaWizard
{

namespace ua
{

class CorrelationTab;
class MenuBarUA;
class ModelInputsTab;
class ResponseSurfacesTab;
class TargetTab;
class UAResultsTab;

class Window : public MainWindow
{
  Q_OBJECT

public:
  Window(QWidget* parent = 0);

  ModelInputsTab* modelInputsTab() const;
  TargetTab* targetTab() const;
  ResponseSurfacesTab* responseSurfacesTab() const;
  UAResultsTab* uaResultsTab() const;
  CorrelationTab* correlationsTab() const;
  MenuBar* menu() const override;
  MenuBarUA* menuUA() const;

private:
  ModelInputsTab* m_modelInputsTab;
  TargetTab* targetTab_;
  ResponseSurfacesTab* m_responseSurfacesTab;
  UAResultsTab* m_uaResultsTab;
  CorrelationTab* correlationsTab_;
  MenuBarUA* menuBarUA_;
};

} // namespace ua

} // namespace casaWizard

