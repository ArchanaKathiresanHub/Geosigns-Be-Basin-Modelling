//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QWidget>

class QPushButton;
class QComboBox;
class QCheckBox;
class QGridLayout;
class QLabel;

namespace casaWizard
{

class CustomCheckbox;

namespace sac
{

class MapPlotOptions : public QWidget
{
   Q_OBJECT
public:
   explicit MapPlotOptions(QWidget *parent = nullptr);

   QComboBox* percentageRange() const;
   QComboBox* colorMapSelection() const;
   QComboBox* layerSelection() const;
   QComboBox* lithotypeSelection() const;
   CustomCheckbox* singleMapLayout() const;
   CustomCheckbox* stretched() const;
   CustomCheckbox* wellsVisible() const;

private:
   QComboBox* percentageRange_;
   QComboBox* colorMapSelection_;
   QComboBox* layerSelection_;
   QComboBox* lithotypeSelection_;
   QLabel* lithoSelectionLabel_;

   CustomCheckbox* singleMapLayout_;
   CustomCheckbox* stretched_;
   CustomCheckbox* wellsVisible_;

private slots:
   void slotUpdateSingleMapLayout(int state);
};

}

}
