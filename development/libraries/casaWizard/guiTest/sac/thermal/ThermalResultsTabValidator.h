//
// Copyright (C) 2012 - 2023 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "SacResultsTabValidator.h"

namespace casaWizard
{

namespace sac
{

namespace thermal
{

class ThermalController;
class ThermalGuiTester;

class ThermalResultsTabValidator : public SacResultsTabValidator
{
public:
   ThermalResultsTabValidator(ThermalController& controller, ThermalGuiTester* tester);

   void validateResultsTab()       final;

   void validateDepthPlots()       final;
   void validateCorrelationPlots() final;
   void validateTable()            final;
   void validateBirdsView()        final;

private:
   ThermalController& m_controller;
   ThermalGuiTester* m_tester;
};

} // thermal

} // sac

} // casaWizard
