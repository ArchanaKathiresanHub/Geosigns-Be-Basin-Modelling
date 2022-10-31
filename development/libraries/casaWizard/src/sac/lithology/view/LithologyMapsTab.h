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
#include "assets/lithofractionVisualisation.h"

namespace casaWizard
{

class CustomCheckbox;

namespace sac
{

class LithologyMapsTab : public SacMapsTab
{
  Q_OBJECT

public:
  explicit LithologyMapsTab(QWidget* parent = nullptr);

  LithofractionVisualisation* mapsVisualisation() const override;
  CustomCheckbox* smartGridding() const;

private:
  void setGridGenerationOptionsLayout() override;
  void setTotalLayout();

  LithofractionVisualisation* m_lithofractionVisualisation;
  CustomCheckbox* m_smartGridding;
};

} // namespace sac

} // namespace casaWizard
