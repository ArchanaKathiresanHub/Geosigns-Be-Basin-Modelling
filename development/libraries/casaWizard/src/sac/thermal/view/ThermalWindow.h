//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Main window for the SAC Thermal wizard
#pragma once

#include "view/SacWindow.h"

namespace casaWizard
{

namespace sac
{

namespace thermal
{

class ThermalInputTab;
class ThermalResultsTab;
class ThermalMapsTab;

class ThermalWindow : public sac::Window
{
  Q_OBJECT

public:
  ThermalWindow(QWidget* parent = nullptr);

  ThermalInputTab* inputTab() const;
  ThermalResultsTab* resultsTab() const;
  ThermalMapsTab* mapsTab() const;

private:
  ThermalInputTab* m_inputTab;
  ThermalResultsTab* m_resultsTab;
  ThermalMapsTab* m_mapsTab;
};

} // namespace lithology

} // namespace sac

} // namespace casaWizard
