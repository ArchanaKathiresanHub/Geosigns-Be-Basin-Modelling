// Main window for the SAC wizard
#pragma once

#include "view/SacWindow.h"

namespace casaWizard
{

namespace sac
{

namespace thermal
{

class MapsTab;
class InputTab;
class ResultsTab;

class Window : public sac::Window
{
  Q_OBJECT

public:
  Window(QWidget* parent = nullptr);

  InputTab* inputTab() const;
  ResultsTab* resultsTab() const;
  MapsTab* mapsTab() const;

private:
  InputTab* inputTab_;
  ResultsTab* resultsTab_;
  MapsTab* mapsTab_;
};

} // namespace lithology

} // namespace sac

} // namespace casaWizard

