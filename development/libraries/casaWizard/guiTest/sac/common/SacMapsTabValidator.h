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

class SacMapsTabValidator
{
public:
   SacMapsTabValidator() = default;

   virtual void validateMapsTab()                 = 0;

   virtual void validateInitialMapsTabOptions()   = 0;
   virtual void validateGridding()                = 0;
   virtual void validateRun3DOptimization()       = 0;
   virtual void validateMapOptionsAfterGridding() = 0;
   virtual void validateExports()                 = 0;
};

} // sac

} // casaWizard
