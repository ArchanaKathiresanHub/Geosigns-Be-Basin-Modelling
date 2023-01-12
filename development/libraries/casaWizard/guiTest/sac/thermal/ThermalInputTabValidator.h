//
// Copyright (C) 2012 - 2023 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "SacInputTabValidator.h"

namespace casaWizard
{

namespace sac
{

namespace thermal
{

class ThermalGuiTester;
class ThermalController;

class ThermalInputTabValidator : public SacInputTabValidator
{
public:
   ThermalInputTabValidator(ThermalController& controller, ThermalGuiTester* tester);
   void validateInputTab() final;

   void validateImportProject3DFile() final;
   void validateImportWells();
   void validateCalibrationTargets()  final;
   void validateRunOptions()          final;
   void validateRun1DOptimization()   final;
   void validateRun1DOriginal()       final;
   void validateRun3DOriginal()       final;

private:
   ThermalController& m_controller;
   ThermalGuiTester* m_tester;

   void processSelectProject(bool proceed);
   void processSelectTargets(bool proceed);
   void processDestinationFolderPopup();
   void processCpuPopup();
   void processSubSamplingPopup();
};

} // thermal

} // sac

} // casaWizard
