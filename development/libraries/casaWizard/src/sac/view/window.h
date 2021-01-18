// Main window for the SAC wizard
#pragma once

#include "view/mainWindow.h"

namespace casaWizard
{

namespace sac
{

class MapsTab;
class SACtab;
class ResultsTab;
class T2Ztab;

class Window : public MainWindow
{
  Q_OBJECT

public:
  Window(QWidget* parent = 0);

  SACtab* sacTab() const;
  //T2Ztab* t2zTab() const;
  MapsTab* mapsTab() const;
  ResultsTab* resultsTab() const;
  MenuBar* menu() const override;

private:
  SACtab* sacTab_;
  //T2Ztab* t2zTab_;
  MapsTab* mapsTab_;
  ResultsTab* resultsTab_;
  MenuBar* menuBarSAC_;
};

} // namespace sac

} // namespace casaWizard

