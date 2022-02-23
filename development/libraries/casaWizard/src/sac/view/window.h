// Main window for the SAC wizard
#pragma once

#include "view/mainWindow.h"

namespace casaWizard
{

class WellPrepTab;

namespace sac
{

class MapsTab;
class InputTab;
class ResultsTab;
class T2Ztab;

class Window : public MainWindow
{
  Q_OBJECT

public:
  Window(QWidget* parent = nullptr);

  WellPrepTab* wellPrepTab() const;
  InputTab* inputTab() const;
  T2Ztab* t2zTab() const;
  ResultsTab* resultsTab() const;
  MapsTab* mapsTab() const;
  MenuBar* menu() const override;

private:
  WellPrepTab* wellPrepTab_;
  InputTab* inputTab_;
  T2Ztab* t2zTab_;
  ResultsTab* resultsTab_;
  MapsTab* mapsTab_;
  MenuBar* menuBarSAC_;
};

} // namespace sac

} // namespace casaWizard

