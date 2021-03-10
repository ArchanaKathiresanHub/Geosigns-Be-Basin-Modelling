//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef BIOMARKERS_CALCULATOR_H
#define BIOMARKERS_CALCULATOR_H

class AppCtx;

#include "ConstantsFastcauldron.h"

namespace Calibration
{
   class BiomarkersSimulator;
}
class BiomarkersCalculator
{
   public:
      BiomarkersCalculator( AppCtx* Application_Context );
      ~BiomarkersCalculator();

      bool computeSnapShotBiomarkers( const double time, const Boolean2DArray& validNeedleSet );

      bool computeBiomarkersIncrement( double time, double timeStep );

   private:
      AppCtx* m_Basin_Model;
      Calibration::BiomarkersSimulator *m_BiomarkersSimulator;

};

#endif 
