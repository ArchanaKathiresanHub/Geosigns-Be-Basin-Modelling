//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Tab to display the results, as well as the results from the 1D case
#pragma once

#include "view/SacMapsTab.h"
#include "assets/TCHPVisualisation.h"

namespace casaWizard
{

namespace sac
{

namespace thermal
{

class ThermalMapTab : public SacMapsTab
{
  Q_OBJECT

public:
  explicit ThermalMapTab(QWidget* parent = nullptr);

  TCHPVisualisation* mapsVisualisation() const override;

private:
  void setGridGenerationOptionsLayout() override;
  void setTotalLayout();

  TCHPVisualisation* m_TCHPVisualisation;
};

} // namespace thermal

} // namespace sac

} // namespace casaWizard
