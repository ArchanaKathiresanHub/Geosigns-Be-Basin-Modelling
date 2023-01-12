//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Main window for the SAC wizard
#pragma once

#include "view/mainWindow.h"

namespace casaWizard
{

namespace sac
{

class SacInputTab;
class SacResultsTab;
class SacMapsTab;

class SacWindow : public MainWindow
{
   Q_OBJECT

public:
   SacWindow(QWidget* parent = nullptr);
   virtual ~SacWindow() override = default;
   MenuBar* menu() const override;

private:
   MenuBar* menuBarSAC_;
};

} // namespace sac

} // namespace casaWizard

