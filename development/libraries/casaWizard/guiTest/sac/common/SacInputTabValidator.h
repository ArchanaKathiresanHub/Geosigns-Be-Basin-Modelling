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

class SacInputTabValidator
{
public:
   SacInputTabValidator() = default;

   virtual void validateInputTab()            = 0;

   virtual void validateImportProject3DFile() = 0;
   virtual void validateCalibrationTargets()  = 0;
   virtual void validateRunOptions()          = 0;
   virtual void validateRun1DOptimization()   = 0;
   virtual void validateRun1DOriginal()       = 0;
   virtual void validateRun3DOriginal()       = 0;
};

} // sac

} // casaWizard
