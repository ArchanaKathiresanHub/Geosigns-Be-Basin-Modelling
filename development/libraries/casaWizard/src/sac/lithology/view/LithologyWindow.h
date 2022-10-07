// Main Window for the SAC Lithology wizard
#pragma once

#include "view/SacWindow.h"

namespace casaWizard
{

class WellPrepTab;

namespace sac
{

class MapsTabLithology;
class T2Ztab;

namespace lithology
{

class LithologyInputTab;
class LithologyResultsTab;

class LithologyWindow : public sac::Window
{
   Q_OBJECT

public:
   LithologyWindow(QWidget* parent = nullptr);

   WellPrepTab* wellPrepTab() const;
   LithologyInputTab* inputTab() const;
   T2Ztab* t2zTab() const;
   LithologyResultsTab* resultsTab() const;
   MapsTabLithology* mapsTab() const;

private:
   WellPrepTab* m_wellPrepTab;
   LithologyInputTab* m_inputTab;
   LithologyResultsTab* m_resultsTab;
   MapsTabLithology* m_mapsTab;
   T2Ztab* m_t2zTab;
};

} // namespace lithology

} // namespace sac

} // namespace casaWizard

