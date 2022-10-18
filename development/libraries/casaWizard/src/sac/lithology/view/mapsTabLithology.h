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
#include "lithofractionVisualisation.h"

class QLabel;
class QListWidget;
class QComboBox;
class QPushButton;
class QSpinBox;
class QVBoxLayout;
class EmphasisButton;

namespace casaWizard
{

class CustomCheckbox;
class Well;

namespace sac
{

class ActiveWellsTable;
class OptimizationOptions;

class MapsTabLithology : public SacMapsTab
{
  Q_OBJECT

public:
  explicit MapsTabLithology(QWidget* parent = nullptr);

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
