//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "view/assets/SacMapPlotOptions.h"

class QComboBox;
class QLabel;

namespace casaWizard
{

class CustomCheckbox;

namespace sac
{

namespace lithology
{

class MapPlotOptionsLithofraction : public SacMapPlotOptions
{
   Q_OBJECT

public:
   explicit MapPlotOptionsLithofraction(QWidget *parent = nullptr);

   QComboBox* percentageRange() const;
   QComboBox* layerSelection() const;
   QComboBox* lithotypeSelection() const;
   CustomCheckbox* singleMapLayout() const;

private:
   QComboBox* percentageRange_;
   QComboBox* layerSelection_;
   QComboBox* lithotypeSelection_;
   QLabel* lithoSelectionLabel_;

   CustomCheckbox* singleMapLayout_;

private slots:
   void slotUpdateSingleMapLayout(int state);
};

} // namespace lithology

} // namespace sac

} // namespace casaWizard
