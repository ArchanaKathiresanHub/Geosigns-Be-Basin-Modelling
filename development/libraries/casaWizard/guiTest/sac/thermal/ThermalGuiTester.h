//
// Copyright (C) 2012-2023 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "GuiTester.h"
#include "control/ThermalController.h"
#include <iostream>

#include "ThermalInputTabValidator.h"
#include "ThermalResultsTabValidator.h"
#include "ThermalMapsTabValidator.h"

class QCheckBox;

namespace casaWizard
{

namespace sac
{

namespace thermal
{

class ThermalGuiTester : public GuiTester
{
public:
   ThermalGuiTester(ThermalController& controller);

   void runValidations() final;  

private:
   ThermalController& m_controller;
   ThermalInputTabValidator* m_inputValidator;
   ThermalResultsTabValidator* m_resultsValidator;
   ThermalMapsTabValidator* m_mapsValidator;

   QString m_workingDir;
   bool h_bool; //helper bool
   void validateLoadingSaves() final;

   void validateSaving();
   void validateClearing();
   void validateLoading();

   void openSave(QString filepath); //utility function to skip input tab runs

   void processSaveDialog();
   void processLoadDialog();
};

} // namespace thermal

} // namespace sac

} // namespace casawizard
