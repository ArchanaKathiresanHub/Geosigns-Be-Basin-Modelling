// Main window for the SAC wizard
#pragma once

#include "view/SacWindow.h"

namespace casaWizard
{

class WellPrepTab;

namespace sac
{

class MapsTabLithology;
class ResultsTab;
class T2Ztab;

namespace lithology
{

class InputTab;

class Window : public sac::Window
{
   Q_OBJECT

public:
   Window(QWidget* parent = nullptr);

   WellPrepTab* wellPrepTab() const;
   InputTab* inputTab() const;
   T2Ztab* t2zTab() const;
   ResultsTab* resultsTab() const;
   MapsTabLithology* mapsTab() const;

private:
   WellPrepTab* wellPrepTab_;
   InputTab* inputTab_;
   T2Ztab* t2zTab_;
   ResultsTab* resultsTab_;
   MapsTabLithology* mapsTab_;
};

} // namespace lithology

} // namespace sac

} // namespace casaWizard

