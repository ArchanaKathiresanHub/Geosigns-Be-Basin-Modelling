//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QWidget>

class QComboBox;

namespace casaWizard
{

class CustomCheckbox;

namespace sac
{

class SacMapPlotOptions : public QWidget
{
   Q_OBJECT
public:
   explicit SacMapPlotOptions(QWidget *parent = nullptr);
   virtual ~SacMapPlotOptions() = default;

   QComboBox* colorMapSelection() const;
   CustomCheckbox* stretched() const;
   CustomCheckbox* wellsVisible() const;

private:
   QComboBox* colorMapSelection_;
   CustomCheckbox* stretched_;
   CustomCheckbox* wellsVisible_;
};

} //namespace sac

} //namespace casaWizard
