//
// Copyright (C) 2012 - 2023 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

namespace casaWizard
{

namespace sac
{

class SacResultsTabValidator
{
public:
   SacResultsTabValidator() = default;

   virtual void validateResultsTab()       = 0;

   virtual void validateDepthPlots()       = 0;
   virtual void validateCorrelationPlots() = 0;
   virtual void validateTable()            = 0;
   virtual void validateBirdsView()        = 0;
};

} // sac

} // casaWizard
